// drop_list.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/drop_list.hpp>

namespace neogfx
{
	class drop_list_presentation_model : public item_presentation_model
	{
	public:
		drop_list_presentation_model(drop_list& aDropList) : item_presentation_model{}, iDropList{ aDropList }
		{
		}
	public:
		item_cell_editable column_editable(item_presentation_model_index::value_type) const override
		{
			return item_cell_editable::No;
		}
	public:	
		optional_colour cell_colour(const item_presentation_model_index& aIndex, item_cell_colour_type aColourType) const override
		{
			if (aColourType == item_cell_colour_type::Background && (cell_meta(aIndex).selection & item_cell_selection_flags::Current) == item_cell_selection_flags::Current)
			{
				auto backgroundColour = iDropList.view().background_colour().dark() ? colour::Black : colour::White;
				if (backgroundColour == iDropList.view().background_colour())
					backgroundColour = backgroundColour.dark() ? backgroundColour.lighter(0x20) : backgroundColour.darker(0x20);
				return backgroundColour;
			}
			else
				return item_presentation_model::cell_colour(aIndex, aColourType);
		}
	private:
		drop_list& iDropList;
	};

	drop_list_view::drop_list_view(i_layout& aLayout, drop_list& aDropList) :
		list_view{ aLayout, scrollbar_style::Normal, frame_style::NoFrame, false },
		iDropList{ aDropList }
	{
		set_margins(neogfx::margins{});
		set_selection_model(aDropList.selection_model());
		set_presentation_model(aDropList.presentation_model());
		set_model(aDropList.model());
	}

	drop_list_view::~drop_list_view()
	{
	}

	void drop_list_view::current_index_changed(const i_item_selection_model& aSelectionModel, const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
	{
		list_view::current_index_changed(aSelectionModel, aCurrentIndex, aPreviousIndex);
		std::string text;
		if (aCurrentIndex != boost::none)
			text = presentation_model().cell_to_string(*aCurrentIndex);
		iDropList.input_widget().set_text(text);
	}

	void drop_list_view::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		bool wasCapturing = capturing();
		list_view::mouse_button_released(aButton, aPosition);
		if (aButton == mouse_button::Left && wasCapturing)
		{
			if (selection_model().has_current_index())
			{
				auto item = item_at(aPosition);
				if (item != boost::none && row_rect(*item).contains(aPosition) && item->row() == selection_model().current_index().row())
					iDropList.accept_selection();
			}
		}
	}

