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
		virtual const i_window& as_window() const = 0;
		virtual i_window& as_window() = 0;
	};
}