// tab_button.hpp
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
#include "push_button.hpp"
#include "i_tab.hpp"
#include "i_tab_container.hpp"

namespace neogfx
{
	class tab_button : public push_button, public i_tab
	{
		friend class tab_bar;
	public:
		tab_button(i_tab_container& aContainer, const std::string& aText = std::string());
		tab_button(i_widget& aParent, i_tab_container& aContainer, const std::string& aText = std::string());
		tab_button(i_layout& aLayout, i_tab_container& aContainer, const std::string& aText = std::string());
		~tab_button();
	public:
		virtual bool is_selected() const;
		virtual bool is_deselected() const;
		virtual void select();
		virtual const i_tab_container& container() const;
		virtual i_tab_container& container();
	public:
		virtual const std::string& text() const;
		virtual void set_text(const std::string& aText);
	protected:
		virtual rect path_bounding_rect() const;
		virtual bool spot_colour() const;
	protected:
		virtual size minimum_size() const;
		virtual void handle_pressed();
	protected:
		virtual colour foreground_colour() const;
	protected:
		virtual void paint(graphics_context& aGraphicsContext) const;
	protected:
		void set_selected_state(bool aSelectedState);
	private:
		i_tab_container& iContainer;
		bool iSelectedState;
	};
}