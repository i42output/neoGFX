// sdl_basic_services.hpp
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
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/hid/i_display.hpp>

namespace neogfx
{
	class sdl_basic_services : public i_basic_services
	{
	public:
		sdl_basic_services(neolib::async_task& aAppThread);
	public:
		neogfx::platform platform() const override;
		neolib::async_task& app_task() override;
		void system_beep() override;
		void display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle = 0) const override;
		uint32_t display_count() const override;
		i_display& display(uint32_t aDisplayIndex = 0) const override;
		bool has_system_clipboard() const override;
		i_native_clipboard& system_clipboard() override;
		bool has_system_menu_bar() const override;
		i_shared_menu_bar& system_menu_bar() override;
	private:
		neolib::async_task& iAppTask;
		mutable std::vector<std::unique_ptr<i_display>> iDisplays;
	};
}