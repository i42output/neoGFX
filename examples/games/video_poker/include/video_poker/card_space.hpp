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

#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <card_games/card.hpp>

namespace video_poker
{
	using namespace neogames::card_games;

	class card_widget : public neogfx::widget
	{
	public:
		card_widget(neogfx::i_layout& aLayout);
	public:
		neogfx::size minimum_size(const neogfx::optional_size& aAvailableSpace = neogfx::optional_size{}) const override;
		neogfx::size maximum_size(const neogfx::optional_size& aAvailableSpace = neogfx::optional_size{}) const override;
	protected:
		void paint(neogfx::graphics_context& aGraphicsContext) const override;
	};

	class card_space : neogfx::widget
	{
	public:
		struct no_card : std::runtime_error { no_card() : std::runtime_error("video_poker::card_space::no_card") {} };
	public:
		card_space(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane);
	public:
		bool has_card() const;
		const video_poker::card& card() const;
		void set_card(const video_poker::card& aCard);
		void clear_card();
	public:
		neogfx::sprite_plane& iSpritePlane;
		neogfx::vertical_layout iVerticalLayout;
		card_widget iCardWidget;
		boost::optional<video_poker::card> iCard;
		neogfx::push_button iHoldButton;
	};
}