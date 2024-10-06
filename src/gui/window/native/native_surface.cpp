// native_surface.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <numeric>

#include <neolib/task/thread.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gui/window/i_window.hpp>

#include "native_surface.hpp"

namespace neogfx
{
    native_surface::native_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow) :
        iRenderingEngine{ aRenderingEngine },
        iSurfaceWindow{ aWindow },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGui },
        iFrameCounter{ 0 },
        iPaused{ 0 },
        iRendering{ false },
        iDebug{ false }
    {
    }

    i_rendering_engine& native_surface::rendering_engine() const
    {
        return iRenderingEngine;
    }

    i_surface_window& native_surface::surface_window() const
    {
        return iSurfaceWindow;
    }

    native_surface::~native_surface()
    {
        set_destroyed();
    }

    render_target_type native_surface::target_type() const
    {
        return render_target_type::Surface;
    }

    void* native_surface::target_handle() const
    {
        return surface_window().native_window().target_handle();
    }

    void* native_surface::target_device_handle() const
    {
        return surface_window().native_window().target_device_handle();
    }

    point native_surface::target_origin() const
    {
        return {};
    }

    size native_surface::target_extents() const
    {
        return extents();
    }

    dimension native_surface::horizontal_dpi() const
    {
        return surface_window().native_window().pixel_density().cx;
    }

    dimension native_surface::vertical_dpi() const
    {
        return surface_window().native_window().pixel_density().cy;
    }

    dimension native_surface::ppi() const
    {
        return surface_window().native_window().pixel_density().magnitude() / std::sqrt(2.0);
    }

    bool native_surface::metrics_available() const
    {
        return true;
    }

    size native_surface::extents() const
    {
        return surface_window().native_window().surface_extents();
    }

    dimension native_surface::em_size() const
    {
        return 0; /* todo */
    }

    void native_surface::activate_target() const
    {
        bool alreadyActive = target_active();
        if (!alreadyActive)
        {
            TargetActivating.trigger();
            rendering_engine().activate_context(*this);
        }
        do_activate_target();
        if (!alreadyActive)
            TargetActivated.trigger();
    }

    bool native_surface::target_active() const
    {
        return rendering_engine().active_target() == this;
    }

    void native_surface::deactivate_target() const
    {
        if (target_active())
        {
            TargetDeactivating.trigger();
            rendering_engine().deactivate_context();
            TargetDeactivated.trigger();
            return;
        }
//        throw not_active();
    }

    pixel_format_t native_surface::pixel_format() const
    {
        if (iPixelFormat == std::nullopt)
            iPixelFormat = rendering_engine().set_pixel_format(*this);
        return *iPixelFormat;
    }

    color_space native_surface::color_space() const
    {
        return service<i_surface_manager>().display(surface_window()).color_space();
    }

    neogfx::logical_coordinate_system native_surface::logical_coordinate_system() const
    {
        return iLogicalCoordinateSystem;
    }

    void native_surface::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    logical_coordinates native_surface::logical_coordinates() const
    {
        if (iLogicalCoordinates != std::nullopt)
            return *iLogicalCoordinates;
        neogfx::logical_coordinates result;
        switch (iLogicalCoordinateSystem)
        {
        case neogfx::logical_coordinate_system::Specified:
            throw logical_coordinates_not_specified();
            break;
        case neogfx::logical_coordinate_system::AutomaticGui:
            result.bottomLeft = vec2{ 0.0, extents().cy };
            result.topRight = vec2{ extents().cx, 0.0 };
            break;
        case neogfx::logical_coordinate_system::AutomaticGame:
            result.bottomLeft = vec2{ 0.0, 0.0 };
            result.topRight = vec2{ extents().cx, extents().cy };
            break;
        }
        return result;
    }

    void native_surface::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    std::uint64_t native_surface::frame_counter() const
    {
        return iFrameCounter;
    }

    double native_surface::fps() const
    {
        if (iFpsData.size() < 2)
            return 0.0;
        double const totalDuration_s = std::chrono::duration_cast<std::chrono::microseconds>(iFpsData.back().second - iFpsData.front().first).count() / 1000000.0;
        double const averageDuration_s = totalDuration_s / iFpsData.size();
        return 1.0 / averageDuration_s;
    }

    double native_surface::potential_fps() const
    {
        if (iFpsData.size() < 1)
            return 0.0;
        double const totalDuration_s = std::accumulate(iFpsData.begin(), iFpsData.end(), 0.0, [](double sum, const frame_times& frameTimes) -> double
            { return sum + std::chrono::duration_cast<std::chrono::microseconds>(frameTimes.second - frameTimes.first).count() / 1000000.0; });
        double const averageDuration_s = totalDuration_s / iFpsData.size();
        return 1.0 / averageDuration_s;
    }

    void native_surface::invalidate(const rect& aInvalidatedRect)
    {
        if (aInvalidatedRect.cx != 0.0 && aInvalidatedRect.cy != 0.0)
        {
            if (!has_invalidated_area())
                iInvalidatedArea = aInvalidatedRect.ceil();
            else
                iInvalidatedArea = invalidated_area().combined(aInvalidatedRect).ceil();
        }
    }

    bool native_surface::has_invalidated_area() const
    {
        return iInvalidatedArea != std::nullopt;
    }

    const rect& native_surface::invalidated_area() const
    {
        if (has_invalidated_area())
            return *iInvalidatedArea;
        throw no_invalidated_area();
    }

    rect native_surface::validate()
    {
        if (has_invalidated_area())
        {
            rect validatedArea = invalidated_area();
            iInvalidatedArea = std::nullopt;
            return validatedArea;
        }
        throw no_invalidated_area();
    }

    bool native_surface::can_render() const
    {
        return !iPaused && 
            surface_window().as_window().visible() && 
            surface_window().as_window().ready_to_render();
    }

    void native_surface::pause()
    {
        ++iPaused;
    }

    void native_surface::resume()
    {
        if (iPaused == 0)
            throw bad_pause_count();
        --iPaused;
    }

    void native_surface::render(bool aOOBRequest)
    {
        if (iRendering || rendering_engine().creating_window() || !can_render())
        {
            debug_message("can't render");
            return;
        }

        auto const now = std::chrono::high_resolution_clock::now();

        if (!aOOBRequest)
        {
            if (rendering_engine().frame_rate_limited() && iLastFrameTime != std::nullopt &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - *iLastFrameTime).count() < 
                    1000 / (rendering_engine().frame_rate_limit() * (!rendering_engine().use_rendering_priority() ? 1.0 : surface_window().rendering_priority())))
            {
                debug_message("frame rate limited");
                return;
            }

            if (!surface_window().native_window_ready_to_render())
            {
                debug_message("native window not ready");
                return;
            }
        }

        if (!has_invalidated_area())
        {
            debug_message("no invalidated area");
            return;
        }

        if (invalidated_area().cx <= 0.0 || invalidated_area().cy <= 0.0)
        {
            debug_message("bad invalid area");
            validate();
            return;
        }

        if (iDebug)
        {
            std::ostringstream oss;
            oss << "to render (frame " << iFrameCounter << "): " << invalidated_area();
            debug_message(oss.str());
        }

        ++iFrameCounter;

        iRendering = true;
        iLastFrameTime = now;

        surface_window().rendering().trigger();

        scoped_render_target srt{ *this };

        do_render();

        surface_window().native_window().display();

        iRendering = false;
        validate();

        surface_window().rendering_finished().trigger();

        iFpsData.push_back(frame_times{ *iLastFrameTime, std::chrono::high_resolution_clock::now() });
        if (iFpsData.size() > 100)
            iFpsData.pop_front();        
    }

    bool native_surface::is_rendering() const
    {
        return iRendering;
    }

    void native_surface::debug(bool aEnableDebug)
    {
        iDebug = aEnableDebug;
    }

    void native_surface::set_destroying()
    {
        if (!is_alive())
            return;
        base_type::set_destroying();
        if (target_active())
            deactivate_target();
    }

    void native_surface::set_destroyed()
    {
        base_type::set_destroyed();
    }

    void native_surface::debug_message(std::string const& aMessage)
    {
#ifdef NEOGFX_DEBUG
        if (iDebug)
            service<debug::logger>() << neolib::logger::severity::Debug << aMessage << std::endl;
#endif // NEOGFX_DEBUG
    }
}