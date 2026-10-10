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
    // The module's sample list (which is also where MOD authors left their greetings), with the
    // samples struck on the current row lit up.
    class sample_view : public ng::widget<>
    {
    public:
        sample_view(ng::i_layout& aLayout, player& aPlayer);
    public:
        // called once per frame, after the player has been updated
        void refresh();
    protected:
        void paint(ng::i_graphics_context& aGc) const override;
    private:
        player& iPlayer;
        ng::font iFont;
        std::optional<std::size_t> iRow;
        std::vector<float> iGlow;
    };
}
