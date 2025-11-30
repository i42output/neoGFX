// i_context_menu.hpp
/*
neogfx C++ App/Game Engine
Copyright (c) 2025 Leigh Johnston.  All Rights Reserved.

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

namespace neogfx
{
    class i_menu;

    class i_context_menu : public i_service
    {
    public:
        virtual bool context_menu_active() const noexcept = 0;
        virtual bool context_menu_cancelling() const noexcept = 0;
    public:
        virtual void activate_context_menu(i_menu& aMenu) noexcept = 0;
        virtual void deactivate_context_menu() noexcept = 0;
        virtual void cancel_context_menu() noexcept = 0;
        virtual void close_context_menu() noexcept = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0xd60d38cb, 0x7b0d, 0x4d15, 0x8720, { 0x7d, 0x77, 0x8b, 0xb, 0x6, 0x81 } }; return sIid; }
    };
}