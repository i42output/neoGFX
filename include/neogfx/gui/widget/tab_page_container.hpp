// tab_page_container.hpp
/*
neogfx C++ GUI Library
Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.

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
    class tab_page_container : public widget, public i_tab_page_container
    {
    public:
        define_declared_event(StyleChanged, style_changed)
        define_declared_event(SelectedTabPageChanged, selected_tab_page_changed, i_tab_page&)
    public:
        class default_tab_page : public scrollable_widget, public i_tab_page
        {
        public:
            define_declared_event(Selected, selected)
            define_declared_event(Deselected, deselected)
        public:
            default_tab_page(i_widget& aParent, i_tab& aTab);
            default_tab_page(i_layout& aLayout, i_tab& aTab);
        public:
            neogfx::size_policy size_policy() const override;
            size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
            size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
        public:
            bool transparent_background() const override;
            void paint_non_client_after(i_graphics_context& aGraphicsContext) const override;
        public:
            colour background_colour() const override;
        public:
            const i_tab& tab() const override;
            i_tab& tab() override;
            const widget& as_widget() const override;
            widget& as_widget() override;
        private:
            i_tab& iTab;
            sink iSink;
        };
    private:
        typedef std::shared_ptr<i_tab_page> tab_page_pointer;
        typedef std::map<const i_tab*, tab_page_pointer> tab_list;
    public:
        tab_page_container(bool aClosableTabs = false, tab_container_style aStyle = tab_container_style::TabAlignmentTop);
        tab_page_container(i_widget& aParent, bool aClosableTabs = false, tab_container_style aStyle = tab_container_style::TabAlignmentTop);
        tab_page_container(i_layout& aLayout, bool aClosableTabs = false, tab_container_style aStyle = tab_container_style::TabAlignmentTop);
        ~tab_page_container();
    public:
        tab_container_style style() const override;
        void set_style(tab_container_style aStyle) override;
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
        bool is_tab_selected() const override;
        const i_tab& selected_tab() const override;
        i_tab& selected_tab() override;
        const i_tab_page& selected_tab_page() const override;
        i_tab_page& selected_tab_page() override;
        i_tab& add_tab(const std::string& aTabText) override;
        i_tab& insert_tab(tab_index aTabIndex, const std::string& aTabText) override;
        void remove_tab(tab_index aTabIndex) override;
        void show_tab(tab_index aTabIndex) override;
        void hide_tab(tab_index aTabIndex) override;
        optional_tab_index next_visible_tab(tab_index aStartFrom) const override;
        optional_tab_index previous_visible_tab(tab_index aStartFrom) const  override;
        void select_next_tab() override;
        void select_previous_tab() override;
    public:
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
        const widget& as_widget() const override;
        widget& as_widget() override;
    protected:
        bool can_defer_layout() const override;
        bool is_managing_layout() const override;
    private:
        void update_tab_bar_placement();
    private:
        border_layout iContainerLayout;
        tab_bar iTabBar;
        tab_list iTabs;
    };
}