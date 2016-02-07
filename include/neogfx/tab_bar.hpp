// tab_bar.hpp
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
#include "tab_button.hpp"
#include "i_tab_page_container.hpp"

namespace neogfx
{
	class tab_bar : public widget, public i_tab_container
	{
	private:
		typedef std::unique_ptr<tab_button> tab_pointer;
		typedef std::vector<tab_pointer> tab_list;
		typedef tab_list::size_type tab_index;
	public:
		tab_bar(i_tab_page_container& aContainer);
		tab_bar(i_widget& aParent, i_tab_page_container& aContainer);
		tab_bar(i_layout& aLayout, i_tab_page_container& aContainer);
	public:
		virtual bool has_tabs() const;
		virtual uint32_t tab_count() const;
		virtual const i_tab& tab(tab_index aTabIndex) const;
		virtual i_tab& tab(tab_index aTabIndex);
		virtual const i_tab& selected_tab() const;
		virtual i_tab& selected_tab();
		virtual i_tab& add_tab(const std::string& aTabText);
		virtual i_tab& insert_tab(tab_index aTabIndex, const std::string& aTabText);
		virtual void remove_tab(tab_index aTabIndex);
	public:
		virtual void adding_tab(i_tab& aTab);
		virtual void selecting_tab(i_tab& aTab);
		virtual void removing_tab(i_tab& aTab);
	protected:
		virtual size maximum_size() const;
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		i_tab_page_container& iContainer;
		tab_list iTabs;
	};
}