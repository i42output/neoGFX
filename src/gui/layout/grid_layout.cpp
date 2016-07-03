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

#include <neogfx/neogfx.hpp>
#include <map>
#include <neolib/bresenham_counter.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>

namespace neogfx
{
	grid_layout::grid_layout(i_widget& aParent) :
		layout(aParent), iRowLayout(*this)
	{
		iRowLayout.set_margins(neogfx::margins{});
		iRowLayout.set_spacing(spacing());
		iRowLayout.set_always_use_spacing(true);
	}

	grid_layout::grid_layout(i_layout& aParent) :
		layout(aParent), iRowLayout(*this)
	{
		iRowLayout.set_margins(neogfx::margins{});
		iRowLayout.set_spacing(spacing());
		iRowLayout.set_always_use_spacing(true);
	}

	grid_layout::grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_widget& aParent) :
		layout(aParent), iRowLayout(*this), iDimensions{aColumns, aRows}
	{
		iRowLayout.set_margins(neogfx::margins{});
		iRowLayout.set_spacing(spacing());
		iRowLayout.set_always_use_spacing(true);
	}

	grid_layout::grid_layout(cell_coordinate aRows, cell_coordinate aColumns, i_layout& aParent) :
		layout(aParent), iRowLayout(*this), iDimensions(aColumns, aRows)
	{
		iRowLayout.set_margins(neogfx::margins{});
		iRowLayout.set_spacing(spacing());
		iRowLayout.set_always_use_spacing(true);
	}

	grid_layout::cell_coordinate grid_layout::rows() const
	{
		return iDimensions.cy;
	}

	grid_layout::cell_coordinate grid_layout::columns() const
	{
		return iDimensions.cx;
	}

	grid_layout::cell_coordinates grid_layout::dimensions() const
	{
		return iDimensions;
	}

	void grid_layout::set_dimensions(cell_coordinate aRows, cell_coordinate aColumns)
	{
		iDimensions = cell_dimensions{aColumns, aRows};
	}

	void grid_layout::add_item(i_widget& aWidget)
	{
		add_item(iCursor.y, iCursor.x, aWidget);
		increment_cursor();
	}

	void grid_layout::add_item(std::shared_ptr<i_widget> aWidget)
	{
		add_item(iCursor.y, iCursor.x, aWidget);
		increment_cursor();
	}

	void grid_layout::add_item(i_layout& aLayout)
	{
		add_item(iCursor.y, iCursor.x, aLayout);
		increment_cursor();
	}

	void grid_layout::add_item(std::shared_ptr<i_layout> aLayout)
	{
		add_item(iCursor.y, iCursor.x, aLayout);
		increment_cursor();
	}

	void grid_layout::add_item(cell_coordinate aRow, cell_coordinate aColumn, i_widget& aWidget)
	{
		if (aWidget.has_layout() && &aWidget.layout() == this)
			throw widget_already_added();
		if (iCells.find(cell_coordinates{aColumn, aRow}) != iCells.end())
			remove_item(aRow, aColumn);
		for (cell_coordinate col = 0; col < aColumn; ++col)
			if (iCells.find(cell_coordinates{col, aRow}) == iCells.end())
				add_spacer(aRow, col);
		iCells[cell_coordinates{aColumn, aRow}] = items().insert(items().end(), item(*this, aWidget));
		iDimensions.cy = std::max(iDimensions.cy, aRow + 1);
		iDimensions.cx = std::max(iDimensions.cx, aColumn + 1);
		if (owner() != 0)
			items().back().set_owner(owner());
		row_layout(aRow).replace_item(aColumn, aWidget);
	}

	void grid_layout::add_item(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_widget> aWidget)
	{
		if (aWidget->has_layout() && &aWidget->layout() == this)
			throw widget_already_added();
		if (iCells.find(cell_coordinates{aColumn, aRow}) != iCells.end())
			remove_item(aRow, aColumn);
		for (cell_coordinate col = 0; col < aColumn; ++col)
			if (iCells.find(cell_coordinates{ col, aRow }) == iCells.end())
				add_spacer(aRow, col);
		iCells[cell_coordinates{aColumn, aRow}] = items().insert(items().end(), item(*this, aWidget));
		iDimensions.cy = std::max(iDimensions.cy, aRow + 1);
		iDimensions.cx = std::max(iDimensions.cx, aColumn + 1);
		if (owner() != 0)
			items().back().set_owner(owner());
		row_layout(aRow).replace_item(aColumn, aWidget);
	}

	void grid_layout::add_item(cell_coordinate aRow, cell_coordinate aColumn, i_layout& aLayout)
	{
		if (&aLayout == &iRowLayout)
		{
			if (owner() != 0)
				aLayout.set_owner(owner());
			return;
		}
		if (iCells.find(cell_coordinates{aColumn, aRow}) != iCells.end())
			remove_item(aRow, aColumn);
		for (cell_coordinate col = 0; col < aColumn; ++col)
			if (iCells.find(cell_coordinates{ col, aRow }) == iCells.end())
				add_spacer(aRow, col);
		iCells[cell_coordinates{aColumn, aRow}] = items().insert(items().end(), item(*this, aLayout));
		iDimensions.cy = std::max(iDimensions.cy, aRow + 1);
		iDimensions.cx = std::max(iDimensions.cx, aColumn + 1);
		if (owner() != 0)
			items().back().set_owner(owner());
		row_layout(aRow).replace_item(aColumn, aLayout);
	}

	void grid_layout::add_item(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_layout> aLayout)
	{
		if (iCells.find(cell_coordinates{aColumn, aRow}) != iCells.end())
			remove_item(aRow, aColumn);
		for (cell_coordinate col = 0; col < aColumn; ++col)
			if (iCells.find(cell_coordinates{ col, aRow }) == iCells.end())
				add_spacer(aRow, col);
		iCells[cell_coordinates{aColumn, aRow}] = items().insert(items().end(), item(*this, aLayout));
		iDimensions.cy = std::max(iDimensions.cy, aRow + 1);
		iDimensions.cx = std::max(iDimensions.cx, aColumn + 1);
		if (owner() != 0)
			items().back().set_owner(owner());
		row_layout(aRow).replace_item(aColumn, aLayout);
	}

	void grid_layout::add_item(cell_coordinate aRow, cell_coordinate aColumn, i_spacer& aSpacer)
	{
		if (iCells.find(cell_coordinates{ aColumn, aRow }) != iCells.end())
			remove_item(aRow, aColumn);
		for (cell_coordinate col = 0; col < aColumn; ++col)
			if (iCells.find(cell_coordinates{ col, aRow }) == iCells.end())
				add_spacer(aRow, col);
		iCells[cell_coordinates{ aColumn, aRow }] = items().insert(items().end(), item(*this, aSpacer));
		iDimensions.cy = std::max(iDimensions.cy, aRow + 1);
		iDimensions.cx = std::max(iDimensions.cx, aColumn + 1);
		if (owner() != 0)
			items().back().set_owner(owner());
		aSpacer.set_parent(*this);
		row_layout(aRow).replace_item(aColumn, aSpacer);
	}

	void grid_layout::add_item(cell_coordinate aRow, cell_coordinate aColumn, std::shared_ptr<i_spacer> aSpacer)
	{
		if (iCells.find(cell_coordinates{ aColumn, aRow }) != iCells.end())
			remove_item(aRow, aColumn);
		for (cell_coordinate col = 0; col < aColumn; ++col)
			if (iCells.find(cell_coordinates{ col, aRow }) == iCells.end())
				add_spacer(aRow, col);
		iCells[cell_coordinates{ aColumn, aRow }] = items().insert(items().end(), item(*this, aSpacer));
		iDimensions.cy = std::max(iDimensions.cy, aRow + 1);
		iDimensions.cx = std::max(iDimensions.cx, aColumn + 1);
		if (owner() != 0)
			items().back().set_owner(owner());
		aSpacer->set_parent(*this);
		row_layout(aRow).replace_item(aColumn, aSpacer);
	}

	i_spacer& grid_layout::add_spacer()
	{
		auto s = std::make_shared<spacer>(static_cast<i_spacer::expansion_policy_e>(i_spacer::ExpandHorizontally | i_spacer::ExpandVertically));
		add_item(iCursor.y, iCursor.x, s);
		increment_cursor();
		return *s;
	}

	i_spacer& grid_layout::add_spacer(uint32_t aPosition)
	{
		auto s = std::make_shared<spacer>(static_cast<i_spacer::expansion_policy_e>(i_spacer::ExpandHorizontally | i_spacer::ExpandVertically));
		cell_coordinates oldCursor = iCursor;
		iCursor.y = 0;
		iCursor.x = 0;
		while (aPosition--)
			increment_cursor();
		add_item(iCursor.y, iCursor.x, s);
		iCursor = oldCursor;
		return *s;
	}

	i_spacer& grid_layout::add_spacer(cell_coordinate aRow, cell_coordinate aColumn)
	{
		if (iCells.find(cell_coordinates{ aColumn, aRow }) != iCells.end())
			remove_item(aRow, aColumn);
		auto s = std::make_shared<spacer>(static_cast<i_spacer::expansion_policy_e>(i_spacer::ExpandHorizontally | i_spacer::ExpandVertically));
		add_item(aRow, aColumn, s);
		return *s;
	}

	void grid_layout::remove_item(item_index aIndex)
	{
		auto itemIter = std::next(items().begin(), aIndex);
		for (cell_list::reverse_iterator i = iCells.rbegin(); i != iCells.rend(); ++i)
		{
			if (i->second == itemIter)
			{
				remove_item(i->first.y, i->first.x);
				break;
			}
		}
	}

	bool grid_layout::remove_item(i_layout& aItem)
	{
		auto item = find_item(aItem);
		if (item != boost::none)
		{
			remove_item(*item);
			return true;
		}
		return false;
	}

	bool grid_layout::remove_item(i_widget& aItem)
	{
		auto item = find_item(aItem);
		if (item != boost::none)
		{
			remove_item(*item);
			return true;
		}
		return false;
	}

	void grid_layout::remove_item(cell_coordinate aRow, cell_coordinate aColumn)
	{
		auto iterExistingCell = iCells.find(cell_coordinates{ aColumn, aRow });
		if (iterExistingCell == iCells.end())
			throw cell_unoccupied();
		auto iterExistingItem = iterExistingCell->second;
		{
			auto existing = row_layout(aRow).find_item(*iterExistingItem);
			if (existing != boost::none)
				row_layout(aRow).remove_item(*existing);
		}
		if (aColumn < row_layout(aRow).item_count())
			row_layout(aRow).remove_item(aColumn);
		iCells.erase(iterExistingCell);
		iDimensions = cell_dimensions{};
		for (const auto& cell : iCells)
		{
			iDimensions.cy = std::max(iDimensions.cy, cell.first.y);
			iDimensions.cx = std::max(iDimensions.cx, cell.first.x);
		}
		iCursor = cell_coordinates{};
		layout::remove_item(iterExistingItem);
	}

	void grid_layout::remove_items()
	{
		layout::remove_items();
		iRowLayout.remove_items();
		iRows.clear();
		iCells.clear();
		iDimensions = cell_dimensions{};
		iCursor = cell_coordinates{};
	}

	i_widget& grid_layout::get_widget(cell_coordinate aRow, cell_coordinate aColumn)
	{
		if (iCells.find(cell_coordinates{aColumn, aRow}) == iCells.end())
			throw cell_unoccupied();
		if (iCells[cell_coordinates{aColumn, aRow}]->get().is<item::widget_pointer>())
			return *static_variant_cast<item::widget_pointer&>(iCells[cell_coordinates{aColumn, aRow}]->get());
		else
			throw wrong_item_type();
	}

	i_layout& grid_layout::get_layout(cell_coordinate aRow, cell_coordinate aColumn)
	{
		if (iCells.find(cell_coordinates{aColumn, aRow}) == iCells.end())
			throw cell_unoccupied();
		if (iCells[cell_coordinates{aColumn, aRow}]->get().is<item::layout_pointer>())
			return *static_variant_cast<item::layout_pointer&>(iCells[cell_coordinates{aColumn, aRow}]->get());
		else
			throw wrong_item_type();
	}

	size grid_layout::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (items_visible() == 0)
			return size{};
		size result;
		uint32_t visibleColumns = visible_columns();
		uint32_t visibleRows = visible_rows();
		if (visibleRows == 6)
			std::cerr << "ff" << std::endl;
		for (cell_coordinate row = 0; row < rows(); ++row)
		{
			if (!is_row_visible(row))
				continue;
			result.cy += row_minimum_size(row, aAvailableSpace);
		}
		for (cell_coordinate column = 0; column < columns(); ++column)
		{
			if (!is_column_visible(column))
				continue;
			result.cx += column_minimum_size(column, aAvailableSpace);
		}
		result.cx += (margins().left + margins().right);
		result.cy += (margins().top + margins().bottom);
		if (result.cx != std::numeric_limits<size::dimension_type>::max() && visibleColumns> 0)
			result.cx += (spacing().cx * (visibleColumns - 1));
		if (result.cy != std::numeric_limits<size::dimension_type>::max() && visibleRows > 0)
			result.cy += (spacing().cy * (visibleRows - 1));
		result.cx = std::max(result.cx, layout::minimum_size(aAvailableSpace).cx);
		result.cy = std::max(result.cy, layout::minimum_size(aAvailableSpace).cy);
		return result;
	}

	size grid_layout::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
			return size{};
		size result;
		for (cell_coordinate row = 0; row < visible_rows(); ++row)
		{
			size::dimension_type rowMaxSize = row_maximum_size(row, aAvailableSpace);
			if (rowMaxSize == std::numeric_limits<size::dimension_type>::max())
				result.cy = rowMaxSize;
			else if (result.cy != std::numeric_limits<size::dimension_type>::max())
				result.cy += rowMaxSize;
		}
		for (cell_coordinate column = 0; column < visible_columns(); ++column)
		{
			size::dimension_type columnMaxSize = column_maximum_size(column, aAvailableSpace);
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
			result.cx = std::min(result.cx, layout::maximum_size(aAvailableSpace).cx);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy = std::min(result.cy, layout::maximum_size(aAvailableSpace).cy);
		return result;
	}

	void grid_layout::set_spacing(const size& aSpacing)
	{
		layout::set_spacing(aSpacing);
		iRowLayout.set_spacing(aSpacing);
		for (auto& r : iRows)
			r->set_spacing(aSpacing);
	}

	void grid_layout::add_span(cell_coordinate aRowFrom, cell_coordinate aColumnFrom, uint32_t aRows, uint32_t aColumns)
	{
		add_span({ aColumnFrom, aRowFrom }, { aColumnFrom + aColumns - 1, aRowFrom + aRows - 1});
	}

	void grid_layout::add_span(const cell_coordinates& aFrom, const cell_coordinates& aTo)
	{
		iSpans.push_back(std::make_pair(aFrom, aTo));
		if (owner() != 0)
			owner()->ultimate_ancestor().layout_items(true);
	}

	void grid_layout::layout_items(const point& aPosition, const size& aSize)
	{
		if (!enabled())
			return;
		owner()->layout_items_started();
		next_layout_id();
		set_position(aPosition);
		set_extents(aSize);
		for (auto& r : iRows)
			while (r->item_count() < iDimensions.cx)
				r->add_spacer();
		point availablePos = aPosition + point{ margins().left, margins().top };
		size availableSize = aSize;
		availableSize.cx -= (margins().left + margins().right);
		availableSize.cy -= (margins().top + margins().bottom);
		iRowLayout.layout_items(availablePos, availableSize);
		std::vector<dimension> maxRowHeight(iDimensions.cy);
		std::vector<dimension> maxColWidth(iDimensions.cx);
		for (cell_coordinate row = 0; row < iDimensions.cy; ++row)
		{
			for (cell_coordinate col = 0; col < iDimensions.cx; ++col)
			{
				auto i = iCells.find(cell_coordinates{ col, row });
				if (i != iCells.end())
				{
					if (i->second->get().is<item::spacer_pointer>() && column_minimum_size(col) != 0.0)
						continue;
					auto s = find_span(cell_coordinates{ col, row });
					if (s == iSpans.end())
					{
						maxRowHeight[row] = std::max(maxRowHeight[row], i->second->extents().cy);
						maxColWidth[col] = std::max(maxColWidth[col], i->second->extents().cx);
					}
					else
					{
						maxRowHeight[row] = std::max(maxRowHeight[row], 
							(i->second->extents().cy - spacing().cy * (s->second.y - s->first.y)) / (s->second.y - s->first.y + 1));
						maxColWidth[col] = std::max(maxColWidth[col],
							(i->second->extents().cx - spacing().cx * (s->second.x - s->first.x)) / (s->second.x - s->first.x + 1));
					}
				}
			}
		}
		point rowPos = availablePos;
		for (cell_coordinate row = 0; row < iDimensions.cy; ++row)
		{
			if (maxRowHeight[row] == 0.0)
				continue;
			point colPos = rowPos;
			for (cell_coordinate col = 0; col < iDimensions.cx; ++col)
			{
				if (maxColWidth[col] == 0.0)
					continue;
				auto i = iCells.find(cell_coordinates{ col, row });
				if (i != iCells.end())
				{
					bool foundSpan = false;
					for (auto& s : iSpans)
					{
						if (col >= s.first.x && col <= s.second.x &&
							row >= s.first.y && row <= s.second.y)
						{
							point fromPos;
							point toPos;
							point rowPos2 = availablePos;
							for (cell_coordinate row2 = 0; !foundSpan && row2 <= s.second.y; ++row2)
							{
								point colPos2 = rowPos2;
								for (cell_coordinate col2 = 0; !foundSpan && col2 <= s.second.x; ++col2)
								{
									if (row2 == s.first.y && col2 == s.first.x)
										fromPos = colPos2;
									if (row2 == s.second.y && col2 == s.second.x)
									{
										toPos = colPos2 + size{ maxColWidth[col2], maxRowHeight[row2] };
										i->second->layout(fromPos, toPos - fromPos);
										foundSpan = true;
									}
									colPos2.x += maxColWidth[col2];
									colPos2.x += spacing().cx;
								}
								rowPos2.y += maxRowHeight[row2];
								rowPos2.y += spacing().cy;
							}
						}
					}
					if (!foundSpan)
						i->second->layout(colPos, size{maxColWidth[col], maxRowHeight[row]});
				}
				colPos.x += maxColWidth[col];
				colPos.x += spacing().cx;
			}
			rowPos.y += maxRowHeight[row];
			rowPos.y += spacing().cy;
		}
		owner()->layout_items_completed();
	}

	uint32_t grid_layout::visible_rows() const
	{
		uint32_t result = 0;
		for (cell_coordinate row = 0; row < iDimensions.cy; ++row)
			for (cell_coordinate col = 0; col < iDimensions.cx; ++col)
			{
				auto i = iCells.find(cell_coordinates{col, row});
				if (i != iCells.end() && !i->second->get().is<item::spacer_pointer>() && i->second->visible() && i->second->minimum_size().cy != 0.0)
				{
					++result;
					break;
				}
			}
		return result;
	}

	bool grid_layout::is_row_visible(uint32_t aRow) const
	{
		for (cell_coordinate col = 0; col < iDimensions.cx; ++col)
		{
			auto i = iCells.find(cell_coordinates{col, aRow});
			if (i != iCells.end() && !i->second->get().is<item::spacer_pointer>() && i->second->visible() && i->second->minimum_size().cy != 0.0)
				return true;
		}
		return false;
	}

	uint32_t grid_layout::visible_columns() const
	{
		uint32_t result = 0;
		for (cell_coordinate col = 0; col < iDimensions.cx; ++col)
			for (cell_coordinate row = 0; row < iDimensions.cy; ++row)
			{
				auto i = iCells.find(cell_coordinates{col, row});
				if (i != iCells.end() && !i->second->get().is<item::spacer_pointer>() && i->second->visible() && i->second->minimum_size().cx != 0.0)
				{
					++result;
					break;
				}
			}
		return result;
	}

	bool grid_layout::is_column_visible(uint32_t aColumn) const
	{
		for (cell_coordinate row = 0; row < iDimensions.cy; ++row)
		{
			auto i = iCells.find(cell_coordinates{aColumn, row});
			if (i != iCells.end() && !i->second->get().is<item::spacer_pointer>() && i->second->visible() && i->second->minimum_size().cx != 0.0)
				return true;
		}
		return false;
	}

	size::dimension_type grid_layout::row_minimum_size(cell_coordinate aRow, const optional_size& aAvailableSpace) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.y == aRow)
			{
				auto s = find_span(item.first);
				if (s == iSpans.end())
					result = std::max(result, item.second->minimum_size(aAvailableSpace).cy);
				else
					result = std::max(result, (item.second->minimum_size(aAvailableSpace).cy - spacing().cy * (s->second.y - s->first.y)) / (s->second.y - s->first.y + 1));
			}
		return result;
	}

	size::dimension_type grid_layout::column_minimum_size(cell_coordinate aColumn, const optional_size& aAvailableSpace) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.x == aColumn)
			{
				auto s = find_span(item.first);
				if (s == iSpans.end())
					result = std::max(result, item.second->minimum_size(aAvailableSpace).cx);
				else
					result = std::max(result, (item.second->minimum_size(aAvailableSpace).cx - spacing().cx * (s->second.x - s->first.x)) / (s->second.x - s->first.x + 1));
			}
		return result;
	}

	size::dimension_type grid_layout::row_maximum_size(cell_coordinate aRow, const optional_size& aAvailableSpace) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.y == aRow)
				result = std::max(result, item.second->maximum_size(aAvailableSpace).cy);
		return result;
	}

	size::dimension_type grid_layout::column_maximum_size(cell_coordinate aColumn, const optional_size& aAvailableSpace) const
	{
		size::dimension_type result {};
		for (const auto& item : iCells)
			if (item.first.x == aColumn)
				result = std::max(result, item.second->maximum_size(aAvailableSpace).cx);
		return result;
	}

	void grid_layout::increment_cursor()
	{
		++iCursor.x;
		if (iCursor.x >= columns())
		{
			++iCursor.y;
			iCursor.x = 0;
		}
	}

	horizontal_layout& grid_layout::row_layout(cell_coordinate aRow)
	{
		while (aRow >= iRows.size())
		{
			iRows.push_back(std::make_shared<horizontal_layout>(iRowLayout));
			iRows.back()->set_always_use_spacing(true);
			iRows.back()->set_margins(neogfx::margins{});
			iRows.back()->set_spacing(spacing());
		}
		return *iRows[aRow];
	}

	grid_layout::span_list::const_iterator grid_layout::find_span(const cell_coordinates& aCell) const
	{
		for (auto s = iSpans.begin(); s != iSpans.end(); ++s)
			if (aCell.x >= s->first.x && aCell.x <= s->second.x && aCell.y >= s->first.y && aCell.y <= s->second.y)
				return s;
		return iSpans.end();
	}
}