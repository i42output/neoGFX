// i_item_presentation_model.hpp
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
#include <boost/format.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gfx/text/glyph.hpp>
#include "item_index.hpp"
#include "i_item_model.hpp"

namespace neogfx
{
	class graphics_context;

	class i_item_presentation_model;

	typedef item_index item_presentation_model_index;
	typedef optional_item_index optional_item_presentation_model_index;

	class i_item_presentation_model_subscriber
	{
	public:
		virtual void column_info_changed(const i_item_presentation_model& aModel, item_presentation_model_index::column_type aColumnIndex) = 0;
		virtual void item_model_changed(const i_item_presentation_model& aModel, const i_item_model& aItemModel) = 0;
		virtual void item_added(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) = 0;
		virtual void item_changed(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) = 0;
		virtual void item_removed(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) = 0;
		virtual void items_sorting(const i_item_presentation_model& aModel) = 0;
		virtual void items_sorted(const i_item_presentation_model& aModel) = 0;
		virtual void items_filtering(const i_item_presentation_model& aModel) = 0;
		virtual void items_filtered(const i_item_presentation_model& aModel) = 0;
		virtual void model_destroyed(const i_item_presentation_model& aModel) = 0;
	public:
		enum notify_type { NotifyColumnInfoChanged, NotifyItemModelChanged, NotifyItemAdded, NotifyItemChanged, NotifyItemRemoved, NotifyItemsSorting, NotifyItemsSorted, NotifyItemsFiltering, NotifyItemsFiltered, NotifyModelDestroyed };
	};

	enum class item_cell_selection_flags
	{
		Current = 0x01,
		Selected = 0x02
	};

	enum class item_cell_editable
	{
		No,
		WhenFocused,
		OnInputEvent
	};

	enum class item_cell_colour_type
	{
		Foreground = 0x01,
		Background = 0x02,
	};
	typedef item_cell_colour_type item_cell_color_type;

