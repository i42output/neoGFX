// i_basic_services.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neolib/io_thread.hpp>
#include <neogfx/core/geometry.hpp>

namespace neogfx
{
	class i_native_clipboard;
	class i_shared_menu_bar;

	enum class platform
	{
		Windows,
		Mac,
		Kde,
		Gnome
	};

	class i_basic_services
	{
	public:
		struct bad_display_index : std::logic_error { bad_display_index() : std::logic_error("neogfx::i_basic_services::bad_display_index") {} };
		struct no_system_menu_bar : std::logic_error { no_system_menu_bar() : std::logic_error("neogfx::i_basic_services::no_system_menu_bar") {} };
	public:
		virtual ~i_basic_services() {}
	public:
		virtual neogfx::platform platform() const = 0;
		virtual neolib::io_thread& app_thread() = 0;
		virtual void system_beep() = 0;
		virtual void display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle = 0) const = 0;
		virtual uint32_t display_count() const = 0;
		virtual rect desktop_rect(uint32_t aDisplayIndex = 0) const = 0;
		virtual bool has_system_clipboard() const = 0;
		virtual i_native_clipboard& system_clipboard() = 0;
		virtual bool has_system_menu_bar() const = 0;
		virtual i_shared_menu_bar& system_menu_bar() = 0;
	};
}