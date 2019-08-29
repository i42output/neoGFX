// stack_layout.cpp
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

    i_spacer& stack_layout::add_spacer_at(item_index)
    {
        throw wrong_item_type();
    }

    size stack_layout::minimum_size(const optional_size& aAvailableSpace) const
    {
        size result;
        if (items_visible() != 0)
        {
            for (const auto& item : items())
            {
                result.cx = std::max(result.cx, item.minimum_size(aAvailableSpace).cx);
                result.cy = std::max(result.cy, item.minimum_size(aAvailableSpace).cy);
            }
            result.cx += margins().size().cx;
            result.cy += margins().size().cy;
            result.cx = std::max(result.cx, layout::minimum_size(aAvailableSpace).cx);
            result.cy = std::max(result.cy, layout::minimum_size(aAvailableSpace).cy);
        }
        return result;
    }

    size stack_layout::maximum_size(const optional_size& aAvailableSpace) const
    {
        size result{ size::max_size() };
        for (const auto& item : items())
        {
            const auto& itemMaxSize = item.maximum_size(aAvailableSpace);
            if (itemMaxSize.cx != 0.0)
                result.cx = std::min(result.cx, itemMaxSize.cx);
            if (itemMaxSize.cy != 0.0)
                result.cy = std::min(result.cy, itemMaxSize.cy);
        }
        if (result.cx != size::max_dimension())
            result.cx += margins().size().cx;
        if (result.cy != size::max_dimension())
            result.cy += margins().size().cy;
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
        if (debug == this)
            std::cerr << "stack_layout::layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
        if (has_layout_owner())
            layout_owner().layout_items_started();
        next_layout_id();
        validate();
        for (auto& item : *this)
        {
            if (!item.visible())
                continue;
            item.layout_as(aPosition + margins().top_left(), aSize - margins().size());
        }
        if (has_layout_owner())
            layout_owner().layout_items_completed();
        evLayoutCompleted.trigger();
    }
}