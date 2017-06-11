// i_item_model.hpp
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
#include <boost/format.hpp>
#include <neolib/variant.hpp>
#include <neolib/generic_iterator.hpp>
#include "item_model_index.hpp"
#include "i_item_presentation_model.hpp"

namespace neogfx
{
	class i_item_model;

	class i_item_model_subscriber
	{
	public:
		virtual void column_info_changed(const i_item_model& aModel, item_model_index::value_type aColumnIndex) = 0;
		virtual void item_added(const i_item_model& aModel, const item_model_index& aItemIndex) = 0;
		virtual void item_changed(const i_item_model& aModel, const item_model_index& aItemIndex) = 0;
		virtual void item_removed(const i_item_model& aModel, const item_model_index& aItemIndex) = 0;
		virtual void items_sorted(const i_item_model& aModel) = 0;
		virtual void model_destroyed(const i_item_model& aModel) = 0;
	public:
		enum notify_type { NotifyColumnInfoChanged, NotifyItemAdded, NotifyItemChanged, NotifyItemRemoved, NotifyItemsSorted, NotifyModelDestroyed };
	};

	class i_item_model
	{
	public:
		typedef neolib::generic_iterator iterator;
		typedef neolib::generic_iterator const_iterator;
	public:
		template <typename T>
		struct choice_type
		{
			typedef T value_type;
			typedef std::pair<value_type, std::string> option;
			typedef std::vector<option> type;
		};
		typedef neolib::variant<
			bool, 
			int32_t, 
			uint32_t, 
			int64_t, 
			uint64_t, 
			float, 
			double, 
			std::string, 
			choice_type<bool>::type::const_iterator, 
			choice_type<int32_t>::type::const_iterator,
			choice_type<uint32_t>::type::const_iterator,
			choice_type<int64_t>::type::const_iterator,
			choice_type<uint64_t>::type::const_iterator,
			choice_type<float>::type::const_iterator,
			choice_type<double>::type::const_iterator,
			choice_type<std::string>::type::const_iterator> cell_data_type;
		typedef std::pair<cell_data_type, i_item_presentation_model::cell_meta_type> cell_type;
		enum sort_direction_e
		{
			SortAscending,
			SortDescending,
		};
		typedef boost::optional<sort_direction_e> optional_sort_direction;
		typedef std::pair<item_model_index::value_type, sort_direction_e> sort_order;
		typedef boost::optional<sort_order> optional_sort_order;
	public:
		struct bad_column_index : std::logic_error { bad_column_index() : std::logic_error("neogfx::i_item_model::bad_column_index") {} };
	public:
		virtual ~i_item_model() {}
	public:
		virtual uint32_t rows() const = 0;
		virtual uint32_t columns() const = 0;
		virtual uint32_t columns(const item_model_index& aIndex) const = 0;
		virtual const std::string& column_heading_text(item_model_index::value_type aColumnIndex) const = 0;
		virtual size column_heading_extents(item_model_index::value_type aColumnIndex, const graphics_context& aGraphicsContext) const = 0;
		virtual void set_column_heading_text(item_model_index::value_type aColumnIndex, const std::string& aHeadingText) = 0;
		virtual iterator index_to_iterator(const item_model_index& aIndex) = 0;
		virtual const_iterator index_to_iterator(const item_model_index& aIndex) const = 0;
		virtual item_model_index iterator_to_index(const_iterator aPosition) const = 0;
		virtual iterator begin() = 0;
		virtual const_iterator begin() const = 0;
		virtual iterator end() = 0;
		virtual const_iterator end() const = 0;
		virtual iterator sibling_begin() = 0;
		virtual const_iterator sibling_begin() const = 0;
		virtual iterator sibling_end() = 0;
		virtual const_iterator sibling_end() const = 0;
		virtual iterator parent(const_iterator aChild) = 0;
		virtual const_iterator parent(const_iterator aChild) const = 0;
		virtual iterator sibling_begin(const_iterator aParent) = 0;
		virtual const_iterator sibling_begin(const_iterator aParent) const = 0;
		virtual iterator sibling_end(const_iterator aParent) = 0;
		virtual const_iterator sibling_end(const_iterator aParent) const = 0;
	public:
		virtual void reserve(uint32_t aItemCount) = 0;
		virtual uint32_t capacity() const = 0;
		virtual iterator insert_item(const_iterator aPosition, const cell_data_type& aCellData) = 0;
		virtual iterator insert_item(const item_model_index& aIndex, const cell_data_type& aCellData) = 0;
		virtual iterator append_item(const_iterator aParent, const cell_data_type& aCellData) = 0;
		virtual void insert_cell_data(const_iterator aItem, item_model_index::value_type aColumnIndex, const cell_data_type& aCellData) = 0;
		virtual void insert_cell_data(const item_model_index& aIndex, const cell_data_type& aCellData) = 0;
		virtual void update_cell_data(const item_model_index& aIndex, const cell_data_type& aCellData) = 0;
	public:
		virtual optional_sort_order sorting_by() const = 0;
		virtual void sort_by(item_model_index::value_type aColumnIndex, const optional_sort_direction& aSortDirection = optional_sort_direction()) = 0;
		virtual void reset_sort() = 0;
	public:
		virtual const cell_data_type& cell_data(const item_model_index& aIndex) const = 0;
		virtual const i_item_presentation_model::cell_meta_type& cell_meta(const item_model_index& aIndex) const = 0;
	public:
		virtual void subscribe(i_item_model_subscriber& aSubscriber) = 0;
		virtual void unsubscribe(i_item_model_subscriber& aSubscriber) = 0;
	};
}