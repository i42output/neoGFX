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

template<> neogfx::i_basic_services& services::start_service<neogfx::i_basic_services>()
{ 
    static neogfx::native::windows::basic_services sWindowsBasicServices{ service<i_async_task>() };
    return sWindowsBasicServices; 
}

template<> neogfx::i_hid_devices& services::start_service<neogfx::i_hid_devices>()
{
    static neogfx::native::windows::hid_devices sHidDevices;
    return sHidDevices;
}

template<> neogfx::i_game_controllers& services::start_service<neogfx::i_game_controllers>()
{
    static neogfx::native::windows::game_controllers sGameControllers;
    return sGameControllers;
}

template<> neogfx::i_keyboard& services::start_service<neogfx::i_keyboard>()
{ 
    static auto sWindowsKeyboard = service<neogfx::i_hid_devices>().add_device<neogfx::native::windows::keyboard>();
    return *sWindowsKeyboard;
}

template<> neogfx::i_mouse& services::start_service<neogfx::i_mouse>()
{
    static auto sWindowsMouse = service<neogfx::i_hid_devices>().add_device<neogfx::native::windows::mouse>();
    return *sWindowsMouse;
}

template<> neogfx::i_rendering_engine& services::start_service<neogfx::i_rendering_engine>()
{ 
    auto const& programOptions = service<neogfx::i_app>().program_options();
    static neogfx::native::windows::renderer sWindowsRenderer{ programOptions.renderer(), programOptions.double_buffering() };
    return sWindowsRenderer; 
}

template<> void services::teardown_service<neogfx::i_rendering_engine>()
{
    static_cast<neogfx::native::windows::renderer&>(service<neogfx::i_rendering_engine>()).~renderer();
    new(&service<neogfx::i_rendering_engine>()) neogfx::native::windows::renderer{ neogfx::renderer::None, false };
}

template<> neogfx::i_window_manager& services::start_service<neogfx::i_window_manager>()
{
    static neogfx::native::windows::window_manager sWindowsWindowManager;
    return sWindowsWindowManager;
}

//    template<> i_audio& start_service<i_audio>()
//    {
//        static native::windows::audio sWindowsAudio;
//        return sWindowsAudio;
//    };

template<> neogfx::i_clipboard& services::start_service<neogfx::i_clipboard>()
{
    static neogfx::clipboard sClipboard{ service<neogfx::i_basic_services>().system_clipboard() };
    return sClipboard;
}
