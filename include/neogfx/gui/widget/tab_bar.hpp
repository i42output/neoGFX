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

#include <neogfx/neogfx.hpp>
#include "widget.hpp"
#include "tab_button.hpp"

namespace neogfx
{
	class tab_bar : public widget, public i_tab_container
	{
	private:
		typedef std::unique_ptr<tab_button> tab_pointer;
		typedef std::vector<tab_pointer> tab_list;
	public:
		typedef tab_list::size_type tab_index;
	public:
		tab_bar(i_tab_container& aContainer);
		tab_bar(i_widget& aParent, i_tab_container& aContainer);
		tab_bar(i_layout& aLayout, i_tab_container& aContainer);
	public:
		bool has_tabs() const override;
		uint32_t tab_count() const override;
		tab_index index_of(const i_tab& aTab) const override;
		const i_tab& tab(tab_index aTabIndex) const override;
		i_tab& tab(tab_index aTabIndex) override;
		const i_tab& selected_tab() const override;
		i_tab& selected_tab() override;
		i_tab& add_tab(const std::string& aTabText) override;
		i_tab& insert_tab(tab_index aTabIndex, const std::string& aTabText) override;
		void remove_tab(tab_index aTabIndex) override;
		void show_tab(tab_index aTabIndex) override;
		void hide_tab(tab_index aTabIndex) override;
	public:
		void adding_tab(i_tab& aTab) override;
		void selecting_tab(i_tab& aTab) override;
		void removing_tab(i_tab& aTab) override;
	public:
		bool has_tab_page(tab_index aTabIndex) const override;
		const i_tab_page& tab_page(tab_index aTabIndex) const override;
		i_tab_page& tab_page(tab_index aTabIndex) override;
	public:
		bool has_parent_container() const override;
		const i_tab_container& parent_container() const override;
		i_tab_container& parent_container() override;
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	public:
		size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void paint(graphics_context& aGraphicsContext) const override;
	public:
		bool visible() const override;
	private:
		i_tab_container& iContainer;
		tab_list iTabs;
	};
}