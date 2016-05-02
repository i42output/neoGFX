// item_presentation_model.hpp
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
#include "graphics_context.hpp"
#include "app.hpp"
#include "i_item_presentation_model.hpp"
#include "i_item_model.hpp"

namespace neogfx
{
	class item_presentation_model : public i_item_presentation_model, private i_item_model_subscriber
	{
	private:
		typedef boost::optional<i_scrollbar::value_type> optional_position;
	public:
		item_presentation_model() : iItemModel(0)
		{
		}
		~item_presentation_model()
		{
			if (has_item_model())
				item_model().unsubscribe(*this);
		}
	public:
		virtual bool has_item_model() const
		{
			return iItemModel != 0;
		}
		virtual i_item_model& item_model() const
		{
			if (iItemModel == 0)
				throw no_item_model();
			return *iItemModel;
		}
		virtual void set_item_model(i_item_model& aItemModel)
		{
			if (iItemModel == &aItemModel)
				return;
			iItemModel = &aItemModel;
			item_model().subscribe(*this);
			reset_position_meta(0);
		}
	public:
		virtual dimension item_height(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const
		{
			dimension height = 0.0;
			for (uint32_t col = 0; col < item_model().columns(aIndex.row()); ++col)
			{
				optional_font cellFont = cell_font(aIndex);
				if (cellFont == boost::none && iFont != font())
				{
					reset_meta();
					iFont = font();
				}
				if (item_model().cell_meta(aIndex).extents != boost::none)
					height = std::max(height, units_converter(aGraphicsContext).from_device_units(*item_model().cell_meta(aIndex).extents).cy);
				else
				{
					std::string cellString = cell_to_string(item_model_index(aIndex.row(), col));
					const font& effectiveFont = (cellFont == boost::none ? iFont : *cellFont);
					height = std::max(height, units_converter(aGraphicsContext).from_device_units(size(0.0, std::ceil(effectiveFont.height()))).cy *
						(1 + std::count(cellString.begin(), cellString.end(), '\n')));
				}
			}
			return height;
		}
		virtual double total_height(const graphics_context& aGraphicsContext) const
		{
			if (iTotalHeight != boost::none)
				return *iTotalHeight;
			i_scrollbar::value_type height = 0.0;
			for (item_model_index::value_type row = 0; row < item_model().rows(); ++row)
				height += item_height(item_model_index(row, 0), aGraphicsContext);
			iTotalHeight = height;
			return *iTotalHeight;
		}
		virtual double item_position(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const
		{
			if (iPositions[aIndex.row()] == boost::none)
			{
				auto pred = [](const optional_position& lhs, const optional_position& rhs) -> bool
				{
					if (lhs == boost::none && rhs == boost::none)
						return false;
					else if (lhs != boost::none && rhs == boost::none)
						return true;
					else if (lhs == boost::none && rhs != boost::none)
						return false;
					else
						return *lhs < *rhs;
				};
				auto i = std::lower_bound(iPositions.begin(), iPositions.end(), optional_position(), pred);
				uint32_t row = std::distance(iPositions.begin(), i);
				double position = (row == 0 ? 0.0 : **(std::prev(i)) + item_height(item_model_index(row - 1), aGraphicsContext));
				while (row <= aIndex.row())
				{
					iPositions[row] = position;
					position += item_height(item_model_index(row), aGraphicsContext);
					++row;
				}
			}
			return *iPositions[aIndex.row()];
		}
		virtual std::pair<item_model_index::value_type, coordinate> item_at(double aPosition, const graphics_context& aGraphicsContext) const
		{
			if (item_model().rows() == 0)
				return std::pair<item_model_index::value_type, coordinate>(0, 0.0);
			auto pred = [](const optional_position& lhs, const optional_position& rhs) -> bool
			{
				if (lhs == boost::none && rhs == boost::none)
					return false;
				else if (lhs != boost::none && rhs == boost::none)
					return true;
				else if (lhs == boost::none && rhs != boost::none)
					return false;
				else
					return *lhs < *rhs;
			};
			auto i = std::lower_bound(iPositions.begin(), iPositions.end(), aPosition, pred);
			if (*i == boost::none)
			{
				auto j = std::lower_bound(iPositions.begin(), iPositions.end(), optional_position(), pred);
				uint32_t row = std::distance(iPositions.begin(), j);
				double position = (row == 0 ? 0.0 : **(std::prev(j)) + item_height(item_model_index(row - 1), aGraphicsContext));
				while (row < iPositions.size())
				{
					iPositions[row] = position;
					position += item_height(item_model_index(row), aGraphicsContext);
					++row;
				}
				i = std::lower_bound(iPositions.begin(), iPositions.end(), aPosition, pred);
			}
			if (i == iPositions.end() || *i > aPosition && i != iPositions.begin())
				--i;
			return std::pair<item_model_index::value_type, coordinate>(std::distance(iPositions.begin(), i), static_cast<coordinate>(**i - aPosition));
		}
		virtual std::string cell_to_string(const item_model_index& aIndex) const
		{
			switch (item_model().cell_data(aIndex).which())
			{
			case 0:
				return "";
			case i_item_model::cell_data_type::type_id<bool>::value:
				return (cell_format(aIndex) % static_variant_cast<bool>(item_model().cell_data(aIndex))).str();
			case i_item_model::cell_data_type::type_id<int32_t>::value:
				return (cell_format(aIndex) % static_variant_cast<int32_t>(item_model().cell_data(aIndex))).str();
			case i_item_model::cell_data_type::type_id<uint32_t>::value:
				return (cell_format(aIndex) % static_variant_cast<uint32_t>(item_model().cell_data(aIndex))).str();
			case i_item_model::cell_data_type::type_id<int64_t>::value:
				return (cell_format(aIndex) % static_variant_cast<int64_t>(item_model().cell_data(aIndex))).str();
			case i_item_model::cell_data_type::type_id<uint64_t>::value:
				return (cell_format(aIndex) % static_variant_cast<uint64_t>(item_model().cell_data(aIndex))).str();
			case i_item_model::cell_data_type::type_id<float>::value:
				return (cell_format(aIndex) % static_variant_cast<float>(item_model().cell_data(aIndex))).str();
			case i_item_model::cell_data_type::type_id<double>::value:
				return (cell_format(aIndex) % static_variant_cast<double>(item_model().cell_data(aIndex))).str();
			case i_item_model::cell_data_type::type_id<std::string>::value:
				return (cell_format(aIndex) % static_variant_cast<const std::string&>(item_model().cell_data(aIndex))).str();
			default:
				return "";
			}
		}
		virtual boost::basic_format<char> cell_format(const item_model_index&) const
		{
			static const boost::basic_format<char> sDefaultFormat("%1%");
			return sDefaultFormat;
		}
		virtual optional_colour cell_colour(const item_model_index&, colour_type_e) const
		{
			return optional_colour();
		}
		virtual optional_font cell_font(const item_model_index&) const
		{
			return optional_font();
		}
		virtual neogfx::glyph_text& cell_glyph_text(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const
		{
			optional_font cellFont = cell_font(aIndex);
			if (cellFont == boost::none && iFont != font())
			{
				reset_meta();
				iFont = font();
			}
			if (item_model().cell_meta(aIndex).text != boost::none)
				return *item_model().cell_meta(aIndex).text;
			item_model().cell_meta(aIndex).text = aGraphicsContext.to_glyph_text(cell_to_string(aIndex), cellFont == boost::none ? iFont : *cellFont);
			return *item_model().cell_meta(aIndex).text;
		}
		virtual size cell_extents(const item_model_index& aIndex, const graphics_context& aGraphicsContext) const
		{
			auto oldItemHeight = item_height(aIndex, aGraphicsContext);
			optional_font cellFont = cell_font(aIndex);
			if (cellFont == boost::none && iFont != font())
			{
				reset_meta();
				iFont = font();
			}
			if (item_model().cell_meta(aIndex).extents != boost::none)
				return units_converter(aGraphicsContext).from_device_units(*item_model().cell_meta(aIndex).extents);
			size cellExtents = cell_glyph_text(aIndex, aGraphicsContext).extents();
			item_model().cell_meta(aIndex).extents = units_converter(aGraphicsContext).to_device_units(cellExtents);
			item_model().cell_meta(aIndex).extents->cx = std::ceil(item_model().cell_meta(aIndex).extents->cx);
			item_model().cell_meta(aIndex).extents->cy = std::ceil(item_model().cell_meta(aIndex).extents->cy);
			if (iTotalHeight != boost::none)
				*iTotalHeight += (item_height(aIndex, aGraphicsContext) - oldItemHeight);
			return units_converter(aGraphicsContext).from_device_units(*item_model().cell_meta(aIndex).extents);
		}
	private:
		virtual void column_info_changed(const i_item_model&, item_model_index::value_type)
		{
		}
		virtual void item_added(const i_item_model&, const item_model_index& aItemIndex)
		{
			reset_position_meta(aItemIndex.row());
		}
		virtual void item_changed(const i_item_model&, const item_model_index& aItemIndex)
		{
			reset_position_meta(aItemIndex.row());
		}
		virtual void item_removed(const i_item_model&, const item_model_index&)
		{
		}
		virtual void items_sorted(const i_item_model&)
		{
			reset_position_meta(0);
		}
		virtual void model_destroyed(const i_item_model&)
		{
			iItemModel = 0;
		}
	private:
		void reset_meta() const
		{
			for (uint32_t row = 0; row < item_model().rows(); ++row)
			{
				for (uint32_t col = 0; col < item_model().columns(item_model_index(row)); ++col)
				{
					item_model().cell_meta(item_model_index(row, col)).text = boost::none;
					item_model().cell_meta(item_model_index(row, col)).extents = boost::none;
				}
			}
		}
		void reset_position_meta(item_model_index::value_type aFromRow) const
		{
			iTotalHeight = boost::none;
			iPositions.resize(item_model().rows());
			for (std::size_t i = aFromRow; i < iPositions.size() && iPositions[i] != boost::none; ++i)
				iPositions[i] = boost::none;
		}
	private:
		i_item_model* iItemModel;
		mutable font iFont;
		mutable boost::optional<i_scrollbar::value_type> iTotalHeight;
		mutable neolib::segmented_array<optional_position, 256> iPositions;
	};
}