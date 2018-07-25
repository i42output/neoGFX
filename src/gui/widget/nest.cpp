// nest.cpp
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
#include <neogfx/gui/widget/nest.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
	nest::nest(i_widget& aWidget) :
		iWidget{ aWidget }
	{
	}

	std::size_t nest::nested_window_count() const
	{
		return iNestedWindows.size();
	}

	const i_nested_window& nest::nested_window(std::size_t aIndex) const
	{
		return *iNestedWindows[aIndex];
	}

	i_nested_window& nest::nested_window(std::size_t aIndex)
	{
		return *iNestedWindows[aIndex];
	}

	void nest::add(i_nested_window& aWindow)
	{
		iNestedWindows.push_back(&aWindow);
		as_widget().update();
	}

	void nest::remove(i_nested_window& aWindow)
	{
		iNestedWindows.erase(std::find(iNestedWindows.begin(), iNestedWindows.end(), &aWindow));
		as_widget().update();
	}

	const i_widget& nest::as_widget() const
	{
		return iWidget;
	}

	i_widget& nest::as_widget() 
	{
		return iWidget;
	}
}