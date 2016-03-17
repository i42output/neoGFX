// grid_layout.hpp
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
#include <map>
#include <boost/pool/pool_alloc.hpp>
#include "layout.hpp"
#include "vertical_layout.hpp"
#include "horizontal_layout.hpp"

namespace neogfx
{
	class grid_layout : public layout
	{
	public:
		struct cell_unoccupied : std::logic_error { cell_unoccupied() : std::logic_error("neogfx::grid_layout::cell_unoccupied") {} };
	public:
		typedef uint32_t cell_coordinate;
		typedef basic_point<cell_coordinate> cell_coordinates;
		typedef basic_size<cell_coordinate> cell_dimensions;
	private:
		struct row_major;
		struct column_major;
		typedef std::map<cell_coordinates, item_list::iterator, std::less<cell_coordinates>, boost::pool_allocator<std::pair<cell_coordinates, item_list::iterator>>> cell_list;
		typedef std::vector<std::pair<cell_coordinates, cell_coordinates>> span_list;
	public:
		grid_layout(i_widget& aParent);
		grid_layout(i_layout& aParent);
		grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_widget& aParent);
		grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_layout& aParent);
	public:
		cell_coordinate rows() const;
		cell_coordinate columns() const;
		cell_coordinates dimensions() const;
		void set_dimensions(cell_coordinate aRows, cell_coordinate aColumns);
		virtual void add_item(i_widget& aWidget);
		virtual void add_item(std::shared_ptr<i_widget> aWidget);
		virtual void add_item(i_layout& aLayout);
		virtual void add_item(std::shared_ptr<i_layout> aLayout);
		virtual void add_item(cell_coordinate aRow, cell_coordinate aColumn, i_widget& aWidget);
		virtual void add_item(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_widget> aWidget);
		virtual void add_item(cell_coordinate aRow, cell_coordinate aColumn, i_layout& aLayout);
		virtual void add_item(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_layout> aLayout);
		virtual void add_item(cell_coordinate aRow, cell_coordinate aColumn, i_spacer& aSpacer);
		virtual void add_item(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_spacer> aSpacer);
		using layout::add_item;
		virtual i_spacer& add_spacer();
		virtual i_spacer& add_spacer(uint32_t aPosition);
		virtual i_spacer& add_spacer(cell_coordinate aRow, cell_coordinate aColumn);
		virtual void remove_item(std::size_t aIndex);
		virtual void remove_item(cell_coordinate aRow, cell_coordinate aColumn);
		virtual void remove_items();
		using layout::get_widget;
		i_widget& get_widget(cell_coordinate aRow, cell_coordinate aColumn);
		template <typename WidgetT>
		WidgetT& get_widget(cell_coordinate aRow, cell_coordinate aColumn)
		{
			return static_cast<WidgetT&>(get_widget(aRow, aColumn));
		}
		using layout::get_layout;
		i_layout& get_layout(cell_coordinate aRow, cell_coordinate aColumn);
	public:
		virtual size minimum_size() const;
		virtual size maximum_size() const;
	public:
		virtual void set_spacing(const size& aSpacing);
		void add_span(cell_coordinate aRowFrom, cell_coordinate aColumnFrom, uint32_t aRows, uint32_t aColumns);
		void add_span(const cell_coordinates& aFrom, const cell_coordinates& aTo);
	public:
		virtual void layout_items(const point& aPosition, const size& aSize);
	private:
		uint32_t visible_rows() const;
		bool is_row_visible(uint32_t aRow) const;
		uint32_t visible_columns() const;
		bool is_column_visible(uint32_t aColumn) const;
		size::dimension_type row_minimum_size(cell_coordinate aRow) const;
		size::dimension_type column_minimum_size(cell_coordinate aColumn) const;
		size::dimension_type row_maximum_size(cell_coordinate aRow) const;
		size::dimension_type column_maximum_size(cell_coordinate aColumn) const;
		void increment_cursor();
		horizontal_layout& row_layout(cell_coordinate aRow);
		uint32_t row_column(horizontal_layout& aRow, cell_coordinate aColumn) const;
	private:
		vertical_layout iRowLayout;
		std::vector<std::shared_ptr<horizontal_layout>> iRows;
		cell_list iCells;
		cell_dimensions iDimensions;
		cell_coordinates iCursor;
		span_list iSpans;
	};
}