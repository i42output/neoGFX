// sdl_window_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <SDL.h>
#include <SDL_mouse.h>
#include <neogfx/hid/window_manager.hpp>

namespace neogfx
{
    class sdl_window_manager : public window_manager
    {
    public:
        struct no_cursors_saved : std::logic_error { no_cursors_saved() : std::logic_error("neogfx::sdl_window_manager::no_cursors_saved") {} };
    private:
        typedef std::shared_ptr<SDL_Cursor> cursor_pointer;
    public:
        sdl_window_manager();
        ~sdl_window_manager();
    public:
        point mouse_position() const override;
        point mouse_position(const i_window& aWindow) const override;
        bool is_mouse_button_pressed(mouse_button aButton) const override;
    public:
        void save_mouse_cursor() override;
        void set_mouse_cursor(mouse_system_cursor aSystemCursor) override;
        void restore_mouse_cursor(const i_window& aWindow) override;
        void update_mouse_cursor(const i_window& aWindow) override;
    private:
        cursor_pointer iCurrentCursor;
        std::vector<cursor_pointer> iSavedCursors;
    };
}