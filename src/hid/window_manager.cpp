// window_manager.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/app/app.hpp>
#include <neogfx/hid/window_manager.hpp>

namespace neogfx
{
	window_manager::window_manager()
	{
	}

	window_manager::~window_manager()
	{
	}

	void window_manager::add_window(i_window& aWindow)
	{
		iWindows.insert(&aWindow);
	}

	void window_manager::remove_window(i_window& aWindow)
	{
		auto existing = iWindows.find(&aWindow);
		if (existing == iWindows.end())
			throw window_not_found();
		iWindows.erase(existing);
	}

	std::size_t window_manager::window_count() const
	{
		return iWindows.size();
	}

	i_window& window_manager::window(std::size_t aIndex)
	{
		if (aIndex >= iWindows.size())
			throw window_not_found();
		return **std::next(iWindows.begin(), aIndex);
	}

	bool window_manager::any_strong_windows() const
	{
		for (auto const& w : iWindows)
			if (w->is_strong())
				return true;
		return false;
	}

	rect window_manager::desktop_rect(const i_window& aWindow) const
	{
		if ((aWindow.style() & window_style::Nested) != window_style::Nested || !aWindow.has_parent_window())
			return app::instance().surface_manager().desktop_rect(aWindow.surface());
		else
			return rect{ point{}, window_rect(aWindow.nested_container().as_widget().root()).extents() };
	}

	rect window_manager::window_rect(const i_window& aWindow) const
	{
		if ((aWindow.style() & window_style::Nested) != window_style::Nested || !aWindow.has_parent_window())
			return rect{ aWindow.surface().surface_position(), aWindow.surface().surface_size() };
		else
			return rect{ aWindow.as_widget().position(), aWindow.as_widget().extents() };
	}

	void window_manager::move_window(i_window& aWindow, const point& aPosition)
	{
		if ((aWindow.style() & window_style::Nested) != window_style::Nested || !aWindow.has_parent_window())
			aWindow.surface().move_surface(aPosition);
		else
			aWindow.as_widget().move(aPosition);
	}

	void window_manager::resize_window(i_window& aWindow, const size& aExtents)
	{
		if ((aWindow.style() & window_style::Nested) != window_style::Nested || !aWindow.has_parent_window())
			aWindow.surface().resize_surface(aExtents);
		else
			aWindow.as_widget().resize(aExtents);
	}
}