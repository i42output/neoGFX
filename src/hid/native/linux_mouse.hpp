// linux_mouse.hpp
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

#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/hid/mouse.hpp>

namespace neogfx
{
    namespace native::linux
    {
        class mouse : public neogfx::mouse
        {
        public:
            mouse();
        public:
            point position() const override;
            mouse_button button_state() const override;
        public:
            bool capturing() const override;
            i_surface& capture_target() const override;
            mouse_capture_type capture_type() const override;
            void capture(i_surface& aTarget, bool aLockCursor = true) override;
            void capture_raw(i_surface& aTarget, bool aLockCursor = true) override;
            void release_capture() override;
        private:
            i_surface* iCaptureTarget;
            mouse_capture_type iCaptureType;
        };
    }
}
