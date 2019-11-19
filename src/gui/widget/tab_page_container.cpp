// tab_page_container.cpp
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
#include <neogfx/gui/widget/tab_page_container.hpp>
#include <neogfx/gui/widget/tab_page.hpp>

namespace neogfx
{
    tab_page_container::tab_page_container(bool aClosableTabs, tab_container_style aStyle) :
        widget{}, iContainerLayout{ *this }, iTabBar{ iContainerLayout.top(), *this, aClosableTabs, aStyle }
    {
        set_margins(neogfx::margins{});
        update_tab_bar_placement();
    }

    tab_page_container::tab_page_container(i_widget& aParent, bool aClosableTabs, tab_container_style aStyle) :
        widget{ aParent }, iContainerLayout{ *this }, iTabBar{ iContainerLayout.top(), *this, aClosableTabs, aStyle }
    {
        set_margins(neogfx::margins{});
        update_tab_bar_placement();
    }

    tab_page_container::tab_page_container(i_layout& aLayout, bool aClosableTabs, tab_container_style aStyle) :
        widget{ aLayout }, iContainerLayout{ *this }, iTabBar{ iContainerLayout.top(), *this, aClosableTabs, aStyle }
    {
        set_margins(neogfx::margins{});
        update_tab_bar_placement();
    }

    tab_page_container::~tab_page_container()
    {
        set_destroying();
        while (tab_count() > 0)
            remove_tab(tab_count() - 1);
    }

    stack_layout& tab_page_container::page_layout()
    {
        return iContainerLayout.centre();
    }

    tab_container_style tab_page_container::style() const
    {
        return iTabBar.style();
    }

    void tab_page_container::set_style(tab_container_style aStyle)
    {
        if (style() != aStyle)
        {
            iTabBar.set_style(aStyle);
            update_tab_bar_placement();
            StyleChanged.trigger();
        }
    }

    bool tab_page_container::has_tabs() const
    {
        return !iTabs.empty();
    }

    uint32_t tab_page_container::tab_count() const
    {
        return static_cast<uint32_t>(iTabs.size());
    }

    tab_page_container::tab_index tab_page_container::index_of(const i_tab& aTab) const
    {
        return iTabBar.index_of(aTab);
    }

    const i_tab& tab_page_container::tab(tab_index aTabIndex) const
    {
        if (aTabIndex >= iTabs.size())
            throw tab_not_found();
        return iTabBar.tab(aTabIndex);
    }

    i_tab& tab_page_container::tab(tab_index aTabIndex)
    {
        return const_cast<i_tab&>(to_const(*this).tab(aTabIndex));
    }

    bool tab_page_container::has_tab_page(tab_index aTabIndex) const
    {
        return aTabIndex < iTabs.size();
    }

    const i_tab_page& tab_page_container::tab_page(tab_index aTabIndex) const
    {
        if (aTabIndex >= iTabs.size())
            throw tab_not_found();
        const auto& tabPagePtr = iTabs.find(&tab(aTabIndex))->second;
        if (tabPagePtr == nullptr)
            throw tab_page_not_found();
        return *tabPagePtr;
    }

    i_tab_page& tab_page_container::tab_page(tab_index aTabIndex)
    {
        return const_cast<i_tab_page&>(to_const(*this).tab_page(aTabIndex));
    }

    bool tab_page_container::is_tab_selected() const
    {
        for (auto& tab : iTabs)
            if (tab.first->is_selected())
                return true;
        return false;
    }

    const i_tab& tab_page_container::selected_tab() const
    {
        for (auto& tab : iTabs)
            if (tab.first->is_selected())
                return *tab.first;
        throw tab_not_found();
    }

    i_tab& tab_page_container::selected_tab()
    {
        return const_cast<i_tab&>(to_const(*this).selected_tab());
    }

    const i_tab_page& tab_page_container::selected_tab_page() const
    {
        for (auto& tab : iTabs)
            if (tab.first->is_selected())
            {
                if (tab.second == nullptr)
                    throw tab_page_not_found();
                return *tab.second;
            }
        throw tab_not_found();
    }

    i_tab_page& tab_page_container::selected_tab_page()
    {
        return const_cast<i_tab_page&>(to_const(*this).selected_tab_page());
    }

    i_tab& tab_page_container::add_tab(const std::string& aTabText)
    {
        auto& newTab = iTabBar.add_tab(aTabText);
        iTabs.emplace(&newTab, tab_page_pointer());
        return newTab;
    }

    i_tab& tab_page_container::insert_tab(tab_index aTabIndex, const std::string& aTabText)
    {
        auto& newTab = iTabBar.insert_tab(aTabIndex, aTabText);
        iTabs.emplace(&newTab, tab_page_pointer());
        return newTab;
    }

    void tab_page_container::remove_tab(tab_index aTabIndex)
    {
        iTabBar.remove_tab(aTabIndex);
    }