	bool drop_list_view::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		bool handled = false;
		switch (aScanCode)
		{
		case ScanCode_ESCAPE:
			iDropList.cancel_selection();
			handled = true;
			break;
		case ScanCode_RETURN:
			if (selection_model().has_current_index())
				iDropList.accept_selection();
			else
				app::instance().basic_services().system_beep();
			handled = true;
			break;
		default:
			handled = list_view::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			break;
		}
		return handled;
	}

	colour drop_list_view::background_colour() const
	{
		if (list_view::has_background_colour())
			return list_view::background_colour();
		return app::instance().current_style().palette().colour();
	}

	drop_list_popup::drop_list_popup(drop_list& aDropList) :
		window{ 
			aDropList,
			aDropList.window_rect().bottom_left() + aDropList.root().window_position(),
			aDropList.window_rect().extents(),
			window_style::NoDecoration | window_style::NoActivate | window_style::RequiresOwnerFocus | window_style::DismissOnOwnerClick | window_style::InitiallyHidden | window_style::DropShadow },
		iDropList{ aDropList },
		iView{ client_layout(), aDropList }
	{
		client_layout().set_margins(neogfx::margins{});
		update_placement();
	}

	drop_list_popup::~drop_list_popup()
	{
	}

	const drop_list_view& drop_list_popup::view() const
	{
		return iView;
	}

	drop_list_view& drop_list_popup::view()
	{
		return iView;
	}

	colour drop_list_popup::frame_colour() const
	{
		if (window::has_frame_colour())
			return window::frame_colour();
		auto viewBackgroundColour = iView.background_colour();
		auto backgroundColour = viewBackgroundColour.dark() ? viewBackgroundColour.darker(0x20) : viewBackgroundColour.lighter(0x20);
		if (backgroundColour == viewBackgroundColour)
			backgroundColour = viewBackgroundColour.dark() ? viewBackgroundColour.lighter(0x20) : viewBackgroundColour.darker(0x20);
		return backgroundColour;
	}

	bool drop_list_popup::has_rendering_priority() const
	{
		return window::has_rendering_priority() || visible();
	}

	bool drop_list_popup::show(bool aVisible)
	{
		if (aVisible && !visible())
		{
			update_placement();
			if (!app::instance().keyboard().is_keyboard_grabbed_by(view()))
				app::instance().keyboard().grab_keyboard(view());
		}
		else if (!aVisible)
		{
			if (app::instance().keyboard().is_keyboard_grabbed_by(view()))
				app::instance().keyboard().ungrab_keyboard(view());
		}
		return window::show(aVisible);
	}

	size_policy drop_list_popup::size_policy() const
	{
		if (window::has_size_policy())
			return window::size_policy();
		return neogfx::size_policy::Minimum;
	}

	size drop_list_popup::minimum_size(const optional_size&) const
	{
		if (window::has_minimum_size())
			return window::minimum_size();
		auto totalArea = iView.total_item_area(*this);
		auto minimumSize = size{ effective_frame_width() * 2.0 } + margins().size() + totalArea + iView.margins().size();
		minimumSize.cy = std::min(minimumSize.cy, parent().root().as_widget().extents().cy / 2.0);
		if (minimumSize.cy - (effective_frame_width() * 2.0 + margins().size().cy) < totalArea.cy)
			minimumSize.cx += vertical_scrollbar().width(*this);
		minimumSize.cx = std::max(minimumSize.cx, iDropList.minimum_size().cx);
		return minimumSize;
	}

	bool drop_list_popup::can_dismiss(const i_widget*) const
	{
		return true;
	}

	window::dismissal_type_e drop_list_popup::dismissal_type() const
	{
		return CloseOnDismissal;
	}

	bool drop_list_popup::dismissed() const
	{
		return surface().is_closed();
	}

	void drop_list_popup::dismiss()
	{
		if (app::instance().keyboard().is_keyboard_grabbed_by(view()))
			app::instance().keyboard().ungrab_keyboard(view());
		close();
	}

	void drop_list_popup::update_placement()
	{
		resize(minimum_size());
		point currentItemPos;
		if (view().presentation_model().rows() > 0 && view().presentation_model().columns() > 0)
		{
			auto index = (view().selection_model().has_current_index() ?
				view().selection_model().current_index() :
				item_presentation_model_index{ 0, 0 });
			view().make_visible(index);
			currentItemPos += view().cell_rect(index).top_left();
			currentItemPos += view().presentation_model().cell_margins(*this).top_left();
			currentItemPos += view().presentation_model().cell_spacing(*this) / 2.0;
			currentItemPos -= point{ effective_frame_width(), effective_frame_width() };
		}
		point dropListPos{ iDropList.input_widget().text_widget().window_rect().top_left() + iDropList.input_widget().text_widget().margins().top_left() + iDropList.root().window_position() };
		surface().move_surface(-currentItemPos + dropListPos);
		resize(extents() + size{ dropListPos.x + iDropList.extents().cx - surface().surface_position().x - extents().cx + currentItemPos.x * 2.0, 0.0 });
		correct_popup_rect(*this);
	}

	drop_list::popup_proxy::popup_proxy(drop_list& aDropList) :
		iDropList{ aDropList }
	{
	}

	bool drop_list::popup_proxy::popup_created() const
	{
		return iPopup != boost::none;
	}

	drop_list_popup& drop_list::popup_proxy::popup() const
	{
		if (iPopup == boost::none)
		{
			iPopup.emplace(iDropList);
			iPopup->closed([this]() { iPopup = boost::none; });
		}
		return *iPopup;
	}

	namespace
	{
		class non_editable_input_widget : public push_button, public drop_list::i_input_widget
		{
		public:
			non_editable_input_widget(i_layout& aLayout) :
				push_button{ aLayout, push_button_style::DropList }
			{
			}
		public:
			const i_widget& as_widget() const override
			{
				return *this;
			}
			i_widget& as_widget() override
			{
				return *this;
			}
		public:
			bool editable() const override
			{
				return false;
			}
			const i_widget& text_widget() const override
			{
				return text();
			}
			i_widget& text_widget() override
			{
				return text();
			}
			void set_text(const std::string& aText) override
			{
				return text().set_text(aText);
			}
		};

		class editable_input_widget : public framed_widget, public drop_list::i_input_widget
		{
		public:
			editable_input_widget(i_layout& aLayout) :
				framed_widget{ aLayout, frame_style::SolidFrame },
				iLayout{ *this },
				iEditor{ iLayout, frame_style::NoFrame }
			{
				set_margins(neogfx::margins{});
			}
		public:
			const i_widget& as_widget() const override
			{
				return *this;
			}
			i_widget& as_widget() override
			{
				return *this;
			}
		public:
			bool editable() const override
			{
				return true;
			}
			const i_widget& text_widget() const override
			{
				return iEditor;
			}
			i_widget& text_widget() override
			{
				return iEditor;
			}
			void set_text(const std::string& aText) override
			{
				iEditor.set_text(aText);
			}
		private:
			horizontal_layout iLayout;
			line_edit iEditor;
		};
	}

	drop_list::drop_list() :
		iLayout{ *this },
		iEditable{ false }, 
		iDownArrow{ texture{} }, 
		iPopupProxy{ *this } 
	{
		init();
	}

	drop_list::drop_list(i_widget& aParent) :
		widget{ aParent },
		iLayout{ *this },
		iEditable{ false }, 
		iDownArrow{ texture{} }, 
		iPopupProxy{ *this } 
	{
		init();
	}

	drop_list::drop_list(i_layout& aLayout) :
		widget{ aLayout },
		iLayout{ *this },
		iEditable{ false }, 
		iDownArrow{ texture{} }, 
		iPopupProxy{ *this } 
	{
		init();
	}

	drop_list::~drop_list()
	{
	}

	bool drop_list::has_model() const
	{
		if (iModel)
			return true;
		else
			return false;
	}

	const i_item_model& drop_list::model() const
	{
		return *iModel;
	}

	i_item_model& drop_list::model()
	{
		return *iModel;
	}

	void drop_list::set_model(i_item_model& aModel)
	{
		iModel = std::shared_ptr<i_item_model>(std::shared_ptr<i_item_model>(), &aModel);
		if (has_model())
		{
			if (has_presentation_model())
				presentation_model().set_item_model(aModel);
		}
		update();
	}

	void drop_list::set_model(std::shared_ptr<i_item_model> aModel)
	{
		iModel = aModel;
		if (has_model())
		{
			if (has_presentation_model())
				presentation_model().set_item_model(*aModel);
		}
		update();
	}

	bool drop_list::has_presentation_model() const
	{
		if (iPresentationModel)
			return true;
		else
			return false;
	}

	const i_item_presentation_model& drop_list::presentation_model() const
	{
		return *iPresentationModel;
	}

	i_item_presentation_model& drop_list::presentation_model()
	{
		return *iPresentationModel;
	}

	void drop_list::set_presentation_model(i_item_presentation_model& aPresentationModel)
	{
		iPresentationModel = std::shared_ptr<i_item_presentation_model>(std::shared_ptr<i_item_presentation_model>(), &aPresentationModel);
		if (has_model())
			presentation_model().set_item_model(model());
		if (has_selection_model())
			selection_model().set_presentation_model(aPresentationModel);
		update();
	}

	void drop_list::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
	{
		iPresentationModel = aPresentationModel;
		if (has_presentation_model() && has_model())
			presentation_model().set_item_model(model());
		if (has_presentation_model() && has_selection_model())
			selection_model().set_presentation_model(*aPresentationModel);
		update();
	}

	bool drop_list::has_selection_model() const
	{
		if (iSelectionModel)
			return true;
		else
			return false;
	}

	const i_item_selection_model& drop_list::selection_model() const
	{
		return *iSelectionModel;
	}

	i_item_selection_model& drop_list::selection_model()
	{
		return *iSelectionModel;
	}

	void drop_list::set_selection_model(i_item_selection_model& aSelectionModel)
	{
		iSelectionModel = std::shared_ptr<i_item_selection_model>(std::shared_ptr<i_item_selection_model>(), &aSelectionModel);
		if (has_presentation_model())
			selection_model().set_presentation_model(presentation_model());
		update();
	}

	void drop_list::set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel)
	{
		iSelectionModel = aSelectionModel;
		if (has_presentation_model() && has_selection_model())
			selection_model().set_presentation_model(presentation_model());
		update();
	}

	bool drop_list::view_created() const
	{
		return iPopupProxy.popup_created();
	}

	drop_list_view& drop_list::view() const
	{
		return popup().view();
	}

	drop_list_popup& drop_list::popup() const
	{
		return iPopupProxy.popup();
	}

	void drop_list::accept_selection()
	{
		optional_item_model_index newSelection = (selection_model().has_current_index() ?
			presentation_model().to_item_model_index(selection_model().current_index()) : optional_item_model_index{});
		if (iSavedSelection != newSelection)
			selection_changed.async_trigger();
		popup().dismiss();
		iSavedSelection = boost::none;
	}

	void drop_list::cancel_selection()
	{
		if (iSavedSelection != boost::none)
			selection_model().set_current_index(presentation_model().from_item_model_index(*iSavedSelection));
		else
			selection_model().unset_current_index();
		popup().dismiss();
		iSavedSelection = boost::none;
	}

	bool drop_list::editable() const
	{
		return iEditable;
	}

	void drop_list::set_editable(bool aEditable)
	{
		if (iEditable != aEditable)
		{
			iEditable = aEditable;
			update_input_widget();
		}
	}

	const drop_list::i_input_widget& drop_list::input_widget() const
	{
		return *iInputWidget;
	}

	drop_list::i_input_widget& drop_list::input_widget()
	{
		return *iInputWidget;
	}

	size drop_list::minimum_size(const optional_size& aAvailableSpace) const
	{
		auto minimumSize = widget::minimum_size(aAvailableSpace);
		if (widget::has_minimum_size())
			return minimumSize;
		minimumSize.cx -= input_widget().text_widget().minimum_size().cx;
		minimumSize.cx += input_widget().text_widget().margins().size().cx;
		minimumSize.cx += presentation_model().column_width(0, graphics_context{ *this, graphics_context::type::Unattached }, false);
		return minimumSize;
	}

	void drop_list::init()
	{
		set_selection_model(std::shared_ptr<i_item_selection_model>(new item_selection_model{}));
		set_presentation_model(std::shared_ptr<i_item_presentation_model>(new drop_list_presentation_model{ *this }));
		set_model(std::shared_ptr<i_item_model>(new item_model{}));

		set_margins(neogfx::margins{});
		set_size_policy(neogfx::size_policy::Minimum);

		iLayout.set_margins(neogfx::margins{});

		update_input_widget();

		iSink += app::instance().current_style_changed([this](style_aspect) { update_arrow(); });

		presentation_model().set_cell_margins(neogfx::margins{ 3.0, 3.0 }, *this);
	}

	void drop_list::update_input_widget()
	{
		bool changed = false;

		if (!editable() && (iInputWidget == nullptr || iInputWidget->editable()))
		{
			iInputWidget = std::make_unique<non_editable_input_widget>(iLayout);
			auto& inputWidget = static_cast<non_editable_input_widget&>(iInputWidget->as_widget());

			inputWidget.clicked([this]() { handle_clicked(); });

			inputWidget.set_size_policy(size_policy::Expanding);
			
			auto& inputLabelLayout = inputWidget.label().layout();
			inputLabelLayout.set_alignment(neogfx::alignment::Left | neogfx::alignment::VCentre);
			auto& s1 = inputWidget.layout().add_spacer();
			scoped_units su1{ s1, inputLabelLayout, units::Pixels };
			s1.set_minimum_width(inputLabelLayout.spacing().cx);
			inputWidget.layout().add(iDownArrow);
			auto& s2 = inputWidget.layout().add_spacer();
			scoped_units su2{ s2, inputLabelLayout, units::Pixels };
			s2.set_fixed_size(size{ inputLabelLayout.spacing().cx, 0.0 });

			changed = true;
		}
		else if (editable() && (iInputWidget == nullptr || !iInputWidget->editable()))
		{
			iInputWidget = std::make_unique<editable_input_widget>(iLayout);
			auto& inputWidget = static_cast<editable_input_widget&>(iInputWidget->as_widget());

			inputWidget.mouse_event([&inputWidget, this](const neogfx::mouse_event& aEvent)
			{
				if (aEvent.type() == mouse_event_type::ButtonReleased &&
					aEvent.mouse_button() == mouse_button::Left &&
					inputWidget.client_rect().contains(aEvent.position() - window_rect().top_left()) &&
					inputWidget.capturing())
					handle_clicked();
			});

			inputWidget.set_size_policy(size_policy::Expanding);

			inputWidget.layout().add(iDownArrow);

			changed = true;
		}

		if (changed)
		{
			update_arrow();

			std::string text;
			if (selection_model().has_current_index())
				text = presentation_model().cell_to_string(selection_model().current_index());
			input_widget().set_text(text);
		}
	}

	void drop_list::update_arrow()
	{
		auto ink = app::instance().current_style().palette().text_colour();
		if (iDownArrowTexture == boost::none || iDownArrowTexture->first != ink)
		{
			const uint8_t sDownArrowImagePattern[4][8]
			{
				{ 1, 1, 1, 1, 1, 1, 1, 1 },
				{ 0, 1, 1, 1, 1, 1, 1, 0 },
				{ 0, 0, 1, 1, 1, 1, 0, 0 },
				{ 0, 0, 0, 1, 1, 0, 0, 0 },
			};
			iDownArrowTexture = std::make_pair(ink, neogfx::image{ "neogfx::drop_list::iDownArrowTexture::" + ink.to_string(), sDownArrowImagePattern, { { 0_u8, colour{} }, { 1_u8, ink } } });
		}
		iDownArrow.set_image(iDownArrowTexture->second);
	}

	void drop_list::handle_clicked()
	{
		if (view().effectively_hidden())
		{
			app::instance().window_manager().move_window(popup(), window_rect().bottom_left() + root().window_position());
			if (selection_model().has_current_index())
				iSavedSelection = selection_model().current_index();
			popup().show();
		}
		else
			popup().dismiss();
	}
}