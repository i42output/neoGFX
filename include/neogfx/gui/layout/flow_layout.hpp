// flow_layout.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/core/geometrical.hpp>
#include "layout.hpp"

namespace neogfx
{
	class flow_layout : public layout
	{
	public:
		enum flow_direction_e
		{
			FlowDirectionHorizontal,
			FlowDirectionVertical,
		};
	public:
		flow_layout(flow_direction_e aFlowDirection = FlowDirectionHorizontal);
		flow_layout(i_widget& aParent, flow_direction_e aFlowDirection = FlowDirectionHorizontal);
		flow_layout(i_layout& aParent, flow_direction_e aFlowDirection = FlowDirectionHorizontal);
	public:
		virtual i_spacer& add_spacer();
		virtual i_spacer& add_spacer_at(item_index aPosition);
	public:
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void layout_items(const point& aPosition, const size& aSize);
	protected:
		template <typename AxisPolicy>
		size do_minimum_size(const optional_size& aAvailableSpace) const;
		template <typename AxisPolicy>
		size do_maximum_size(const optional_size& aAvailableSpace) const;
		template <typename AxisPolicy>
		void do_layout_items(const point& aPosition, const size& aSize);
	private:
		flow_direction_e iFlowDirection;
	};
}
