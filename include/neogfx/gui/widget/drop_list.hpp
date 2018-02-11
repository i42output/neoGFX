// drop_list.hpp
/*
neogfx C++ GUI Library
Copyright(C) 2015-present Leigh Johnston

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
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/list_view.hpp>

namespace neogfx
{
	class drop_list;

	class drop_list_view : public list_view
	{
		friend class drop_list_popup;
	public:
		drop_list_view(i_layout& aLayout, drop_list& aDropList);
		~drop_list_view();
	public:
		bool changing_text() const;
	public:
		using list_view::total_item_area;
	protected:
		void items_filtered(const i_item_presentation_model& aPresentationModel) override;
	protected:
		void current_index_changed(const i_item_selection_model& aSelectionModel, const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex) override;
	protected:
		void mouse_button_released(mouse_button aButton, const point& aPosition) override;
	protected:
		bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
	public:
		colour background_colour() const override;
	private:
		drop_list& iDropList;
		bool iChangingText;
	};
	
	class drop_list_popup : public window
	{
		friend class drop_list;
		friend class drop_list_view;
	public:
		drop_list_popup(drop_list& aDropList);
		~drop_list_popup();
	public:
		const drop_list_view& view() const;
		drop_list_view& view();
	protected:
		colour frame_colour() const override;
	protected:
		bool has_rendering_priority() const override;
	public:
		using window::show;
		bool show(bool aVisible) override;
	protected:
		neogfx::size_policy size_policy() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	public:
		bool can_dismiss(const i_widget*) const override;
		dismissal_type_e dismissal_type() const override;
		bool dismissed() const override;
		void dismiss() override;
	private:
		size ideal_size() const;
		void update_placement();
	private:
		drop_list& iDropList;
		drop_list_view iView;
	};

	class i_drop_list_input_widget
	{
	public:
		class i_visitor
		{
		public:
			virtual void visit(i_drop_list_input_widget& aInputWidget, line_edit& aTextWidget) = 0;
		};
	public:
		virtual ~i_drop_list_input_widget() {}
	public:
		virtual void accept(i_visitor& aVisitor) = 0;
	public:
		virtual const i_widget& as_widget() const = 0;
		virtual i_widget& as_widget() = 0;
	public:
		virtual bool editable() const = 0;
		virtual const i_widget& text_widget() const = 0;
		virtual i_widget& text_widget() = 0;
		virtual std::string text() const = 0;
		virtual void set_text(const std::string& aText) = 0;
	};

	class drop_list : public widget, private i_drop_list_input_widget::i_visitor
	{
	public:
		event<optional_item_model_index> selection_changed;
	public:
		struct no_selection : std::runtime_error { no_selection() : std::runtime_error("neogfx::drop_list::no_selection") {} };
	private:
		class popup_proxy
		{
		public:
			popup_proxy(drop_list& aDropList);
		public:
			bool popup_created() const;
			drop_list_popup& popup() const;
		private:
			drop_list& iDropList;
			mutable boost::optional<drop_list_popup> iPopup;
		};
	public:
		drop_list();
		drop_list(i_widget& aParent);
		drop_list(i_layout& aLayout);
		~drop_list();
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
		bool has_selection() const;
		const item_model_index& selection() const;
	public:
		bool view_created() const;
		void show_view();
		void hide_view();
		drop_list_view& view() const;
		drop_list_popup& popup() const;
		void accept_selection();
		void cancel_selection();
	public:
		bool editable() const;
		void set_editable(bool aEditable);
		const i_drop_list_input_widget& input_widget() const;
		i_drop_list_input_widget& input_widget();
	public:
		bool handling_text_change() const;
	public:
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	private:
		void visit(i_drop_list_input_widget& aInputWidget, line_edit& aTextWidget) override;
	private:
		void init();
		void update_input_widget();
		void update_arrow();
		void handle_clicked();
	private:
		horizontal_layout iLayout;
		std::unique_ptr<i_drop_list_input_widget> iInputWidget;
		std::shared_ptr<i_item_model> iModel;
		std::shared_ptr<i_item_presentation_model> iPresentationModel;
		std::shared_ptr<i_item_selection_model> iSelectionModel;
		sink iSink;
		bool iEditable;
		mutable boost::optional<std::pair<colour, texture>> iDownArrowTexture;
		image_widget iDownArrow;
		popup_proxy iPopupProxy;
		optional_item_model_index iSavedSelection;
		optional_item_model_index iSelection;
		bool iHandlingTextChange;
	};
}