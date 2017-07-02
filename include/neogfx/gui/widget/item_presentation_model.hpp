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

#include <neogfx/neogfx.hpp>
#include <vector>
#include <deque>
#include <boost/algorithm/string.hpp>
#include <neolib/vecarray.hpp>
#include <neolib/segmented_array.hpp>
#include <neolib/observable.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/app/app.hpp>
#include "item_model.hpp"
#include "i_item_presentation_model.hpp"

namespace neogfx
{
	template <typename Model>
	class basic_item_presentation_model : public i_item_presentation_model, private i_item_model_subscriber
	{
	private:
		typedef typename Model::container_traits::template rebind<item_presentation_model_index::row_type, cell_meta_type>::other container_traits;
		typedef boost::optional<i_scrollbar::value_type> optional_position;
	private:
		struct column_info
		{
			column_info(item_model_index::column_type aModelColumn) : modelColumn{ aModelColumn }
			item_model_index::column_type modelColumn;
			mutable boost::optional<std::string> headingText;
			mutable font headingFont;
			mutable optional_size extents;
		};
	public:
		basic_item_presentation_model() : iItemModel(0)
		{
			iSink = app::instance().rendering_engine().subpixel_rendering_changed([this]()
			{
				reset_meta();
			});
		}
		~basic_item_presentation_model()
		{
			if (has_item_model())
				item_model().unsubscribe(*this);
		}
	public:
		bool has_item_model() const override
		{
			return iItemModel != 0;
		}
		i_item_model& item_model() const override
		{
			if (iItemModel == 0)
				throw no_item_model();
			return *iItemModel;
		}
		void set_item_model(i_item_model& aItemModel) override
		{
			if (iItemModel == &aItemModel)
				return;
			iItemModel = &aItemModel;
			item_model().subscribe(*this);
			reset_position_meta(0);
		}
	public:
		const std::string& column_heading_text(item_presentation_model_index::column_type aColumnIndex) const override
		{
			if (iColumns.size() < aColumnIndex + 1)
				throw bad_column_index();
			if (iColumns[aColumnIndex].headingText != boost::none)
				return *iColumns[aColumnIndex].headingText;
			else
				return item_model().column_name(iColumns[aColumnIndex].modelColumn);
		}
		size column_heading_extents(item_presentation_model_index::column_type aColumnIndex, const graphics_context& aGraphicsContext) const override
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
		void set_column_heading_text(item_presentation_model_index::column_type aColumnIndex, const std::string& aHeadingText) override
		{
			if (iColumns.size() < aColumnIndex + 1)
				iColumns.resize(aColumnIndex + 1);
			iColumns[aColumnIndex].headingText = aHeadingText;
			iColumns[aColumnIndex].extents = boost::none;
			notify_observers(i_item_model_subscriber::NotifyColumnInfoChanged, aColumnIndex);
		}
	public:
		dimension item_height(const item_presentation_model_index& aIndex, const graphics_context& aGraphicsContext) const override
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
					std::string cellString = cell_to_string(item_presentation_model_index(aIndex.row(), col));
					const font& effectiveFont = (cellFont == boost::none ? iFont : *cellFont);
					height = std::max(height, units_converter(aGraphicsContext).from_device_units(size(0.0, std::ceil(effectiveFont.height()))).cy *
						(1 + std::count(cellString.begin(), cellString.end(), '\n')));
				}
			}
			return height;
		}
		double total_height(const graphics_context& aGraphicsContext) const override
		{
			if (iTotalHeight != boost::none)
				return *iTotalHeight;
			i_scrollbar::value_type height = 0.0;
			for (item_presentation_model_index::row_type row = 0; row < item_model().rows(); ++row)
				height += item_height(item_presentation_model_index(row, 0), aGraphicsContext);
			iTotalHeight = height;
			return *iTotalHeight;
		}
		double item_position(const item_presentation_model_index& aIndex, const graphics_context& aGraphicsContext) const override
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
				double position = (row == 0 ? 0.0 : **(std::prev(i)) + item_height(item_presentation_model_index(row - 1), aGraphicsContext));
				while (row <= aIndex.row())
				{
					iPositions[row] = position;
					position += item_height(item_presentation_model_index(row), aGraphicsContext);
					++row;
				}
			}
			return *iPositions[aIndex.row()];
		}
		std::pair<item_presentation_model_index::row_type, coordinate> item_at(double aPosition, const graphics_context& aGraphicsContext) const override
		{
			if (item_model().rows() == 0)
				return std::pair<item_presentation_model_index::row_type, coordinate>(0, 0.0);
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
				double position = (row == 0 ? 0.0 : **(std::prev(j)) + item_height(item_presentation_model_index(row - 1), aGraphicsContext));
				while (row < iPositions.size())
				{
					iPositions[row] = position;
					position += item_height(item_presentation_model_index(row), aGraphicsContext);
					++row;
				}
				i = std::lower_bound(iPositions.begin(), iPositions.end(), aPosition, pred);
			}
			if (i == iPositions.end() || *i > aPosition && i != iPositions.begin())
				--i;
			return std::pair<item_presentation_model_index::row_type, coordinate>(std::distance(iPositions.begin(), i), static_cast<coordinate>(**i - aPosition));
		}
		std::string cell_to_string(const item_presentation_model_index& aIndex) const override
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
		boost::basic_format<char> cell_format(const item_presentation_model_index&) const override
		{
			static const boost::basic_format<char> sDefaultFormat("%1%");
			return sDefaultFormat;
		}
		optional_colour cell_colour(const item_presentation_model_index&, colour_type_e) const override
		{
			return optional_colour();
		}
		optional_font cell_font(const item_presentation_model_index&) const override
		{
			return optional_font();
		}
		neogfx::glyph_text& cell_glyph_text(const item_presentation_model_index& aIndex, const graphics_context& aGraphicsContext) const override
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
		size cell_extents(const item_presentation_model_index& aIndex, const graphics_context& aGraphicsContext) const override
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
	public:
		optional_sort_order sorting_by() const override
		{
			if (!iSortOrder.empty())
				return iSortOrder.front();
			else
				return optional_sort_order();
		}
		void sort_by(item_presentation_model_index::column_type aColumnIndex, const optional_sort_direction& aSortDirection = optional_sort_direction()) override
		{
			iSortOrder.push_front(sort_order(aColumnIndex, aSortDirection == boost::none ? SortAscending : *aSortDirection));
			for (auto i = std::next(iSortOrder.begin()); i != iSortOrder.end(); ++i)
			{
				if (i->first == aColumnIndex)
				{
					if (aSortDirection == boost::none)
					{
						if (i == std::next(iSortOrder.begin()))
							iSortOrder.front().second = (i->second == SortAscending ? SortDescending : SortAscending);
						else
							iSortOrder.front().second = i->second;
					}
					iSortOrder.erase(i);
					break;
				}
			}
			sort();
		}
		void reset_sort() override
		{
			iSortOrder.clear();
			sort_by(0, SortAscending);
		}
	private:
		void sort() override
		{
			if (iSortOrder.empty())
				return;
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
		void column_info_changed(const i_item_model&, item_presentation_model_index::column_type) override
		{
		}
		void item_added(const i_item_model&, const item_presentation_model_index& aItemIndex) override
		{
			reset_position_meta(aItemIndex.row());
		}
		void item_changed(const i_item_model&, const item_presentation_model_index& aItemIndex) override
		{
			reset_position_meta(aItemIndex.row());
		}
		void item_removed(const i_item_model&, const item_presentation_model_index&) override
		{
		}
		void items_sorted(const i_item_model&) override
		{
			reset_position_meta(0);
		}
		void model_destroyed(const i_item_model&) override
		{
			iItemModel = 0;
		}
	private:
		void reset_meta() const
		{
			for (uint32_t row = 0; row < item_model().rows(); ++row)
			{
				for (uint32_t col = 0; col < item_model().columns(item_presentation_model_index(row)); ++col)
				{
					item_model().cell_meta(item_presentation_model_index(row, col)).text = boost::none;
					item_model().cell_meta(item_presentation_model_index(row, col)).extents = boost::none;
				}
			}
		}
		void reset_position_meta(item_presentation_model_index::row_type aFromRow) const
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
		std::deque<sort_order> iSortOrder;
		sink iSink;
	};

	typedef basic_item_presentation_model<item_model> item_presentation_model;
}