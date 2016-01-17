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
#include "layout.hpp"

namespace neogfx
{
	class grid_layout : public layout
	{
	public:
		struct cell_occupied : std::logic_error { cell_occupied() : std::logic_error("neogfx::grid_layout::cell_occupied") {} };
		struct cell_unoccupied : std::logic_error { cell_unoccupied() : std::logic_error("neogfx::grid_layout::cell_unoccupied") {} };
	private:
		typedef uint32_t cell_coordinate;
		typedef std::pair<cell_coordinate, cell_coordinate> cell_coordinates;
		typedef std::map<cell_coordinates, item_list::iterator> cell_list;
	public:
		grid_layout(i_widget& aParent);
		grid_layout(i_layout& aParent);
		grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_widget& aParent);
		grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_layout& aParent);
	public:
		void set_dimensions(cell_coordinate aRows, cell_coordinate aColumns);
		virtual void add_widget(i_widget& aWidget);
		virtual void add_widget(std::shared_ptr<i_widget> aWidget);
		virtual void add_layout(i_layout& aLayout);
		virtual void add_layout(std::shared_ptr<i_layout> aLayout);
		virtual void add_widget(cell_coordinate aRow, cell_coordinate aColumn, i_widget& aWidget);
		virtual void add_widget(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_widget> aWidget);
		virtual void add_layout(cell_coordinate aRow, cell_coordinate aColumn, i_layout& aLayout);
		virtual void add_layout(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_layout> aLayout);
		using layout::add_spacer;
		virtual i_spacer& add_spacer();
		virtual i_spacer& add_spacer(uint32_t aPosition);
		virtual i_spacer& add_spacer(cell_coordinate aRow, cell_coordinate aColumn);
		virtual void add_spacer(cell_coordinate aRow, cell_coordinate aColumn, i_spacer& aSpacer);
		virtual void add_spacer(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_spacer> aSpacer);
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
		virtual void layout_items(const point& aPosition, const size& aSize);
	private:
		uint32_t visible_rows() const;
		uint32_t visible_columns() const;
		size::dimension_type row_minimum_size(cell_coordinate aRow) const;
		size::dimension_type column_minimum_size(cell_coordinate aColumn) const;
		size::dimension_type row_maximum_size(cell_coordinate aRow) const;
		size::dimension_type column_maximum_size(cell_coordinate aColumn) const;
		void increment_cursor();
	private:
		cell_list iCells;
		cell_coordinates iDimensions;
		cell_coordinates iCursor;
	};
}