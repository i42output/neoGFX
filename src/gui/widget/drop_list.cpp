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
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/drop_list.hpp>

namespace neogfx
{
	class drop_list_presentation_model : public item_presentation_model
	{
	public:
		drop_list_presentation_model(drop_list_view& aView) : item_presentation_model{}, iView{ aView }
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
				auto backgroundColour = iView.background_colour().dark() ? colour::Black : colour::White;
				if (backgroundColour == iView.background_colour())
					backgroundColour = backgroundColour.dark() ? backgroundColour.lighter(0x20) : backgroundColour.darker(0x20);
				return backgroundColour;
			}
			else
				return item_presentation_model::cell_colour(aIndex, aColourType);
		}
	private:
		drop_list_view& iView;
	};

	drop_list_view::drop_list_view(i_layout& aLayout, drop_list& aDropList) :
		list_view{ aLayout, scrollbar_style::Normal, frame_style::NoFrame },
		iDropList{ aDropList }
	{
		set_margins(neogfx::optional_margins{});
		set_presentation_model(std::shared_ptr<i_item_presentation_model>(new drop_list_presentation_model{ *this }));
		presentation_model().set_cell_margins(neogfx::margins{ 3.0, 3.0 }, *this);
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
		iDropList.text().set_text(text);
		if (surface().as_surface_window().has_clicked_widget() && &surface().as_surface_window().clicked_widget() == this)
			root().as_widget().hide();
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
			window_style::NoDecoration | window_style::NoActivate | window_style::RequiresOwnerFocus | window_style::HideOnOwnerClick | window_style::InitiallyHidden | window_style::DropShadow },
		iDropList{ aDropList },
		iView{ client_layout(), aDropList }
	{
		client_layout().set_margins(neogfx::margins{});
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
			resize(minimum_size());
			point currentItemPos;
			if (view().presentation_model().rows() > 0 && view().presentation_model().columns() > 0)
			{
				auto index = (view().selection_model().has_current_index() ?
					view().selection_model().current_index() :
					item_presentation_model_index{ 0, 0 });
				view().make_visible(index);
				currentItemPos.y += view().cell_rect(index).y;
				currentItemPos.y += view().presentation_model().cell_margins(*this).top;
				currentItemPos.y += view().presentation_model().cell_spacing(*this).cy / 2.0;
				currentItemPos.y -= effective_frame_width();
			}
			surface().move_surface(-currentItemPos + 
				point{ iDropList.window_rect().x, iDropList.text().window_rect().top_left().y } + iDropList.root().window_position());
			correct_popup_rect(*this);
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
		auto minimumSize = size{ effective_frame_width() * 2.0 } + margins().size() + totalArea;
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

	drop_list_view& drop_list::view() const
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
		auto minimumSize = push_button::minimum_size(aAvailableSpace);
		if (push_button::has_minimum_size())
			return minimumSize;
		minimumSize.cx -= text().minimum_size().cx;
		minimumSize.cx += view().presentation_model().column_width(0, graphics_context{ *this, graphics_context::type::Unattached }, false);
		return minimumSize;
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