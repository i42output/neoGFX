// view_container.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/view/view_container.hpp>
#include <neogfx/gui/view/i_controller.hpp>

namespace neogfx
{
	view_stack::view_stack(i_layout& aLayout, i_view_container& aParent) :
		scrollable_widget{ aLayout, scrollbar_style::Normal, frame_style::NoFrame }, iLayout{ *this }, iParent{ aParent }
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{});
	}

	view_container::view_container(i_widget& aParent, view_container_style aStyle) :
		widget{ aParent }, iStyle(aStyle), iLayout0{ *this }, iTabContainer{ *this }, iLayout1{ iLayout0 }, iViewStack{ iLayout1, *this }
	{
		set_margins(neogfx::margins{});
		iLayout0.set_margins(neogfx::margins{});
	}

	view_container::view_container(i_layout& aLayout, view_container_style aStyle) :
		widget{ aLayout }, iStyle(aStyle), iLayout0{ *this }, iTabContainer{ *this }, iLayout1{ iLayout0 }, iViewStack{ iLayout1, *this }
	{
		set_margins(neogfx::margins{});
		iLayout0.set_margins(neogfx::margins{});
	}

	const i_widget& view_container::as_widget() const
	{
		return *this;
	}

	i_widget& view_container::as_widget()
	{
		return *this;
	}

	const view_stack& view_container::view_stack() const
	{
		return iViewStack;
	}

	view_stack& view_container::view_stack()
	{
		return iViewStack;
	}

	view_container_style view_container::style() const
	{
		return iStyle;
	}

	void view_container::change_style(view_container_style aNewStyle)
	{
		if (iStyle != aNewStyle)
		{
			iStyle = aNewStyle;
			/* todo: update everything */
		}
	}

	void view_container::add_controller(i_controller& aController)
	{
		iControllers.push_back(std::shared_ptr<i_controller>(std::shared_ptr<i_controller>(), &aController));
		aController.view_added([this](i_view& aView) { view_added.trigger(aView); });
		aController.view_removed([this](i_view& aView) { view_removed.trigger(aView); });
	}

	void view_container::add_controller(std::shared_ptr<i_controller> aController)
	{
		iControllers.push_back(aController);
		aController->view_added([this](i_view& aView) { view_added.trigger(aView); });
		aController->view_removed([this](i_view& aView) { view_removed.trigger(aView); });
	}

	void view_container::remove_controller(i_controller& aController)
	{
		for (auto c = iControllers.begin(); c != iControllers.end(); ++c)
		{
			if (&**c == &aController)
			{
				iControllers.erase(c);
				return;
			}
		}
		throw controller_not_found();
	}

	bool view_container::can_defer_layout() const
	{
		return true;
	}

	bool view_container::is_managing_layout() const
	{
		return true;
	}

	view_container::tab_container::tab_container(view_container& aOwner) :
		iOwner{ aOwner }, iTabBar{ aOwner.iLayout0, *this }
	{
		if (aOwner.style() != view_container_style::Tabbed)
			iTabBar.hide();
	}

	tab_container_style view_container::tab_container::style() const
	{
		return iTabBar.style();
	}

	void view_container::tab_container::set_style(tab_container_style aStyle)
	{
		if (style() != aStyle)
		{
			iTabBar.set_style(aStyle);
			style_changed.trigger();
		}
	}

	bool view_container::tab_container::has_tabs() const
	{
		return !iTabs.empty();
	}

	uint32_t view_container::tab_container::tab_count() const
	{
		return iTabs.size();
	}

	view_container::tab_container::tab_index view_container::tab_container::index_of(const i_tab& aTab) const
	{
		return iTabBar.index_of(aTab);
	}

	const i_tab& view_container::tab_container::tab(tab_index aTabIndex) const
	{
		if (aTabIndex >= iTabs.size())
			throw tab_not_found();
		return *std::next(iTabs.begin(), aTabIndex)->first;
	}

	i_tab& view_container::tab_container::tab(tab_index aTabIndex)
	{
		return const_cast<i_tab&>(const_cast<const tab_container*>(this)->tab(aTabIndex));
	}

	bool view_container::tab_container::is_tab_selected() const
	{
		for (auto& tab : iTabs)
			if (tab.first->is_selected())
				return true;
		return false;
	}

	const i_tab& view_container::tab_container::selected_tab() const
	{
		for (auto& tab : iTabs)
			if (tab.first->is_selected())
				return *tab.first;
		throw tab_not_found();
	}

	i_tab& view_container::tab_container::selected_tab()
	{
		return const_cast<i_tab&>(const_cast<const tab_container*>(this)->selected_tab());
	}

	i_tab& view_container::tab_container::add_tab(const std::string& aTabText)
	{
		return *iTabs.emplace(&iTabBar.add_tab(aTabText), nullptr).first->first;
	}

	i_tab& view_container::tab_container::insert_tab(tab_index aTabIndex, const std::string& aTabText)
	{
		return *iTabs.emplace(&iTabBar.insert_tab(aTabIndex, aTabText), nullptr).first->first;
	}

	void view_container::tab_container::remove_tab(tab_index aTabIndex)
	{
		iTabBar.remove_tab(aTabIndex);
	}

	void view_container::tab_container::show_tab(tab_index aTabIndex)
	{
		tab(aTabIndex).as_widget().show();
		if (has_tab_page(aTabIndex))
			tab_page(aTabIndex).as_widget().show();
	}

	void view_container::tab_container::hide_tab(tab_index aTabIndex)
	{
		tab(aTabIndex).as_widget().hide();
		if (has_tab_page(aTabIndex))
			tab_page(aTabIndex).as_widget().hide();
	}

	view_container::tab_container::optional_tab_index view_container::tab_container::next_visible_tab(tab_index aStartFrom) const
	{
		return iTabBar.next_visible_tab(aStartFrom);
	}

	view_container::tab_container::optional_tab_index view_container::tab_container::previous_visible_tab(tab_index aStartFrom) const
	{
		return iTabBar.previous_visible_tab(aStartFrom);
	}

	void view_container::tab_container::select_next_tab()
	{
		iTabBar.select_next_tab();
	}

	void view_container::tab_container::select_previous_tab()
	{
		iTabBar.select_previous_tab();
	}

	void view_container::tab_container::adding_tab(i_tab& aTab)
	{
		iTabs.emplace(&aTab, nullptr);
		if (iTabs.size() == 1)
			aTab.select();
	}

	void view_container::tab_container::selecting_tab(i_tab& aTab)
	{
		for (auto& tab : iTabs)
			if (tab.second != nullptr)
			{
				if (tab.first == &aTab)
					tab.second->as_widget().show();
				else
					tab.second->as_widget().hide();
			}
		iOwner.layout_items();
	}

	void view_container::tab_container::removing_tab(i_tab& aTab)
	{
		auto existingTab = iTabs.find(&aTab);
		if (existingTab == iTabs.end())
			throw tab_not_found();
		iTabs.erase(existingTab);
		iOwner.layout_items();
	}

	bool view_container::tab_container::has_tab_page(tab_index) const
	{
		return false;
	}

	const i_tab_page& view_container::tab_container::tab_page(tab_index) const
	{
		throw no_tab_page();
	}

	i_tab_page& view_container::tab_container::tab_page(tab_index)
	{
		throw no_tab_page();
	}

	bool view_container::tab_container::has_parent_container() const
	{
		return false;
	}

	const i_tab_container& view_container::tab_container::parent_container() const
	{
		throw no_parent_container();
	}

	i_tab_container& view_container::tab_container::parent_container()
	{
		throw no_parent_container();
	}

	const i_widget& view_container::tab_container::as_widget() const
	{
		return iOwner;
	}

	i_widget& view_container::tab_container::as_widget() 
	{
		return iOwner;
	}
}