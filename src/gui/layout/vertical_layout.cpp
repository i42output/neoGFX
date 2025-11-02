// vertical_layout.cpp
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

    vertical_layout::~vertical_layout()
    {
    }

    layout_direction vertical_layout::direction() const
    {
        return layout_direction::Vertical;
    }

    i_spacer& vertical_layout::add_spacer()
    {
        auto s = make_ref<vertical_spacer>();
        add(s);
        return *s;
    }

    i_spacer& vertical_layout::add_spacer_at(layout_item_index aPosition)
    {
        auto s = make_ref<vertical_spacer>();
        add_at(aPosition, s);
        return *s;
    }

    size vertical_layout::minimum_size(optional_size const& aAvailableSpace) const
    {
        return layout::do_minimum_size<layout::column_major<vertical_layout>>(aAvailableSpace);
    }

    size vertical_layout::maximum_size(optional_size const& aAvailableSpace) const
    {
        return layout::do_maximum_size<layout::column_major<vertical_layout>>(aAvailableSpace);
    }

    void vertical_layout::layout_items(const point& aPosition, const size& aSize)
    {
        if (!enabled())
            return;
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "vertical_layout::layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
        if (has_parent_widget())
            parent_widget().layout_items_started();
        scoped_layout_items layoutItems;
        validate();
        layout::do_layout_items<layout::column_major<vertical_layout>>(aPosition, aSize);
        if (has_parent_widget())
            parent_widget().layout_items_completed();
        LayoutCompleted();
    }
}