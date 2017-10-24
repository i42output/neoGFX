// i_nested_window.hpp
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
#include <neogfx/gui/window/i_window.hpp>

namespace neogfx
{
	class i_nested_window_container;

	class i_nested_window
	{
	public:
		struct no_nested_parent : std::logic_error { no_nested_parent() : std::logic_error("neogfx::i_nested_window::no_nested_parent") {} };
	public:
		virtual const i_nested_window_container& nested_root() const = 0;
		virtual i_nested_window_container& nested_root() = 0;
		virtual bool has_nested_parent() const = 0;
		virtual const i_nested_window& nested_parent() const = 0;
		virtual i_nested_window& nested_parent() = 0;
	public:
		virtual const std::string& title_text() const = 0;
		virtual void set_title_text(const std::string& aTitleText) = 0;
	public:
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual bool is_iconic() const = 0;
		virtual void iconize() = 0;
		virtual bool is_maximized() const = 0;
		virtual void maximize() = 0;
		virtual bool is_restored() const = 0;
		virtual void restore() = 0;
		virtual window_placement placement() const = 0;
		virtual void set_placement(const window_placement& aPlacement) = 0;
		virtual void centre() = 0;
		virtual void centre_on_parent() = 0;
		virtual bool window_enabled() const = 0;
		virtual void counted_window_enable(bool aEnable) = 0;
	public:
		virtual const i_layout& non_client_layout() const = 0;
		virtual i_layout& non_client_layout() = 0;
		virtual const i_layout& title_bar_layout() const = 0;
		virtual i_layout& title_bar_layout() = 0;
		virtual const i_layout& menu_layout() const = 0;
		virtual i_layout& menu_layout() = 0;
		virtual const i_layout& toolbar_layout() const = 0;
		virtual i_layout& toolbar_layout() = 0;
		virtual const i_layout& client_layout() const = 0;
		virtual i_layout& client_layout() = 0;
		virtual const i_layout& status_bar_layout() const = 0;
		virtual i_layout& status_bar_layout() = 0;
	public:
		virtual const i_widget& as_widget() const = 0;
		virtual i_widget& as_widget() = 0;
	};
}