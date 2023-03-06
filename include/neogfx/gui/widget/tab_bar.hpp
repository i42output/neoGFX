// tab_bar.hpp
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
#include "scrollable_widget.hpp"
#include "tab_button.hpp"

namespace neogfx
{
    class tab_bar : public framed_scrollable_widget, public i_tab_container
    {
        meta_object(framed_scrollable_widget)
    public:
        define_declared_event(StyleChanged, style_changed)
    private:
        typedef std::unique_ptr<tab_button> tab_pointer;
        typedef std::vector<tab_pointer> tab_list;
    public:
        typedef i_tab_container::tab_index tab_index;
    public:
        tab_bar(i_tab_container& aContainer, bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
        tab_bar(i_widget& aParent, i_tab_container& aContainer, bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
        tab_bar(i_layout& aLayout, i_tab_container& aContainer, bool aClosableTabs = false, neogfx::tab_container_style aStyle = neogfx::tab_container_style::TabAlignmentTop);
    public:
        bool closable_tabs() const noexcept override;
        void set_closable_tabs(bool aClosableTabs) override;
        neogfx::tab_container_style tab_container_style() const noexcept override;
        void set_tab_container_style(neogfx::tab_container_style aStyle) override;
        void set_tab_icon_size(const size& aIconSize) override;
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        bool has_tabs() const noexcept override;
        uint32_t tab_count() const noexcept override;
        tab_index index_of(const i_tab& aTab) const override;
        const i_tab& tab(tab_index aTabIndex) const override;
        i_tab& tab(tab_index aTabIndex) override;
        bool is_tab_selected() const noexcept override;
        const i_tab& selected_tab() const override;
        i_tab& selected_tab() override;
        i_tab& add_tab(i_string const& aTabText) override;
        i_tab& insert_tab(tab_index aTabIndex, i_string const& aTabText) override;
        void remove_tab(tab_index aTabIndex) override;
        void show_tab(tab_index aTabIndex) override;
        void hide_tab(tab_index aTabIndex) override;
        optional_tab_index next_visible_tab(tab_index aStartFrom) const override;
        optional_tab_index previous_visible_tab(tab_index aStartFrom) const override;
        void select_next_tab() override;
        void select_previous_tab() override;
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
    public:
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
    public:
        bool visible() const override;
    protected:
        bool is_managing_layout() const override;
    private:
        void update_placement();
    private:
        i_tab_container& iContainer;
        bool iClosableTabs;
        neogfx::tab_container_style iStyle;
        size iIconSize = size{ 16.0, 16.0 };
        tab_list iTabs;
        std::optional<horizontal_layout> iHorizontalLayout;
        std::optional<vertical_layout> iVerticalLayout;
    };
}