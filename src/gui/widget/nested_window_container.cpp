// nested_window_container.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/nested_window_container.hpp>

namespace neogfx
{
	nested_window_container::nested_window_container(i_widget& aWidget) :
		iWidget{ aWidget }
	{
	}

	void nested_window_container::add(i_nested_window& aWindow)
	{
		// todo
	}

	void nested_window_container::remove(i_nested_window& aWindow)
	{
		// todo
	}

	const i_widget& nested_window_container::as_widget() const
	{
		return iWidget;
	}

	i_widget& nested_window_container::as_widget() 
	{
		return iWidget;
	}
}