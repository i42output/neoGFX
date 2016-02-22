// table_view.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include "item_view.hpp"
#include "vertical_layout.hpp"
#include "spacer.hpp"

namespace neogfx
{
	class table_view : public item_view
	{
	public:
		table_view();
		table_view(i_widget& aParent);
		table_view(i_layout& aLayout);
		~table_view();
	public:
		bool can_defer_layout() const;
		bool is_managing_layout() const;
	public:
		const header_view& column_header() const;
		header_view& column_header();
	protected:
		virtual void model_changed();
		virtual void presentation_model_changed();
		virtual void selection_model_changed();
		virtual void batch_update_started();
		virtual void batch_update_ended();
	protected:
		virtual rect item_display_rect() const;
		virtual size item_total_area(graphics_context& aGraphicsContext) const;
		virtual size cell_spacing() const;
		virtual dimension column_width(uint32_t aColumn) const;
	protected:
		virtual child_widget_scrolling_disposition_e scrolling_disposition(const i_widget& aChildWidget) const;
	protected:
		dimension column_separator_width() const;
	private:
		vertical_layout iLayout;
		header_view iColumnHeader;
		vertical_spacer iSpacer;
	};
}