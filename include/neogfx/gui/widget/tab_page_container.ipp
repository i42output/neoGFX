// tab_page_container.ipp
/*
neogfx C++ App/Game Engine
Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.

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
    template <typename Base>
    inline tab_page_container<Base>::tab_page_container(bool aClosableTabs, neogfx::tab_container_style aStyle) :
        base_type{}, iContainerLayout{ *this }, iTabBar{ iContainerLayout.top(), *this, aClosableTabs, aStyle }
    {
        init();
    }

    template <typename Base>
    inline tab_page_container<Base>::tab_page_container(i_widget& aParent, bool aClosableTabs, neogfx::tab_container_style aStyle) :
        base_type{ aParent }, iContainerLayout{ *this }, iTabBar{ iContainerLayout.top(), *this, aClosableTabs, aStyle }
    {
        init();
    }

    template <typename Base>
    inline tab_page_container<Base>::tab_page_container(i_widget& aParent, i_layout& aTabBarLayout, bool aClosableTabs, neogfx::tab_container_style aStyle) :
        base_type{ aParent }, iContainerLayout{ *this }, iTabBar{ aTabBarLayout, *this, aClosableTabs, aStyle }
    {
        init();
    }

    template <typename Base>
    inline tab_page_container<Base>::tab_page_container(i_layout& aLayout, bool aClosableTabs, neogfx::tab_container_style aStyle) :
        base_type{ aLayout }, iContainerLayout{ *this }, iTabBar{ iContainerLayout.top(), *this, aClosableTabs, aStyle }
    {
        init();
    }

    template <typename Base>
    inline tab_page_container<Base>::tab_page_container(i_layout& aLayout, i_layout& aTabBarLayout, bool aClosableTabs, neogfx::tab_container_style aStyle) :
        base_type{ aLayout }, iContainerLayout{ *this }, iTabBar{ aTabBarLayout, *this, aClosableTabs, aStyle }
    {
        init();
    }

    template <typename Base>
    inline tab_page_container<Base>::~tab_page_container()
    {
        base_type::set_destroying();
        while (tab_count() > 0)
            remove_tab(tab_count() - 1);
    }

    template <typename Base>
    inline i_tab_container& tab_page_container<Base>::tab_bar()
    {
        return iTabBar;
    }

    template <typename Base>
    inline i_layout& tab_page_container<Base>::page_layout()
    {
        return iContainerLayout.center();
    }

    template <typename Base>
    bool tab_page_container<Base>::closable_tabs() const noexcept
    {
        return iTabBar.closable_tabs();
    }

    template <typename Base>
    void tab_page_container<Base>::set_closable_tabs(bool aClosableTabs)
    {
        iTabBar.set_closable_tabs(aClosableTabs);
    }

    template <typename Base>
    inline tab_container_style tab_page_container<Base>::tab_container_style() const noexcept
    {
        return iTabBar.tab_container_style();
    }

    template <typename Base>
    inline void tab_page_container<Base>::set_tab_container_style(neogfx::tab_container_style aStyle)
    {
        if (tab_container_style() != aStyle)
        {
            iTabBar.set_tab_container_style(aStyle);
            update_tab_bar_placement();
            StyleChanged.trigger();
        }
    }

    template <typename Base>
    inline void tab_page_container<Base>::set_tab_icon_size(const size& aIconSize)
    {
        iTabBar.set_tab_icon_size(aIconSize);
    }

    template <typename Base>
    inline bool tab_page_container<Base>::has_tabs() const noexcept
    {
        return !iTabs.empty();
    }

    template <typename Base>
    inline std::uint32_t tab_page_container<Base>::tab_count() const noexcept
    {
        return static_cast<std::uint32_t>(iTabs.size());
    }

    template <typename Base>
    inline tab_page_container<Base>::tab_index tab_page_container<Base>::index_of(const i_tab& aTab) const
    {
        return iTabBar.index_of(aTab);
    }

    template <typename Base>
    inline const i_tab& tab_page_container<Base>::tab(tab_index aTabIndex) const
    {
        if (aTabIndex >= iTabs.size())
            throw tab_not_found();
        return iTabBar.tab(aTabIndex);
    }

    template <typename Base>
    inline i_tab& tab_page_container<Base>::tab(tab_index aTabIndex)
    {
        return const_cast<i_tab&>(to_const(*this).tab(aTabIndex));
    }

    template <typename Base>
    inline bool tab_page_container<Base>::has_tab_page(tab_index aTabIndex) const
    {
        if (aTabIndex >= iTabs.size())
            throw tab_not_found();
        auto existing = iTabs.find(&tab(aTabIndex));
        if (existing == iTabs.end())
            throw tab_not_found();
        return existing->second != nullptr;
    }

    template <typename Base>
    inline const i_tab_page& tab_page_container<Base>::tab_page(tab_index aTabIndex) const
    {
        if (aTabIndex >= iTabs.size())
            throw tab_not_found();
        auto existing = iTabs.find(&tab(aTabIndex));
        if (existing == iTabs.end())
            throw tab_not_found();
        auto const& tabPagePtr = existing->second;
        if (tabPagePtr == nullptr)
            throw tab_page_not_found();
        return *tabPagePtr;
    }

    template <typename Base>
    inline i_tab_page& tab_page_container<Base>::tab_page(tab_index aTabIndex)
    {
        return const_cast<i_tab_page&>(to_const(*this).tab_page(aTabIndex));
    }

    template <typename Base>
    inline bool tab_page_container<Base>::is_tab_selected() const noexcept
    {
        for (auto& tab : iTabs)
            if (tab.first->is_selected())
                return true;
        return false;
    }

    template <typename Base>
    inline const i_tab& tab_page_container<Base>::selected_tab() const
    {
        for (auto& tab : iTabs)
            if (tab.first->is_selected())
                return *tab.first;
        throw tab_not_found();
    }

    template <typename Base>
    inline i_tab& tab_page_container<Base>::selected_tab()
    {
        return const_cast<i_tab&>(to_const(*this).selected_tab());
    }

    template <typename Base>
    inline const i_tab_page& tab_page_container<Base>::selected_tab_page() const
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

    template <typename Base>
    inline i_tab_page& tab_page_container<Base>::selected_tab_page()
    {
        return const_cast<i_tab_page&>(to_const(*this).selected_tab_page());
    }

    template <typename Base>
    inline i_tab& tab_page_container<Base>::add_tab(i_string const& aTabText)
    {
        auto& newTab = iTabBar.add_tab(aTabText);
        iTabs.emplace(&newTab, tab_page_pointer{});
        return newTab;
    }

    template <typename Base>
    inline i_tab& tab_page_container<Base>::insert_tab(tab_index aTabIndex, i_string const& aTabText)
    {
        auto& newTab = iTabBar.insert_tab(aTabIndex, aTabText);
        iTabs.emplace(&newTab, tab_page_pointer{});
        return newTab;
    }

    template <typename Base>
    inline void tab_page_container<Base>::remove_tab(tab_index aTabIndex)
    {
        iTabBar.remove_tab(aTabIndex);
    }

    template <typename Base>
    inline void tab_page_container<Base>::show_tab(tab_index aTabIndex)
    {
        tab(aTabIndex).as_widget().show();
        if (has_tab_page(aTabIndex))
            tab_page(aTabIndex).as_widget().show();
    }

    template <typename Base>
    inline void tab_page_container<Base>::hide_tab(tab_index aTabIndex)
    {
        tab(aTabIndex).as_widget().hide();
        if (has_tab_page(aTabIndex))
            tab_page(aTabIndex).as_widget().hide();
    }

    template <typename Base>
    inline tab_page_container<Base>::optional_tab_index tab_page_container<Base>::next_visible_tab(tab_index aStartFrom) const
    {
        return iTabBar.next_visible_tab(aStartFrom);
    }

    template <typename Base>
    inline tab_page_container<Base>::optional_tab_index tab_page_container<Base>::previous_visible_tab(tab_index aStartFrom) const
    {
        return iTabBar.previous_visible_tab(aStartFrom);
    }

    template <typename Base>
    inline void tab_page_container<Base>::select_next_tab()
    {
        iTabBar.select_next_tab();
    }

    template <typename Base>
    inline void tab_page_container<Base>::select_previous_tab()
    {
        iTabBar.select_previous_tab();
    }

    template <typename Base>
    inline i_tab_page& tab_page_container<Base>::add_tab_page(i_string const& aTabText)
    {
        return add_tab_page(add_tab(aTabText));
    }

    template <typename Base>
    inline i_tab_page& tab_page_container<Base>::insert_tab_page(tab_index aTabIndex, i_string const& aTabText)
    {
        return add_tab_page(insert_tab(aTabIndex, aTabText));
    }

    template <typename Base>
    inline i_tab_page& tab_page_container<Base>::add_tab_page(i_tab& aTab)
    {
        return add_tab_page(aTab, tab_page_pointer{ new neogfx::tab_page{ page_layout(), aTab } });
    }

    template <typename Base>
    inline i_tab_page& tab_page_container<Base>::add_tab_page(i_tab& aTab, i_tab_page& aWidget)
    {
        return add_tab_page(aTab, tab_page_pointer{ tab_page_pointer{}, &aWidget });
    }

    template <typename Base>
    inline i_tab_page& tab_page_container<Base>::add_tab_page(i_tab& aTab, i_ref_ptr<i_tab_page> const& aWidget)
    {
        auto existingTab = iTabs.find(&aTab);
        if (existingTab == iTabs.end())
            throw tab_not_found();
        existingTab->second = aWidget;
        if (aTab.is_selected())
        {
            existingTab->second->as_widget().show();
            if ((as_widget().focus_policy() & neogfx::focus_policy::StrongFocus) != neogfx::focus_policy::NoFocus)
                existingTab->second->as_widget().set_focus();
            iContainerLayout.update_layout();
        }
        else
            existingTab->second->as_widget().hide();
        TabPageAdded.trigger(*aWidget);
        return *existingTab->second;
    }

    template <typename Base>
    inline void tab_page_container<Base>::remove_tab_page(i_tab_page& aPage)
    {
        auto existingTab = iTabs.find(&aPage.tab());
        if (existingTab != iTabs.end())
            remove_tab(index_of(aPage.tab()));
    }

    template <typename Base>
    inline void tab_page_container<Base>::adding_tab(i_tab& aTab)
    {
        iTabs.emplace(&aTab, tab_page_pointer{});
        if (iTabs.size() == 1)
            aTab.select();
    }

    template <typename Base>
    inline void tab_page_container<Base>::selecting_tab(i_tab& aTab)
    {
        for (auto& tab : iTabs)
            if (tab.second != nullptr)
            {
                if (tab.first == &aTab)
                {
                    tab.second->as_widget().show();
                    if ((as_widget().focus_policy() & neogfx::focus_policy::StrongFocus) != neogfx::focus_policy::NoFocus)
                        tab.second->as_widget().set_focus();
                }
                else
                    tab.second->as_widget().hide();
            }
        iContainerLayout.update_layout();
    }

    template <typename Base>
    inline void tab_page_container<Base>::removing_tab(i_tab& aTab)
    {
        auto existingTab = iTabs.find(&aTab);
        if (existingTab == iTabs.end())
            throw tab_not_found();
        auto temp = existingTab->second;
        iTabs.erase(existingTab);
        TabPageRemoved.trigger(*temp);
    }

    template <typename Base>
    inline bool tab_page_container<Base>::has_parent_container() const
    {
        return false;
    }

    template <typename Base>
    inline const i_tab_container& tab_page_container<Base>::parent_container() const
    {
        throw no_parent_container();
    }

    template <typename Base>
    inline i_tab_container& tab_page_container<Base>::parent_container()
    {
        throw no_parent_container();
    }

    template <typename Base>
    inline const i_widget& tab_page_container<Base>::as_widget() const
    {
        return *this;
    }

    template <typename Base>
    inline i_widget& tab_page_container<Base>::as_widget()
    {
        return *this;
    }

    template <typename Base>
    inline bool tab_page_container<Base>::is_managing_layout() const
    {
        return true;
    }

    template <typename Base>
    inline void tab_page_container<Base>::init()
    {
        base_type::set_padding(neogfx::padding{});
        iContainerLayout.set_padding(neogfx::padding{});
        if ((tab_container_style() & neogfx::tab_container_style::ResizeToPages) == neogfx::tab_container_style::ResizeToPages)
            page_layout().parent_layout().set_minimum_size({});
        update_tab_bar_placement();

        base_type::set_focus_policy(neogfx::focus_policy::StrongFocus | neogfx::focus_policy::KeepChildFocus);
    }

    template <typename Base>
    inline void tab_page_container<Base>::update_tab_bar_placement()
    {
        switch (tab_container_style() & neogfx::tab_container_style::TabAlignmentMask)
        {
        case neogfx::tab_container_style::TabAlignmentTop:
            iTabBar.parent_layout().set_size_policy({});
            iTabBar.parent_layout().remove(iTabBar);
            iContainerLayout.top().add(iTabBar);
            iTabBar.parent_layout().set_size_policy(size_constraint::MinimumExpanding, size_constraint::Minimum);
            break;
        case neogfx::tab_container_style::TabAlignmentBottom:
            iTabBar.parent_layout().set_size_policy({});
            iTabBar.parent_layout().remove(iTabBar);
            iContainerLayout.bottom().add(iTabBar);
            iTabBar.parent_layout().set_size_policy(size_constraint::MinimumExpanding, size_constraint::Minimum);
            break;
        case neogfx::tab_container_style::TabAlignmentLeft:
            iTabBar.parent_layout().set_size_policy({});
            iTabBar.parent_layout().remove(iTabBar);
            iContainerLayout.left().add(iTabBar);
            iTabBar.parent_layout().set_size_policy(size_constraint::Minimum, size_constraint::MinimumExpanding);
            break;
        case neogfx::tab_container_style::TabAlignmentRight:
            iTabBar.parent_layout().set_size_policy({});
            iTabBar.parent_layout().remove(iTabBar);
            iContainerLayout.right().add(iTabBar);
            iTabBar.parent_layout().set_size_policy(size_constraint::Minimum, size_constraint::MinimumExpanding);
            break;
        }
    }
}