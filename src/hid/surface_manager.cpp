// surface_manager.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <neogfx/neogfx.hpp>
#include <neolib/core/string_utils.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/hid/i_native_surface.hpp>
#include <neogfx/gui/window/i_native_window.hpp>

template <> neogfx::i_surface_manager& services::start_service<neogfx::i_surface_manager>() 
{ 
    static neogfx::surface_manager sSurfaceManager{ service<neogfx::i_basic_services>(), service<neogfx::i_rendering_engine>() }; 
    return sSurfaceManager;
}

namespace neogfx
{
    surface_manager::surface_manager(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine) :
        iBasicServices{ aBasicServices }, 
        iRenderingEngine{ aRenderingEngine }, 
        iRenderingSurfaces{ false }
    {
        iSink = service<i_app>().current_style_changed([this](style_aspect aAspect)
        {
            if ((aAspect & (style_aspect::Geometry | style_aspect::Font)) != style_aspect::None)
                layout_surfaces();
            else
                invalidate_surfaces();
        });
    }

    bool surface_manager::initialising_surface() const
    {
        for (auto s : iSurfaces)
        {
            if (s->has_native_surface() && s->native_surface().initialising())
                return true;
        }
        return false;
    }

    void surface_manager::add_surface(i_surface& aSurface)
    {
        iSurfaces.push_back(&aSurface);
        std::sort(iSurfaces.begin(), iSurfaces.end(), surface_sorter{});
        if (aSurface.is_window())
        {
            iSink += aSurface.as_surface_window().as_window().activated([&]()
            {
                std::sort(iSurfaces.begin(), iSurfaces.end(), surface_sorter{});
            });
        }
    }
    
    void surface_manager::remove_surface(i_surface& aSurface)
    {
        auto existingSurface = std::find(iSurfaces.begin(), iSurfaces.end(), &aSurface);
        if (existingSurface != iSurfaces.end())
        {
            iSurfaces.erase(existingSurface);
            for (auto s = iSurfaces.begin(); s != iSurfaces.end();)
            {
                if (aSurface.is_owner_of(**s))
                {
                    auto& childSurface = **s;
                    s = iSurfaces.erase(s);
                    childSurface.close();
                }
                else
                    ++s;
            }
        }    
    }

    const i_surface& surface_manager::surface_at_position(const i_surface& aProgenitor, const point& aPosition, bool aForMouseEvent) const
    {
        const i_surface* match = nullptr;
        for (auto s : iSurfaces)
        {
            if (!s->is_window() || !aProgenitor.is_owner_of(*s) || !s->as_surface_window().native_window().visible())
                continue;
            if (aForMouseEvent)
            {
                if (!s->as_surface_window().native_window().enabled())
                    continue;
                auto const location = aProgenitor.as_surface_window().current_mouse_event_location();
                if (location == mouse_event_location::None ||
                    (location == mouse_event_location::Client && s->as_surface_window().as_widget().ignore_mouse_events()) ||
                    (location == mouse_event_location::NonClient && s->as_surface_window().as_widget().ignore_non_client_mouse_events()))
                    continue;
            }
            rect const surfaceRect{ s->as_surface_window().surface_position(), s->as_surface_window().surface_extents() };
            if (surfaceRect.contains(aPosition))
            {
                if (match == nullptr || match->is_owner_of(*s))
                    match = s;
            }
        }
        if (match != nullptr)
            return *match;
        return aProgenitor;
    }

    i_surface& surface_manager::surface_at_position(const i_surface& aProgenitor, const point& aPosition, bool aForMouseEvent)
    {
        return const_cast<i_surface&>(to_const(*this).surface_at_position(aProgenitor, aPosition, aForMouseEvent));
    }

    i_nest& surface_manager::nest_for(i_widget& aNestWidget, nest_type aNestType) const
    {
        auto existing = std::find_if(iNests.begin(), iNests.end(), [&](auto&& e) { return &e->widget() == &aNestWidget; });
        if (existing == iNests.end())
            existing = iNests.insert(iNests.end(), std::make_unique<nest>(aNestWidget, aNestType));
        if ((**existing).type() != aNestType)
            throw wrong_nest_type();
        return **existing;
    }

    i_nest& surface_manager::find_nest(i_native_window const& aNestedWindow) const
    {
        for (auto& n : iNests)
            if (n->has(aNestedWindow))
                return *n;
        throw nest_not_found();
    }

    void surface_manager::destroy_nest(i_nest& aNest)
    {
        auto existing = std::find_if(iNests.begin(), iNests.end(), [&](auto&& e) { return &*e == &aNest; });
        auto existingActive = std::find(iActiveNest.begin(), iActiveNest.end(), &aNest);
        if (existingActive != iActiveNest.end())
            throw cannot_destroy_active_nest();
        iNests.erase(existing);
    }

