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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/gui/widget/i_tab_page_container.hpp>
#include <neogfx/gui/widget/tab_bar.hpp>

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
			size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		public:
			bool transparent_background() const override;
		public:
			colour background_colour() const override;
		public:
			const i_tab& tab() const override;
			i_tab& tab() override;
			const i_widget& as_widget() const override;
			i_widget& as_widget() override;
		private:
			i_tab& iTab;
		};
	private:
		typedef std::shared_ptr<i_tab_page> tab_page_pointer;
		typedef std::map<const i_tab*, tab_page_pointer> tab_list;
	public:
		tab_page_container();
		tab_page_container(i_widget& aParent);
		tab_page_container(i_layout& aLayout);
		~tab_page_container();
	public:
		stack_layout& page_layout();
	public:
		bool has_tabs() const override;
		uint32_t tab_count() const override;
		tab_index index_of(const i_tab& aTab) const override;
		const i_tab& tab(tab_index aTabIndex) const override;
		i_tab& tab(tab_index aTabIndex) override;
		bool has_tab_page(tab_index aTabIndex) const override;
		const i_tab_page& tab_page(tab_index aTabIndex) const override;
		i_tab_page& tab_page(tab_index aTabIndex) override;
		const i_tab& selected_tab() const override;
		i_tab& selected_tab() override;
		const i_tab_page& selected_tab_page() const override;
		i_tab_page& selected_tab_page() override;
		i_tab& add_tab(const std::string& aTabText) override;
		i_tab& insert_tab(tab_index aTabIndex, const std::string& aTabText) override;
		void remove_tab(tab_index aTabIndex) override;
		i_tab_page& add_tab_page(const std::string& aTabText) override;
		i_tab_page& insert_tab_page(tab_index aTabIndex, const std::string& aTabText) override;
		i_tab_page& add_tab_page(i_tab& aTab) override;
		i_tab_page& add_tab_page(i_tab& aTab, i_tab_page& aWidget) override;
		i_tab_page& add_tab_page(i_tab& aTab, std::shared_ptr<i_tab_page> aWidget) override;
	public:
		void adding_tab(i_tab& aTab) override;
		void selecting_tab(i_tab& aTab) override;
		void removing_tab(i_tab& aTab) override;
	public:
		bool has_parent_container() const override;
		const i_tab_container& parent_container() const override;
		i_tab_container& parent_container() override;
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	protected:
		bool can_defer_layout() const override;
		bool is_managing_layout() const override;
	private:
		vertical_layout iContainerLayout;
		tab_bar iTabBar;
		stack_layout iPageLayout;
		tab_list iTabs;
	};
}