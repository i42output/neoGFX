// nest.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.

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

#include <neolib/core/scoped.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/hid/nest.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    nest::nest(i_widget& aWidget, nest_type aType) :
        iWidget{ aWidget }, iType{ aType }
    {
        iSink += widget().destroyed([&]()
        {
            service<i_surface_manager>().destroy_nest(*this);
        });
    }
 
    nest::~nest()
    {
    }
#
    i_widget& nest::widget() const
    {
        return iWidget;
    }

    nest_type nest::type() const
    {
        return iType;
    }

    bool nest::has(i_native_window const& aNestedWindow) const
    {
        return std::find(iNestedWindows.begin(), iNestedWindows.end(), &aNestedWindow) != iNestedWindows.end();
    }

    void nest::add(i_native_window& aNestedWindow)
    {
        // a window is in one nest at a time (e.g. one made in one place (a caddy) then nested in another (MDI))
        auto& surfaceManager = service<i_surface_manager>();
        for (std::size_t n = 0; n < surfaceManager.nest_count(); ++n)
            if (auto& otherNest = surfaceManager.nest_at(n); &otherNest != this && otherNest.has(aNestedWindow))
                otherNest.remove(aNestedWindow);
        if (!has(aNestedWindow))
            iNestedWindows.push_back(&aNestedWindow);
        widget().bring_child_to_front(aNestedWindow.surface_window().as_widget());
        iSink += aNestedWindow.destroyed([&]()
        {
            remove(aNestedWindow);
        });
        iSink += aNestedWindow.surface_window().as_window().activated([&]()
        {
            widget().bring_child_to_front(aNestedWindow.surface_window().as_widget());
        });
        if (type() == nest_type::MDI)
        {
            auto refit = [&]()
            {
                // (n.b. this sink outlives the nested window)
                if (!has(aNestedWindow))
                    return;
                if (aNestedWindow.is_maximized())
                    aNestedWindow.maximize();
                else if (aNestedWindow.is_iconic())
                    aNestedWindow.iconize();
            };
            iSink += widget().position_changed(refit);
            iSink += widget().size_changed(refit);
            // (the nest's client area can change without it being resized, e.g. a scrollbar appearing)
            iSink += widget().layout_completed(refit);
            iSink += aNestedWindow.surface_window().as_window().window_event([&](neogfx::window_event& aEvent)
            {
                // the windows of an MDI nest are maximized together and restored together
                if (iPropagatingState || !has(aNestedWindow))
                    return;
                if (aEvent.type() != window_event_type::Maximized && aEvent.type() != window_event_type::Restored)
                    return;
                neolib::scoped_flag sf{ iPropagatingState };
                auto const others = iNestedWindows;
                for (auto other : others)
                {
                    if (other == &aNestedWindow || !has(*other))
                        continue;
                    if (aEvent.type() == window_event_type::Maximized && !other->is_maximized())
                        other->maximize();
                    else if (aEvent.type() == window_event_type::Restored && other->is_maximized())
                        other->restore();
                }
            });
            // a window is created maximized unless there is already a window that isn't
            if (std::none_of(iNestedWindows.begin(), iNestedWindows.end(), 
                [&](i_native_window* aOther) { return aOther != &aNestedWindow && !aOther->is_maximized(); }))
                aNestedWindow.maximize();
        }
        if (type() == nest_type::Caddy)
        {
            // (n.b. these sinks outlive the nested window's membership of the nest)
            iSink += widget().position_changed([&]()
            {
                if (has(aNestedWindow))
                    aNestedWindow.surface_window().as_widget().move(widget().client_rect(false).top_left());
            });
            iSink += widget().parent_position_changed([&]()
            {
                if (has(aNestedWindow))
                    aNestedWindow.surface_window().as_widget().move(widget().client_rect(false).top_left());
            });
            iSink += widget().size_changed([&]()
            {
                if (has(aNestedWindow))
                    aNestedWindow.surface_window().as_widget().resize(widget().client_rect(false).extents());
            });
        }
        aNestedWindow.ev_added_to_nest()();
    }

    void nest::remove(i_native_window& aNestedWindow)
    {
        std::erase(iNestedWindows, &aNestedWindow);
    }

    std::size_t nest::nested_window_count() const
    {
        return iNestedWindows.size();
    }

    i_native_window& nest::nested_window(std::size_t aIndex) const
    {
        return *iNestedWindows.at(aIndex);
    }
}