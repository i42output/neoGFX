// vertical_layout.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>
#include "layout.hpp"

namespace neogfx
{
    class vertical_layout : public layout
    {
    public:
        vertical_layout(neogfx::alignment aAlignment = neogfx::alignment::Left);
        vertical_layout(i_widget& aParent, neogfx::alignment aAlignment = neogfx::alignment::Left);
        vertical_layout(i_layout& aParent, neogfx::alignment aAlignment = neogfx::alignment::Left);
        vertical_layout(vertical_layout& aParent);
    public:
        i_spacer& add_spacer() override;
        i_spacer& add_spacer_at(layout_item_index aPosition) override;
    public:
        size minimum_size(const optional_size& aAvailableSpace = optional_size{}) const override;
        size maximum_size(const optional_size& aAvailableSpace = optional_size{}) const override;
    public:
        void layout_items(const point& aPosition, const size& aSize) override;
    protected:
        using layout::items_visible;
    };
}