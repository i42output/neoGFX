// menu_bar.hpp
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
#include "widget.hpp"
#include "menu.hpp"

namespace neogfx
{
	class menu_bar : public widget, public menu
	{
	private:
		typedef std::unique_ptr<i_menu_item> item_pointer;
		typedef std::vector<i_menu_item> item_list;
	public:
		menu_bar();
		menu_bar(i_widget& aParent);
		menu_bar(i_layout& aLayout);
		~menu_bar();
	public:
		virtual neogfx::size_policy size_policy() const;	
	public:
		virtual bool visible() const;
	private:
		void init();
	};
}