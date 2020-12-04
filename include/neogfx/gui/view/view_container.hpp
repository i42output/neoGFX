// view_container.hpp
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
#include <neogfx/app/drag_drop.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/tab_bar.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include "i_view.hpp"
#include "i_view_container.hpp"

namespace neogfx
{
    class view_stack : public drag_drop_target<framed_scrollable_widget>
    {
    public:
        view_stack(i_layout& aLayout, i_view_container& aParent);
    public:
        void add_view(i_view& aView);
        void remove_view(i_view& aView);
    protected:
        neogfx::focus_policy focus_policy() const override;
    private:
        stack_layout iLayout;
        i_view_container& iParent;
    };

    class view_container : public i_view_container, public widget<>
    {
    public:
        define_declared_event(ViewAdded, view_added, i_view&)
        define_declared_event(ViewRemoved, view_removed, i_view&)
    private:
        class tab_container : public i_tab_container
        {
        public:
            define_declared_event(StyleChanged, style_changed)
        public:
            typedef std::map<i_tab*, i_view*> tab_list;
        public:
            tab_container(view_container& aOwner);
        public:
            tab_container_style style() const override;
            void set_style(tab_container_style aStyle) override;
        public:
            bool has_tabs() const override;
            uint32_t tab_count() const override;
            tab_index index_of(const i_tab& aTab) const override;
            const i_tab& tab(tab_index aTabIndex) const override;
            i_tab& tab(tab_index aTabIndex) override;
            bool is_tab_selected() const override;
            const i_tab& selected_tab() const override;
            i_tab& selected_tab() override;
            i_tab& add_tab(std::string const& aTabText) override;
            i_tab& insert_tab(tab_index aTabIndex, std::string const& aTabText) override;
            void remove_tab(tab_index aTabIndex) override;
            void show_tab(tab_index aTabIndex) override;
            void hide_tab(tab_index aTabIndex) override;
            optional_tab_index next_visible_tab(tab_index aStartFrom) const override;
            optional_tab_index previous_visible_tab(tab_index aStartFrom) const  override;
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
            const i_widget& as_widget() const override;
            i_widget& as_widget() override;
        private:
            view_container& iOwner;
            tab_list iTabs;
            tab_bar iTabBar;
        };
    public:
        view_container(i_widget& aParent, view_container_style aStyle = view_container_style::Tabbed);
        view_container(i_layout& aLayout, view_container_style aStyle = view_container_style::Tabbed);
    public:
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
        const neogfx::view_stack& view_stack() const override;
        neogfx::view_stack& view_stack() override;
    public:
        view_container_style style() const override;
        void change_style(view_container_style aNewStyle) override;
    public:
        void add_controller(i_controller& aController) override;
        void add_controller(std::shared_ptr<i_controller> aController) override;
        void remove_controller(i_controller& aController) override;
    public:
        bool is_managing_layout() const override;
    private:
        view_container_style iStyle;
        vertical_layout iLayout0;
        tab_container iTabContainer;
        horizontal_layout iLayout1;
        neogfx::view_stack iViewStack;
        std::vector<std::shared_ptr<i_controller>> iControllers;
    };

    extern template class scrollable_widget<view_container>;
    typedef scrollable_widget<view_container> scrollable_view_container;
}