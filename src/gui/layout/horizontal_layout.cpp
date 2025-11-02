// horizontal_layout.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/bresenham_counter.hpp>

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

    horizontal_layout::~horizontal_layout()
    {
    }

    layout_direction horizontal_layout::direction() const
    {
        return layout_direction::Horizontal;
    }

    i_spacer& horizontal_layout::add_spacer()
    {
        auto s = make_ref<horizontal_spacer>();
        add(s);
        return *s;
    }

    i_spacer& horizontal_layout::add_spacer_at(layout_item_index aPosition)
    {
        auto s = make_ref<horizontal_spacer>();
        add_at(aPosition, s);
        return *s;
    }

    size horizontal_layout::minimum_size(optional_size const& aAvailableSpace) const
    {
        return layout::do_minimum_size<layout::row_major<horizontal_layout>>(aAvailableSpace);
    }

    size horizontal_layout::maximum_size(optional_size const& aAvailableSpace) const
    {
        return layout::do_maximum_size<layout::row_major<horizontal_layout>>(aAvailableSpace);
    }

    void horizontal_layout::layout_items(const point& aPosition, const size& aSize)
    {
        if (!enabled())
            return;
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "horizontal_layout::layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
        if (has_parent_widget())
            parent_widget().layout_items_started();
        scoped_layout_items layoutItems;
        validate();
        layout::do_layout_items<layout::row_major<horizontal_layout>>(aPosition, aSize);
        if (has_parent_widget())
            parent_widget().layout_items_completed();
        LayoutCompleted();
    }
}