    i_nest& surface_manager::active_nest() const
    {
        if (!iActiveNest.empty())
            return *iActiveNest.back();
        throw nest_not_active();
    }

    void surface_manager::activate_nest(i_nest& aNest)
    {
        iActiveNest.push_back(&aNest);
    }

    void surface_manager::deactivate_nest(i_nest& aNest)
    {
        if (iActiveNest.empty() || &*iActiveNest.back() != &aNest)
            throw nest_not_active();
        iActiveNest.pop_back();
    }

    bool surface_manager::is_surface_attached(void* aNativeSurfaceHandle) const
    {
        for (auto s : iSurfaces)
            if (s->has_native_surface() && s->native_surface().handle() == aNativeSurfaceHandle)
                return true;
        return false;
    }

    i_surface& surface_manager::attached_surface(void* aNativeSurfaceHandle)
    {
        for (auto s : iSurfaces)
            if (s->has_native_surface() && s->native_surface().handle() == aNativeSurfaceHandle)
                return *s;
        throw surface_not_found();
    }

    std::size_t surface_manager::surface_count() const
    {
        return iSurfaces.size();
    }

    i_surface& surface_manager::surface(std::size_t aIndex)
    {
        return **std::next(iSurfaces.begin(), aIndex);
    }

    bool surface_manager::any_strong_surfaces() const
    {
        for (auto s : iSurfaces)
            if (s->is_strong())
                return true;
        return false;
    }

    std::size_t surface_manager::strong_surface_count() const
    {
        std::size_t result = 0u;
        for (auto s : iSurfaces)
            if (s->is_strong())
                ++result;
        return result;
    }
        
    bool surface_manager::process_events(bool& aLastWindowClosed)
    {
        bool hadStrong = any_strong_surfaces();
        bool handledEvents = iRenderingEngine.process_events();
        if (hadStrong && !any_strong_surfaces())
            aLastWindowClosed = true;
        return handledEvents;
    }

    void surface_manager::layout_surfaces()
    {
        for (auto s : iSurfaces)
            s->layout_surface();
    }

    void surface_manager::invalidate_surfaces()
    {
        for (auto s : iSurfaces)
            s->invalidate_surface(rect(point{}, s->surface_extents()), false);
    }

    void surface_manager::render_surfaces()
    {
        if (iRenderingSurfaces || iRenderingEngine.creating_window())
            return;
        iRenderingSurfaces = true;
        for (auto s = iSurfaces.rbegin(); s != iSurfaces.rend(); ++s)
            (**s).render_surface();
        iRenderingSurfaces = false;
    }

    void surface_manager::display_error_message(std::string const& aTitle, std::string const& aMessage) const
    {
        for (auto s : iSurfaces)
        {
            if (!s->has_native_surface())
                continue;
            if (s->surface_type() == surface_type::Window && s->as_surface_window().as_window().is_active())
            {
                display_error_message(s->native_surface(), aTitle, aMessage);
                return;
            }
        }
        iBasicServices.display_error_dialog(aTitle.c_str(), aMessage.c_str(), 0);
    }

    void surface_manager::display_error_message(const i_native_surface& aParent, std::string const& aTitle, std::string const& aMessage) const
    {
        iBasicServices.display_error_dialog(aTitle.c_str(), aMessage.c_str(), aParent.handle());
    }

    uint32_t surface_manager::display_count() const
    {
        return iBasicServices.display_count();
    }

    i_display& surface_manager::display(uint32_t aDisplayIndex) const
    {
        return iBasicServices.display(aDisplayIndex);
    }

    i_display& surface_manager::display(const i_surface& aSurface) const
    {
        if (aSurface.is_window() && (aSurface.as_surface_window().style() & surface_style::Nested) == surface_style::Nested)
            return display(aSurface.parent_surface());
        rect rectSurface{ aSurface.surface_position(), aSurface.surface_extents() };
        std::multimap<double, uint32_t> matches;
        for (uint32_t i = 0; i < display_count(); ++i)
        {
            rect rectDisplay = desktop_rect(i);
            rect rectIntersection = rectDisplay.intersection(rectSurface);
            if (!rectIntersection.empty())
                matches.insert(std::make_pair(rectIntersection.width() * rectIntersection.height(), i));
        }
        if (matches.empty())
            return display(0);
        return display(matches.rbegin()->second);
    }

    rect surface_manager::desktop_rect(uint32_t aDisplayIndex) const
    {
        return display(aDisplayIndex).desktop_rect();
    }

    rect surface_manager::desktop_rect(const i_surface& aSurface) const
    {
#ifdef WIN32
        HMONITOR monitor = MonitorFromWindow(reinterpret_cast<HWND>(aSurface.native_surface().native_handle()), MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(monitor, &mi);
        return basic_rect<LONG>{ basic_point<LONG>{ mi.rcWork.left, mi.rcWork.top }, basic_size<LONG>{ mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top } };
#else
        return display(aSurface).desktop_rect();
#endif
    }
}