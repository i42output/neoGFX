/*
neogfx C++ App/Game Engine - Examples - Games - Video Poker
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

#include <neolib/timer.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/push_button.hpp>

namespace video_poker
{
    class flashing_button : public neogfx::push_button
    {
    public:
        flashing_button(neogfx::i_layout& aLayout, const std::string aText) :
            push_button{ aLayout, aText }
        {
            neogfx::service<neogfx::i_rendering_engine>().register_frame_counter(*this, 250);
        }
        ~flashing_button()
        {
            neogfx::service<neogfx::i_rendering_engine>().unregister_frame_counter(*this, 250);
        }
    public:
        neogfx::color animation_color() const override
        {
            neogfx::color faceColor = push_button::animation_color();
            faceColor = faceColor.with_lightness(
                effectively_enabled() && 
                (push_button::is_checked() || neogfx::service<neogfx::i_rendering_engine>().frame_counter(250) % 2 == 1) ? 0.9 : 0.5);
            return faceColor;
        }
    private:
    };
}