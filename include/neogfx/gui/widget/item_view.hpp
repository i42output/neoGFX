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

#include <neogfx/neogfx.hpp>
#include "scrollable_widget.hpp"
#include "header_view.hpp"
#include "item_editor.hpp"
#include "i_item_model.hpp"
#include "i_item_presentation_model.hpp"
#include "i_item_selection_model.hpp"

namespace neogfx
{
	class item_view : public scrollable_widget, protected header_view::i_owner, private i_item_model_subscriber, private i_item_presentation_model_subscriber, private i_item_selection_model_subscriber
	{
	public:
		struct no_editor : std::logic_error { no_editor() : std::logic_error("neogfx::item_view::no_editor") {} };
		struct unknown_editor_type : std::logic_error { unknown_editor_type() : std::logic_error("neogfx::item_view::unknown_editor_type") {} };
	public:
		item_view(scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame);
		item_view(i_widget& aParent, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame);
		item_view(i_layout& aLayout, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame);
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
	public:
		bool is_visible(const item_presentation_model_index& aItemIndex) const;
		void make_visible(const item_presentation_model_index& aItemIndex);
		const optional_item_presentation_model_index& editing() const;
		void edit(const item_presentation_model_index& aItemIndex);
		void begin_edit();
		void end_edit(bool aCommit);
		bool beginning_edit() const;
		bool ending_edit() const;
		i_widget& editor() const;
		bool editor_has_text_edit() const;
		text_edit& editor_text_edit() const;
	protected:
		void header_view_updated(header_view& aHeaderView, header_view_update_reason aUpdateReason) override;
	protected:
		virtual void model_changed() = 0;
		virtual void presentation_model_changed() = 0;
		virtual void selection_model_changed() = 0;
	protected:
		virtual rect item_display_rect() const = 0;
		virtual size item_total_area(const i_units_context& aUnitsContext) const = 0;
		virtual dimension column_width(uint32_t aColumn) const = 0;
		std::pair<item_model_index::value_type, coordinate> first_visible_item(graphics_context& aGraphicsContext) const;
		std::pair<item_model_index::value_type, coordinate> last_visible_item(graphics_context& aGraphicsContext) const;
	protected:
		void layout_items_completed() override;
	protected:
		widget_part hit_test(const point& aPosition) const override;
	protected:
		neogfx::size_policy size_policy() const override;
	protected:
		void paint(graphics_context& aGraphicsContext) const override;
	protected:
		void released() override;
		neogfx::focus_policy focus_policy() const override;
		void focus_gained(focus_reason aFocusReason) override;
	protected:
		void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
	protected:
		bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		bool text_input(const std::string& aText) override;
	protected:
		using scrollable_widget::scrolling_disposition;
		neogfx::scrolling_disposition scrolling_disposition() const override;
		void update_scrollbar_visibility() override;
		void update_scrollbar_visibility(usv_stage_e aStage) override;
	protected:
		void column_info_changed(const i_item_model& aModel, item_model_index::value_type aColumnIndex) override;
		void item_added(const i_item_model& aModel, const item_model_index& aItemIndex) override;
		void item_changed(const i_item_model& aModel, const item_model_index& aItemIndex) override;
		void item_removed(const i_item_model& aModel, const item_model_index& aItemIndex) override;
		void model_destroyed(const i_item_model& aModel) override;
	protected:
		void column_info_changed(const i_item_presentation_model& aModel, item_presentation_model_index::column_type aColumnIndex) override;
		void item_model_changed(const i_item_presentation_model& aModel, const i_item_model& aItemModel) override;
		void item_added(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) override;
		void item_changed(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) override;
		void item_removed(const i_item_presentation_model& aModel, const item_presentation_model_index& aItemIndex) override;
		void items_sorting(const i_item_presentation_model& aModel) override;
		void items_sorted(const i_item_presentation_model& aModel) override;
		void items_filtering(const i_item_presentation_model& aModel) override;
		void items_filtered(const i_item_presentation_model& aModel) override;
		void model_destroyed(const i_item_presentation_model& aModel) override;
	protected:
		void model_added(const i_item_selection_model& aSelectionModel, i_item_presentation_model& aNewModel) override;
		void model_changed(const i_item_selection_model& aSelectionModel, i_item_presentation_model& aNewModel, i_item_presentation_model& aOldModel) override;
		void model_removed(const i_item_selection_model& aSelectionModel, i_item_presentation_model& aOldModel) override;
		void selection_mode_changed(const i_item_selection_model& aSelectionModel, item_selection_mode aNewMode) override;
		void current_index_changed(const i_item_selection_model& aSelectionModel, const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex) override;
		void selection_changed(const i_item_selection_model& aSelectionModel, const item_selection& aCurrentSelection, const item_selection& aPreviousSelection) override;
		void selection_model_destroyed(const i_item_selection_model& aSelectionModel) override;
	public:
		rect cell_rect(const item_presentation_model_index& aItemIndex, bool aBackground = false) const;
		optional_item_presentation_model_index item_at(const point& aPosition) const;
	private:
		std::shared_ptr<i_item_model> iModel;
		std::shared_ptr<i_item_presentation_model> iPresentationModel;
		std::shared_ptr<i_item_selection_model> iSelectionModel;
		boost::optional<std::shared_ptr<neolib::callback_timer>> iMouseTracker;
		optional_item_presentation_model_index iEditing;
		std::shared_ptr<i_item_editor> iEditor;
		bool iBeginningEdit;
		bool iEndingEdit;
		optional_item_model_index iSavedModelIndex;
	};
}