// i_basic_services.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/core/async_task.hpp>
#include <neogfx/core/primitives.hpp>

namespace neogfx
{
    class i_display;
    class i_native_clipboard;
    class i_shared_menu_bar;

    enum class platform : std::uint32_t
    {
        Unknown,
        Windows,
        macOS,
        Linux,
        Android,
        iOS
    };

    enum class windowing_system : std::uint32_t
    {
        Native,
        Wayland,
        X11
    };

    enum class environment : std::uint32_t
    {
        Native,
        Gnome,
        Kde
    };

    class i_basic_services : public i_service
    {
    public:
        struct bad_display_index : std::logic_error { bad_display_index() : std::logic_error("neogfx::i_basic_services::bad_display_index") {} };
        struct no_system_menu_bar : std::logic_error { no_system_menu_bar() : std::logic_error("neogfx::i_basic_services::no_system_menu_bar") {} };
    public:
        virtual ~i_basic_services() = default;
    public:
        virtual neogfx::platform platform() const = 0;
        virtual neogfx::windowing_system windowing_system() const = 0;
        virtual neogfx::environment environment() const = 0;
        virtual i_async_task& app_task() = 0;
        virtual void* helper_window_handle() const = 0;
        virtual void system_beep() = 0;
        virtual void display_error_dialog(i_string const& aTitle, i_string const& aMessage, void* aParentWindowHandle = 0) const = 0;
        virtual std::uint32_t display_count() const = 0;
        virtual i_display& display(std::uint32_t aDisplayIndex = 0) const = 0;
        virtual bool has_system_clipboard() const = 0;
        virtual i_native_clipboard& system_clipboard() = 0;
        virtual bool has_system_menu_bar() const = 0;
        virtual i_shared_menu_bar& system_menu_bar() = 0;
        virtual bool open_uri(std::string const& aUri) = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x86d1e3fa, 0xbf79, 0x4940, 0xa648, { 0xaf, 0xad, 0xbf, 0xf2, 0x9c, 0xcd } }; return sIid; }
    };
}