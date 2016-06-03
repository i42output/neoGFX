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
#include "header_view.hpp"
#include "i_item_model.hpp"
#include "i_item_presentation_model.hpp"
#include "i_item_selection_model.hpp"

namespace neogfx
{
	class item_view : public scrollable_widget, protected header_view::i_owner, private i_item_model_subscriber, private i_item_selection_model_subscriber
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
	public:
		void make_visible(const item_model_index& aItemIndex);
	protected:
		virtual void header_view_updated(header_view& aHeaderView);
		virtual neogfx::margins cell_margins() const;
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
		std::pair<item_model_index::value_type, coordinate> last_visible_item(graphics_context& aGraphicsContext) const;
	protected:
		virtual neogfx::size_policy size_policy() const;
	protected:
		virtual void paint(graphics_context& aGraphicsContext) const;
	protected:
		virtual void released();
		virtual void focus_gained();
	protected:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
	protected:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
	protected:
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
	protected:
		virtual void item_model_added(const i_item_selection_model& aSelectionModel, i_item_model& aNewItemModel) { (void)aSelectionModel; (void)aNewItemModel; }
		virtual void item_model_changed(const i_item_selection_model& aSelectionModel, i_item_model& aNewItemModel, i_item_model& aOldItemModel) { (void)aSelectionModel; (void)aNewItemModel; (void)aOldItemModel; }
		virtual void item_model_removed(const i_item_selection_model& aSelectionModel, i_item_model& aOldItemModel) { (void)aSelectionModel; (void)aOldItemModel; }
		virtual void selection_mode_changed(const i_item_selection_model& aSelectionModel, item_selection_mode aNewMode) { (void)aSelectionModel; (void)aNewMode; }
		virtual void current_index_changed(const i_item_selection_model& aSelectionModel, const optional_item_model_index& aCurrentIndex, const optional_item_model_index& aPreviousIndex);
		virtual void selection_changed(const i_item_selection_model& aSelectionModel, const item_selection& aCurrentSelection, const item_selection& aPreviousSelection) { (void)aSelectionModel; (void)aCurrentSelection; (void)aPreviousSelection; }
		virtual void selection_model_destroyed(const i_item_selection_model& aSelectionModel) { (void)aSelectionModel; }
	public:
		rect cell_rect(const item_model_index& aItemIndex) const;
		optional_item_model_index item_at(const point& aPosition) const;
	private:
		std::shared_ptr<i_item_model> iModel;
		std::shared_ptr<i_item_presentation_model> iPresentationModel;
		std::shared_ptr<i_item_selection_model> iSelectionModel;
		uint32_t iBatchUpdatesInProgress;
		boost::optional<std::shared_ptr<neolib::callback_timer>> iMouseTracker;
	};
}