// sdl_mouse.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 201u Leigh Johnston
  
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
#include <SDL.h>
#include <SDL_mouse.h>
#include <neogfx/hid/mouse.hpp>

namespace neogfx
{
	inline mouse_button convert_mouse_button(Uint32 aButton)
	{
		mouse_button result = mouse_button::None;
		if (aButton & SDL_BUTTON(SDL_BUTTON_LEFT))
			result = static_cast<mouse_button>(result | mouse_button::Left);
		if (aButton & SDL_BUTTON(SDL_BUTTON_RIGHT))
			result = static_cast<mouse_button>(result | mouse_button::Right);
		if (aButton & SDL_BUTTON(SDL_BUTTON_MIDDLE))
			result = static_cast<mouse_button>(result | mouse_button::Middle);
		if (aButton & SDL_BUTTON(SDL_BUTTON_X1))
			result = static_cast<mouse_button>(result | mouse_button::X1);
		if (aButton & SDL_BUTTON(SDL_BUTTON_X2))
			result = static_cast<mouse_button>(result | mouse_button::X2);
		return result;
	}

	inline mouse_button convert_mouse_button(Uint8 aButtonIndex)
	{
		switch (aButtonIndex)
		{
		case SDL_BUTTON_LEFT:
			return mouse_button::Left;
		case SDL_BUTTON_RIGHT:
			return mouse_button::Right;
		case SDL_BUTTON_MIDDLE:
			return mouse_button::Middle;
		case SDL_BUTTON_X1:
			return mouse_button::X1;
		case SDL_BUTTON_X2:
			return mouse_button::X2;
		default:
			return mouse_button::Left;
		}
	}

	inline Uint32 convert_mouse_button(mouse_button aButton)
	{
		Uint32 result = 0u;
		if ((aButton & mouse_button::Left) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_LEFT);
		if ((aButton & mouse_button::Right) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_RIGHT);
		if ((aButton & mouse_button::Middle) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_MIDDLE);
		if ((aButton & mouse_button::X1) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_X1);
		if ((aButton & mouse_button::X2) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_X2);
		return result;
	}
}