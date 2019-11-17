// system.hpp
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
#include <neolib/set.hpp>
#include <neolib/allocator.hpp>
#include <neogfx/game/i_system.hpp>

namespace neogfx::game
{
    class i_ecs;

    class system : public i_system
    {
    private:
        typedef neolib::set<component_id, std::less<component_id>, neolib::fast_pool_allocator<component_id>> component_list;
    public:
        system(game::i_ecs& aEcs);
        template <typename ComponentIdIter>
        system(game::i_ecs& aEcs, ComponentIdIter aFirstComponent, ComponentIdIter aLastComponent) :
            iEcs{ aEcs }, iComponents{ aFirstComponent, aLastComponent }, iPaused{ 0u }
        {
        }
        system(const system& aOther);
        system(system&& aOther);
    public:
        game::i_ecs& ecs() const;
    public:
        const neolib::i_set<component_id>& components() const override;
        neolib::i_set<component_id>& components() override;
    public:
        const i_component& component(component_id aComponentId) const override;
        i_component& component(component_id aComponentId) override;
    public:
        bool paused() const override;
        void pause() override;
        void resume() override;
        void terminate() override;
    protected:
        void yield();
    private:
        game::i_ecs& iEcs;
        component_list iComponents;
        std::atomic<uint32_t> iPaused;
    };
}