// menu_bar.cpp
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

#include "neogfx.hpp"
#include "menu_bar.hpp"
#include "flow_layout.hpp"
#include "app.hpp"

namespace neogfx
{
	menu_bar::menu_bar()
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<flow_layout>(*this));
	}

	menu_bar::menu_bar(i_widget& aParent) : widget(aParent)
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<flow_layout>(*this));
	}

	menu_bar::menu_bar(i_layout& aLayout) : widget(aLayout)
	{
		set_margins(neogfx::margins{});
		set_layout(std::make_shared<flow_layout>(*this));
	}

	size_policy menu_bar::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Minimum;
	}

	bool menu_bar::visible() const
	{
		if (app::instance().basic_services().has_shared_menu_bar())
			return false;
		return widget::visible();
	}

	const i_widget& menu_bar::as_widget() const
	{
		return *this;
	}

	i_widget& menu_bar::as_widget()
	{
		return *this;
	}
}