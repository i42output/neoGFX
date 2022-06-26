// window_manager.hpp
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
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/window/i_window.hpp>

namespace neogfx
{
    class window_manager : public i_window_manager
    {
    private:
        struct window_sorter
        {
            bool operator()(i_window* left, i_window* right) const
            {
                if (left->is_owner_of(*right))
                    return true;
                else if (right->is_owner_of(*left))
                    return false;
                else
                    return false;
            }
        };
        typedef std::vector<i_window*> window_list;
    public:
        window_manager();
        ~window_manager();
    public:
        void add_window(i_window& aWindow) final;
        void remove_window(i_window& aWindow) final;
        bool has_window(i_window& aWindow) const final;
        std::size_t window_count() const final;
        i_window& window(std::size_t aIndex) final;
        bool any_strong_windows() const final;
        i_window& hosting_window(const i_window& aNestedWindow) const final;
    public:
        rect desktop_rect(const i_window& aWindow) const final;
        rect window_rect(const i_window& aWindow) const final;
        double z_order(const i_window& aWindow) const final;
        void move_window(i_window& aWindow, const point& aPosition) final;
        void resize_window(i_window& aWindow, const size& aExtents) final;
    public:
        bool window_activated() const final;
        i_window& active_window() const final;
        void activate_window(i_window& aWindow) final;
        void deactivate_window(i_window& aWindow) final;
    public:
        point mouse_position() const final;
        point mouse_position(const i_window& aWindow) const final;
        bool is_mouse_button_pressed(mouse_button aButton) const final;
    private:
        sink iSink;
        window_list iWindows;
        i_window* iActiveWindow = nullptr;
    };
}