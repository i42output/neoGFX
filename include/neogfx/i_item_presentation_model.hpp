// i_item_presentation_model.hpp
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
#include <boost/format.hpp>
#include "colour.hpp"
#include "font.hpp"
#include "text.hpp"
#include "item_model_index.hpp"

namespace neogfx
{
	class graphics_context;

	class i_item_model;

	class i_item_presentation_model
	{
	public:
		struct cell_meta_type
		{
			mutable optional_glyph_text text;
			mutable optional_size extents;
			mutable enum class selection_flags
			{
				Current			= 0x01,
				Selected		= 0x02
			} selection;
		};
		enum colour_type_e
		{
			ForegroundColour = 0x01,
			BackgroundColour = 0x02,
		};
	public:
		struct no_item_model : std::logic_error { no_item_model() : std::logic_error("neogfx::i_item_presentation_model::no_item_model") {} };
	public:
		virtual ~i_item_presentation_model() {}
	public:
		virtual bool has_item_model() const = 0;
		virtual i_item_model& item_model() const = 0;
		virtual void set_item_model(i_item_model& aItemModel) = 0;
	public:
		virtual dimension item_height(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const = 0;
		virtual double total_height(const graphics_context& aGraphicsContext) const = 0;
		virtual double item_position(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const = 0;
		virtual std::pair<item_model_index::value_type, coordinate> item_at(double aPosition, const graphics_context& aGraphicsContext) const = 0;
	public:
		virtual std::string cell_to_string(const item_model_index& aIndex) const = 0;
		virtual boost::basic_format<char> cell_format(const item_model_index& aIndex) const = 0;
		virtual optional_colour cell_colour(const item_model_index& aIndex, colour_type_e aColourType) const = 0;
		virtual optional_font cell_font(const item_model_index& aIndex) const = 0;
		virtual neogfx::glyph_text& cell_glyph_text(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const = 0;
		virtual size cell_extents(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const = 0;
	};

	inline i_item_presentation_model::cell_meta_type::selection_flags operator|(i_item_presentation_model::cell_meta_type::selection_flags aLhs, i_item_presentation_model::cell_meta_type::selection_flags aRhs)
	{
		return static_cast<i_item_presentation_model::cell_meta_type::selection_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline i_item_presentation_model::cell_meta_type::selection_flags operator&(i_item_presentation_model::cell_meta_type::selection_flags aLhs, i_item_presentation_model::cell_meta_type::selection_flags aRhs)
	{
		return static_cast<i_item_presentation_model::cell_meta_type::selection_flags>(static_cast<uint32_t>(aLhs)& static_cast<uint32_t>(aRhs));
	}

	inline i_item_presentation_model::cell_meta_type::selection_flags operator~(i_item_presentation_model::cell_meta_type::selection_flags aLhs)
	{
		return static_cast<i_item_presentation_model::cell_meta_type::selection_flags>(~static_cast<uint32_t>(aLhs));
	}
}