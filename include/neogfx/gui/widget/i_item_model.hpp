// i_item_model.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
#include "item_index.hpp"

namespace neogfx
{
	class i_item_model;

	typedef item_index item_model_index;
	typedef optional_item_index optional_item_model_index;

	class i_item_model_subscriber
	{
	public:
		virtual void column_info_changed(const i_item_model& aModel, item_model_index::column_type aColumnIndex) = 0;
		virtual void item_added(const i_item_model& aModel, const item_model_index& aItemIndex) = 0;
		virtual void item_changed(const i_item_model& aModel, const item_model_index& aItemIndex) = 0;
		virtual void item_removed(const i_item_model& aModel, const item_model_index& aItemIndex) = 0;
		virtual void model_destroyed(const i_item_model& aModel) = 0;
	public:
		enum notify_type { NotifyColumnInfoChanged, NotifyItemAdded, NotifyItemChanged, NotifyItemRemoved, NotifyModelDestroyed };
	};

	enum item_cell_data_type
	{
		Unknown,
		Custom,
		Bool,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Float,
		Double,
		String,
		ChoiceCustom,
		ChoiceBool,
		ChoiceInt32,
		ChoiceUInt32,
		ChoiceInt64,
		ChoiceUInt16,
		ChoiceFloat,
		ChoiceDouble,
		ChoiceString
	};

	template <typename T>
	struct item_cell_choice_type
	{
		typedef T value_type;
		typedef std::pair<value_type, std::string> option;
		typedef std::vector<option> type;
	};

	typedef neolib::variant<
		void*,
		bool,
		int32_t,
		uint32_t,
		int64_t,
		uint64_t,
		float,
		double,
		std::string,
		item_cell_choice_type<void*>::type::const_iterator,
		item_cell_choice_type<bool>::type::const_iterator,
		item_cell_choice_type<int32_t>::type::const_iterator,
		item_cell_choice_type<uint32_t>::type::const_iterator,
		item_cell_choice_type<int64_t>::type::const_iterator,
		item_cell_choice_type<uint64_t>::type::const_iterator,
		item_cell_choice_type<float>::type::const_iterator,
		item_cell_choice_type<double>::type::const_iterator,
		item_cell_choice_type<std::string>::type::const_iterator> item_cell_data_variant;

	class item_cell_data : public item_cell_data_variant
	{
	public:
		item_cell_data() {}
		item_cell_data(void* aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(bool aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(int32_t aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(uint32_t aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(int64_t aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(uint64_t aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(float aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(double aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(std::string aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<void*>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<bool>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<int32_t>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<uint32_t>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<int64_t>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<uint64_t>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<float>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<double>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
		item_cell_data(item_cell_choice_type<std::string>::type::const_iterator aValue) : item_cell_data_variant{ aValue } {}
	public:
		item_cell_data(const char* aString) : item_cell_data_variant(std::string{ aString }) {}
	public:
		std::string to_string() const
		{
			switch (which())
			{
			case 0:
				return "";
			case item_cell_data::type_id<bool>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<bool>(*this)).str();
			case item_cell_data::type_id<int32_t>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<int32_t>(*this)).str();
			case item_cell_data::type_id<uint32_t>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<uint32_t>(*this)).str();
			case item_cell_data::type_id<int64_t>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<int64_t>(*this)).str();
			case item_cell_data::type_id<uint64_t>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<uint64_t>(*this)).str();
			case item_cell_data::type_id<float>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<float>(*this)).str();
			case item_cell_data::type_id<double>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<double>(*this)).str();
			case item_cell_data::type_id<std::string>::value:
				return (boost::basic_format<char>{"%1%"} % static_variant_cast<const std::string&>(*this)).str();
			default:
				return "";
			}
		}
	};

	struct item_cell_data_info
	{
		bool unselectable;
		bool readOnly;
		item_cell_data_type type;
		item_cell_data min;
		item_cell_data max;
		item_cell_data step;
	};

	typedef boost::optional<item_cell_data_info> optional_item_cell_data_info;

	class i_item_model
	{
	public:
		typedef neolib::generic_iterator iterator;
		typedef neolib::generic_iterator const_iterator;
	public:
		struct bad_column_index : std::logic_error { bad_column_index() : std::logic_error("neogfx::i_item_model::bad_column_index") {} };
	public:
		virtual ~i_item_model() {}
	public:
		virtual uint32_t rows() const = 0;
		virtual uint32_t columns() const = 0;
		virtual uint32_t columns(const item_model_index& aIndex) const = 0;
		virtual const std::string& column_name(item_model_index::column_type aColumnIndex) const = 0;
		virtual void set_column_name(item_model_index::column_type aColumnIndex, const std::string& aName) = 0;
		virtual bool column_selectable(item_model_index::column_type aColumnIndex) const = 0;
		virtual void set_column_selectable(item_model_index::column_type aColumnIndex, bool aSelectable) = 0;
		virtual bool column_read_only(item_model_index::column_type aColumnIndex) const = 0;
		virtual void set_column_read_only(item_model_index::column_type aColumnIndex, bool aReadOnly) = 0;
		virtual item_cell_data_type column_data_type(item_model_index::column_type aColumnIndex) const = 0;
		virtual void set_column_data_type(item_model_index::column_type aColumnIndex, item_cell_data_type aType) = 0;
		virtual const item_cell_data& column_min_value(item_model_index::column_type aColumnIndex) const = 0;
		virtual void set_column_min_value(item_model_index::column_type aColumnIndex, const item_cell_data& aValue) = 0;
		virtual const item_cell_data& column_max_value(item_model_index::column_type aColumnIndex) const = 0;
		virtual void set_column_max_value(item_model_index::column_type aColumnIndex, const item_cell_data& aValue) = 0;
		virtual const item_cell_data& column_step_value(item_model_index::column_type aColumnIndex) const = 0;
		virtual void set_column_step_value(item_model_index::column_type aColumnIndex, const item_cell_data& aValue) = 0;
	public:
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
		virtual iterator insert_item(const_iterator aPosition, const item_cell_data& aCellData) = 0;
		virtual iterator insert_item(const item_model_index& aIndex, const item_cell_data& aCellData) = 0;
		virtual iterator append_item(const_iterator aParent, const item_cell_data& aCellData) = 0;
		virtual void remove(const_iterator aPosition) = 0;
		virtual void insert_cell_data(const_iterator aItem, item_model_index::column_type aColumnIndex, const item_cell_data& aCellData) = 0;
		virtual void insert_cell_data(const item_model_index& aIndex, const item_cell_data& aCellData) = 0;
		virtual void update_cell_data(const item_model_index& aIndex, const item_cell_data& aCellData) = 0;
	public:
		virtual const item_cell_data_info& cell_data_info(const item_model_index& aIndex) const = 0;
		virtual const item_cell_data& cell_data(const item_model_index& aIndex) const = 0;
	public:
		virtual void subscribe(i_item_model_subscriber& aSubscriber) = 0;
		virtual void unsubscribe(i_item_model_subscriber& aSubscriber) = 0;
	};
}