// sdl_basic_services.cpp
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
#include <SDL_messagebox.h>
#include "sdl_basic_services.hpp"

namespace neogfx
{
	sdl_basic_services::sdl_basic_services(neolib::io_thread& aAppThread) : 
		iAppThread(aAppThread)
	{
	}

	neolib::io_thread& sdl_basic_services::app_thread()
	{
		return iAppThread;
	}

	void sdl_basic_services::display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle) const
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, aTitle.c_str(), aMessage.c_str(), static_cast<SDL_Window*>(aParentWindowHandle));
	}

	bool sdl_basic_services::has_shared_menu_bar() const
	{
		return false;
	}

	i_shared_menu_bar& sdl_basic_services::shared_menu_bar()
	{
		throw no_shared_menu_bar();
	}
}