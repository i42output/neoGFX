// default_item_model.hpp
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
#include <vector>
#include <deque>
#include <boost/algorithm/string.hpp>
#include <neolib/vecarray.hpp>
#include <neolib/segmented_array.hpp>
#include <neolib/observable.hpp>
#include "i_basic_item_model.hpp"
#include "graphics_context.hpp"
#include "app.hpp"

namespace neogfx
{
	template <uint32_t Columns, typename Alloc>
	struct row_container_type
	{
		typedef Alloc allocator_type;
		typedef i_item_model::cell_type cell_type;
		typedef neolib::vecarray<cell_type, Columns, Columns, neolib::check<neolib::vecarray_overflow>, typename allocator_type::template rebind<cell_type>::other> type;
	};

	template <typename Alloc>
	struct row_container_type<0, Alloc>
	{
		typedef Alloc allocator_type;
		typedef i_item_model::cell_type cell_type;
		typedef std::vector<cell_type, typename allocator_type::template rebind<cell_type>::other> type;
	};

	template <typename T, uint32_t Columns = 0, template <class, class> class Container = std::vector, typename Alloc = std::allocator<T> >
	struct default_item_model_types
	{
		typedef T value_type;
		typedef Alloc allocator_type;
		typedef typename row_container_type<Columns, Alloc>::type row_container_type;
		typedef std::pair<value_type, row_container_type> row_type;
		typedef Container<row_type, typename allocator_type::template rebind<row_type>::other> container_type;
		typedef typename container_type::iterator sibling_iterator;
		typedef typename container_type::const_iterator const_sibling_iterator;
	};

	template <class Container>
	struct container_helper
	{
		struct operation_not_supported : std::logic_error { operation_not_supported() : std::logic_error("neogfx::container_helper::operation_not_supported") {} };
		template <typename T>
		static typename Container::iterator append(Container& aContainer, typename Container::const_iterator aParent, const T& aValue)
		{
			throw operation_not_supported();
		}
		static i_item_model::iterator sibling_begin(Container& aContainer)
		{
			throw operation_not_supported();
		}
		static i_item_model::const_iterator sibling_begin(const Container& aContainer)
		{
			throw operation_not_supported();
		}
		static i_item_model::iterator sibling_end(Container& aContainer)
		{
			throw operation_not_supported();
		}
		static i_item_model::const_iterator sibling_end(const Container& aContainer)
		{
			throw operation_not_supported();
		}
		static i_item_model::iterator parent(Container& aContainer, i_item_model::iterator aChild)
		{
			throw operation_not_supported();
		}
		static i_item_model::const_iterator parent(const Container& aContainer, i_item_model::const_iterator aChild)
		{
			throw operation_not_supported();
		}
		static i_item_model::iterator sibling_begin(Container& aContainer, i_item_model::iterator aParent)
		{
			throw operation_not_supported();
		}
		static i_item_model::const_iterator sibling_begin(const Container& aContainer, i_item_model::const_iterator aParent)
		{
			throw operation_not_supported();
		}
		static i_item_model::iterator sibling_end(Container& aContainer, i_item_model::iterator aParent)
		{
			throw operation_not_supported();
		}
		static i_item_model::const_iterator sibling_end(const Container& aContainer, i_item_model::const_iterator aParent)
		{
			throw operation_not_supported();
		}
	};

