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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/hid/nest.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    nest::nest(i_widget const& aWidget, nest_type aType) :
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
    i_widget const& nest::widget() const
    {
        return iWidget;
    }

    nest_type nest::type() const
    {
        return iType;
    }

    bool nest::has(i_native_window const& aNestedWindow) const
    {
        return iNestedWindows.find(&aNestedWindow) != iNestedWindows.end();
    }

    void nest::add(i_native_window& aNestedWindow)
    {
        iNestedWindows.insert(&aNestedWindow);
        iSink += aNestedWindow.destroyed([&]()
        {
            remove(aNestedWindow);
        });
        if (type() == nest_type::Caddy)
        {
            iSink += widget().position_changed([&]()
            {
                aNestedWindow.surface_window().as_widget().move(widget().origin() + widget().client_rect(false).top_left());
            });
            iSink += widget().parent_position_changed([&]()
            {
                aNestedWindow.surface_window().as_widget().move(widget().origin() + widget().client_rect(false).top_left());
            });
            iSink += widget().size_changed([&]()
            {
                aNestedWindow.surface_window().as_widget().resize(widget().client_rect(false).extents());
            });
        }
    }

    void nest::remove(i_native_window& aNestedWindow)
    {
        iNestedWindows.erase(&aNestedWindow);
    }
}