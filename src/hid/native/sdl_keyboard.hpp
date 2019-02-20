// sdl_keyboard.hpp
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
#include <SDL_keyboard.h>
#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
    class sdl_keyboard : public keyboard
    {
    public:
        sdl_keyboard();
    public:
        bool is_key_pressed(scan_code_e aScanCode) const override;
        keyboard_locks locks() const override;
    public:
        static scan_code_e from_sdl_scan_code(SDL_Scancode aScanCode);
        static SDL_Scancode to_sdl_scan_code(scan_code_e aScanCode);
    };
}