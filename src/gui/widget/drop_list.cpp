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
#include <neogfx/gui/widget/drop_list.hpp>

namespace neogfx
{
	drop_list_view::drop_list_view(i_layout& aLayout) :
		list_view{ aLayout, scrollbar_style::Normal, frame_style::NoFrame }
	{
		set_margins(neogfx::optional_margins{});
	}

	drop_list_view::~drop_list_view()
	{
	}

	colour drop_list_view::background_colour() const
	{
		auto backgroundColour = item_view::background_colour();
		if (item_view::has_background_colour())
			return backgroundColour;
		else
			return backgroundColour.dark() ? backgroundColour.lighter(0x20) : backgroundColour.darker(0x20);
	}

	drop_list_popup::drop_list_popup(drop_list& aDropList) :
		window{ 
			aDropList,
			aDropList.window_rect().bottom_left() + aDropList.root().window_position(),
			aDropList.window_rect().extents(),
			window_style::NoDecoration | window_style::NoActivate | window_style::RequiresOwnerFocus | window_style::HideOnOwnerClick | window_style::InitiallyHidden | window_style::DropShadow },
		iDropList{ aDropList },
		iView{ client_layout() }
	{
		client_layout().set_margins(neogfx::margins{});
		resize(minimum_size());
	}

	const list_view& drop_list_popup::view() const
	{
		return iView;
	}

	list_view& drop_list_popup::view()
	{
		return iView;
	}

	colour drop_list_popup::frame_colour() const
	{
		if (window::has_frame_colour())
			return window::frame_colour();
		auto backgroundColour = iView.background_colour();
		return backgroundColour.dark() ? backgroundColour.darker(0x10) : backgroundColour.lighter(0x10);
	}

	bool drop_list_popup::has_rendering_priority() const
	{
		return window::has_rendering_priority() || visible();
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
		auto minimumSize = size{ effective_frame_width() * 2.0 } + margins().size() + totalArea;
		minimumSize.cy = std::min(minimumSize.cy, parent().root().as_widget().extents().cy / 2.0);
		if (minimumSize.cy - (effective_frame_width() * 2.0 + margins().size().cy) < totalArea.cy)
			minimumSize.cx += vertical_scrollbar().width(*this);
		return minimumSize;
	}

	bool drop_list_popup::can_dismiss(const i_widget*) const
	{
		return true;
	}

	window::dismissal_type_e drop_list_popup::dismissal_type() const
	{
		return HideOnDismissal;
	}

	bool drop_list_popup::dismissed() const
	{
		return hidden();
	}

	void drop_list_popup::dismiss()
	{
		hide();
	}

	drop_list::popup_proxy::popup_proxy(drop_list& aDropList) :
		iDropList{ aDropList }
	{
	}

	drop_list_popup& drop_list::popup_proxy::popup() const
	{
		if (iPopup == boost::none)
			iPopup.emplace(iDropList);
		return *iPopup;
	}

	drop_list::drop_list() :
		push_button{ push_button_style::DropList }, iEditable{ false }, iDownArrow{ texture{} }, iPopupProxy{ *this }
	{
		init();
	}

	drop_list::drop_list(i_widget& aParent) :
		push_button{ aParent, push_button_style::DropList }, iEditable{ false }, iDownArrow{ texture{} }, iPopupProxy{ *this }
	{
		init();
	}

	drop_list::drop_list(i_layout& aLayout) :
		push_button{ aLayout, push_button_style::DropList }, iEditable{ false }, iDownArrow{ texture{} }, iPopupProxy{ *this }
	{
		init();
	}

	drop_list::~drop_list()
	{
	}

	bool drop_list::has_model() const
	{
		return view().has_model();
	}

	const i_item_model& drop_list::model() const
	{
		return view().model();
	}

	i_item_model& drop_list::model()
	{
		return view().model();
	}

	void drop_list::set_model(i_item_model& aModel)
	{
		view().set_model(aModel);
	}

	void drop_list::set_model(std::shared_ptr<i_item_model> aModel)
	{
		view().set_model(aModel);
	}

	bool drop_list::has_presentation_model() const
	{
		return view().has_presentation_model();
	}

	const i_item_presentation_model& drop_list::presentation_model() const
	{
		return view().presentation_model();
	}

	i_item_presentation_model& drop_list::presentation_model()
	{
		return view().presentation_model();
	}

	void drop_list::set_presentation_model(i_item_presentation_model& aPresentationModel)
	{
		return view().set_presentation_model(aPresentationModel);
	}

	void drop_list::set_presentation_model(std::shared_ptr<i_item_presentation_model> aPresentationModel)
	{
		return view().set_presentation_model(aPresentationModel);
	}

	bool drop_list::has_selection_model() const
	{
		return view().has_selection_model();
	}

	const i_item_selection_model& drop_list::selection_model() const
	{
		return view().selection_model();
	}

	i_item_selection_model& drop_list::selection_model()
	{
		return view().selection_model();
	}

	void drop_list::set_selection_model(i_item_selection_model& aSelectionModel)
	{
		view().set_selection_model(aSelectionModel);
	}

	void drop_list::set_selection_model(std::shared_ptr<i_item_selection_model> aSelectionModel)
	{
		view().set_selection_model(aSelectionModel);
	}

	drop_list_popup& drop_list::popup() const
	{
		return iPopupProxy.popup();
	}

	list_view& drop_list::view() const
	{
		return popup().view();
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
		}
	}

	size drop_list::minimum_size(const optional_size& aAvailableSpace) const
	{
		return push_button::minimum_size(aAvailableSpace);
	}

	void drop_list::handle_clicked()
	{
		if (view().effectively_hidden())
		{
			app::instance().window_manager().move_window(popup(), window_rect().bottom_left() + root().window_position());
			popup().show();
		}
		else
			popup().hide();
	}

	void drop_list::init()
	{
		set_size_policy(neogfx::size_policy::Minimum);
		label().layout().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCentre);
		auto& s = layout().add_spacer();
		scoped_units su{ s, units::Pixels };
		s.set_minimum_width(std::max(0.0, 8.0 - label().layout().spacing().cx * 2));
		layout().add(iDownArrow);
		update_arrow();
		iSink += app::instance().current_style_changed([this](style_aspect) { update_arrow(); });
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
			iDownArrowTexture = std::make_pair(ink, neogfx::image{ "neogfx::drop_list::iDownArrowTexture::" + ink.to_string(), sDownArrowImagePattern,{ { 0, colour{} },{ 1, ink } } });
		}
		iDownArrow.set_image(iDownArrowTexture->second);
	}
}