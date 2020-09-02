// i_skin_manager.hpp
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
#include <neogfx/core/i_event.hpp>
#include <neogfx/gui/widget/i_skin.hpp>

namespace neogfx
{
    class i_skin_manager : public i_service
    {
    public:
        declare_event(skin_registered, i_skin&)
        declare_event(skin_unregistered, i_skin&)
    public:
        struct skin_not_found : std::logic_error { skin_not_found() : std::logic_error{ "neogfx::i_skin_manager::skin_not_found" } {} };
        struct no_skin_active : std::logic_error { no_skin_active() : std::logic_error{ "neogfx::i_skin_manager::no_skin_active" } {} };
    public:
        virtual ~i_skin_manager() = default;
    public:
        virtual uint32_t skin_count() const = 0;
        virtual const i_skin& skin(uint32_t aIndex) const = 0;
        virtual i_skin& skin(uint32_t aIndex) = 0;
        virtual void register_skin(i_skin& aSkin) = 0;
        virtual void unregister_skin(i_skin& aSkin) = 0;
    public:
        virtual bool skin_active() const = 0;
        virtual const i_skin& active_skin() const = 0;
        virtual i_skin& active_skin() = 0;
        virtual void activate_skin(i_skin& aSkin) = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0xd764030b, 0x4b1, 0x411c, 0xa36c, { 0x74, 0xcf, 0xb4, 0x7a, 0x8a, 0x55 } }; return sIid; }
    };
}