	inline item_cell_selection_flags operator|(item_cell_selection_flags aLhs, item_cell_selection_flags aRhs)
	{
		return static_cast<item_cell_selection_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline item_cell_selection_flags operator&(item_cell_selection_flags aLhs, item_cell_selection_flags aRhs)
	{
		return static_cast<item_cell_selection_flags>(static_cast<uint32_t>(aLhs)& static_cast<uint32_t>(aRhs));
	}

	inline item_cell_selection_flags operator~(item_cell_selection_flags aLhs)
	{
		return static_cast<item_cell_selection_flags>(~static_cast<uint32_t>(aLhs));
	}

	class i_item_presentation_model
	{
	public:
		struct cell_meta_type
		{
			mutable item_cell_selection_flags selection;
			mutable optional_texture texture;
			mutable optional_glyph_text text;
			mutable optional_size extents;
		};
		enum sort_direction_e
		{
			SortAscending,
			SortDescending,
		};
		typedef std::optional<sort_direction_e> optional_sort_direction;
		typedef std::pair<item_presentation_model_index::column_type, sort_direction_e> sort;
		typedef std::optional<sort> optional_sort;
		typedef std::string filter_search_key;
		enum filter_search_type_e
		{
			Prefix,
			Glob,
			Regex
		};
		enum case_sensitivity_e
		{
			CaseInsensitive,
			CaseSensitive
		};
		typedef std::tuple<item_presentation_model_index::column_type, filter_search_key, filter_search_type_e, case_sensitivity_e> filter;
		typedef std::optional<filter> optional_filter;
	public:
		struct no_item_model : std::logic_error { no_item_model() : std::logic_error("neogfx::i_item_presentation_model::no_item_model") {} };
		struct bad_column_index : std::logic_error { bad_column_index() : std::logic_error("neogfx::i_item_presentation_model::bad_column_index") {} };
		struct bad_item_model_index : std::logic_error { bad_item_model_index() : std::logic_error("neogfx::i_item_presentation_model::bad_item_model_index") {} };
	public:
		virtual ~i_item_presentation_model() {}
	public:
		virtual bool initializing() const = 0;
		virtual bool has_item_model() const = 0;
		virtual i_item_model& item_model() const = 0;
		virtual void set_item_model(i_item_model& aItemModel) = 0;
		virtual item_model_index to_item_model_index(const item_presentation_model_index& aIndex) const = 0;
		virtual bool have_item_model_index(const item_model_index& aIndex) const = 0;
		virtual item_presentation_model_index from_item_model_index(const item_model_index& aIndex) const = 0;
	public:
		virtual uint32_t rows() const = 0;
		virtual uint32_t columns() const = 0;
		virtual uint32_t columns(const item_presentation_model_index& aIndex) const = 0;
		virtual dimension column_width(item_presentation_model_index::value_type aColumnIndex, const graphics_context& aGraphicsContext, bool aIncludeMargins = true) const = 0;
		virtual const std::string& column_heading_text(item_presentation_model_index::value_type aColumnIndex) const = 0;
		virtual size column_heading_extents(item_presentation_model_index::value_type aColumnIndex, const graphics_context& aGraphicsContext) const = 0;
		virtual void set_column_heading_text(item_presentation_model_index::value_type aColumnIndex, const std::string& aHeadingText) = 0;
		virtual item_cell_editable column_editable(item_presentation_model_index::value_type aColumnIndex) const = 0;
		virtual void set_column_editable(item_presentation_model_index::value_type aColumnIndex, item_cell_editable aEditable) = 0;
	public:
		virtual const font& default_font() const = 0;
		virtual void set_default_font(const optional_font& aDefaultFont) = 0;
		virtual size cell_spacing(const i_units_context& aUnitsContext) const = 0;
		virtual void set_cell_spacing(const optional_size& aSpacing, const i_units_context& aUnitsContext) = 0;
		virtual neogfx::margins cell_margins(const i_units_context& aUnitsContext) const = 0;
		virtual void set_cell_margins(const optional_margins& aMargins, const i_units_context& aUnitsContext) = 0;
	public:
		virtual dimension item_height(const item_presentation_model_index& aIndex, const i_units_context& aUnitsContext) const = 0;
		virtual double total_height(const i_units_context& aUnitsContext) const = 0;
		virtual double item_position(const item_presentation_model_index& aIndex, const i_units_context& aUnitsContext) const = 0;
		virtual std::pair<item_presentation_model_index::value_type, coordinate> item_at(double aPosition, const i_units_context& aUnitsContext) const = 0;
	public:
		virtual const cell_meta_type& cell_meta(const item_presentation_model_index& aIndex) const = 0;
	public:
		virtual item_cell_editable cell_editable(const item_presentation_model_index& aIndex) const = 0;
		virtual std::string cell_to_string(const item_presentation_model_index& aIndex) const = 0;
		virtual item_cell_data string_to_cell_data(const item_presentation_model_index& aIndex, const std::string& aString) const = 0;
		virtual item_cell_data string_to_cell_data(const item_presentation_model_index& aIndex, const std::string& aString, bool& aError) const = 0;
		virtual boost::basic_format<char> cell_format(const item_presentation_model_index& aIndex) const = 0;
		virtual optional_colour cell_colour(const item_presentation_model_index& aIndex, item_cell_colour_type aColourType) const = 0;
		virtual optional_font cell_font(const item_presentation_model_index& aIndex) const = 0;
		virtual neogfx::glyph_text& cell_glyph_text(const item_presentation_model_index& aIndex, const graphics_context& aGraphicsContext) const = 0;
		virtual size cell_extents(const item_presentation_model_index& aIndex, const graphics_context& aGraphicsContext) const = 0;
	public:
		virtual optional_sort sorting_by() const = 0;
		virtual void sort_by(item_presentation_model_index::column_type aColumnIndex, const optional_sort_direction& aSortDirection = optional_sort_direction{}) = 0;
		virtual void reset_sort() = 0;
	public:
		virtual optional_item_presentation_model_index find_item(const filter_search_key& aFilterSearchKey, item_presentation_model_index::column_type aColumnIndex = 0, filter_search_type_e aFilterSearchType = Prefix, case_sensitivity_e aCaseSensitivity = CaseInsensitive) const = 0;
	public:
		virtual bool filtering() const = 0;
		virtual optional_filter filtering_by() const = 0;
		virtual void filter_by(item_presentation_model_index::column_type aColumnIndex, const filter_search_key& aFilterSearchKey, filter_search_type_e aFilterSearchType = Prefix, case_sensitivity_e aCaseSensitivity = CaseInsensitive) = 0;
		virtual void reset_filter() = 0;
	public:
		virtual void subscribe(i_item_presentation_model_subscriber& aSubscriber) = 0;
		virtual void unsubscribe(i_item_presentation_model_subscriber& aSubscriber) = 0;
	};
}