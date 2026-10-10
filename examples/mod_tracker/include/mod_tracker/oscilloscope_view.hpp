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

#include <string>
#include <vector>

#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gfx/text/font.hpp>

#include <mod_tracker/player.hpp>

namespace mod_tracker
{
    // A real-time oscilloscope for one channel of the output (0 = left, 1 = right): the waveform of what is
    // being heard, redrawn once per frame and triggered on a rising zero crossing, so that a steady tone holds
    // still.
    class oscilloscope_view : public ng::widget<>
    {
    public:
        oscilloscope_view(ng::i_layout& aLayout, player& aPlayer, std::uint32_t aChannel, std::string const& aTitle);
    public:
        // called once per frame, after the player has been updated
        void refresh();
    protected:
        void paint(ng::i_graphics_context& aGc) const override;
    private:
        player& iPlayer;
        std::uint32_t const iChannel;
        std::string const iTitle;
        ng::font iFont;
        // the latest output, and where in it the trace starts
        std::vector<float> iSamples;
        std::size_t iStart = 0u;
        bool iLive = false;
    };
}
