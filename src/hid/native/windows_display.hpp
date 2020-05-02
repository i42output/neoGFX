// windows_display.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

namespace neogfx
{
    namespace native::windows
    {
        class display : public neogfx::display
        {
        public:
            display(uint32_t aIndex, const neogfx::rect& aRect, const neogfx::rect& aDesktopRect, void* aNativeDisplayHandle, void* aNativeDeviceContextHandle);
            ~display();
        public:
            void update_dpi() override;
        public:
            bool is_fullscreen() const override;
            const video_mode& fullscreen_video_mode() const override;
            void enter_fullscreen(const video_mode& aVideoMode) override;
            void leave_fullscreen() override;
            color read_pixel(const point& aPosition) const override;
        private:
            void* iNativeDisplayHandle;
            void* iNativeDeviceContextHandle;
            std::optional<DEVMODE> iDesktopDisplaySettings;
            std::optional<std::pair<video_mode, DEVMODE>> iFullscreenDisplaySettings;
        };
    }
}