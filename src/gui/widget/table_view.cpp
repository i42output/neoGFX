// table_view.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/basic_item_model.hpp>

namespace neogfx
{
	table_view::table_view() :
		item_view(),
		iLayout(*this),
		iColumnHeader(iLayout, *this),
		iSpacer(iLayout)
	{
		layout().set_margins(neogfx::margins(0.0));
		set_selection_model(std::shared_ptr<i_item_selection_model>(new item_selection_model()));
		set_presentation_model(std::shared_ptr<i_item_presentation_model>(new item_presentation_model()));
		set_model(std::shared_ptr<i_item_model>(new basic_item_model()));
	}

	table_view::table_view(i_widget& aParent) : 
		item_view(aParent),
		iLayout(*this),
		iColumnHeader(iLayout, *this),
		iSpacer(iLayout)
	{
		layout().set_margins(neogfx::margins(0.0));
		set_selection_model(std::shared_ptr<i_item_selection_model>(new item_selection_model()));
		set_presentation_model(std::shared_ptr<i_item_presentation_model>(new item_presentation_model()));
		set_model(std::shared_ptr<i_item_model>(new basic_item_model()));
	}

	table_view::table_view(i_layout& aLayout) :
		item_view(aLayout),
		iLayout(*this),
		iColumnHeader(iLayout, *this),
		iSpacer(iLayout)
	{
		layout().set_margins(neogfx::margins(0.0));
		set_selection_model(std::shared_ptr<i_item_selection_model>(new item_selection_model()));
		set_presentation_model(std::shared_ptr<i_item_presentation_model>(new item_presentation_model()));
		set_model(std::shared_ptr<i_item_model>(new basic_item_model()));
	}

	table_view::~table_view()
	{
	}

	bool table_view::can_defer_layout() const
	{
		return true;
	}

	bool table_view::is_managing_layout() const
	{
		return true;
	}

	const header_view& table_view::column_header() const
	{
		return iColumnHeader;
	}

	header_view& table_view::column_header()
	{
		return iColumnHeader;
	}

	void table_view::model_changed()
	{
		column_header().set_model(model());
	}

	void table_view::presentation_model_changed()
	{
		column_header().set_presentation_model(presentation_model());
	}

	void table_view::selection_model_changed()
	{
	}

	void table_view::batch_update_started()
	{
		column_header().start_batch_update();
	}

	void table_view::batch_update_ended()
	{
		column_header().end_batch_update();
	}

	rect table_view::item_display_rect() const
	{
		return rect(
			point(0.0, column_header().visible() ? column_header().extents().cy : 0.0),
			size(std::min(client_rect().width(), column_header().total_width()), client_rect().height() - (column_header().visible() ? column_header().extents().cy : 0.0)));
	}

	size table_view::item_total_area(graphics_context& aGraphicsContext) const
	{
		return size(column_header().total_width(), presentation_model().total_height(aGraphicsContext));
	}

	size table_view::cell_spacing() const
	{
		return size(column_separator_width(), 0.0);
	}

	dimension table_view::column_width(uint32_t aColumn) const
	{
		return column_header().section_width(aColumn);
	}

	table_view::child_widget_scrolling_disposition_e table_view::scrolling_disposition(const i_widget& aChildWidget) const
	{
		if (&aChildWidget == &column_header())
			return ScrollChildWidgetHorizontally;
		else
			return item_view::scrolling_disposition(aChildWidget);
	}

	dimension table_view::column_separator_width() const
	{
		return iColumnHeader.separator_width();
	}
}