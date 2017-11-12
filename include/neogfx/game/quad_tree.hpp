// quad_tree.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <unordered_map>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/core/geometry.hpp>

namespace neogfx
{
	template <typename T, typename Allocator = boost::fast_pool_allocator<T>>
	class quad_tree
	{
	public:
		typedef T value_type;
		typedef Allocator allocator_type;
		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
		typedef typename std::allocator_traits<Allocator>::pointer pointer;
		typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;
	private:
		typedef std::unordered_map<pointer, rect, std::hash<pointer>, std::equal_to<pointer>, typename allocator_type::template rebind<std::pair<pointer, rect>>::other> item_list;
	public:
		typedef typename item_list::iterator iterator;
		typedef typename item_list::const_iterator const_iterator;
	public:
		quad_tree(const rect& aSpace = rect{ point{}, size{ 3840.0, 2160.0 } }) : iSpace{ aSpace }
		{
		}
	public:
		const_iterator find(reference aItem) const
		{
			return iItems.find(&aItem);
		}
		const_iterator find(const point& aPoint) const
		{
			// todo
			return iItems.end();
		}
		const_iterator find(const rect& aRect) const
		{
			// todo
			return iItems.end();
		}
		iterator find(reference aItem)
		{
			return iItems.find(&aItem);
		}
		iterator find(const point& aPoint)
		{
			// todo
			return iItems.end();
		}
		iterator find(const rect& aRect)
		{
			// todo
			return iItems.end();
		}
	public:
		iterator insert(reference aItem, const rect& aRect)
		{
			// todo
			return iItems.insert(std::make_pair(&aItem, aRect)).first;
		}
		void update(const_reference aItem, const rect& aNewRect)
		{
			// todo
		}
		void erase(iterator aItem)
		{
			// todo
			iItems.erase(aItem);
		}
	private:
		rect iSpace;
		item_list iItems;
	};
}