// linux_display.hpp
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

#include <neogfx/hid/display.hpp>
#include <neogfx/hid/video_mode.hpp>

namespace neogfx
{
    namespace native::linux
    {
        class display_server;

        class display : public neogfx::display
        {
        public:
            display(linux::display_server& aDisplayServer, std::uint32_t aIndex);
            ~display();
        public:
            void update_dpi() override;
        public:
            bool is_fullscreen() const override;
            const video_mode& fullscreen_video_mode() const override;
            void enter_fullscreen(const video_mode& aVideoMode) override;
            void leave_fullscreen() override;
            neogfx::color_space color_space() const override;
            color read_pixel(const point& aPosition) const override;
        private:
            linux::display_server& iDisplayServer;
            std::optional<video_mode> iFullscreenVideoMode;
        };
    }
}
