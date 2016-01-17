// item_view.hpp
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
#include "scrollable_widget.hpp"
#include "i_item_model.hpp"
#include "i_item_presentation_model.hpp"
#include "i_item_selection_model.hpp"

namespace neogfx
{
	class item_view : public scrollable_widget, private i_item_model_subscriber 
	{
	public:
		item_view();
		item_view(i_widget& aParent);
		item_view(i_layout& aLayout);
		~item_view();
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
		bool has_selection_model() const;
		const i_item_selection_model& selection_model() const;
		i_item_selection_model& selection_model();
		void set_selection_model(i_item_selection_model& aSelectionModel);
		void set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel);
		void start_batch_update();
		void end_batch_update();
	protected:
		virtual void model_changed() = 0;
		virtual void presentation_model_changed() = 0;
		virtual void selection_model_changed() = 0;
		virtual void batch_update_started() = 0;
		virtual void batch_update_ended() = 0;
	protected:
		virtual rect item_display_rect() const = 0;
		virtual size item_total_area(graphics_context& aGraphicsContext) const = 0;
		virtual size cell_spacing() const = 0;
		virtual dimension column_width(uint32_t aColumn) const = 0;
		std::pair<item_model_index::value_type, coordinate> first_visible_item(graphics_context& aGraphicsContext) const;
	protected:
		virtual void paint(graphics_context& aGraphicsContext) const;
		using scrollable_widget::scrolling_disposition;
		virtual child_widget_scrolling_disposition_e scrolling_disposition() const;
		using scrollable_widget::update_scrollbar_visibility;
		virtual void update_scrollbar_visibility(usv_stage_e aStage);
	protected:
		virtual void column_info_changed(const i_item_model& aModel, item_model_index::value_type aColumnIndex);
		virtual void item_added(const i_item_model& aModel, const item_model_index& aItemIndex);
		virtual void item_changed(const i_item_model& aModel, const item_model_index& aItemIndex);
		virtual void item_removed(const i_item_model& aModel, const item_model_index& aItemIndex);
		virtual void items_sorted(const i_item_model& aModel);
		virtual void model_destroyed(const i_item_model& aModel);
	private:
		std::shared_ptr<i_item_model> iModel;
		std::shared_ptr<i_item_presentation_model> iPresentationModel;
		std::shared_ptr<i_item_selection_model> iSelectionModel;
		uint32_t iBatchUpdatesInProgress;
	};
}