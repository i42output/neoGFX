// i_tab_container.hpp
/*
neogfx C++ GUI Library
Copyright(C) 2015-present Leigh Johnston

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
#include <neogfx/core/event.hpp>
#include "i_tab.hpp"
#include "i_tab_page.hpp"

namespace neogfx
{
	class i_tab_container
	{
	public:
		typedef uint32_t tab_index;
		typedef boost::optional<tab_index> optional_tab_index;
	public:
		struct tab_not_found : std::logic_error { tab_not_found() : std::logic_error("neogfx::i_tab_container::tab_not_found") {} };
		struct no_parent_container : std::logic_error { no_parent_container() : std::logic_error("neogfx::i_tab_container::no_parent_container") {} };
		struct no_tab_page : std::logic_error { no_tab_page() : std::logic_error("neogfx::i_tab_container::no_tab_page") {} };
	public:
		virtual bool has_tabs() const = 0;
		virtual uint32_t tab_count() const = 0;
		virtual tab_index index_of(const i_tab& aTab) const = 0;
		virtual const i_tab& tab(tab_index aTabIndex) const = 0;
		virtual i_tab& tab(tab_index aTabIndex) = 0;
		virtual bool is_tab_selected() const = 0;
		virtual const i_tab& selected_tab() const = 0;
		virtual i_tab& selected_tab() = 0;
		virtual i_tab& add_tab(const std::string& aTabText) = 0;
		virtual i_tab& insert_tab(tab_index aTabIndex, const std::string& aTabText) = 0;
		virtual void remove_tab(tab_index aTabIndex) = 0;
		virtual void show_tab(tab_index aTabIndex) = 0;
		virtual void hide_tab(tab_index aTabIndex) = 0;
		virtual optional_tab_index next_visible_tab(tab_index aStartFrom) const = 0;
		virtual optional_tab_index previous_visible_tab(tab_index aStartFrom) const = 0;
		virtual void select_next_tab() = 0;
		virtual void select_previous_tab() = 0;
	public:
		virtual void adding_tab(i_tab& aTab) = 0;
		virtual void selecting_tab(i_tab& aTab) = 0;
		virtual void removing_tab(i_tab& aTab) = 0;
	public:
		virtual bool has_tab_page(tab_index aTabIndex) const = 0;
		virtual const i_tab_page& tab_page(tab_index aTabIndex) const = 0;
		virtual i_tab_page& tab_page(tab_index aTabIndex) = 0;
	public:
		virtual bool has_parent_container() const = 0;
		virtual const i_tab_container& parent_container() const = 0;
		virtual i_tab_container& parent_container() = 0;
		virtual const i_widget& as_widget() const = 0;
		virtual i_widget& as_widget() = 0;
	};
}