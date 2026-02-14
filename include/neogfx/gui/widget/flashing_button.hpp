// flashing_button.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/neogfx.hpp>

#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx
{
    class flashing_button : public push_button
    {
    public:
        /// @todo custom/variable blink interval
        static constexpr std::chrono::milliseconds DEFAULT_BLINK_INTERVAL_ms{ 250 };
    public:
        flashing_button(const std::string aText) :
            push_button{ aText }
        {
            service<i_rendering_engine>().register_frame_counter(*this, DEFAULT_BLINK_INTERVAL_ms);
        }
        flashing_button(const neogfx::image& aImage) :
            push_button{ aImage }
        {
            service<i_rendering_engine>().register_frame_counter(*this, DEFAULT_BLINK_INTERVAL_ms);
        }
        flashing_button(const std::string aText, const neogfx::image& aImage) :
            push_button{ aText, aImage }
        {
            service<i_rendering_engine>().register_frame_counter(*this, DEFAULT_BLINK_INTERVAL_ms);
        }
        flashing_button(i_layout& aLayout, const std::string aText) :
            push_button{ aLayout, aText }
        {
            service<i_rendering_engine>().register_frame_counter(*this, DEFAULT_BLINK_INTERVAL_ms);
        }
        flashing_button(i_layout& aLayout, const neogfx::image& aImage) :
            push_button{ aLayout, aImage }
        {
            service<i_rendering_engine>().register_frame_counter(*this, DEFAULT_BLINK_INTERVAL_ms);
        }
        ~flashing_button()
        {
            service<i_rendering_engine>().unregister_frame_counter(*this, DEFAULT_BLINK_INTERVAL_ms);
        }
    public:
        bool blinking() const
        {
            return iBlinking;
        }
        void blink(bool aBlinking)
        {
            iBlinking = aBlinking;
        }
        void blink_on()
        {
            blink(true);
        }
        void blink_off()
        {
            blink(false);
        }
    public:
        color face_color() const override
        {
            color faceColor = push_button::face_color();
            bool const on = effectively_enabled() && (!blinking() || push_button::is_checked() || service<i_rendering_engine>().frame_counter(DEFAULT_BLINK_INTERVAL_ms) % 2 == 1);
            faceColor = faceColor.with_lightness(on ? 0.9 : 0.5);
            return faceColor;
        }
    private:
        bool iBlinking = false;
    };
}