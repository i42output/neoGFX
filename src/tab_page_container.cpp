// tab_page_container.cpp
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
#include "tab_page_container.hpp"

namespace neogfx
{
	tab_page_container::default_tab_page::default_tab_page(i_widget& aParent, i_tab& aTab) : 
		scrollable_widget(aParent, framed_widget::ContainerFrame), iTab(aTab)
	{
	}

	tab_page_container::default_tab_page::default_tab_page(i_layout& aLayout, i_tab& aTab) :
		scrollable_widget(aLayout, framed_widget::ContainerFrame), iTab(aTab)
	{
	}

	size tab_page_container::default_tab_page::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return scrollable_widget::minimum_size(aAvailableSpace);
		else
			return size{};
	}

	bool tab_page_container::default_tab_page::transparent_background() const
	{
		return false;
	}

	colour tab_page_container::default_tab_page::background_colour() const
	{
		if (has_background_colour())
			return scrollable_widget::background_colour();
		else
			return container_background_colour();
	}

	const i_tab& tab_page_container::default_tab_page::tab() const
	{
		return iTab;
	}

	i_tab& tab_page_container::default_tab_page::tab()
	{
		return iTab;
	}

	const i_widget& tab_page_container::default_tab_page::widget() const
	{
		return *this;
	}

	i_widget& tab_page_container::default_tab_page::widget()
	{
		return *this;
	}

	tab_page_container::tab_page_container() : 
		widget(), iContainerLayout(*this), iTabBar(iContainerLayout, *this), iPageLayout(iContainerLayout)
	{
		set_margins(neogfx::margins{});
		iContainerLayout.set_margins(neogfx::margins{});
		iContainerLayout.set_spacing(size{});
		iPageLayout.set_margins(neogfx::margins{});
	}

	tab_page_container::tab_page_container(i_widget& aParent) :
		widget(aParent), iContainerLayout(*this), iTabBar(iContainerLayout, *this), iPageLayout(iContainerLayout)
	{
		set_margins(neogfx::margins{});
		iContainerLayout.set_margins(neogfx::margins{});
		iContainerLayout.set_spacing(size{});
		iPageLayout.set_margins(neogfx::margins{});
	}

	tab_page_container::tab_page_container(i_layout& aLayout) :
		widget(aLayout), iContainerLayout(*this), iTabBar(iContainerLayout, *this), iPageLayout(iContainerLayout)
	{
		set_margins(neogfx::margins{});
		iContainerLayout.set_margins(neogfx::margins{});
		iContainerLayout.set_spacing(size{});
		iPageLayout.set_margins(neogfx::margins{});
	}

	stack_layout& tab_page_container::page_layout()
	{
		return iPageLayout;
	}

	bool tab_page_container::has_tabs() const
	{
		return !iTabs.empty();
	}

	uint32_t tab_page_container::tab_count() const
	{
		return iTabs.size();
	}

	const i_tab& tab_page_container::tab(tab_index aTabIndex) const
	{
		if (aTabIndex >= iTabs.size())
			throw tab_not_found();
		return *std::next(iTabs.begin(), aTabIndex)->first;
	}

	i_tab& tab_page_container::tab(tab_index aTabIndex)
	{
		return const_cast<i_tab&>(const_cast<const tab_page_container*>(this)->tab(aTabIndex));
	}

	const i_tab_page& tab_page_container::tab_page(tab_index aTabIndex) const
	{
		if (aTabIndex >= iTabs.size())
			throw tab_not_found();
		const auto& tabPagePtr = std::next(iTabs.begin(), aTabIndex)->second;
		if (tabPagePtr.get() == 0)
			throw tab_page_not_found();
		return *tabPagePtr;
	}

	i_tab_page& tab_page_container::tab_page(tab_index aTabIndex)
	{
		return const_cast<i_tab_page&>(const_cast<const tab_page_container*>(this)->tab_page(aTabIndex));
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
		return const_cast<i_tab&>(const_cast<const tab_page_container*>(this)->selected_tab());
	}

	const i_tab_page& tab_page_container::selected_tab_page() const
	{
		for (auto& tab : iTabs)
			if (tab.first->is_selected())
			{
				if (tab.second.get() == 0)
					throw tab_page_not_found();
				return *tab.second;
			}
		throw tab_not_found();
	}

	i_tab_page& tab_page_container::selected_tab_page()
	{
		return const_cast<i_tab_page&>(const_cast<const tab_page_container*>(this)->selected_tab_page());
	}

	i_tab& tab_page_container::add_tab(const std::string& aTabText)
	{
		return *iTabs.emplace(&iTabBar.add_tab(aTabText), tab_page_pointer()).first->first;
	}

	i_tab& tab_page_container::insert_tab(tab_index aTabIndex, const std::string& aTabText)
	{
		return *iTabs.emplace(&iTabBar.insert_tab(aTabIndex, aTabText), tab_page_pointer()).first->first;
	}

	void tab_page_container::remove_tab(tab_index aTabIndex)
	{
		iTabBar.remove_tab(aTabIndex);
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
		existingTab->second = tab_page_pointer(new default_tab_page(iPageLayout, aTab));
		if (aTab.is_selected())
		{
			existingTab->second->widget().show();
			layout_items();
		}
		else
			existingTab->second->widget().hide();
		return *existingTab->second;
	}

	i_tab_page& tab_page_container::add_tab_page(i_tab& aTab, i_tab_page& aWidget)
	{
		auto existingTab = iTabs.find(&aTab);
		if (existingTab == iTabs.end())
			throw tab_not_found();
		existingTab->second = tab_page_pointer(tab_page_pointer(), &aWidget);
		if (aTab.is_selected())
		{
			existingTab->second->widget().show();
			layout_items();
		}
		else
			existingTab->second->widget().hide();
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
			existingTab->second->widget().show();
			layout_items();
		}
		else
			existingTab->second->widget().hide();
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
			if (tab.second.get() != 0)
			{
				if (tab.first == &aTab)
					tab.second->widget().show();
				else
					tab.second->widget().hide();
			}
		layout_items();
	}

	void tab_page_container::removing_tab(i_tab& aTab)
	{
		auto existingTab = iTabs.find(&aTab);
		if (existingTab == iTabs.end())
			throw tab_not_found();
		iTabs.erase(existingTab);
	}

	bool tab_page_container::can_defer_layout() const
	{
		return true;
	}

	bool tab_page_container::is_managing_layout() const
	{
		return true;
	}
}