// tab_bar.cpp
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
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/tab_bar.hpp>
#include <neogfx/gui/widget/tab_button.hpp>

namespace neogfx
{
	tab_bar::tab_bar(i_tab_container& aContainer, bool aClosableTabs, tab_container_style aStyle) :
		scrollable_widget{ scrollbar_style::Scroller, frame_style::NoFrame }, iContainer{ aContainer }, iClosableTabs{ aClosableTabs }, iStyle{ aStyle }
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<horizontal_layout>(*this, alignment::Bottom));
		layout().set_margins(neogfx::margins{});
		layout().set_spacing(size{});
		update_placement();
	}

	tab_bar::tab_bar(i_widget& aParent, i_tab_container& aContainer, bool aClosableTabs, tab_container_style aStyle) :
		scrollable_widget{ aParent, scrollbar_style::Scroller, frame_style::NoFrame }, iContainer{ aContainer }, iClosableTabs{ aClosableTabs }, iStyle{ aStyle }
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<horizontal_layout>(*this, alignment::Bottom));
		layout().set_margins(neogfx::margins{});
		layout().set_spacing(size{});
		update_placement();
	}

	tab_bar::tab_bar(i_layout& aLayout, i_tab_container& aContainer, bool aClosableTabs, tab_container_style aStyle) :
		scrollable_widget{ aLayout, scrollbar_style::Scroller, frame_style::NoFrame }, iContainer{ aContainer }, iClosableTabs{ aClosableTabs }, iStyle{ aStyle }
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<horizontal_layout>(*this, alignment::Bottom));
		layout().set_margins(neogfx::margins{});
		layout().set_spacing(size{});
		update_placement();
	}

	tab_container_style tab_bar::style() const
	{
		return iStyle;
	}
	
	void tab_bar::set_style(tab_container_style aStyle)
	{
		if (iStyle != aStyle)
		{
			iStyle = aStyle;
			update_placement();
			style_changed.trigger();
		}
	}

	size tab_bar::minimum_size(const optional_size& aAvailableSpace) const
	{
		auto result = scrollable_widget::minimum_size(aAvailableSpace);
		result.cx = 0.0;
		return result;
	}

	bool tab_bar::transparent_background() const
	{
		return true;
	}

	bool tab_bar::has_tabs() const
	{
		return !iTabs.empty();
	}

	tab_bar::tab_list::size_type tab_bar::tab_count() const
	{
		return iTabs.size();
	}

	tab_bar::tab_index tab_bar::index_of(const i_tab& aTab) const
	{
		for (auto i = iTabs.begin(); i != iTabs.end(); ++i)
			if (&**i == &aTab)
				return i - iTabs.begin();
		throw tab_not_found();
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

	bool tab_bar::is_tab_selected() const
	{
		for (auto& tab : iTabs)
			if (tab->is_selected())
				return true;
		return false;
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
		iTabs.push_back(std::make_unique<tab_button>(layout(), *this, aTabText, iClosableTabs));
		return *iTabs.back();
	}

	i_tab& tab_bar::insert_tab(tab_index aTabIndex, const std::string& aTabText)
	{
		iTabs.insert(iTabs.begin() + aTabIndex, std::make_unique<tab_button>(layout(), *this, aTabText, iClosableTabs));
		return *iTabs.back();
	}

	void tab_bar::remove_tab(tab_index aTabIndex)
	{
		if (aTabIndex >= iTabs.size())
			throw tab_not_found();
		bool wasSelected = tab(aTabIndex).is_selected();
		{
			auto keep = std::move(iTabs[aTabIndex]);
			iTabs.erase(iTabs.begin() + aTabIndex);
		}
		if (wasSelected)
		{
			auto nextVisible = (aTabIndex < tab_count() ? next_visible_tab(aTabIndex) : previous_visible_tab(aTabIndex));
			if (nextVisible)
				tab(*nextVisible).select();
		}
	}

	void tab_bar::show_tab(tab_index aTabIndex)
	{
		tab(aTabIndex).as_widget().show();
		if (has_tab_page(aTabIndex))
			tab_page(aTabIndex).as_widget().show();
	}

	void tab_bar::hide_tab(tab_index aTabIndex)
	{
		tab(aTabIndex).as_widget().hide();
		if (has_tab_page(aTabIndex))
			tab_page(aTabIndex).as_widget().hide();
	}

	tab_bar::optional_tab_index tab_bar::next_visible_tab(tab_index aStartFrom) const
	{
		if (tab_count() == 0)
			return optional_tab_index{};
		if (aStartFrom > tab_count() - 1)
			aStartFrom = 0;
		auto next = aStartFrom;
		while (tab(next).as_widget().hidden())
			if ((next = (next + 1) % tab_count()) == aStartFrom)
				break;
		if (tab(next).as_widget().visible())
			return next;
		return optional_tab_index{};
	}

	tab_bar::optional_tab_index tab_bar::previous_visible_tab(tab_index aStartFrom) const
	{
		if (tab_count() == 0)
			return optional_tab_index{};
		if (aStartFrom > tab_count() - 1)
			aStartFrom = tab_count() - 1;
		auto previous = aStartFrom;
		while (tab(previous).as_widget().hidden())
			if ((previous = (previous > 0 ? previous - 1 : tab_count() - 1)) == aStartFrom)
				break;
		if (tab(previous).as_widget().visible())
			return previous;
		return optional_tab_index{};
	}

	void tab_bar::select_next_tab()
	{
		auto next = next_visible_tab(is_tab_selected() ? index_of(selected_tab()) + 1 : 0);
		if (next)
			tab(*next).select();
	}

	void tab_bar::select_previous_tab()
	{
		auto previous = previous_visible_tab(is_tab_selected() ? index_of(selected_tab()) - 1 : 0);
		if (previous)
			tab(*previous).select();
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

	bool tab_bar::has_tab_page(tab_index aTabIndex) const
	{
		return iContainer.has_tab_page(aTabIndex);
	}

	const i_tab_page& tab_bar::tab_page(tab_index aTabIndex) const
	{
		return iContainer.tab_page(aTabIndex);
	}

	i_tab_page& tab_bar::tab_page(tab_index aTabIndex)
	{
		return iContainer.tab_page(aTabIndex);
	}

	bool tab_bar::has_parent_container() const
	{
		return true;
	}
	
	const i_tab_container& tab_bar::parent_container() const
	{
		return iContainer;
	}

	i_tab_container& tab_bar::parent_container()
	{
		return iContainer;
	}

	const i_widget& tab_bar::as_widget() const
	{
		return *this;
	}

	i_widget& tab_bar::as_widget()
	{
		return *this;
	}

	bool tab_bar::visible() const
	{
		if (iTabs.empty())
			return false;
		else
			return widget::visible();
	}

	void tab_bar::update_placement()
	{
		switch (style() & tab_container_style::TabAlignmentMask)
		{
		case tab_container_style::TabAlignmentTop:
		case tab_container_style::TabAlignmentLeft: // todo
		case tab_container_style::TabAlignmentRight: // todo
			layout().set_alignment(alignment::Bottom);
			break;
		case tab_container_style::TabAlignmentBottom:
			layout().set_alignment(alignment::Top);
			break;
		}
	}
}