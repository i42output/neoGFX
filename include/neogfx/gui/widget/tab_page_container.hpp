// tab_page_container.hpp
/*
neogfx C++ App/Game Engine
Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/gui/layout/border_layout.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/gui/widget/i_tab_page_container.hpp>
#include <neogfx/gui/widget/tab_bar.hpp>

namespace neogfx
{
    template <typename Base = widget<>>
    class tab_page_container : public Base, public i_tab_page_container
    {
        typedef Base base_type;
    public:
        define_declared_event(StyleChanged, style_changed)
        define_declared_event(SelectedTabPageChanged, selected_tab_page_changed, i_tab_page&)
    private:
        typedef ref_ptr<i_tab_page> tab_page_pointer;
        typedef std::map<const i_tab*, tab_page_pointer> tab_list;
    public:
        tab_page_container(bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
        tab_page_container(i_widget& aParent, bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
        tab_page_container(i_widget& aParent, i_layout& aTabBarLayout, bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
        tab_page_container(i_layout& aLayout, bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
        tab_page_container(i_layout& aLayout, i_layout& aTabBarLayout, bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
        ~tab_page_container();
    public:
        neogfx::tab_container_style tab_container_style() const override;
        void set_tab_container_style(neogfx::tab_container_style aStyle) override;
        void set_tab_icon_size(const size& aIconSize) override;
    public:
        i_layout& page_layout() override;
    public:
        bool has_tabs() const override;
        uint32_t tab_count() const override;
        tab_index index_of(const i_tab& aTab) const override;
        const i_tab& tab(tab_index aTabIndex) const override;
        i_tab& tab(tab_index aTabIndex) override;
        bool has_tab_page(tab_index aTabIndex) const override;
        const i_tab_page& tab_page(tab_index aTabIndex) const override;
        i_tab_page& tab_page(tab_index aTabIndex) override;
        bool is_tab_selected() const override;
        const i_tab& selected_tab() const override;
        i_tab& selected_tab() override;
        const i_tab_page& selected_tab_page() const override;
        i_tab_page& selected_tab_page() override;
        i_tab& add_tab(i_string const& aTabText) override;
        i_tab& insert_tab(tab_index aTabIndex, i_string const& aTabText) override;
        void remove_tab(tab_index aTabIndex) override;
        void show_tab(tab_index aTabIndex) override;
        void hide_tab(tab_index aTabIndex) override;
        optional_tab_index next_visible_tab(tab_index aStartFrom) const override;
        optional_tab_index previous_visible_tab(tab_index aStartFrom) const  override;
        void select_next_tab() override;
        void select_previous_tab() override;
    public:
        i_tab_page& add_tab_page(i_string const& aTabText) override;
        i_tab_page& insert_tab_page(tab_index aTabIndex, i_string const& aTabText) override;
        i_tab_page& add_tab_page(i_tab& aTab) override;
        i_tab_page& add_tab_page(i_tab& aTab, i_tab_page& aWidget) override;
        i_tab_page& add_tab_page(i_tab& aTab, i_ref_ptr<i_tab_page> const& aWidget) override;
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
        neogfx::focus_policy focus_policy() const override;
    protected:
        bool is_managing_layout() const override;
    private:
        void init();
        void update_tab_bar_placement();
    private:
        tab_list iTabs;
        border_layout iContainerLayout;
        tab_bar iTabBar;
    };
}