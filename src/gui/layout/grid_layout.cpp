// grid_layout.cpp
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

#include <neogfx/neogfx.hpp>
#include <map>
#include <neolib/core/bresenham_counter.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>

namespace neogfx
{
    grid_layout::grid_layout(neogfx::alignment aAlignment) :
        layout{ aAlignment }, iRowLayout{ *this, aAlignment }
    {
        init();
    }

    grid_layout::grid_layout(cell_coordinate aRows, cell_coordinate aColumns, neogfx::alignment aAlignment) :
        layout{ aAlignment }, iDimensions{ aColumns, aRows }, iRowLayout{ *this, aAlignment }
    {
        init();
    }

    grid_layout::grid_layout(i_widget& aParent, neogfx::alignment aAlignment) :
        layout{ aParent, aAlignment }, iRowLayout{ *this, aAlignment }
    {
        init();
    }

    grid_layout::grid_layout(i_layout& aParent, neogfx::alignment aAlignment) :
        layout{ aParent, aAlignment }, iRowLayout{ *this, aAlignment }
    {
        init();
    }

    grid_layout::grid_layout(i_widget& aParent, cell_coordinate aRows, cell_coordinate aColumns, neogfx::alignment aAlignment) :
        layout{ aParent, aAlignment }, iDimensions{ aColumns, aRows }, iRowLayout{ *this, aAlignment }
    {
        init();
    }

    grid_layout::grid_layout(i_layout& aParent, cell_coordinate aRows, cell_coordinate aColumns, neogfx::alignment aAlignment) :
        layout{ aParent, aAlignment }, iDimensions(aColumns, aRows), iRowLayout{ *this, aAlignment }
    {
        init();
    }

