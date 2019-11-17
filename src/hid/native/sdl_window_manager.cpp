// sdl_window_manager.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include "sdl_mouse.hpp"
#include "sdl_window_manager.hpp"

namespace neogfx
{
    sdl_window_manager::sdl_window_manager()
    {
    }

    sdl_window_manager::~sdl_window_manager()
    {
    }

    point sdl_window_manager::mouse_position() const
    {
        int x, y;
        SDL_GetGlobalMouseState(&x, &y);
        return basic_point<int>{x, y};
    }

    point sdl_window_manager::mouse_position(const i_window& aWindow) const
    {
        if ((aWindow.style() & window_style::Nested) != window_style::Nested || !aWindow.has_parent_window())
            return point{ mouse_position() - aWindow.surface().surface_position() };
        else
            return point{ mouse_position() - aWindow.surface().surface_position() } + window_rect(aWindow).position();
    }

    bool sdl_window_manager::is_mouse_button_pressed(mouse_button aButton) const
    {
        return (aButton & convert_mouse_button(SDL_GetMouseState(0, 0))) != mouse_button::None;
    }

    void sdl_window_manager::save_mouse_cursor()
    {
        iSavedCursors.push_back(cursor_pointer(cursor_pointer(), SDL_GetCursor()));
    }

    void sdl_window_manager::set_mouse_cursor(mouse_system_cursor aSystemCursor)
    {
        SDL_SystemCursor sdlCursor = SDL_SYSTEM_CURSOR_ARROW;
        switch (aSystemCursor)
        {
        case mouse_system_cursor::Arrow:
            sdlCursor = SDL_SYSTEM_CURSOR_ARROW;
            break;
        case mouse_system_cursor::Ibeam:
            sdlCursor = SDL_SYSTEM_CURSOR_IBEAM;
            break;
        case mouse_system_cursor::Wait:
            sdlCursor = SDL_SYSTEM_CURSOR_WAIT;
            break;
        case mouse_system_cursor::Crosshair:
            sdlCursor = SDL_SYSTEM_CURSOR_CROSSHAIR;
            break;
        case mouse_system_cursor::WaitArrow:
            sdlCursor = SDL_SYSTEM_CURSOR_WAITARROW;
            break;
        case mouse_system_cursor::SizeNWSE:
            sdlCursor = SDL_SYSTEM_CURSOR_SIZENWSE;
            break;
        case mouse_system_cursor::SizeNESW:
            sdlCursor = SDL_SYSTEM_CURSOR_SIZENESW;
            break;
        case mouse_system_cursor::SizeWE:
            sdlCursor = SDL_SYSTEM_CURSOR_SIZEWE;
            break;
        case mouse_system_cursor::SizeNS:
            sdlCursor = SDL_SYSTEM_CURSOR_SIZENS;
            break;
        case mouse_system_cursor::SizeAll:
            sdlCursor = SDL_SYSTEM_CURSOR_SIZEALL;
            break;
        case mouse_system_cursor::No:
            sdlCursor = SDL_SYSTEM_CURSOR_NO;
            break;
        case mouse_system_cursor::Hand:
            sdlCursor = SDL_SYSTEM_CURSOR_HAND;
            break;
        }
        iCurrentCursor = cursor_pointer(cursor_pointer(), SDL_CreateSystemCursor(sdlCursor));
        SDL_SetCursor(&*iCurrentCursor);
    }

    void sdl_window_manager::restore_mouse_cursor(const i_window& aWindow)
    {
        if (iSavedCursors.empty())
            throw no_cursors_saved();
        iCurrentCursor = iSavedCursors.back();
        iSavedCursors.pop_back();
        SDL_SetCursor(&*iCurrentCursor);
        update_mouse_cursor(aWindow);
    }

    void sdl_window_manager::update_mouse_cursor(const i_window& aWindow)
    {
        if (iSavedCursors.empty())
            set_mouse_cursor(aWindow.surface().as_surface_window().native_window_mouse_cursor().system_cursor());
    }


}