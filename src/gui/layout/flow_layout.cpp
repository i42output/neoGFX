// flow_layout.cpp
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

#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/layout/flow_layout.hpp>
#include "layout.ipp"
#include "flow_layout.ipp"

namespace neogfx
{
    template size layout::do_minimum_size<layout::row_major<flow_layout>>(optional_size const& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::row_major<flow_layout>>(optional_size const& aAvailableSpace) const;
    template void layout::do_layout_items<layout::row_major<flow_layout>>(const point& aPosition, const size& aSize);

    template size layout::do_minimum_size<layout::column_major<flow_layout>>(optional_size const& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::column_major<flow_layout>>(optional_size const& aAvailableSpace) const;
    template void layout::do_layout_items<layout::column_major<flow_layout>>(const point& aPosition, const size& aSize);

    flow_layout::flow_layout(flow_layout_direction aFlowDirection) :
        layout(), iFlowDirection(aFlowDirection)
    {
    }

    flow_layout::flow_layout(i_widget& aParent, flow_layout_direction aFlowDirection) :
        layout(aParent), iFlowDirection(aFlowDirection)
    {
    }

    flow_layout::flow_layout(i_layout& aParent, flow_layout_direction aFlowDirection) :
        layout(aParent), iFlowDirection(aFlowDirection)
    {
    }

    i_spacer& flow_layout::add_spacer()
    {
        return add_spacer_at(count());
    }

    i_spacer& flow_layout::add_spacer_at(layout_item_index aPosition)
    {
        if (iFlowDirection == flow_layout_direction::Horizontal)
        {
            auto s = make_ref<horizontal_spacer>();
            add_at(aPosition, s);
            return *s;
        }
        else
        {
            auto s = make_ref<vertical_spacer>();
            add_at(aPosition, s);
            return *s;
        }
    }

    size_policy flow_layout::size_policy() const
    {
        if (has_size_policy())
            return layout::size_policy();
        if (iFlowDirection == flow_layout_direction::Horizontal)
            return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
        else
            return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
    }

    size flow_layout::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (iFlowDirection == flow_layout_direction::Horizontal)
            return do_minimum_size<layout::row_major<flow_layout>>(aAvailableSpace);
        else
            return do_minimum_size<layout::column_major<flow_layout>>(aAvailableSpace);
    }

    size flow_layout::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (iFlowDirection == flow_layout_direction::Horizontal)
            return do_maximum_size<layout::row_major<flow_layout>>(aAvailableSpace);
        else
            return do_maximum_size<layout::column_major<flow_layout>>(aAvailableSpace);
    }

    alignment flow_layout::alignment() const
    {
        if (has_alignment())
            return layout::alignment();
        if (iFlowDirection == flow_layout_direction::Horizontal)
            return neogfx::alignment::Left | neogfx::alignment::VCenter;
        else
            return neogfx::alignment::Top | neogfx::alignment::Center;
    }

    void flow_layout::layout_items(const point& aPosition, const size& aSize)
    {
        if (!enabled())
            return;
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "flow_layout::layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
        if (has_parent_widget())
            parent_widget().layout_items_started();
        scoped_layout_items layoutItems;
        validate();
        if (iFlowDirection == flow_layout_direction::Horizontal)
            do_layout_items<layout::row_major<flow_layout>>(aPosition, aSize);
        else
            do_layout_items<layout::column_major<flow_layout>>(aPosition, aSize);
        if (has_parent_widget())
            parent_widget().layout_items_completed();
        LayoutCompleted();
    }
}