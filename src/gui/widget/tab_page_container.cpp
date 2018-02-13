// tab_page_container.cpp
/*
neogfx C++ GUI Library
Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.

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

namespace neogfx
{
	tab_page_container::default_tab_page::default_tab_page(i_widget& aParent, i_tab& aTab) :
		scrollable_widget{ aParent, scrollbar_style::Normal, frame_style::ContainerFrame }, iTab{ aTab }
	{
	}

	tab_page_container::default_tab_page::default_tab_page(i_layout& aLayout, i_tab& aTab) :
		scrollable_widget{ aLayout, scrollbar_style::Normal, frame_style::ContainerFrame }, iTab{ aTab }
	{
	}

	neogfx::size_policy tab_page_container::default_tab_page::size_policy() const
	{
		if (has_size_policy())
			return scrollable_widget::size_policy();
		else
			return neogfx::size_policy::Expanding;
	}

	size tab_page_container::default_tab_page::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return scrollable_widget::minimum_size(aAvailableSpace);
		else
			return size{};
	}

	size tab_page_container::default_tab_page::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size() || size_policy() != neogfx::size_policy::Expanding)
			return scrollable_widget::maximum_size(aAvailableSpace);
		else
			return size::max_size();
	}

	bool tab_page_container::default_tab_page::transparent_background() const
	{
		return false;
	}

	void tab_page_container::default_tab_page::paint_non_client(graphics_context& aGraphicsContext) const
	{
		scrollable_widget::paint_non_client(aGraphicsContext);
		if (iTab.is_selected())
		{
			auto hole = to_client_coordinates(iTab.as_widget().window_rect().inflate(size{ -effective_frame_width() / 2.0, effective_frame_width() }).intersection(window_rect()));
			hole = hole.intersection(to_client_coordinates(window_rect().deflate(size{ effective_frame_width() / 2.0, 0.0 })));
			aGraphicsContext.fill_rect(hole, inner_frame_colour());
			hole.deflate(size{ effective_frame_width() / 2.0, 0.0 });
			aGraphicsContext.fill_rect(hole, background_colour());
		}
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

	const i_widget& tab_page_container::default_tab_page::as_widget() const
	{
		return *this;
	}

	i_widget& tab_page_container::default_tab_page::as_widget()
	{
		return *this;
	}

	tab_page_container::tab_page_container(bool aClosableTabs) :
		widget(), iContainerLayout(*this), iTabBar(iContainerLayout, *this, aClosableTabs), iPageLayout(iContainerLayout)
	{
		set_margins(neogfx::margins{});
		iContainerLayout.set_margins(neogfx::margins{});
		iContainerLayout.set_spacing(size{});
		iPageLayout.set_margins(neogfx::margins{});
	}

	tab_page_container::tab_page_container(i_widget& aParent, bool aClosableTabs) :
		widget(aParent), iContainerLayout(*this), iTabBar(iContainerLayout, *this, aClosableTabs), iPageLayout(iContainerLayout)
	{
		set_margins(neogfx::margins{});
		iContainerLayout.set_margins(neogfx::margins{});
		iContainerLayout.set_spacing(size{});
		iPageLayout.set_margins(neogfx::margins{});
	}

	tab_page_container::tab_page_container(i_layout& aLayout, bool aClosableTabs) :
		widget(aLayout), iContainerLayout(*this), iTabBar(iContainerLayout, *this, aClosableTabs), iPageLayout(iContainerLayout)
	{
		set_margins(neogfx::margins{});
		iContainerLayout.set_margins(neogfx::margins{});
		iContainerLayout.set_spacing(size{});
		iPageLayout.set_margins(neogfx::margins{});
	}

	tab_page_container::~tab_page_container()
	{
		while (tab_count() > 0)
			remove_tab(tab_count() - 1);
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
		return const_cast<i_tab&>(const_cast<const tab_page_container*>(this)->tab(aTabIndex));
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
		return const_cast<i_tab_page&>(const_cast<const tab_page_container*>(this)->tab_page(aTabIndex));
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
		return const_cast<i_tab&>(const_cast<const tab_page_container*>(this)->selected_tab());
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
		return const_cast<i_tab_page&>(const_cast<const tab_page_container*>(this)->selected_tab_page());
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
		existingTab->second = tab_page_pointer(new default_tab_page(iPageLayout, aTab));
		if (aTab.is_selected())
		{
			existingTab->second->as_widget().show();
			layout_items();
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
		existingTab->second = tab_page_pointer(tab_page_pointer(), &aWidget);
		if (aTab.is_selected())
		{
			existingTab->second->as_widget().show();
			layout_items();
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
			layout_items();
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
		layout_items();
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



	const i_widget& tab_page_container::as_widget() const
	{
		return *this;
	}

	i_widget& tab_page_container::as_widget()
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
}