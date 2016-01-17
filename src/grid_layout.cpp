// grid_layout.cpp
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

#include "neogfx.hpp"
#include <map>
#include <neolib/algorithm.hpp>
#include "grid_layout.hpp"
#include "i_widget.hpp"
#include "spacer.hpp"

namespace neogfx
{
	grid_layout::grid_layout(i_widget& aParent) :
		layout(aParent)
	{
	}

	grid_layout::grid_layout(i_layout& aParent) :
		layout(aParent)
	{
	}

	grid_layout::grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_widget& aParent) :
		layout(aParent), iDimensions(aRows, aColumns)
	{
	}

	grid_layout::grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_layout& aParent) :
		layout(aParent), iDimensions(aRows, aColumns)
	{
	}

	void grid_layout::set_dimensions(cell_coordinate aRows, cell_coordinate aColumns)
	{
		iDimensions = cell_coordinates(aRows, aColumns);
	}

	void grid_layout::add_widget(i_widget& aWidget)
	{
		add_widget(iCursor.first, iCursor.second, aWidget);
		increment_cursor();
	}

	void grid_layout::add_widget(std::shared_ptr<i_widget> aWidget)
	{
		add_widget(iCursor.first, iCursor.second, aWidget);
		increment_cursor();
	}

	void grid_layout::add_layout(i_layout& aLayout)
	{
		add_layout(iCursor.first, iCursor.second, aLayout);
		increment_cursor();
	}

	void grid_layout::add_layout(std::shared_ptr<i_layout> aLayout)
	{
		add_layout(iCursor.first, iCursor.second, aLayout);
		increment_cursor();
	}

	void grid_layout::add_widget(cell_coordinate aRow, cell_coordinate aColumn, i_widget& aWidget)
	{
		if (&aWidget.layout() == this)
			throw widget_already_added();
		if (iCells.find(cell_coordinates(aRow, aColumn)) != iCells.end())
			throw cell_occupied();
		iCells[cell_coordinates(aRow, aColumn)] = items().insert(items().end(), item(aWidget));
		iDimensions.first = std::max(iDimensions.first, aRow);
		iDimensions.second = std::max(iDimensions.second, aColumn);
		if (owner() != 0)
			items().back().set_owner(owner());
	}

	void grid_layout::add_widget(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_widget> aWidget)
	{
		if (&aWidget->layout() == this)
			throw widget_already_added();
		if (iCells.find(cell_coordinates(aRow, aColumn)) != iCells.end())
			throw cell_occupied();
		iCells[cell_coordinates(aRow, aColumn)] = items().insert(items().end(), item(aWidget));
		iDimensions.first = std::max(iDimensions.first, aRow);
		iDimensions.second = std::max(iDimensions.second, aColumn);
		if (owner() != 0)
			items().back().set_owner(owner());
	}

	void grid_layout::add_layout(cell_coordinate aRow, cell_coordinate aColumn, i_layout& aLayout)
	{
		if (iCells.find(cell_coordinates(aRow, aColumn)) != iCells.end())
			throw cell_occupied();
		iCells[cell_coordinates(aRow, aColumn)] = items().insert(items().end(), item(aLayout));
		iDimensions.first = std::max(iDimensions.first, aRow);
		iDimensions.second = std::max(iDimensions.second, aColumn);
		if (owner() != 0)
			items().back().set_owner(owner());
	}

	void grid_layout::add_layout(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_layout> aLayout)
	{
		if (iCells.find(cell_coordinates(aRow, aColumn)) != iCells.end())
			throw cell_occupied();
		iCells[cell_coordinates(aRow, aColumn)] = items().insert(items().end(), item(aLayout));
		iDimensions.first = std::max(iDimensions.first, aRow);
		iDimensions.second = std::max(iDimensions.second, aColumn);
		if (owner() != 0)
			items().back().set_owner(owner());
	}

	i_spacer& grid_layout::add_spacer()
	{
		auto s = std::make_shared<spacer>(static_cast<i_spacer::expansion_policy_e>(i_spacer::ExpandHorizontally | i_spacer::ExpandVertically));
		add_spacer(iCursor.first, iCursor.second, s);
		increment_cursor();
		return *s;
	}

	i_spacer& grid_layout::add_spacer(uint32_t aPosition)
	{
		auto s = std::make_shared<spacer>(static_cast<i_spacer::expansion_policy_e>(i_spacer::ExpandHorizontally | i_spacer::ExpandVertically));
		cell_coordinates oldCursor = iCursor;
		iCursor.first = 0;
		iCursor.second = 0;
		while (aPosition)
			increment_cursor();
		add_spacer(iCursor.first, iCursor.second, s);
		iCursor = oldCursor;
		return *s;
	}

	i_spacer& grid_layout::add_spacer(cell_coordinate aRow, cell_coordinate aColumn)
	{
		if (iCells.find(cell_coordinates(aRow, aColumn)) != iCells.end())
			throw cell_occupied();
		auto s = std::make_shared<spacer>(static_cast<i_spacer::expansion_policy_e>(i_spacer::ExpandHorizontally | i_spacer::ExpandVertically));
		add_spacer(aRow, aColumn, s);
		return *s;
	}

	void grid_layout::add_spacer(cell_coordinate aRow, cell_coordinate aColumn, i_spacer& aSpacer)
	{
		if (iCells.find(cell_coordinates(aRow, aColumn)) != iCells.end())
			throw cell_occupied();
		iCells[cell_coordinates(aRow, aColumn)] = items().insert(items().end(), item(aSpacer));
		iDimensions.first = std::max(iDimensions.first, aRow);
		iDimensions.second = std::max(iDimensions.second, aColumn);
		if (owner() != 0)
			items().back().set_owner(owner());
		aSpacer.set_parent(*this);
	}

	void grid_layout::add_spacer(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_spacer> aSpacer)
	{
		if (iCells.find(cell_coordinates(aRow, aColumn)) != iCells.end())
			throw cell_occupied();
		iCells[cell_coordinates(aRow, aColumn)] = items().insert(items().end(), item(aSpacer));
		iDimensions.first = std::max(iDimensions.first, aRow);
		iDimensions.second = std::max(iDimensions.second, aColumn);
		if (owner() != 0)
			items().back().set_owner(owner());
		aSpacer->set_parent(*this);
	}

	i_widget& grid_layout::get_widget(cell_coordinate aRow, cell_coordinate aColumn)
	{
		if (iCells.find(cell_coordinates(aRow, aColumn)) == iCells.end())
			throw cell_unoccupied();
		if (iCells[cell_coordinates(aRow, aColumn)]->get().is<item::widget_pointer>())
			return *static_variant_cast<item::widget_pointer&>(iCells[cell_coordinates(aRow, aColumn)]->get());
		else
			throw wrong_item_type();
	}

	i_layout& grid_layout::get_layout(cell_coordinate aRow, cell_coordinate aColumn)
	{
		if (iCells.find(cell_coordinates(aRow, aColumn)) == iCells.end())
			throw cell_unoccupied();
		if (iCells[cell_coordinates(aRow, aColumn)]->get().is<item::layout_pointer>())
			return *static_variant_cast<item::layout_pointer&>(iCells[cell_coordinates(aRow, aColumn)]->get());
		else
			throw wrong_item_type();
	}

	size grid_layout::minimum_size() const
	{
		if (items_visible() == 0)
			return size{};
		size result;
		for (cell_coordinate row = 0; row < visible_rows(); ++row)
			result.cy += row_minimum_size(row);
		for (cell_coordinate column = 0; column < visible_columns(); ++column)
			result.cx += column_minimum_size(column);
		result.cx += (margins().left + margins().right);
		result.cy += (margins().top + margins().bottom);
		if (result.cx != std::numeric_limits<size::dimension_type>::max() && visible_columns() > 0)
			result.cx += (spacing().cx * (visible_columns() - 1));
		if (result.cy != std::numeric_limits<size::dimension_type>::max() && visible_rows() > 0)
			result.cy += (spacing().cy * (visible_rows() - 1));
		result.cx = std::max(result.cx, layout::minimum_size().cx);
		result.cy = std::max(result.cy, layout::minimum_size().cy);
		return result;
	}

	size grid_layout::maximum_size() const
	{
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
			return size{};
		size result;
		for (cell_coordinate row = 0; row < visible_rows(); ++row)
		{
			size::dimension_type rowMaxSize = row_maximum_size(row);
			if (rowMaxSize == std::numeric_limits<size::dimension_type>::max())
				result.cy = rowMaxSize;
			else if (result.cy != std::numeric_limits<size::dimension_type>::max())
				result.cy += rowMaxSize;
		}
		for (cell_coordinate column = 0; column < visible_columns(); ++column)
		{
			size::dimension_type columnMaxSize = column_maximum_size(column);
			if (columnMaxSize == std::numeric_limits<size::dimension_type>::max())
				result.cx = columnMaxSize;
			else if (result.cx != std::numeric_limits<size::dimension_type>::max())
				result.cx += columnMaxSize;
		}
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx += (margins().left + margins().right);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy += (margins().top + margins().bottom);
		if (result.cx != std::numeric_limits<size::dimension_type>::max() && visible_columns() > 0)
			result.cx += (spacing().cx * (visible_columns() - 1));
		if (result.cy != std::numeric_limits<size::dimension_type>::max() && visible_rows() > 0)
			result.cy += (spacing().cy * (visible_rows() - 1));
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx = std::min(result.cx, layout::maximum_size().cx);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy = std::min(result.cy, layout::maximum_size().cy);
		return result;
	}

	void grid_layout::layout_items(const point& aPosition, const size& aSize)
	{
		if (!enabled())
			return;
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
			return;
		size availableSize = aSize;
		availableSize.cx -= (margins().left + margins().right);
		availableSize.cy -= (margins().top + margins().bottom);
		if (visible_rows() > 0)
			availableSize.cy -= (spacing().cy * (visible_rows() - 1));
		if (visible_columns() > 0)
			availableSize.cx -= (spacing().cx * (visible_columns() - 1));
		size leftover = availableSize;
		size eachLeftover = size(std::floor(leftover.cx / visible_columns()), std::floor(leftover.cy / visible_rows()));
		enum why_e { TooSmall, TooBig };
		std::map<cell_coordinate, why_e> rowsNotUsingLeftover;
		std::map<cell_coordinate, why_e> columnsNotUsingLeftover;
		bool done = false;
		while (!done)
		{
			done = true;
			for (cell_coordinate row = 0; row < visible_rows(); ++row)
			{
				size::dimension_type rowMaxSize = row_maximum_size(row);
				size::dimension_type rowMinSize = row_minimum_size(row);
				if (rowMaxSize < eachLeftover.cy && rowsNotUsingLeftover.find(row) == rowsNotUsingLeftover.end())
				{
					rowsNotUsingLeftover[row] = TooSmall;
					leftover.cy -= rowMaxSize;
					eachLeftover.cy = std::floor(leftover.cy / (visible_rows() - rowsNotUsingLeftover.size()));
					done = false;
					break;
				}
				else if (rowMinSize > eachLeftover.cy && rowsNotUsingLeftover.find(row) == rowsNotUsingLeftover.end())
				{
					rowsNotUsingLeftover[row] = TooBig;
					leftover.cy -= rowMinSize;
					eachLeftover.cy = std::floor(leftover.cy / (visible_rows() - rowsNotUsingLeftover.size()));
					done = false;
					break;
				}
			}
		}
		done = false;
		while (!done)
		{
			done = true;
			for (cell_coordinate column = 0; column < visible_columns(); ++column)
			{
				size::dimension_type columnMaxSize = column_maximum_size(column);
				size::dimension_type columnMinSize = column_minimum_size(column);
				if (columnMaxSize < eachLeftover.cx && columnsNotUsingLeftover.find(column) == columnsNotUsingLeftover.end())
				{
					columnsNotUsingLeftover[column] = TooSmall;
					leftover.cx -= columnMaxSize;
					eachLeftover.cx = std::floor(leftover.cx / (visible_columns() - columnsNotUsingLeftover.size()));
					done = false;
					break;
				}
				else if (columnMinSize > eachLeftover.cx && columnsNotUsingLeftover.find(column) == columnsNotUsingLeftover.end())
				{
					columnsNotUsingLeftover[column] = TooBig;
					leftover.cx -= columnMinSize;
					eachLeftover.cx = std::floor(leftover.cx / (visible_columns() - columnsNotUsingLeftover.size()));
					done = false;
					break;
				}
			}
		}
		std::pair<uint32_t, uint32_t> numberUsingLeftover(
			visible_rows() - rowsNotUsingLeftover.size(), 
			visible_columns() - columnsNotUsingLeftover.size());
		std::pair<uint32_t, uint32_t> bitsLeft(
			static_cast<int32_t>(leftover.cy - (eachLeftover.cy * numberUsingLeftover.first)), 
			static_cast<int32_t>(leftover.cx - (eachLeftover.cx * numberUsingLeftover.second)));
		std::pair<neolib::bresenham_counter<int32_t>, neolib::bresenham_counter<int32_t>> bits(
			neolib::bresenham_counter<int32_t>(bitsLeft.first, numberUsingLeftover.first),
			neolib::bresenham_counter<int32_t>(bitsLeft.second, numberUsingLeftover.second));
		std::pair<uint32_t, uint32_t> previousBit(0, 0);
		point nextPos = aPosition;
		nextPos.y += margins().top;
		for (cell_coordinate row = 0; row < visible_rows(); ++row)
		{
			size s{ 0, 0 };
			if (rowsNotUsingLeftover.find(row) != rowsNotUsingLeftover.end())
				s.cy = rowsNotUsingLeftover[row] == TooBig ? row_minimum_size(row) : row_maximum_size(row);
			else
			{
				uint32_t bit = bitsLeft.first != 0 ? bits.first() : 0;
				s.cy = eachLeftover.cy + static_cast<size::dimension_type>(bit - previousBit.first);
				previousBit.first = bit;
			}
			nextPos.x = aPosition.x + margins().left;
			bitsLeft.second = static_cast<int32_t>(leftover.cx - (eachLeftover.cx * numberUsingLeftover.second));
			bits.second = neolib::bresenham_counter<int32_t>(bitsLeft.second, numberUsingLeftover.second);
			previousBit.second = 0;
			for (cell_coordinate column = 0; column < visible_columns(); ++column)
			{
				if (columnsNotUsingLeftover.find(column) != columnsNotUsingLeftover.end())
					s.cx = columnsNotUsingLeftover[column] == TooBig ? column_minimum_size(column) : column_maximum_size(column);
				else
				{
					uint32_t bit = bitsLeft.second != 0 ? bits.second() : 0;
					s.cx = eachLeftover.cx + static_cast<size::dimension_type>(bit - previousBit.second);
					previousBit.second = bit;
				}
				if (iCells.find(cell_coordinates(row, column)) != iCells.end())
					iCells[cell_coordinates(row, column)]->layout(nextPos, s);
				nextPos.x += s.cx;
				nextPos.x += spacing().cx;
			}
			nextPos.y += s.cy;
			nextPos.y += spacing().cy;
		}
		owner()->layout_items_completed();
	}

	uint32_t grid_layout::visible_rows() const
	{
		uint32_t result = 0;
		for (cell_coordinate row = 0; row < iDimensions.first; ++row)
			for (cell_coordinate col = 0; col < iDimensions.second; ++col)
			{
				auto i = iCells.find(std::make_pair(row, col));
				if (i != iCells.end() && i->second->get().is<item::widget_pointer>() && i->second->visible())
				{
					++result;
					break;
				}
			}
		return result;
	}

	uint32_t grid_layout::visible_columns() const
	{
		uint32_t result = 0;
		for (cell_coordinate col = 0; col < iDimensions.second; ++col)
			for (cell_coordinate row = 0; row < iDimensions.first; ++row)
			{
				auto i = iCells.find(std::make_pair(row, col));
				if (i != iCells.end() && i->second->get().is<item::widget_pointer>() && i->second->visible())
				{
					++result;
					break;
				}
			}
		return result;
	}

	size::dimension_type grid_layout::row_minimum_size(cell_coordinate aRow) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.first == aRow)
				result = std::max(result, item.second->minimum_size().cy);
		return result;
	}

	size::dimension_type grid_layout::column_minimum_size(cell_coordinate aColumn) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.second == aColumn)
				result = std::max(result, item.second->minimum_size().cx);
		return result;
	}

	size::dimension_type grid_layout::row_maximum_size(cell_coordinate aRow) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.first == aRow)
				result = std::max(result, item.second->maximum_size().cy);
		return result;
	}

	size::dimension_type grid_layout::column_maximum_size(cell_coordinate aColumn) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.second == aColumn)
				result = std::max(result, item.second->maximum_size().cx);
		return result;
	}

	void grid_layout::increment_cursor()
	{
		++iCursor.second;
		if (iCursor.second >= visible_columns())
		{
			++iCursor.first;
			iCursor.second = 0;
		}
	}
}