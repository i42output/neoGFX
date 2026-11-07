// linux_display.cpp
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

#include <neogfx/neogfx.hpp>

#include "../../app/native/linux_display_server.hpp"
#include "linux_display.hpp"

namespace neogfx
{
    namespace native::linux
    {
        display::display(linux::display_server& aDisplayServer, std::uint32_t aIndex) :
            neogfx::display{ aIndex, aDisplayServer.output(aIndex).monitor, aDisplayServer.output(aIndex).workArea },
            iDisplayServer{ aDisplayServer }
        {
            update_dpi();
            iSubpixelFormat = iDisplayServer.output(index()).subpixelFormat;
        }

        display::~display()
        {
        }

        void display::update_dpi()
        {
            iPixelDensityDpi = iDisplayServer.output(index()).dpi;
        }

        bool display::is_fullscreen() const
        {
            return iFullscreenVideoMode != std::nullopt;
        }

        const video_mode& display::fullscreen_video_mode() const
        {
            if (is_fullscreen())
                return *iFullscreenVideoMode;
            throw fullscreen_not_active();
        }

        void display::enter_fullscreen(const video_mode& aVideoMode)
        {
            // n.b. a client cannot change the video mode (on Wayland, or on X11 without XRandR, which is for the
            // desktop's settings to do), so a fullscreen window is the size of the monitor
            iFullscreenVideoMode = aVideoMode;
        }

        void display::leave_fullscreen()
        {
            if (!is_fullscreen())
                throw fullscreen_not_active();
            iFullscreenVideoMode = std::nullopt;
        }

        color_space display::color_space() const
        {
            return neogfx::color_space::sRGB;
        }

        color display::read_pixel(const point& aPosition) const
        {
            return iDisplayServer.read_pixel(aPosition);
        }
    }
}
