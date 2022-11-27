// windows_basic_services.hpp
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
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gfx/color.hpp>
#include <neogfx/hid/i_display.hpp>

namespace neogfx
{
    namespace native::windows
    {
        class basic_services : public i_basic_services
        {
        public:
            basic_services(i_async_task& aAppThread);
            ~basic_services();
        public:
            neogfx::platform platform() const override;
            i_async_task& app_task() override;
            void system_beep() override;
            void display_error_dialog(std::string const& aTitle, std::string const& aMessage, void* aParentWindowHandle = 0) const override;
            uint32_t display_count() const override;
            i_display& display(uint32_t aDisplayIndex = 0) const override;
            bool has_system_clipboard() const override;
            i_native_clipboard& system_clipboard() override;
            bool has_system_menu_bar() const override;
            i_shared_menu_bar& system_menu_bar() override;
            bool open_uri(std::string const& aUri) override;
        private:
            i_async_task& iAppTask;
            mutable std::optional<uint32_t> iDisplayCount;
            mutable std::vector<std::unique_ptr<i_display>> iDisplays;
        };
    }
}