// stack_layout.cpp
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
#include <boost/pool/pool_alloc.hpp>
#include <neolib/core/bresenham_counter.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    stack_layout::stack_layout(neogfx::alignment aAlignment) :
        layout{ aAlignment }
    {
    }

    stack_layout::stack_layout(i_widget& aParent, neogfx::alignment aAlignment) :
        layout{ aParent, aAlignment }
    {
    }

    stack_layout::stack_layout(i_layout& aParent, neogfx::alignment aAlignment) :
        layout{ aParent, aAlignment }
    {
    }

    i_spacer& stack_layout::add_spacer()
    {
        throw wrong_item_type();
    }

    i_spacer& stack_layout::add_spacer_at(layout_item_index)
    {
        throw wrong_item_type();
    }

    size stack_layout::minimum_size(optional_size const& aAvailableSpace) const
    {
        size result;
        if (items_visible() != 0)
        {
            for (auto const& itemRef : items())
            {
                auto const& item = *itemRef;
                if (!item.visible() && !ignore_visibility())
                    continue;
                result.cx = std::max(result.cx, item.minimum_size(aAvailableSpace).cx);
                result.cy = std::max(result.cy, item.minimum_size(aAvailableSpace).cy);
            }
            result.cx += internal_spacing().size().cx;
            result.cy += internal_spacing().size().cy;
            result.cx = std::max(result.cx, layout::minimum_size(aAvailableSpace).cx);
            result.cy = std::max(result.cy, layout::minimum_size(aAvailableSpace).cy);
        }
        return result;
    }

    size stack_layout::maximum_size(optional_size const& aAvailableSpace) const
    {
        size result{ size::max_size() };
        for (auto const& itemRef : items())
        {
            auto const& item = *itemRef;
            if (!item.visible() && !ignore_visibility())
                continue;
            auto const& itemMaxSize = item.maximum_size(aAvailableSpace);
            if (itemMaxSize.cx != 0.0)
                result.cx = std::min(result.cx, itemMaxSize.cx);
            if (itemMaxSize.cy != 0.0)
                result.cy = std::min(result.cy, itemMaxSize.cy);
        }
        if (result.cx != size::max_dimension())
            result.cx += internal_spacing().size().cx;
        if (result.cy != size::max_dimension())
            result.cy += internal_spacing().size().cy;
        if (result.cx != size::max_dimension())
            result.cx = std::min(result.cx, layout::maximum_size(aAvailableSpace).cx);
        if (result.cy != size::max_dimension())
            result.cy = std::min(result.cy, layout::maximum_size(aAvailableSpace).cy);
        return result;
    }

    void stack_layout::layout_items(const point& aPosition, const size& aSize)
    {
        if (!enabled())
            return;
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << "stack_layout::layout_items(" << aPosition << ", " << aSize << ")" << endl;
#endif // NEOGFX_DEBUG
        if (has_parent_widget())
            parent_widget().layout_items_started();
        scoped_layout_items layoutItems;
        validate();
        set_position(aPosition);
        set_extents(aSize);
        for (auto& itemRef : *this)
        {
            auto& item = *itemRef;
            if (!item.visible())
                continue;
            item.layout_as(aPosition + internal_spacing().top_left(), aSize - internal_spacing().size());
        }
        if (has_parent_widget())
            parent_widget().layout_items_completed();
        LayoutCompleted.trigger();
    }
}