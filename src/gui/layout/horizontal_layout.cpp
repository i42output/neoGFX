// horizontal_layout.cpp
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
#include <unordered_map>
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/bresenham_counter.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>

namespace neogfx
{
	horizontal_layout::horizontal_layout(neogfx::alignment aAlignment) :
		layout(aAlignment)
	{
	}

	horizontal_layout::horizontal_layout(i_widget& aParent, neogfx::alignment aAlignment) :
		layout(aParent, aAlignment)
	{
	}

	horizontal_layout::horizontal_layout(i_layout& aParent, neogfx::alignment aAlignment) :
		layout(aParent, aAlignment)
	{
	}

	horizontal_layout::horizontal_layout(horizontal_layout& aParent) :
		horizontal_layout(static_cast<i_layout&>(aParent))
	{
	}

	i_spacer& horizontal_layout::add_spacer()
	{
		auto s = std::make_shared<horizontal_spacer>();
		add(s);
		return *s;
	}

	i_spacer& horizontal_layout::add_spacer_at(item_index aPosition)
	{
		auto s = std::make_shared<horizontal_spacer>();
		add_at(aPosition, s);
		return *s;
	}

	size horizontal_layout::minimum_size(const optional_size& aAvailableSpace) const
	{
		return layout::do_minimum_size<layout::column_major<horizontal_layout>>(aAvailableSpace);
	}

	size horizontal_layout::maximum_size(const optional_size& aAvailableSpace) const
	{
		return layout::do_maximum_size<layout::column_major<horizontal_layout>>(aAvailableSpace);
	}

	void horizontal_layout::layout_items(const point& aPosition, const size& aSize)
	{
		if (!enabled())
			return;
		if (debug == this)
			std::cerr << "horizontal_layout::layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
		owner()->layout_items_started();
		next_layout_id();
		validate();
		layout::do_layout_items<layout::column_major<horizontal_layout>>(aPosition, aSize);
		owner()->layout_items_completed();
	}
}