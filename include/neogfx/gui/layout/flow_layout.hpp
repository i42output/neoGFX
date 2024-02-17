// flow_layout.hpp
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

#include <neogfx/core/geometrical.hpp>
#include <neogfx/gui/layout/layout.hpp>

namespace neogfx
{
    enum class flow_layout_direction : std::uint32_t
    {
        Horizontal  = 0x00000001,
        Vertical    = 0x00000002,
    };

    class flow_layout : public layout
    {
        meta_object(layout)
    public:
        flow_layout(flow_layout_direction aFlowDirection = flow_layout_direction::Horizontal);
        flow_layout(i_widget& aParent, flow_layout_direction aFlowDirection = flow_layout_direction::Horizontal);
        flow_layout(i_layout& aParent, flow_layout_direction aFlowDirection = flow_layout_direction::Horizontal);
    public:
        i_spacer& add_spacer() override;
        i_spacer& add_spacer_at(layout_item_index aPosition) override;
    public:
        neogfx::size_policy size_policy() const final;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        neogfx::alignment alignment() const override;
    public:
        void layout_items(const point& aPosition, const size& aSize) final;
    protected:
        template <typename AxisPolicy>
        size do_minimum_size(optional_size const& aAvailableSpace) const;
        template <typename AxisPolicy>
        size do_maximum_size(optional_size const& aAvailableSpace) const;
        template <typename AxisPolicy>
        void do_layout_items(const point& aPosition, const size& aSize);
    private:
        flow_layout_direction iFlowDirection;
    };
}
