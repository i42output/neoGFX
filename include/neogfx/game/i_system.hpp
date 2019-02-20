// i_system.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/i_set.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component.hpp>

namespace neogfx::game
{
    class i_system
    {
    public:
        virtual const system_id& id() const = 0;
        virtual const neolib::i_string& name() const = 0;
    public:
        virtual const neolib::i_set<component_id>& components() const = 0;
        virtual neolib::i_set<component_id>& components() = 0;
    public:
        virtual const i_component& component(component_id aComponentId) const = 0;
        virtual const i_component& component(component_id aComponentId) = 0;
    public:
        virtual void apply() = 0;
        virtual bool paused() const = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
        virtual void terminate() = 0;
    };
}