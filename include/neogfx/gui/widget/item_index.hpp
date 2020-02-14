// item_index.hpp
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
#include <tuple>
#include <optional>

namespace neogfx
{
    template <typename ModelIndexType>
    class item_index
    {
    public:
        typedef uint32_t value_type;
        typedef value_type row_type;
        typedef value_type column_type;
        typedef std::optional<row_type> optional_row_type;
        typedef std::optional<column_type> optional_column_type;
    public:
        item_index() :
            iRow{ 0 }, iColumn{ 0 }
        {
        }
        item_index(row_type aRow) :
            iRow{ aRow }, iColumn{ 0 }
        {
        }
        item_index(row_type aRow, column_type aColumn) :
            iRow{ aRow }, iColumn{ aColumn }
        {
        }
    public:
        ModelIndexType& operator+=(const item_index& aRhs)
        {
            iRow += aRhs.iRow;
            iColumn += aRhs.iColumn;
            return static_cast<ModelIndexType&>(*this);
        }
        ModelIndexType& operator-=(const item_index& aRhs)
        {
            iRow -= aRhs.iRow;
            iColumn -= aRhs.iColumn;
            return static_cast<ModelIndexType&>(*this);
        }
    public:
        row_type row() const
        {
            return iRow;
        }
        void set_row(row_type aRow)
        {
            iRow = aRow;
        }
        column_type column() const
        {
            return iColumn;
        }
        void set_column(column_type aColumn)
        {
            iColumn = aColumn;
        }
    private:
        row_type iRow;
        column_type iColumn;
    };

    template <typename ModelIndexType>
    inline bool operator==(const item_index<ModelIndexType>& aLhs, const item_index<ModelIndexType>& aRhs)
    {
        return aLhs.row() == aRhs.row() && aLhs.column() == aRhs.column();
    }

    template <typename ModelIndexType>
    inline bool operator!=(const item_index<ModelIndexType>& aLhs, const item_index<ModelIndexType>& aRhs)
    {
        return !(aLhs == aRhs);
    }

    template <typename ModelIndexType>
    inline bool operator<(const item_index<ModelIndexType>& aLhs, const item_index<ModelIndexType>& aRhs)
    {
        return std::forward_as_tuple(aLhs.row(), aLhs.column()) < std::forward_as_tuple(aRhs.row(), aRhs.column());
    }

    template <typename ModelIndexType>
    inline bool operator>(const item_index<ModelIndexType>& aLhs, const item_index<ModelIndexType>& aRhs)
    {
        return std::forward_as_tuple(aLhs.row(), aLhs.column()) > std::forward_as_tuple(aRhs.row(), aRhs.column());
    }

    template <typename ModelIndexType>
    using optional_item_index = std::optional<item_index<ModelIndexType>> ;
}