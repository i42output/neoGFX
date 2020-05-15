// windows_xinput_controller.hpp
/*
  neogfx C++ App/Game Engine
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
#include <d3d11_1.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <xinput.h>
#pragma comment(lib, "Xinput.lib")
#include <neogfx/hid/game_controller.hpp>

namespace neogfx
{
    namespace native::windows
    {
        class xinput_controller : public game_controller
        {
        public:
            xinput_controller(IDirectInputDevice8* aDevice, hid_device_subclass aSubclass, const hid_device_uuid& aProductId, const hid_device_uuid& aInstanceId);
            ~xinput_controller();
        protected:
            void update_state() override;
        private:
            static const button_map_type& xinput_button_map();
        private:
            IDirectInputDevice8* iDevice;
        };
    }
}