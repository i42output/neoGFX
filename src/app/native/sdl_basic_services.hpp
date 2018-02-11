// sdl_basic_services.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
#include <neogfx/core/colour.hpp>
#include <neogfx/app/i_basic_services.hpp>

namespace neogfx
{
	class display : public i_display
	{
	public:
		display(const neogfx::rect& aRect, const neogfx::rect& aDesktopRect, void* aNativeDisplayHandle);
		~display();
	public:
		neogfx::rect rect() const override;
		neogfx::rect desktop_rect() const override;
		colour read_pixel(const point& aPosition) const override;
	private:
		mutable neogfx::rect iRect;
		mutable neogfx::rect iDesktopRect;
		void* iNativeDisplayHandle;
	};

	class sdl_basic_services : public i_basic_services
	{
	public:
		sdl_basic_services(neolib::io_task& aAppThread);
	public:
		neogfx::platform platform() const override;
		neolib::io_task& app_task() override;
		void system_beep() override;
		void display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle = 0) const override;
		uint32_t display_count() const override;
		const i_display& display(uint32_t aDisplayIndex = 0) const override;
		bool has_system_clipboard() const override;
		i_native_clipboard& system_clipboard() override;
		bool has_system_menu_bar() const override;
		i_shared_menu_bar& system_menu_bar() override;
	private:
		neolib::io_task& iAppTask;
		mutable std::vector<std::unique_ptr<i_display>> iDisplays;
	};
}