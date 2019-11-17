// sdl_services.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include "sdl_basic_services.hpp"
#include "../../hid/native/sdl_keyboard.hpp"
#include "../../hid/native/sdl_window_manager.hpp"
#include "../../gfx/native/sdl_renderer.hpp"
#include "../../audio/native/sdl_audio.hpp"

namespace neogfx
{
    template<> i_basic_services& service<i_basic_services>() 
    { 
        static sdl_basic_services sSdlBasicServices{ service<neolib::async_task>() }; 
        return sSdlBasicServices; 
    }

    template<> i_keyboard& service<i_keyboard>()
    { 
        static sdl_keyboard sSdlKeyboard; 
        return sSdlKeyboard; 
    }

    template<> i_rendering_engine& service<i_rendering_engine>()
    { 
        auto const& programOptions = service<i_app>().program_options();
        static sdl_renderer sSdlRenderer{ programOptions.renderer(), programOptions.double_buffering() };
        return sSdlRenderer; 
    }

    template<> void teardown_service<i_rendering_engine>()
    {
        static_cast<sdl_renderer&>(service<i_rendering_engine>()).~sdl_renderer();
        new(&service<i_rendering_engine>()) sdl_renderer{ neogfx::renderer::None, false };
    }

    template<> i_window_manager& service<i_window_manager>()
    {
        static sdl_window_manager sSdlWindowManager;
        return sSdlWindowManager;
    }

    template<> i_audio& service<i_audio>()
    {
        static sdl_audio sSdlAudio;
        return sSdlAudio;
    };

    template<> i_clipboard& service<i_clipboard>()
    {
        static clipboard sClipboard{ service<i_basic_services>().system_clipboard() };
        return sClipboard;
    }
}