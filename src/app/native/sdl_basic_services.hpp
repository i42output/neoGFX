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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_basic_services.hpp>

namespace neogfx
{
	class sdl_basic_services : public i_basic_services
	{
	public:
		sdl_basic_services(neolib::io_thread& aAppThread);
	public:
		virtual neogfx::platform platform() const;
		virtual neolib::io_thread& app_thread();
		virtual void system_beep();
		virtual void display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle = 0) const;
		virtual uint32_t display_count() const;
		virtual rect desktop_rect(uint32_t aDisplayIndex = 0) const;
		virtual bool has_system_clipboard() const;
		virtual i_native_clipboard& system_clipboard();
		virtual bool has_system_menu_bar() const;
		virtual i_shared_menu_bar& system_menu_bar();
	private:
#ifdef WIN32
		static BOOL CALLBACK enum_display_monitors_proc(HMONITOR aMonitor, HDC, LPRECT, LPARAM aThis);
#endif
	private:
		neolib::io_thread& iAppThread;
		mutable std::vector<rect> iDesktopWorkAreas;
	};
}