    void tab_page_container::show_tab(tab_index aTabIndex)
    {
        tab(aTabIndex).as_widget().show();
        if (has_tab_page(aTabIndex))
            tab_page(aTabIndex).as_widget().show();
    }

    void tab_page_container::hide_tab(tab_index aTabIndex)
    {
        tab(aTabIndex).as_widget().hide();
        if (has_tab_page(aTabIndex))
            tab_page(aTabIndex).as_widget().hide();
    }

    tab_page_container::optional_tab_index tab_page_container::next_visible_tab(tab_index aStartFrom) const
    {
        return iTabBar.next_visible_tab(aStartFrom);
    }

    tab_page_container::optional_tab_index tab_page_container::previous_visible_tab(tab_index aStartFrom) const
    {
        return iTabBar.previous_visible_tab(aStartFrom);
    }

    void tab_page_container::select_next_tab()
    {
        iTabBar.select_next_tab();
    }

    void tab_page_container::select_previous_tab()
    {
        iTabBar.select_previous_tab();
    }

    i_tab_page& tab_page_container::add_tab_page(const std::string& aTabText)
    {
        return add_tab_page(add_tab(aTabText));
    }

    i_tab_page& tab_page_container::insert_tab_page(tab_index aTabIndex, const std::string& aTabText)
    {
        return add_tab_page(insert_tab(aTabIndex, aTabText));
    }

    i_tab_page& tab_page_container::add_tab_page(i_tab& aTab)
    {
        auto existingTab = iTabs.find(&aTab);
        if (existingTab == iTabs.end())
            throw tab_not_found();
        existingTab->second = tab_page_pointer{ new neogfx::tab_page{ page_layout(), aTab } };
        if (aTab.is_selected())
        {
            existingTab->second->as_widget().show();
            iContainerLayout.invalidate();
        }
        else
            existingTab->second->as_widget().hide();
        return *existingTab->second;
    }

    i_tab_page& tab_page_container::add_tab_page(i_tab& aTab, i_tab_page& aWidget)
    {
        auto existingTab = iTabs.find(&aTab);
        if (existingTab == iTabs.end())
            throw tab_not_found();
        existingTab->second = tab_page_pointer{ tab_page_pointer{}, &aWidget };
        if (aTab.is_selected())
        {
            existingTab->second->as_widget().show();
            iContainerLayout.invalidate();
        }
        else
            existingTab->second->as_widget().hide();
        return *existingTab->second;
    }

    i_tab_page& tab_page_container::add_tab_page(i_tab& aTab, std::shared_ptr<i_tab_page> aWidget)
    {
        auto existingTab = iTabs.find(&aTab);
        if (existingTab == iTabs.end())
            throw tab_not_found();
        existingTab->second = aWidget;
        if (aTab.is_selected())
        {
            existingTab->second->as_widget().show();
            iContainerLayout.invalidate();
        }
        else
            existingTab->second->as_widget().hide();
        return *existingTab->second;
    }

    void tab_page_container::adding_tab(i_tab& aTab)
    {
        iTabs.emplace(&aTab, tab_page_pointer());
        if (iTabs.size() == 1)
            aTab.select();
    }

    void tab_page_container::selecting_tab(i_tab& aTab)
    {
        for (auto& tab : iTabs)
            if (tab.second != nullptr)
            {
                if (tab.first == &aTab)
                    tab.second->as_widget().show();
                else
                    tab.second->as_widget().hide();
            }
        iContainerLayout.invalidate();
    }

    void tab_page_container::removing_tab(i_tab& aTab)
    {
        auto existingTab = iTabs.find(&aTab);
        if (existingTab == iTabs.end())
            throw tab_not_found();
        iTabs.erase(existingTab);
    }

    bool tab_page_container::has_parent_container() const
    {
        return false;
    }

    const i_tab_container& tab_page_container::parent_container() const
    {
        throw no_parent_container();
    }

    i_tab_container& tab_page_container::parent_container()
    {
        throw no_parent_container();
    }

    const widget& tab_page_container::as_widget() const
    {
        return *this;
    }

    widget& tab_page_container::as_widget()
    {
        return *this;
    }

    bool tab_page_container::can_defer_layout() const
    {
        return true;
    }

    bool tab_page_container::is_managing_layout() const
    {
        return true;
    }

    void tab_page_container::update_tab_bar_placement()
    {
        switch (style() & tab_container_style::TabAlignmentMask)
        {
        case tab_container_style::TabAlignmentTop:
            iContainerLayout.remove(iTabBar);
            iContainerLayout.top().add(iTabBar);
            break;
        case tab_container_style::TabAlignmentBottom:
            iContainerLayout.remove(iTabBar);
            iContainerLayout.bottom().add(iTabBar);
            break;
        case tab_container_style::TabAlignmentLeft:
            iContainerLayout.remove(iTabBar);
            iContainerLayout.left().add(iTabBar);
            break;
        case tab_container_style::TabAlignmentRight:
            iContainerLayout.remove(iTabBar);
            iContainerLayout.right().add(iTabBar);
            break;
        }
    }
}