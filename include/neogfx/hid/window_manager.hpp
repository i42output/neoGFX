// window_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/window/i_window.hpp>

namespace neogfx
{
	class window_manager : public i_window_manager
	{
	private:
		struct window_sorter
		{
			bool operator()(i_window* left, i_window* right) const
			{
				if (left->is_owner_of(*right))
					return true;
				else if (right->is_owner_of(*left))
					return false;
				else
					return std::less<i_window*>{}(left, right);
			}
		};
		typedef std::vector<i_window*> window_list;
	public:
		window_manager();
		~window_manager();
	public:
		void add_window(i_window& aWindow) override;
		void remove_window(i_window& aWindow) override;
		bool has_window(i_window& aWindow) const override;
		std::size_t window_count() const override;
		i_window& window(std::size_t aIndex) override;
		bool any_strong_windows() const override;
	public:
		rect desktop_rect(const i_window& aWindow) const override;
		rect window_rect(const i_window& aWindow) const override;
		void move_window(i_window& aWindow, const point& aPosition) override;
		void resize_window(i_window& aWindow, const size& aExtents) override;
	private:
		window_list iWindows;
	};
}