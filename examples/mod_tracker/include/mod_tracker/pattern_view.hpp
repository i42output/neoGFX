/*
neogfx C++ App/Game Engine - Examples - MOD Tracker
Copyright(C) 2026 Leigh Johnston

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

#include <mod_tracker/mod_tracker.hpp>

#include <optional>
#include <vector>

#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gfx/text/font.hpp>

#include <mod_tracker/player.hpp>

namespace mod_tracker
{
    // The classic tracker display: the order list along the top, a level meter per channel, and the
    // current pattern scrolling past a fixed playhead row. Clicking an order position, the mouse wheel
    // and the cursor keys all seek; space plays and pauses; clicking a channel's header mutes it.
    class pattern_view : public ng::widget<>
    {
    public:
        pattern_view(ng::i_layout& aLayout, player& aPlayer);
    public:
        // called once per frame, after the player has been updated
        void refresh();
    protected:
        void paint(ng::i_graphics_context& aGc) const override;
    protected:
        ng::focus_policy focus_policy() const override;
        bool key_pressed(ng::scan_code_e aScanCode, ng::key_code_e aKeyCode, ng::key_modifier aKeyModifier) override;
        bool mouse_wheel_scrolled(ng::mouse_wheel aWheel, const ng::point& aPosition, ng::delta aDelta, ng::key_modifier aKeyModifier) override;
        void mouse_button_clicked(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifier aKeyModifier) override;
    private:
        player& iPlayer;
        ng::font iFont;
        std::optional<std::size_t> iRow;
        std::vector<float> iMeters;
        // where the order list and channel headers were last painted, for hit testing
        mutable ng::rect iOrderListRect;
        mutable ng::dimension iOrderCellWidth = 0.0;
        mutable std::uint32_t iFirstOrderShown = 0u;
        mutable ng::rect iChannelHeaderRect;
        mutable ng::dimension iChannelLeft = 0.0;
        mutable ng::dimension iChannelWidth = 0.0;
    };
}
