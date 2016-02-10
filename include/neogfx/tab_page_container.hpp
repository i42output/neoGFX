// tab_page_container.hpp
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
#include "scrollable_widget.hpp"
#include "vertical_layout.hpp"
#include "stack_layout.hpp"
#include "event.hpp"
#include "i_tab_page_container.hpp"
#include "tab_bar.hpp"

namespace neogfx
{
	class tab_page_container : public widget, public i_tab_page_container
	{
	public:
		class default_tab_page : public scrollable_widget, public i_tab_page
		{
		public:
			default_tab_page(i_widget& aParent, i_tab& aTab);
			default_tab_page(i_layout& aLayout, i_tab& aTab);
		public:
			virtual size minimum_size() const;
		public:
			virtual bool transparent_background() const;
		public:
			virtual colour background_colour() const;
		public:
			virtual const i_tab& tab() const;
			virtual i_tab& tab();
			virtual const i_widget& widget() const;
			virtual i_widget& widget();
		private:
			i_tab& iTab;
		};
	private:
		typedef std::shared_ptr<i_tab_page> tab_page_pointer;
		typedef std::map<i_tab*, tab_page_pointer> tab_list;
	public:
		tab_page_container();
		tab_page_container(i_widget& aParent);
		tab_page_container(i_layout& aLayout);
	public:
		stack_layout& page_layout();
	public:
		virtual bool has_tabs() const;
		virtual uint32_t tab_count() const;
		virtual const i_tab& tab(tab_index aTabIndex) const;
		virtual i_tab& tab(tab_index aTabIndex);
		virtual const i_tab_page& tab_page(tab_index aTabIndex) const;
		virtual i_tab_page& tab_page(tab_index aTabIndex);
		virtual const i_tab& selected_tab() const;
		virtual i_tab& selected_tab();
		virtual const i_tab_page& selected_tab_page() const;
		virtual i_tab_page& selected_tab_page();
		virtual i_tab& add_tab(const std::string& aTabText);
		virtual i_tab& insert_tab(tab_index aTabIndex, const std::string& aTabText);
		virtual void remove_tab(tab_index aTabIndex);
		virtual i_tab_page& add_tab_page(const std::string& aTabText);
		virtual i_tab_page& insert_tab_page(tab_index aTabIndex, const std::string& aTabText);
		virtual i_tab_page& add_tab_page(i_tab& aTab);
		virtual i_tab_page& add_tab_page(i_tab& aTab, i_tab_page& aWidget);
		virtual i_tab_page& add_tab_page(i_tab& aTab, std::shared_ptr<i_tab_page> aWidget);
	public:
		virtual void adding_tab(i_tab& aTab);
		virtual void selecting_tab(i_tab& aTab);
		virtual void removing_tab(i_tab& aTab);
	protected:
		virtual bool can_defer_layout() const;
		virtual bool is_managing_layout() const;
	protected:
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		vertical_layout iContainerLayout;
		tab_bar iTabBar;
		stack_layout iPageLayout;
		tab_list iTabs;
	};
}