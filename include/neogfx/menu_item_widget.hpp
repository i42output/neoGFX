// menu_item_widget.hpp
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
#include "horizontal_layout.hpp"
#include "text_widget.hpp"
#include "image_widget.hpp"
#include "i_menu_item.hpp"

namespace neogfx
{
	class menu_item_widget : public widget
	{
	public:
		menu_item_widget(i_menu& aMenu, i_menu_item& aMenuItem);
		menu_item_widget(i_widget& aParent, i_menu& aMenu, i_menu_item& aMenuItem);
		menu_item_widget(i_layout& aLayout, i_menu& aMenu, i_menu_item& aMenuItem);
	public:
		virtual neogfx::size_policy size_policy() const;
	public:
		virtual void paint_non_client(graphics_context& aGraphicsContext) const;
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual void mouse_entered();
		virtual void mouse_left();	
	private:
		void init();
	private:
		i_menu& iMenu;
		i_menu_item& iMenuItem;
		horizontal_layout iLayout;
		image_widget iIcon;
		text_widget iText;
	};
}