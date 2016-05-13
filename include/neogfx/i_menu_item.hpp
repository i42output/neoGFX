// i_menu_item.hpp
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
#include "i_action.hpp"

namespace neogfx
{
	class i_menu;

	class i_menu_item
	{
	public:
		event<> selected;
		event<> deselected;
	public:
		enum type_e
		{
			Action,
			SubMenu,
		};
	public:
		struct wrong_type : std::logic_error { wrong_type() : std::logic_error("neogfx::i_menu_item::wrong_type") {} };
	public:
		virtual type_e type() const = 0;
		virtual const i_action& action() const = 0;
		virtual i_action& action() = 0;
		virtual const i_menu& sub_menu() const = 0;
		virtual i_menu& sub_menu() = 0;
	public:
		virtual bool availabie() const = 0;
	};
}