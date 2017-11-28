/*
neogfx C++ GUI Library - Examples - Games - Video Poker
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
#include <video_poker/card_space.hpp>

namespace video_poker
{
	namespace
	{
		const neogfx::size kBridgeCardSize{ 57.15, 88.9 };
	}

	card_widget::card_widget(neogfx::i_layout& aLayout) :
		widget{ aLayout },
		iCard{ nullptr }
	{
		set_margins(neogfx::margins{});
		set_size_policy(neogfx::size_policy::Expanding, kBridgeCardSize);
	}

	neogfx::size card_widget::minimum_size(const neogfx::optional_size& aAvailableSpace) const
	{
		neogfx::scoped_units su(*this, neogfx::units::Millimetres);
		auto minSize = convert_units(*this, neogfx::units::Pixels, kBridgeCardSize * 0.5);
		su.restore_saved_units();
		minSize = neogfx::units_converter(*this).from_device_units(minSize);
		return minSize.ceil();
	}

	neogfx::size card_widget::maximum_size(const neogfx::optional_size& aAvailableSpace) const
	{
		return (minimum_size(aAvailableSpace) / 0.5).ceil();
	}
		
	void card_widget::paint(neogfx::graphics_context& aGraphicsContext) const
	{
		auto rect = client_rect();
		aGraphicsContext.fill_rounded_rect(rect, rect.cx / 10.0, neogfx::colour::DarkGreen);
		rect.deflate(neogfx::size{ 4.0 });
		aGraphicsContext.fill_rounded_rect(rect, rect.cx / 10.0, background_colour());

		if (has_card())
			aGraphicsContext.draw_text(client_rect().position(), card().to_string(), font(), neogfx::colour::White); // todo: delete this
	}

	bool card_widget::has_card() const
	{
		return iCard != nullptr;
	}

	const card& card_widget::card() const
	{
		return *iCard;
	}

	void card_widget::set_card(const video_poker::card& aCard)
	{
		iCard = &aCard;
		iCard->changed([this](video_poker::card&) { update(); });
		update();
	}

	void card_widget::clear_card()
	{
		iCard = nullptr;
		update();
	}

	card_space::card_space(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane, i_table& aTable) :
		widget{ aLayout },
		iSpritePlane{ aSpritePlane }, 
		iTable{ aTable },
		iVerticalLayout{ *this, neogfx::alignment::Centre | neogfx::alignment::VCentre },
		iCardWidget{ iVerticalLayout }, 
		iHoldButton{ iVerticalLayout, u8"HOLD\n CANCEL " },
		iCard{ nullptr }
	{
		set_size_policy(neogfx::size_policy::ExpandingNoBits);
		iVerticalLayout.set_spacing(neogfx::size{ 8.0 });
		iHoldButton.set_size_policy(neogfx::size_policy::Minimum);
		iHoldButton.set_foreground_colour(neogfx::color::Black);
		iHoldButton.text().set_font(neogfx::font{ "Exo 2", "Black", 16.0 });
		iHoldButton.text().set_text_appearance(neogfx::text_appearance{ neogfx::color::White, neogfx::text_effect{ neogfx::text_effect::Outline, neogfx::colour::Black.with_alpha(128) } });
		iHoldButton.set_checkable();
		auto update_hold = [this]() 
		{ 
			if (has_card())
			{
				if (iHoldButton.is_checked())
					card().undiscard();
				else
					card().discard();
				update_widgets();
			}
		};
		iHoldButton.toggled(update_hold);
		iTable.state_changed([this](table_state) { update_widgets(); });
		update_widgets();
	}

	bool card_space::has_card() const
	{
		return iCard != nullptr;
	}

	const video_poker::card& card_space::card() const
	{
		if (has_card())
			return *iCard;
		throw no_card();
	}

	video_poker::card& card_space::card()
	{
		if (has_card())
			return *iCard;
		throw no_card();
	}

	void card_space::set_card(video_poker::card& aCard)
	{
		iCard = &aCard;
		iCardWidget.set_card(aCard);
		iCard->changed([this](video_poker::card&) { update_widgets(); });
		update_widgets();
	}

	void card_space::clear_card()
	{
		iCard = nullptr;
		iCardWidget.clear_card();
		update_widgets();
	}

	void card_space::update_widgets()
	{
		iHoldButton.set_foreground_colour(has_card() && !card().discarded() && iTable.state() != table_state::DealtSecond ? neogfx::colour::LightYellow1 : neogfx::colour::Black.with_alpha(128));
		iHoldButton.enable(has_card() && iTable.state() != table_state::DealtSecond);
		iHoldButton.set_checked(has_card() && !card().discarded() && iTable.state() != table_state::DealtSecond);
	}
}