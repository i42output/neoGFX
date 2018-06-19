// list_view.hpp
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
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/item_view.hpp>

namespace neogfx
{
	class list_view : public item_view
	{
	public:
		list_view(scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame, bool aCreateDefaultModels = true);
		list_view(i_widget& aParent, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame, bool aCreateDefaultModels = true);
		list_view(i_layout& aLayout, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame, bool aCreateDefaultModels = true);
		~list_view();
	public:
		bool can_defer_layout() const;
		bool is_managing_layout() const;
	protected:
		void model_changed() override;
		void presentation_model_changed() override;
		void selection_model_changed() override;
	protected:
		rect item_display_rect() const override;
		size total_item_area(const i_units_context& aUnitsContext) const override;
		dimension column_width(uint32_t aColumn) const override;
	protected:
		neogfx::scrolling_disposition scrolling_disposition(const i_widget& aChildWidget) const override;
	private:
		vertical_layout iLayout;
		vertical_spacer iSpacer;
	};
}