    grid_layout::~grid_layout()
    {
        set_destroying();
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

    grid_layout& grid_layout::set_dimensions(cell_coordinate aRows, cell_coordinate aColumns)
    {
        iDimensions = cell_dimensions{aColumns, aRows};
        return *this;
    }

    bool grid_layout::is_item_at_position(cell_coordinate aRow, cell_coordinate aColumn) const
    {
        return iCells.find(cell_coordinates{ aColumn, aRow }) != iCells.end();
    }

    i_layout_item& grid_layout::add(i_layout_item& aItem)
    {
        add_item_at_position(iCursor.y, iCursor.x, aItem);
        return aItem;
    }

    i_layout_item& grid_layout::add(i_ref_ptr<i_layout_item> const& aItem)
    {
        add_item_at_position(iCursor.y, iCursor.x, aItem);
        return *aItem;
    }

    i_layout_item& grid_layout::add_item_at_position(cell_coordinate aRow, cell_coordinate aColumn, i_layout_item& aItem)
    {
        return add_item_at_position(aRow, aColumn, ref_ptr<i_layout_item>{ref_ptr<i_layout_item>{}, &aItem});
    }

    i_layout_item& grid_layout::add_item_at_position(cell_coordinate aRow, cell_coordinate aColumn, i_ref_ptr<i_layout_item> const& aItem)
    {
        if (&*aItem == &iRowLayout)
        {
            iRowLayout.set_parent_layout(this);
            if (has_parent_widget())
                iRowLayout.set_parent_widget(&parent_widget());
            return iRowLayout;
        }
        if (aItem->has_parent_layout() && &aItem->parent_layout() == this)
            throw item_already_added();
        if (iCells.find(cell_coordinates{aColumn, aRow}) != iCells.end())
            remove_item_at_position(aRow, aColumn);
        for (cell_coordinate col = 0; col < aColumn; ++col)
            if (iCells.find(cell_coordinates{ col, aRow }) == iCells.end())
                add_spacer_at_position(aRow, col);
        auto& proxy = *layout::find_item(layout::add(aItem));
        iCells[cell_coordinates{ aColumn, aRow }] = proxy.ptr();
        iDimensions.cy = std::max(iDimensions.cy, aRow + 1);
        iDimensions.cx = std::max(iDimensions.cx, aColumn + 1);
        row_layout(aRow).replace_item_at(aColumn, *proxy);
        if (cursor() == cell_coordinates{ aColumn, aRow })
            increment_cursor();
        return *aItem;
    }

    i_spacer& grid_layout::add_spacer()
    {
        auto s = make_ref<spacer>(expansion_policy::ExpandHorizontally | expansion_policy::ExpandVertically);
        add_item_at_position(iCursor.y, iCursor.x, s);
        increment_cursor();
        return *s;
    }

    i_spacer& grid_layout::add_spacer_at(layout_item_index aPosition)
    {
        auto s = make_ref<spacer>(expansion_policy::ExpandHorizontally | expansion_policy::ExpandVertically);
        cell_coordinates oldCursor = iCursor;
        iCursor.y = 0;
        iCursor.x = 0;
        while (aPosition--)
            increment_cursor();
        add_item_at_position(iCursor.y, iCursor.x, s);
        iCursor = oldCursor;
        return *s;
    }

    i_spacer& grid_layout::add_spacer_at_position(cell_coordinate aRow, cell_coordinate aColumn)
    {
        if (iCells.find(cell_coordinates{ aColumn, aRow }) != iCells.end())
            remove_item_at_position(aRow, aColumn);
        auto s = make_ref<spacer>(expansion_policy::ExpandHorizontally | expansion_policy::ExpandVertically);
        add_item_at_position(aRow, aColumn, s);
        return *s;
    }

    void grid_layout::remove_item_at_position(cell_coordinate aRow, cell_coordinate aColumn)
    {
        auto iterExistingCell = iCells.find(cell_coordinates{ aColumn, aRow });
        if (iterExistingCell == iCells.end())
            throw cell_unoccupied();
        layout::remove(*iterExistingCell->second);
    }

    grid_layout::cell_coordinates grid_layout::item_position(const i_layout_item& aItem) const
    {
        for (auto i = iCells.begin(); i != iCells.end(); ++i)
            if (&static_cast<i_layout_item_cache const&>(*i->second).subject() == &aItem)
                return cell_coordinates{ i->first.x, i->first.y };
        throw item_not_found();
    }

    i_layout_item const& grid_layout::item_at_position(cell_coordinate aRow, cell_coordinate aColumn) const
    {
        auto itemIter = iCells.find(cell_coordinates{ aColumn, aRow });
        if (itemIter == iCells.end())
            throw cell_unoccupied();
        return *(*itemIter).second;
    }

    i_layout_item& grid_layout::item_at_position(cell_coordinate aRow, cell_coordinate aColumn)
    {
        return const_cast<i_layout_item&>(const_cast<grid_layout const&>(*this).item_at_position(aRow, aColumn));
    }

    i_widget const& grid_layout::widget_at_position(cell_coordinate aRow, cell_coordinate aColumn) const
    {
        auto& item = item_at_position(aRow, aColumn);
        if (item.is_widget())
            return item.as_widget();
        throw not_a_widget();
    }

    i_widget& grid_layout::widget_at_position(cell_coordinate aRow, cell_coordinate aColumn)
    {
        return const_cast<i_widget&>(const_cast<grid_layout const&>(*this).widget_at_position(aRow, aColumn));
    }

    i_layout const& grid_layout::layout_at_position(cell_coordinate aRow, cell_coordinate aColumn) const
    {
        auto& item = item_at_position(aRow, aColumn);
        if (item.is_layout())
            return item.as_layout();
        throw not_a_layout();
    }

    i_layout& grid_layout::layout_at_position(cell_coordinate aRow, cell_coordinate aColumn)
    {
        return const_cast<i_layout&>(const_cast<grid_layout const&>(*this).layout_at_position(aRow, aColumn));
    }

    void grid_layout::remove_all()
    {
        layout::remove_all();
        iRowLayout.remove_all();
        iRows.clear();
        iCells.clear();
        iDimensions = {};
        iCursor = {};
        iSpans.clear();
    }

    void grid_layout::invalidate(bool aDeferLayout)
    {
        if (!is_alive())
            return;
        iRowLayout.invalidate(aDeferLayout);
        for (auto& row : iRows)
            row->invalidate(aDeferLayout);
        layout::invalidate(aDeferLayout);
    }

    size grid_layout::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (items_visible() == 0)
            return size{};
        auto availableSpaceForChildren = aAvailableSpace;
        if (availableSpaceForChildren != std::nullopt)
            *availableSpaceForChildren -= internal_spacing().size();
        size result;
        uint32_t visibleColumns = visible_columns();
        uint32_t visibleRows = visible_rows();
        for (cell_coordinate row = 0; row < rows(); ++row)
        {
            if (!is_row_visible(row))
                continue;
            result.cy += row_minimum_size(row, availableSpaceForChildren);
        }
        for (cell_coordinate column = 0; column < columns(); ++column)
        {
            if (!is_column_visible(column))
                continue;
            result.cx += column_minimum_size(column, availableSpaceForChildren);
        }
        result.cx += internal_spacing().size().cx;
        result.cy += internal_spacing().size().cy;
        if (result.cx != size::max_dimension() && visibleColumns> 0)
            result.cx += (spacing().cx * (visibleColumns - 1));
        if (result.cy != size::max_dimension() && visibleRows > 0)
            result.cy += (spacing().cy * (visibleRows - 1));
        result.cx = std::max(result.cx, layout::minimum_size(aAvailableSpace).cx);
        result.cy = std::max(result.cy, layout::minimum_size(aAvailableSpace).cy);
        return result;
    }

