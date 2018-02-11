// item_selection.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
#include <neogfx/core/event.hpp>
#include "item_index.hpp"

namespace neogfx
{
	class item_selection
	{
	public:
		typedef std::vector<item_index> index_list;
		class range
		{
		public:
			range(const item_index& aStart, const item_index& aEnd) :
				iStart(aStart), iEnd(aEnd)
			{
			}
		public:
			bool operator<(const range& aRhs) const
			{
				return iStart < aRhs.iStart;
			}
		public:
			const item_index& start() const
			{
				return iStart;
			}
			item_index& start()
			{
				return iStart;
			}
			const item_index& end() const
			{
				return iEnd;
			}
			item_index& end()
			{
				return iEnd;
			}
		private:
			item_index iStart;
			item_index iEnd;
		};
		typedef std::vector<range> range_list;
	public:
		item_selection()
		{
		}
		item_selection(const item_index& aIndex)
		{
			iSelections.push_back(range(aIndex, aIndex));
		}
		item_selection(const range& aRange)
		{
			iSelections.push_back(aRange);
		}
		item_selection(const range_list& aSelections) :
			iSelections(aSelections)
		{
			sort();
		}
		item_selection(const index_list& aIndexes)
		{
			auto sorted = aIndexes;
			std::sort(sorted.begin(), sorted.end());
			for (const auto& i : sorted)
			{
				if (iSelections.empty())
					iSelections.push_back(range(i, i));
				else
				{
					if (iSelections.back().end().row() == i.row() && iSelections.back().end().column() == i.column() - 1)
						iSelections.back().end().set_column(i.column());
					else if (iSelections.back().end().row() == i.row() - 1 && iSelections.back().start().column() == i.column())
					{
						iSelections.back().end().set_row(i.row());
						iSelections.back().end().set_column(i.column());
					}
					else
						iSelections.push_back(range(i, i));
				}
			}
		}
	public:
		const range_list& selections() const
		{
			return iSelections;
		}
		index_list indexes() const
		{
			index_list result;
			for (const auto& s : iSelections)
			{
				for (auto r = s.start().row(); r <= s.end().row(); ++r)
				{
					for (auto c = s.start().column(); c <= s.start().column(); ++c)
					{
						result.push_back(item_index(r, c));
					}
				}
			}
			return result;
		}
		void add(const item_index& aIndex)
		{
			auto i = find(aIndex);
			if (i != iSelections.begin() && std::prev(i)->end().row() == aIndex.row() && std::prev(i)->end().column() == aIndex.column() - 1)
				std::prev(i)->end().set_column(aIndex.column());
			else if (i == iSelections.end())
				iSelections.push_back(range(aIndex, aIndex));
			else if (aIndex < i->start())
				iSelections.insert(i, range(aIndex, aIndex));
			else if (i != iSelections.begin() && aIndex < std::prev(i)->end() && (aIndex.column() < std::prev(i)->start().column() || aIndex.column() > std::prev(i)->end().column()))
				iSelections.insert(i, range(aIndex, aIndex));
		}
		void add(const range& aRange)
		{
			iSelections.push_back(aRange);
			sort();
		}
		void add(const range_list& aSelections)
		{
			iSelections.insert(iSelections.end(), aSelections.begin(), aSelections.end());
			sort();
		}	
	private:
		range_list::iterator find(const item_index& aIndex)
		{
			return std::lower_bound(iSelections.begin(), iSelections.end(), range_list::value_type(aIndex, aIndex), [](const range_list::value_type& lhs, const range_list::value_type& rhs) -> bool
			{
				return lhs.start() < rhs.start();
			});
		}
		void sort()
		{
			std::sort(iSelections.begin(), iSelections.end(), [](const range_list::value_type& lhs, const range_list::value_type& rhs) -> bool
			{
				return lhs.start() < rhs.start();
			});
		}
	private:
		range_list iSelections;
	};
}