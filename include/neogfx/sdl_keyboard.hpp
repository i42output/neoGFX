// sdl_keyboard.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include "keyboard.hpp"
#include "SDL_keyboard.h"

namespace neogfx
{
	class sdl_keyboard : public i_keyboard
	{
	public:
		virtual bool is_key_pressed(scan_code_e aScanCode) const
		{
			const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
			return keyboardState[aScanCode] == 1;
		}
	};
}