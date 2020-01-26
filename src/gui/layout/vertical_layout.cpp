// vertical_layout.cpp
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
#include <unordered_map>
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/bresenham_counter.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>

namespace neogfx
{
    vertical_layout::vertical_layout(neogfx::alignment aAlignment) :
        layout(aAlignment)
    {
    }

    vertical_layout::vertical_layout(i_widget& aParent, neogfx::alignment aAlignment) :
        layout(aParent, aAlignment)
    {
    }

    vertical_layout::vertical_layout(i_layout& aParent, neogfx::alignment aAlignment) :
        layout(aParent, aAlignment)
    {
    }

    vertical_layout::vertical_layout(vertical_layout& aParent) :
        vertical_layout(static_cast<i_layout&>(aParent))
    {
    }

    i_spacer& vertical_layout::add_spacer()
    {
        auto s = std::make_shared<vertical_spacer>();
        add(s);
        return *s;
    }

    i_spacer& vertical_layout::add_spacer_at(item_index aPosition)
    {
        auto s = std::make_shared<vertical_spacer>();
        add_at(aPosition, s);
        return *s;
    }

    size vertical_layout::minimum_size(const optional_size& aAvailableSpace) const
    {
        return layout::do_minimum_size<layout::row_major<vertical_layout>>(aAvailableSpace);
    }

    size vertical_layout::maximum_size(const optional_size& aAvailableSpace) const
    {
        return layout::do_maximum_size<layout::row_major<vertical_layout>>(aAvailableSpace);
    }

    void vertical_layout::layout_items(const point& aPosition, const size& aSize)
    {
        if (!enabled())
            return;
        if (debug == this)
            std::cerr << "vertical_layout::layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
        if (has_layout_owner())
            layout_owner().layout_items_started();
        scoped_layout_items layoutItems;
        validate();
        layout::do_layout_items<layout::row_major<vertical_layout>>(aPosition, aSize);
        if (has_layout_owner())
            layout_owner().layout_items_completed();
        LayoutCompleted.trigger();
    }
}