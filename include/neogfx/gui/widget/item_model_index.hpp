// item_model_index.hpp
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

#include <neogfx/neogfx.hpp>
#include <tuple>
#include <boost/optional.hpp>

namespace neogfx
{
	class item_model_index
	{
	public:
		typedef uint32_t value_type;
	public:
		item_model_index(value_type aRow) :
			iRow(aRow), iColumn(0)
		{
		}
		item_model_index(value_type aRow, value_type aColumn) :
			iRow(aRow), iColumn(aColumn)
		{
		}
	public:
		value_type row() const
		{
			return iRow;
		}
		void set_row(value_type aRow)
		{
			iRow = aRow;
		}
		value_type column() const
		{
			return iColumn;
		}
		void set_column(value_type aColumn)
		{
			iColumn = aColumn;
		}
	private:
		value_type iRow;
		value_type iColumn;
	};

	inline bool operator==(const item_model_index& aLhs, const item_model_index& aRhs)
	{
		return aLhs.row() == aRhs.row() && aLhs.column() == aRhs.column();
	}

	inline bool operator!=(const item_model_index& aLhs, const item_model_index& aRhs)
	{
		return !(aLhs == aRhs);
	}

	inline bool operator<(const item_model_index& aLhs, const item_model_index& aRhs)
	{
		return std::forward_as_tuple(aLhs.row(), aLhs.column()) < std::forward_as_tuple(aRhs.row(), aRhs.column());
	}

	inline bool operator>(const item_model_index& aLhs, const item_model_index& aRhs)
	{
		return std::forward_as_tuple(aLhs.row(), aLhs.column()) > std::forward_as_tuple(aRhs.row(), aRhs.column());
	}

	typedef boost::optional<item_model_index> optional_item_model_index;
}