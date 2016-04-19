// sdl_basic_services.hpp
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
#include "i_basic_services.hpp"

namespace neogfx
{
	class sdl_basic_services : public i_basic_services
	{
	public:
		sdl_basic_services(neolib::io_thread& aAppThread);
	public:
		virtual neolib::io_thread& app_thread();
		virtual void display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle = 0) const;
		virtual bool has_shared_menu_bar() const;
		virtual i_shared_menu_bar& shared_menu_bar();
	private:
		neolib::io_thread& iAppThread;
	};
}