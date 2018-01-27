// drop_list.hpp
/*
neogfx C++ GUI Library
Copyright(C) 2017 Leigh Johnston

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
		using list_view::total_item_area;
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
	};
	
	class drop_list_popup : public window
	{
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
		drop_list& iDropList;
		drop_list_view iView;
	};

	class drop_list : public widget
	{
	public:
		event<> selection_changed;
	public:
		class i_input_widget
		{
		public:
			virtual ~i_input_widget() {}
		public:
			virtual const i_widget& as_widget() const = 0;
			virtual i_widget& as_widget() = 0;
		public:
			virtual bool editable() const = 0;
			virtual const i_widget& text_widget() const = 0;
			virtual i_widget& text_widget() = 0;
			virtual void set_text(const std::string& aText) = 0;
		};
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
		bool view_created() const;
		drop_list_view& view() const;
		drop_list_popup& popup() const;
		void accept_selection();
		void cancel_selection();
	public:
		bool editable() const;
		void set_editable(bool aEditable);
		const i_input_widget& input_widget() const;
		i_input_widget& input_widget();
	public:
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	private:
		void init();
		void update_input_widget();
		void update_arrow();
		void handle_clicked();
	private:
		horizontal_layout iLayout;
		std::unique_ptr<i_input_widget> iInputWidget;
		std::shared_ptr<i_item_model> iModel;
		std::shared_ptr<i_item_presentation_model> iPresentationModel;
		std::shared_ptr<i_item_selection_model> iSelectionModel;
		sink iSink;
		bool iEditable;
		mutable boost::optional<std::pair<colour, texture>> iDownArrowTexture;
		image_widget iDownArrow;
		popup_proxy iPopupProxy;
		optional_item_model_index iSavedSelection;
	};
}