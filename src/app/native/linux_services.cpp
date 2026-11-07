// linux_services.cpp
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

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/clipboard.hpp>
#include <neogfx/app/drag_drop.hpp>
#include "linux_basic_services.hpp"
#include "../../hid/native/linux_hid_devices.hpp"
#include "../../hid/native/linux_game_controllers.hpp"
#include "../../hid/native/linux_keyboard.hpp"
#include "../../hid/native/linux_mouse.hpp"
#include "../../hid/native/linux_window_manager.hpp"
#include "../../gfx/native/linux_renderer.hpp"
#include "linux_accessibility.hpp"

template<> neogfx::i_basic_services& services::start_service<neogfx::i_basic_services>()
{
    static neogfx::native::linux::basic_services sLinuxBasicServices{ service<i_async_task>() };
    return sLinuxBasicServices;
}

template<> neogfx::i_hid_devices& services::start_service<neogfx::i_hid_devices>()
{
    static neogfx::native::linux::hid_devices sHidDevices;
    return sHidDevices;
}

template<> neogfx::i_game_controllers& services::start_service<neogfx::i_game_controllers>()
{
    static neogfx::native::linux::game_controllers sGameControllers;
    return sGameControllers;
}

template<> neogfx::i_keyboard& services::start_service<neogfx::i_keyboard>()
{
    static auto sLinuxKeyboard = service<neogfx::i_hid_devices>().add_device<neogfx::native::linux::keyboard>();
    return *sLinuxKeyboard;
}

template<> neogfx::i_mouse& services::start_service<neogfx::i_mouse>()
{
    static auto sLinuxMouse = service<neogfx::i_hid_devices>().add_device<neogfx::native::linux::mouse>();
    return *sLinuxMouse;
}

template<> neogfx::i_rendering_engine& services::start_service<neogfx::i_rendering_engine>()
{
    auto const& programOptions = service<neogfx::i_app>().program_options();
    static neogfx::native::linux::renderer sLinuxRenderer{ programOptions.renderer() };
    return sLinuxRenderer;
}

template<> void services::teardown_service<neogfx::i_rendering_engine>()
{
    static_cast<neogfx::native::linux::renderer&>(service<neogfx::i_rendering_engine>()).~renderer();
    new(&service<neogfx::i_rendering_engine>()) neogfx::native::linux::renderer{ neogfx::renderer::None };
}

template<> neogfx::i_window_manager& services::start_service<neogfx::i_window_manager>()
{
    static neogfx::native::linux::window_manager sLinuxWindowManager;
    return sLinuxWindowManager;
}

template<> neogfx::i_clipboard& services::start_service<neogfx::i_clipboard>()
{
    static neogfx::clipboard sClipboard{ service<neogfx::i_basic_services>().system_clipboard() };
    return sClipboard;
}

template<> neogfx::i_drag_drop& services::start_service<neogfx::i_drag_drop>()
{
    // n.b. drag and drop within the application (not yet with other applications)
    static neogfx::drag_drop sDragDrop{};
    return sDragDrop;
}

template<> neogfx::i_accessibility& services::start_service<neogfx::i_accessibility>()
{
    static neogfx::native::linux::accessibility sAccessibility{};
    return sAccessibility;
}