	template <typename T, uint32_t Columns = 0, template <class, class> class Container = std::vector, typename Alloc = std::allocator<T> >
	class basic_default_item_model : public i_basic_item_model<T>, private neolib::observable<i_item_model_subscriber>
	{
	public:
		typedef typename default_item_model_types<T, Columns, Container, Alloc>::value_type value_type;
		typedef typename default_item_model_types<T, Columns, Container, Alloc>::allocator_type allocator_type;
		typedef typename default_item_model_types<T, Columns, Container, Alloc>::row_container_type row_container_type;
		typedef typename default_item_model_types<T, Columns, Container, Alloc>::container_type container_type;
		typedef typename container_type::value_type row_type;
		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;
		typedef neolib::specialized_generic_iterator<iterator> base_iterator;
		typedef neolib::specialized_generic_iterator<const_iterator> const_base_iterator;
		typedef typename default_item_model_types<T, Columns, Container, Alloc>::sibling_iterator sibling_iterator;
		typedef typename default_item_model_types<T, Columns, Container, Alloc>::const_sibling_iterator const_sibling_iterator;
		typedef typename row_type::second_type::iterator column_iterator;
		typedef typename row_type::second_type::const_iterator const_column_iterator;
	private:
		struct column_info
		{
			column_info() : count(0) {}
			uint32_t count;
			std::string headingText;
			mutable font headingFont;
			mutable optional_size extents;
		};
	public:
		~basic_default_item_model()
		{
			notify_observers(i_item_model_subscriber::NotifyModelDestroyed);
		}
	public:
		virtual uint32_t rows() const
		{
			return iItems.size();
		}
		virtual uint32_t columns() const
		{
			return iColumns.size();
		}
		virtual uint32_t columns(const item_model_index& aIndex) const
		{
			return iItems[aIndex.row()].second.size();
		}
		virtual const std::string& column_heading_text(item_model_index::value_type aColumnIndex) const
		{
			if (iColumns.size() < aColumnIndex + 1)
				throw bad_column_index();
			return iColumns[aColumnIndex].headingText;
		}
		virtual size column_heading_extents(item_model_index::value_type aColumnIndex, const graphics_context& aGraphicsContext) const
		{
			if (iColumns.size() < aColumnIndex + 1)
				throw bad_column_index();
			if (iColumns[aColumnIndex].headingFont != font())
			{
				iColumns[aColumnIndex].headingFont = font();
				iColumns[aColumnIndex].extents = boost::none;
			}
			if (iColumns[aColumnIndex].extents != boost::none)
				return units_converter(aGraphicsContext).from_device_units(*iColumns[aColumnIndex].extents);
			size columnHeadingExtents = aGraphicsContext.text_extent(column_heading_text(aColumnIndex), iColumns[aColumnIndex].headingFont);
			iColumns[aColumnIndex].extents = units_converter(aGraphicsContext).to_device_units(columnHeadingExtents);
			iColumns[aColumnIndex].extents->cx = std::ceil(iColumns[aColumnIndex].extents->cx);
			iColumns[aColumnIndex].extents->cy = std::ceil(iColumns[aColumnIndex].extents->cy);
			return units_converter(aGraphicsContext).from_device_units(*iColumns[aColumnIndex].extents);
		}
		virtual void set_column_heading_text(item_model_index::value_type aColumnIndex, const std::string& aHeadingText)
		{
			if (iColumns.size() < aColumnIndex + 1)
				iColumns.resize(aColumnIndex + 1);
			iColumns[aColumnIndex].headingText = aHeadingText;
			iColumns[aColumnIndex].extents = boost::none;
			notify_observers(i_item_model_subscriber::NotifyColumnInfoChanged, aColumnIndex);
		}
		virtual i_item_model::iterator index_to_iterator(const item_model_index& aIndex)
		{
			return base_iterator(std::next(iItems.begin(), aIndex.row()));
		}
		virtual i_item_model::const_iterator index_to_iterator(const item_model_index& aIndex) const
		{
			return const_base_iterator(std::next(iItems.begin(), aIndex.row()));
		}
		virtual item_model_index iterator_to_index(i_item_model::const_iterator aPosition) const
		{
			return item_model_index(std::distance(iItems.begin(), const_base_iterator(aPosition).get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>()), 0);
		}
		virtual i_item_model::iterator begin()
		{
			return base_iterator(iItems.begin());
		}
		virtual i_item_model::const_iterator begin() const
		{
			return const_base_iterator(iItems.begin());
		}
		virtual i_item_model::iterator end()
		{
			return base_iterator(iItems.end());
		}
		virtual i_item_model::const_iterator end() const
		{
			return const_base_iterator(iItems.end());
		}
		virtual i_item_model::iterator sibling_begin()
		{
			return base_iterator(container_helper<container_type>::sibling_begin(iItems));
		}
		virtual i_item_model::const_iterator sibling_begin() const
		{
			return const_base_iterator(container_helper<container_type>::sibling_begin(iItems));
		}
		virtual i_item_model::iterator sibling_end()
		{
			return base_iterator(container_helper<container_type>::sibling_end(iItems));
		}
		virtual i_item_model::const_iterator sibling_end() const
		{
			return const_base_iterator(container_helper<container_type>::sibling_end(iItems));
		}
		virtual i_item_model::iterator parent(i_item_model::iterator aChild)
		{
			return base_iterator(container_helper<container_type>::parent(iItems, aChild));
		}
		virtual i_item_model::const_iterator parent(i_item_model::const_iterator aChild) const
		{
			return const_base_iterator(container_helper<container_type>::parent(iItems, aChild));
		}
		virtual i_item_model::iterator sibling_begin(i_item_model::iterator aParent)
		{
			return base_iterator(container_helper<container_type>::sibling_begin(iItems, aParent));
		}
		virtual i_item_model::const_iterator sibling_begin(i_item_model::const_iterator aParent) const
		{
			return const_base_iterator(container_helper<container_type>::sibling_begin(iItems, aParent));
		}
		virtual i_item_model::iterator sibling_end(i_item_model::iterator aParent)
		{
			return base_iterator(container_helper<container_type>::sibling_end(iItems, aParent));
		}
		virtual i_item_model::const_iterator sibling_end(i_item_model::const_iterator aParent) const
		{
			return const_base_iterator(container_helper<container_type>::sibling_end(iItems, aParent));
		}
	public:
		virtual void subscribe(i_item_model_subscriber& aSubscriber)
		{
			add_observer(aSubscriber);
		}
		virtual void unsubscribe(i_item_model_subscriber& aSubscriber)
		{
			remove_observer(aSubscriber);
		}
	public:
		virtual const cell_data_type& cell_data(const item_model_index& aIndex) const
		{
			return iItems[aIndex.row()].second[aIndex.column()].first;
		}
		virtual const i_item_presentation_model::cell_meta_type& cell_meta(const item_model_index& aIndex) const
		{
			return iItems[aIndex.row()].second[aIndex.column()].second;
		}
	public:
		virtual void reserve(uint32_t aItemCount)
		{
			iItems.reserve(aItemCount);
		}
		virtual uint32_t capacity() const
		{
			return iItems.capacity();
		}
		virtual i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const value_type& aValue)
		{
			base_iterator i = iItems.insert(aPosition.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>(), row_type(aValue, row_container_type()));
			notify_observers(i_item_model_subscriber::NotifyItemAdded, iterator_to_index(i));
			return i;
		}
		virtual i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const value_type& aValue, const cell_data_type& aCellData)
		{
			auto i = insert_item(aPosition, aValue);
			insert_cell_data(i, 0, aCellData);
			return i;
		}
		virtual i_item_model::iterator insert_item(i_item_model::const_iterator aPosition, const cell_data_type& aCellData)
		{
			return insert_item(aPosition, value_type(), aCellData);
		}
		virtual i_item_model::iterator insert_item(const item_model_index& aIndex, const cell_data_type& aCellData)
		{
			return insert_item(index_to_iterator(aIndex), aCellData);
		}
		virtual i_item_model::iterator append_item(i_item_model::const_iterator aParent, const value_type& aValue)
		{
			base_iterator i = container_helper<container_type>::append(iItems, aParent.get<const_iterator, const_iterator, iterator, const_sibling_iterator, sibling_iterator>(), row_type(aValue, row_container_type()));
			notify_observers(i_item_model_subscriber::NotifyItemAdded, iterator_to_index(i));
			return i;
		}
		virtual i_item_model::iterator append_item(i_item_model::const_iterator aParent, const value_type& aValue, const cell_data_type& aCellData)
		{
			auto i = append_item(aParent, aValue);
			insert_cell_data(i, 0, aCellData);
			return i;
		}
		virtual i_item_model::iterator append_item(i_item_model::const_iterator aParent, const cell_data_type& aCellData)
		{
			return append_item(aParent, value_type(), aCellData);
		}
		virtual void insert_cell_data(i_item_model::iterator aItem, item_model_index::value_type aColumnIndex, const cell_data_type& aCellData)
		{
			auto ri = aItem.get<iterator, iterator, sibling_iterator>();
			if (ri->second.size() < aColumnIndex + 1)
				ri->second.resize(aColumnIndex + 1);
			ri->second[aColumnIndex] = std::make_pair(aCellData, i_item_presentation_model::cell_meta_type());
			if (iColumns.size() < aColumnIndex + 1)
				iColumns.resize(aColumnIndex + 1);
			++iColumns[aColumnIndex].count;
			item_model_index index = iterator_to_index(aItem);
			index.set_column(aColumnIndex);
			notify_observers(i_item_model_subscriber::NotifyItemChanged, index);
		}
		virtual void insert_cell_data(const item_model_index& aIndex, const cell_data_type& aCellData)
		{
			insert_cell_data(index_to_iterator(aIndex), aIndex.column(), aCellData);
		}
		virtual void update_cell_data(const item_model_index& aIndex, const cell_data_type& aCellData)
		{
			iItems[aIndex.row()].second[aIndex.column()].first = aCellData;
			iItems[aIndex.row()].second[aIndex.column()].second = i_item_presentation_model::cell_meta_type();
			notify_observers(i_item_model_subscriber::NotifyItemChanged, aIndex);
		}
	public:
		virtual value_type& item(const item_model_index& aIndex)
		{
			return iItems[aIndex.row()].first;
		}
		virtual const value_type& item(const item_model_index& aIndex) const
		{
			return iItems[aIndex.row()].first;
		}
	public:
		virtual optional_sort_order sorting_by() const
		{
			if (!iSortOrder.empty())
				return iSortOrder.front();
			else
				return optional_sort_order();
		}
		virtual void sort_by(item_model_index::value_type aColumnIndex, const optional_sort_direction& aSortDirection = optional_sort_direction())
		{
			iSortOrder.push_front(sort_order(aColumnIndex, aSortDirection == boost::none ? SortAscending : *aSortDirection));
			for (auto i = std::next(iSortOrder.begin()); i != iSortOrder.end(); ++i)
			{
				if (i->first == aColumnIndex)
				{
					if (aSortDirection == boost::none && i == std::next(iSortOrder.begin()))
						iSortOrder.front().second = (i->second == SortAscending ? SortDescending : SortAscending);
					iSortOrder.erase(i);
					break;
				}
			}
			sort();
		}
		virtual void reset_sort()
		{
			iSortOrder.clear();
		}
	private:
		void sort()
		{
			std::sort(iItems.begin(), iItems.end(), [&](const container_type::value_type& aLhs, const container_type::value_type& aRhs) -> bool
			{
				for (std::size_t i = 0; i < iSortOrder.size(); ++i)
				{
					auto col = iSortOrder[i].first;
					const auto& v1 = aLhs.second[col].first;
					const auto& v2 = aRhs.second[col].first;
					if (v1.is<std::string>())
					{
						std::string s1 = boost::to_upper_copy<std::string>(v1);
						std::string s2 = boost::to_upper_copy<std::string>(v2);
						if (s1 < s2)
							return iSortOrder[i].second == SortAscending;
						else if (s2 < s1)
							return iSortOrder[i].second == SortDescending;
					}
					if (v1 < v2)
						return iSortOrder[i].second == SortAscending;
					else if (v2 < v1)
						return iSortOrder[i].second == SortDescending;
				}
				return false;
			});
			notify_observers(i_item_model_subscriber::NotifyItemsSorted);
		}
	private:
		virtual void notify_observer(i_item_model_subscriber& aObserver, i_item_model_subscriber::notify_type aType, const void* aParameter, const void* aParameter2)
		{
			switch (aType)
			{
			case i_item_model_subscriber::NotifyColumnInfoChanged:
				aObserver.column_info_changed(*this, *static_cast<const item_model_index::value_type*>(aParameter));
				break;
			case i_item_model_subscriber::NotifyItemAdded:
				aObserver.item_added(*this, *static_cast<const item_model_index*>(aParameter));
				break;
			case i_item_model_subscriber::NotifyItemChanged:
				aObserver.item_changed(*this, *static_cast<const item_model_index*>(aParameter));
				break;
			case i_item_model_subscriber::NotifyItemRemoved:
				aObserver.item_removed(*this, *static_cast<const item_model_index*>(aParameter));
				break;
			case i_item_model_subscriber::NotifyItemsSorted:
				aObserver.items_sorted(*this);
				break;
			case i_item_model_subscriber::NotifyModelDestroyed:
				aObserver.model_destroyed(*this);
				break;
			}
		}
	private:
		container_type iItems;
		std::vector<column_info> iColumns;
		std::deque<sort_order> iSortOrder;
	};

	typedef basic_default_item_model<i_default_item_model::value_type> default_item_model;
}