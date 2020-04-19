// dock.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/dock.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>

namespace neogfx
{
    dock::dock(i_widget& aParent, dock_area aArea) :
        widget{ aParent }, iArea { aArea }
    {
        set_margins(neogfx::margins{});
        update_layout();
    }

    dock::dock(i_layout& aLayout, dock_area aArea) :
        widget{ aLayout }, iArea{ aArea }
    {
        set_margins(neogfx::margins{});
        update_layout();
    }

    dock_area dock::area() const
    {
        return iArea;
    }

    void dock::set_area(dock_area aArea)
    {
        if (iArea != aArea)
        {
            iArea = aArea;
            update_layout();
        }
    }

    const dock::item_list& dock::items() const
    {
        return iItems;
    }

    void dock::add(const abstract_item& aItem)
    {
        iItems.push_back(aItem);
        try
        {
            layout().add(aItem->as_widget());
        }
        catch (...)
        {
            iItems.pop_back();
            throw;
        }
    }

    void dock::remove(const abstract_item& aItem)
    {
        auto existing = std::find(iItems.begin(), iItems.end(), aItem);
        if (existing == iItems.end())
            throw item_not_found();
        layout().remove(aItem->as_widget());
        iItems.erase(existing);
    }

    void dock::update_layout()
    {
        if ((area() & dock_area::Horizontal) != dock_area::None)
            set_layout(std::make_shared<horizontal_layout>());
        else
            set_layout(std::make_shared<vertical_layout>());
        layout().set_margins(neogfx::margins{ 1.5_mm });
        layout().set_spacing(neogfx::size{ 1.5_mm });
        layout().set_size_policy(neogfx::size_constraint::Expanding);
    }

    bool dock::transparent_background() const
    {
        return false;
    }

    color dock::background_color() const
    {
        if (has_background_color())
            return widget::background_color();
        else
            return container_background_color();
    }

}