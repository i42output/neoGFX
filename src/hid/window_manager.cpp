// window_manager.cpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/hid/window_manager.hpp>

namespace neogfx
{
    window_manager::window_manager()
    {
    }

    window_manager::~window_manager()
    {
    }

    void window_manager::add_window(i_window& aWindow)
    {
        iWindows.push_back(&aWindow);
        std::sort(iWindows.begin(), iWindows.end(), window_sorter{});
    }

    void window_manager::remove_window(i_window& aWindow)
    {
        auto existing = std::find(iWindows.begin(), iWindows.end(), &aWindow);
        if (existing == iWindows.end())
            throw window_not_found();
        iWindows.erase(existing);
    }

    bool window_manager::has_window(i_window& aWindow) const
    {
        auto query = std::find(iWindows.begin(), iWindows.end(), &aWindow);
        return query != iWindows.end();
    }

    std::size_t window_manager::window_count() const
    {
        return iWindows.size();
    }

    i_window& window_manager::window(std::size_t aIndex)
    {
        if (aIndex >= iWindows.size())
            throw window_not_found();
        return **std::next(iWindows.begin(), aIndex);
    }

    bool window_manager::any_strong_windows() const
    {
        for (auto const& w : iWindows)
            if (w->is_strong())
                return true;
        return false;
    }

    rect window_manager::desktop_rect(const i_window& aWindow) const
    {
       return service<i_surface_manager>().desktop_rect(aWindow.surface());
    }

    rect window_manager::window_rect(const i_window& aWindow) const
    {
        return aWindow.is_nested() ?
            window_rect(aWindow.parent_window()) :
            rect{ aWindow.surface().surface_position(), aWindow.surface().surface_extents() };
    }

    void window_manager::move_window(i_window& aWindow, const point& aPosition)
    {
        aWindow.surface().move_surface(aPosition);
    }

    void window_manager::resize_window(i_window& aWindow, const size& aExtents)
    {
        aWindow.surface().resize_surface(aExtents);
    }

    bool window_manager::window_activated() const
    {
        for (auto const& w : iWindows)
            if (w->is_active())
                return true;
        return false;
    }

    i_window& window_manager::active_window() const
    {
        for (auto const& w : iWindows)
            if (w->is_active())
                return *w;
        throw no_window_active();
    }
}