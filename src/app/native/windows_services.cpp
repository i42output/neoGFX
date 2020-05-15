// windows_services.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/clipboard.hpp>
#include "windows_basic_services.hpp"
#include "../../hid/native/windows_hid_devices.hpp"
#include "../../hid/native/windows_game_controllers.hpp"
#include "../../hid/native/windows_keyboard.hpp"
#include "../../hid/native/windows_mouse.hpp"
#include "../../hid/native/windows_window_manager.hpp"
#include "../../gfx/native/windows_renderer.hpp"
//#include "../../audio/native/windows_audio.hpp"

namespace neogfx
{
    template<> i_basic_services& service<i_basic_services>() 
    { 
        static native::windows::basic_services sWindowsBasicServices{ service<async_task>() }; 
        return sWindowsBasicServices; 
    }

    template<> hid_devices& service<hid_devices>()
    {
        static native::windows::hid_devices sHidDevices;
        return sHidDevices;
    }

    template<> i_hid_devices& service<i_hid_devices>()
    {
        return service<hid_devices>();
    }

    template<> game_controllers& service<game_controllers>()
    {
        static native::windows::game_controllers sGameControllers;
        return sGameControllers;
    }

    template<> i_game_controllers& service<i_game_controllers>()
    {
        return service<game_controllers>();
    }

    template<> i_keyboard& service<i_keyboard>()
    { 
        static auto sWindowsKeyboard = service<hid_devices>().add_device<native::windows::keyboard>();
        return *sWindowsKeyboard;
    }

    template<> i_mouse& service<i_mouse>()
    {
        static auto sWindowsMouse = service<hid_devices>().add_device<native::windows::mouse>();
        return *sWindowsMouse;
    }

    template<> i_rendering_engine& service<i_rendering_engine>()
    { 
        auto const& programOptions = service<i_app>().program_options();
        static native::windows::renderer sWindowsRenderer{ programOptions.renderer(), programOptions.double_buffering() };
        return sWindowsRenderer; 
    }

    template<> void teardown_service<i_rendering_engine>()
    {
        static_cast<native::windows::renderer&>(service<i_rendering_engine>()).~renderer();
        new(&service<i_rendering_engine>()) native::windows::renderer{ neogfx::renderer::None, false };
    }

    template<> i_window_manager& service<i_window_manager>()
    {
        static native::windows::window_manager sWindowsWindowManager;
        return sWindowsWindowManager;
    }

//    template<> i_audio& service<i_audio>()
//    {
//        static native::windows::audio sWindowsAudio;
//        return sWindowsAudio;
//    };

    template<> i_clipboard& service<i_clipboard>()
    {
        static clipboard sClipboard{ service<i_basic_services>().system_clipboard() };
        return sClipboard;
    }
}