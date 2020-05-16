// animator.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/game/system.hpp>

namespace neogfx::game
{
    class animator : public system
    {
    private:
        class thread;
    public:
        animator(game::i_ecs& aEcs);
        ~animator();
    public:
        const system_id& id() const override;
        const i_string& name() const override;
    public:
        bool apply() override;
        void terminate() override;
    public:
        struct meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x3b889df8, 0x9810, 0x4f61, 0xbcfc,{ 0xcf, 0xda, 0xb7, 0xea, 0x90, 0x27 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Animator";
                return sName;
            }
        };
    private:
        std::unique_ptr<thread> iThread;
    };
}