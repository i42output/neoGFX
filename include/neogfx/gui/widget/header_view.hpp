// header_view.hpp
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
#include "splitter.hpp"
#include "i_item_model.hpp"
#include "i_item_presentation_model.hpp"

namespace neogfx
{
	class header_view : public splitter, private i_item_presentation_model_subscriber
	{
	private:
		class updater;
	public:
		enum type_e
		{
			HorizontalHeader,
			VerticalHeader
		};
		class i_owner
		{
		public:
			virtual void header_view_updated(header_view& aHeaderView) = 0;
			virtual neogfx::margins cell_margins() const = 0;
		};
	public:
		header_view(i_owner& aOwner, type_e aType = HorizontalHeader);
		header_view(i_widget& aParent, i_owner& aOwner, type_e aType = HorizontalHeader);
		header_view(i_layout& aLayout, i_owner& aOwner, type_e aType = HorizontalHeader);
		~header_view();
	public:
		bool has_model() const;
		const i_item_model& model() const;
		i_item_model& model();
		void set_model(i_item_model& aModel);
		void set_model(std::shared_ptr<i_item_model> aModel);
		bool has_presentation_model() const;
		const i_item_presentation_model& presentation_model() const;
		i_item_presentation_model& presentation_model();
		void set_presentation_model(i_item_presentation_model& aPresentationModel);
		void set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel);
	public:
		dimension separator_width() const;
		void set_separator_width(const optional_dimension& aWidth);
		uint32_t section_count() const;
		dimension section_width(uint32_t aSectionIndex) const;
		dimension total_width() const;
	public:
		bool can_defer_layout() const override;
		bool is_managing_layout() const override;
	public:
		void panes_resized() override;
		void reset_pane_sizes_requested(const boost::optional<uint32_t>& aPane = boost::optional<uint32_t>()) override;
	private:
		void column_info_changed(const i_item_presentation_model& aModel, item_presentation_model_index::column_type aColumnIndex) override;
		void item_model_changed(const i_item_presentation_model& aModel, const i_item_model& aItemModel) override;
		void item_added(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) override;
		void item_changed(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) override;
		void item_removed(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) override;
		void items_sorted(const i_item_presentation_model& aModel) override;
		void model_destroyed(const i_item_presentation_model& aModel) override;
	private:
		void update_from_row(uint32_t aRow, bool aUpdateOwner, graphics_context& aGc);
	private:
		i_owner& iOwner;
		sink iSink;
		std::vector<sink> iButtonSinks;
		type_e iType;
		std::shared_ptr<i_item_model> iModel;
		std::shared_ptr<i_item_presentation_model> iPresentationModel;
		optional_dimension iSeparatorWidth;
		std::vector<std::pair<optional_dimension, dimension>> iSectionWidths;
		std::unique_ptr<updater> iUpdater;
	};
}