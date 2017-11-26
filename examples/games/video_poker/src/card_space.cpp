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
		widget{ aLayout }
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
	}

	card_space::card_space(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane) :
		widget{ aLayout },
		iSpritePlane{ aSpritePlane }, 
		iVerticalLayout{ *this, neogfx::alignment::Centre | neogfx::alignment::VCentre },
		iCardWidget{ iVerticalLayout }, 
		iHoldButton{ iVerticalLayout, u8"HOLD\n CANCEL " }
	{
		set_size_policy(neogfx::size_policy::ExpandingNoBits);
		iVerticalLayout.set_spacing(neogfx::size{ 8.0 });
		iHoldButton.set_size_policy(neogfx::size_policy::Minimum);
		iHoldButton.set_foreground_colour(neogfx::color::Black);
		iHoldButton.text().set_font(neogfx::font{ "Exo 2", "Black", 16.0 });
		iHoldButton.text().set_text_appearance(neogfx::text_appearance{ neogfx::color::White, neogfx::text_effect{ neogfx::text_effect::Outline, neogfx::colour::Black.with_alpha(128) } });
		iHoldButton.set_checkable();
		auto update_hold = [this]() { iHoldButton.set_foreground_colour(iHoldButton.is_checked() ? neogfx::colour::LightYellow1 : neogfx::colour::Black.with_alpha(128)); };
		iHoldButton.toggled(update_hold);
		update_hold();
	}

	bool card_space::has_card() const
	{
		return iCard != boost::none;
	}

	const video_poker::card& card_space::card() const
	{
		if (has_card())
			return *iCard;
		throw no_card();
	}

	void card_space::set_card(const video_poker::card& aCard)
	{
		iCard = aCard;
	}

	void card_space::clear_card()
	{
		iCard = boost::none;
	}
}