    size grid_layout::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
            return size{};
        auto availableSpaceForChildren = aAvailableSpace;
        if (availableSpaceForChildren != std::nullopt)
            *availableSpaceForChildren -= internal_spacing().size();
        size result;
        for (cell_coordinate row = 0; row < visible_rows(); ++row)
        {
            size::dimension_type rowMaxSize = row_maximum_size(row, availableSpaceForChildren);
            if (rowMaxSize == size::max_dimension())
                result.cy = rowMaxSize;
            else if (result.cy != size::max_dimension())
                result.cy += rowMaxSize;
        }
        for (cell_coordinate column = 0; column < visible_columns(); ++column)
        {
            size::dimension_type columnMaxSize = column_maximum_size(column, availableSpaceForChildren);
            if (columnMaxSize == size::max_dimension())
                result.cx = columnMaxSize;
            else if (result.cx != size::max_dimension())
                result.cx += columnMaxSize;
        }
        if (result.cx != size::max_dimension())
            result.cx += internal_spacing().size().cx;
        if (result.cy != size::max_dimension())
            result.cy += internal_spacing().size().cy;
        if (result.cx != size::max_dimension() && visible_columns() > 0)
            result.cx += (spacing().cx * (visible_columns() - 1));
        if (result.cy != size::max_dimension() && visible_rows() > 0)
            result.cy += (spacing().cy * (visible_rows() - 1));
        if (result.cx != size::max_dimension())
            result.cx = std::min(result.cx, layout::maximum_size(aAvailableSpace).cx);
        if (result.cy != size::max_dimension())
            result.cy = std::min(result.cy, layout::maximum_size(aAvailableSpace).cy);
        return result;
    }

    void grid_layout::invalidate_combined_transformation()
    {
        iRowLayout.invalidate_combined_transformation();
        for (auto& row : iRows)
            row->invalidate_combined_transformation();
        layout::invalidate_combined_transformation();
    }

    void grid_layout::set_spacing(optional_size const& aSpacing, bool aUpdateLayout)
    {
        layout::set_spacing(aSpacing, false);
        iRowLayout.set_spacing(aSpacing, false);
        for (auto& r : iRows)
            r->set_spacing(aSpacing, false);
        if (aUpdateLayout)
            update_layout();
    }

    grid_layout& grid_layout::add_span(cell_coordinate aRowFrom, cell_coordinate aColumnFrom, uint32_t aRows, uint32_t aColumns)
    {
        add_span({ aColumnFrom, aRowFrom }, { aColumnFrom + aColumns - 1, aRowFrom + aRows - 1});
        return *this;
    }

    grid_layout& grid_layout::add_span(const cell_coordinates& aFrom, const cell_coordinates& aTo)
    {
        iSpans.push_back(std::make_pair(aFrom, aTo));
        update_layout();
        return *this;
    }

    void grid_layout::set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout)
    {
        layout::set_alignment(aAlignment, aUpdateLayout);
        iRowLayout.set_alignment(alignment(), aUpdateLayout);
        for (auto& r : iRows)
            r->set_alignment(alignment(), aUpdateLayout);
    }

    void grid_layout::layout_items(const point& aPosition, const size& aSize)
    {
        if (!enabled())
            return;
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "grid_layout::layout_items(" << aPosition << ", " << aSize << ")" << endl;
#endif // NEOGFX_DEBUG
        if (has_parent_widget())
            parent_widget().layout_items_started();
        scoped_layout_items layoutItems;
        validate();
        set_position(aPosition);
        set_extents(aSize);
        for (auto& r : iRows)
            while (r->count() < iDimensions.cx)
                r->add_spacer();
        point availablePos = aPosition + internal_spacing().top_left();
        size availableSize = aSize;
        availableSize.cx -= internal_spacing().size().cx;
        availableSize.cy -= internal_spacing().size().cy;
        iRowLayout.layout_items(availablePos, availableSize);

        struct stack_entry
        {
            std::vector<dimension> maxRowHeight;
            std::vector<dimension> maxColWidth;
        };
        typedef std::vector<std::unique_ptr<stack_entry>> calc_stack_t;
        thread_local calc_stack_t stack;
        thread_local std::size_t stackIndex;
        neolib::scoped_counter<std::size_t> stackCounter{ stackIndex };
        if (stack.size() < stackIndex)
            stack.push_back(std::make_unique<stack_entry>());
        auto& maxRowHeight = stack[stackIndex - 1]->maxRowHeight;
        auto& maxColWidth = stack[stackIndex - 1]->maxColWidth;
        maxRowHeight.clear();
        maxColWidth.clear();
        maxRowHeight.resize(iDimensions.cy);
        maxColWidth.resize(iDimensions.cx);

        bool first = true;
        for (cell_coordinate row = 0; row < iDimensions.cy; ++row)
        {
            if (!is_row_visible(row))
                continue;
            if (first)
            {
                first = false;
                availablePos.y = row_layout(row).position().y;
            }
            for (cell_coordinate col = 0; col < iDimensions.cx; ++col)
            {
                if (!is_column_visible(col))
                    continue;
                auto i = iCells.find(cell_coordinates{ col, row });
                if (i != iCells.end())
                {
                    auto s = find_span(cell_coordinates{ col, row });
                    if (s == iSpans.end() || s->first.y == s->second.y)
                        maxRowHeight[row] = std::max(maxRowHeight[row], i->second->extents().cy);
                    if (s == iSpans.end() || s->first.x == s->second.x)
                        maxColWidth[col] = std::max(maxColWidth[col], i->second->extents().cx);
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
                                        i->second->layout_as(fromPos, size{ toPos - fromPos });
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
                        i->second->layout_as(colPos, size{maxColWidth[col], maxRowHeight[row]});
                }
                colPos.x += maxColWidth[col];
                colPos.x += spacing().cx;
            }
            rowPos.y += maxRowHeight[row];
            rowPos.y += spacing().cy;
        }
        if (has_parent_widget())
            parent_widget().layout_items_completed();
        LayoutCompleted.trigger();
    }

    const grid_layout::cell_coordinates& grid_layout::cursor() const
    {
        return iCursor;
    }

    void grid_layout::remove(item_list::iterator aItem)
    {
        auto& item = (**aItem).subject();
        auto itemPos = item_position(item);
        row_layout(itemPos.y).remove_at(itemPos.x);
        if (itemPos.x < row_layout(itemPos.y).count())
            row_layout(itemPos.y).add_spacer_at(itemPos.x);
        iCells.erase(itemPos);
        iDimensions = cell_dimensions{};
        for (auto const& cell : iCells)
        {
            iDimensions.cy = std::max(iDimensions.cy, cell.first.y);
            iDimensions.cx = std::max(iDimensions.cx, cell.first.x);
        }
        iCursor = cell_coordinates{};
        layout::remove(aItem);
        if (count() == 0)
        {
            auto temp = iRows.back();
            iRows.pop_back();
        }
    }

    uint32_t grid_layout::visible_rows() const
    {
        uint32_t result = 0;
        for (cell_coordinate row = 0; row < iDimensions.cy; ++row)
            for (cell_coordinate col = 0; col < iDimensions.cx; ++col)
            {
                auto i = iCells.find(cell_coordinates{col, row});
                if (i != iCells.end() && i->second->visible() && i->second->minimum_size().cy != 0.0)
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
            if (i != iCells.end() && i->second->visible() && i->second->minimum_size().cy != 0.0)
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
                if (i != iCells.end() && i->second->visible() && i->second->minimum_size().cx != 0.0)
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
            if (i != iCells.end() && i->second->visible() && i->second->minimum_size().cx != 0.0)
                return true;
        }
        return false;
    }

    size::dimension_type grid_layout::row_minimum_size(cell_coordinate aRow, optional_size const& aAvailableSpace) const
    {
        size::dimension_type result {};
        for (auto const& item : iCells)
            if (item.first.y == aRow)
            {
                auto s = find_span(item.first);
                if (s == iSpans.end() || s->first.y == s->second.y)
                    result = std::max(result, item.second->minimum_size(aAvailableSpace).cy);
            }
        return std::ceil(result);
    }

    size::dimension_type grid_layout::column_minimum_size(cell_coordinate aColumn, optional_size const& aAvailableSpace) const
    {
        size::dimension_type result {};
        for (auto const& item : iCells)
            if (item.first.x == aColumn)
            {
                auto s = find_span(item.first);
                if (s == iSpans.end() || s->first.x == s->second.x)
                    result = std::max(result, item.second->minimum_size(aAvailableSpace).cx);
            }
        return std::ceil(result);
    }

    size::dimension_type grid_layout::row_maximum_size(cell_coordinate aRow, optional_size const& aAvailableSpace) const
    {
        size::dimension_type result {};
        for (auto const& item : iCells)
            if (item.first.y == aRow)
                result = std::max(result, item.second->maximum_size(aAvailableSpace).cy);
        return result;
    }

    size::dimension_type grid_layout::column_maximum_size(cell_coordinate aColumn, optional_size const& aAvailableSpace) const
    {
        size::dimension_type result {};
        for (auto const& item : iCells)
            if (item.first.x == aColumn)
                result = std::max(result, item.second->maximum_size(aAvailableSpace).cx);
        return result;
    }

    void grid_layout::increment_cursor()
    {
        if (iRows.empty())
            return;
        auto iterSpan = find_span(iCursor);
        do
        {
            ++iCursor.x;
            if (iCursor.x >= columns())
            {
                ++iCursor.y;
                iCursor.x = 0;
            }
        } while (iterSpan != iSpans.end() && iterSpan == find_span(iCursor));
    }

    horizontal_layout& grid_layout::row_layout(cell_coordinate aRow)
    {
        while (aRow >= iRows.size())
        {
            iRows.push_back(make_ref<horizontal_layout>(iRowLayout));
            iRows.back()->set_always_use_spacing(true);
            iRows.back()->set_padding(neogfx::padding{});
            iRows.back()->set_spacing(spacing());
            iRows.back()->set_alignment(alignment());
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

    void grid_layout::init()
    {
        iRowLayout.set_padding(neogfx::padding{});
        iRowLayout.set_spacing(spacing());
        iRowLayout.set_always_use_spacing(true);
        iRowLayout.set_size_policy(neogfx::size_constraint::Expanding);

        set_alive();
        invalidate();
    }
}