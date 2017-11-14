// aabb_tree.hpp
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
#include <map>
#include <unordered_map>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/core/numerical.hpp>

namespace neogfx
{
	template <typename T, typename Allocator = boost::fast_pool_allocator<T>>
	class aabb_tree
	{
	public:
		typedef T value_type;
		typedef Allocator allocator_type;
		typedef typename allocator_type::reference reference;
		typedef typename allocator_type::const_reference const_reference;
		typedef typename std::allocator_traits<Allocator>::pointer pointer;
		typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;
	private:
		typedef std::multimap<aabb, pointer, std::less<aabb>, typename allocator_type::template rebind<std::pair<const aabb, pointer>>::other> item_list;
		typedef std::unordered_map<pointer, typename item_list::iterator, std::hash<pointer>, std::equal_to<pointer>, typename allocator_type::template rebind<std::pair<const pointer, typename item_list::iterator>>::other> reverse_item_map;
	public:
		typedef typename item_list::iterator iterator;
		typedef typename item_list::const_iterator const_iterator;
	private:
		class node
		{
		public:
			node(aabb_tree& aTree) : iParent{ nullptr }, iLeft{ nullptr }, iRight{ nullptr }, iAabb{}, iItem{ aTree.end() }
			{
			}
		public:
			bool is_leaf() const
			{
				return iLeft == nullptr;
			}
			void set_branch(node* aLeft, node* aRight)
			{
				iLeft->iParent = this;
				iRight->iParent = this;
				iLeft = aLeft;
				iRight = aRIght;
			}
			void set_leaf(const neogfx::aabb& aAabb, pointer aData)
			{
				iAabb = aAabb;
				iData = aData;
				iLeft = nullptr;
				iRight = nullptr;
			}
			void update(float aMultiplier, float aMinimum)
			{
				if (is_leaf())
				{
					// make fat AABB
					const vec3 marginVec{ ((aabb().max - aabb().min) * aMultiplier).max(vec3{ aMinimum, aMinimum, aMinimum }) };
					iAabb = neogfx::aabb{ aabb().min - marginVec, aabb().max + marginVec };
				}
				else // make union of child AABBs of child nodes
					iAabb = aabb_union(iLeft->aabb(), iRight->aabb());
			}
			node* sibling() const
			{
				return this == iParent->iLeft ? iParent->iRight : iParent->iLeft;
			}
			const neogfx::aabb& aabb() const
			{
				return iAabb;
			}
			pointer data() const
			{
				return iData;
			}
		private:
			node* iParent;
			node* iLeft;
			node* iRight;
			neogfx::aabb iAabb;
			iterator iItem;
		};
	public:
		aabb_tree()
		{
		}
	public:
		const_iterator begin() const
		{
			return iItems.begin();
		}
		const_iterator cbegin() const
		{
			return iItems.begin();
		}
		iterator begin()
		{
			return iItems.begin();
		}
		const_iterator end() const
		{
			return iItems.end();
		}
		const_iterator cend() const
		{
			return iItems.end();
		}
		iterator end()
		{
			return iItems.end();
		}
	public:
		const_iterator find(reference aItem) const
		{
			return iReverseItemMap.find(&aItem)->second;
		}
		const_iterator find(const vec3& aPoint) const
		{
			// todo
			return iItems.end();
		}
		const_iterator find(const aabb& aAabb) const
		{
			return iItems.find(aAabb);
		}
		iterator find(reference aItem)
		{
			return iReverseItemMap.find(&aItem)->second;
		}
		iterator find(const vec3& aPoint)
		{
			// todo
			return iItems.end();
		}
		iterator find(const aabb& aAabb)
		{
			return iItems.find(aAabb);
		}
	public:
		iterator insert(reference aItem, const aabb& aAabb)
		{
			auto newItem = iItems.insert(std::make_pair(aAabb, &aItem));
			iReverseItemMap[&aItem] = newItem;
			return newItem;
		}
		void update(const_reference aItem, const aabb& aNewAabb)
		{
			// todo
		}
		void erase(iterator aItem)
		{
			// todo
			iReverseItemMap.erase(aItem->second);
			iItems.erase(aItem);
		}
	private:
		item_list iItems;
		reverse_item_map iReverseItemMap;
	};
}