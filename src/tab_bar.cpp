// tab_bar.cpp
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
#include "horizontal_layout.hpp"
#include "tab_bar.hpp"
#include "tab_button.hpp"

namespace neogfx
{
	tab_bar::tab_bar(i_tab_page_container& aContainer) :
		iContainer(aContainer)
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<horizontal_layout>(*this));
		layout().set_margins(neogfx::margins{});
		layout().set_spacing(size{});
	}

	tab_bar::tab_bar(i_widget& aParent, i_tab_page_container& aContainer) :
		widget(aParent), iContainer(aContainer)
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<horizontal_layout>(*this));
		layout().set_margins(neogfx::margins{});
		layout().set_spacing(size{});
	}

	tab_bar::tab_bar(i_layout& aLayout, i_tab_page_container& aContainer) :
		widget(aLayout), iContainer(aContainer)
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<horizontal_layout>(*this));
		layout().set_margins(neogfx::margins{});
		layout().set_spacing(size{});
	}

	bool tab_bar::has_tabs() const
	{
		return !iTabs.empty();
	}

	tab_bar::tab_list::size_type tab_bar::tab_count() const
	{
		return iTabs.size();
	}

	const i_tab& tab_bar::tab(tab_index aTabIndex) const
	{
		if (aTabIndex >= iTabs.size())
			throw tab_not_found();
		return *iTabs[aTabIndex];
	}

	i_tab& tab_bar::tab(tab_index aTabIndex)
	{
		if (aTabIndex >= iTabs.size())
			throw tab_not_found();
		return *iTabs[aTabIndex];
	}

	const i_tab& tab_bar::selected_tab() const
	{
		for (auto& tab : iTabs)
			if (tab->is_selected())
				return *tab;
		throw tab_not_found();
	}

	i_tab& tab_bar::selected_tab()
	{
		return const_cast<i_tab&>(const_cast<const tab_bar*>(this)->selected_tab());
	}

	i_tab& tab_bar::add_tab(const std::string& aTabText)
	{
		iTabs.push_back(std::make_unique<tab_button>(layout(), *this, aTabText));
		return *iTabs.back();
	}

	i_tab& tab_bar::insert_tab(tab_index aTabIndex, const std::string& aTabText)
	{
		iTabs.insert(iTabs.begin() + aTabIndex, std::make_unique<tab_button>(layout(), *this, aTabText));
		return *iTabs.back();
	}

	void tab_bar::remove_tab(tab_index aTabIndex)
	{
		if (aTabIndex >= iTabs.size())
			throw tab_not_found();
		auto tab = std::move(iTabs[aTabIndex]);
		iTabs.erase(iTabs.begin() + aTabIndex);
	}

	void tab_bar::adding_tab(i_tab& aTab)
	{
		iContainer.adding_tab(aTab);
	}

	void tab_bar::selecting_tab(i_tab& aTab)
	{
		iContainer.selecting_tab(aTab);
		for (auto& tab : iTabs)
			if (tab.get() != &aTab)
				tab->set_selected_state(false);
	}

	void tab_bar::removing_tab(i_tab& aTab)
	{
		iContainer.removing_tab(aTab);
	}

	size tab_bar::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return widget::maximum_size(aAvailableSpace);
		return minimum_size(aAvailableSpace);
	}

	void tab_bar::paint(graphics_context&) const
	{
	}
}