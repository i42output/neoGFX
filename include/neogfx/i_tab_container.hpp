// i_tab_container.hpp
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
#include "event.hpp"
#include "i_tab.hpp"
#include "i_tab_page.hpp"

namespace neogfx
{
	class i_tab_container
	{
	public:
		typedef uint32_t tab_index;
	public:
		struct tab_not_found : std::logic_error { tab_not_found() : std::logic_error("neogfx::i_tab_container::tab_not_found") {} };
	public:
		virtual bool has_tabs() const = 0;
		virtual uint32_t tab_count() const = 0;
		virtual const i_tab& tab(tab_index aTabIndex) const = 0;
		virtual i_tab& tab(tab_index aTabIndex) = 0;
		virtual const i_tab& selected_tab() const = 0;
		virtual i_tab& selected_tab() = 0;
		virtual i_tab& add_tab(const std::string& aTabText) = 0;
		virtual i_tab& insert_tab(tab_index aTabIndex, const std::string& aTabText) = 0;
		virtual void remove_tab(tab_index aTabIndex) = 0;
	public:
		virtual void adding_tab(i_tab& aTab) = 0;
		virtual void selecting_tab(i_tab& aTab) = 0;
		virtual void removing_tab(i_tab& aTab) = 0;
	};
}