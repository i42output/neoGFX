// ecs.hpp
/*
  neolib C++ App/Game Engine
  Copyright (c)  2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/i_vertex_provider.hpp>
#include <neolib/ecs/ecs.hpp>

namespace neogfx
{
    using namespace neolib::ecs;

    namespace game
    {
        class ecs : public neolib::ecs::ecs, public i_vertex_provider
        {
            typedef neolib::ecs::ecs base_type;
        public:
            ecs(ecs_flags aCreationFlags = ecs_flags::Default);
            ~ecs();
        public:
            bool run_threaded(const system_id& aSystemId) const override;
        public:
            void destroy_entity(entity_id aEntityId, bool aNotify = true) override;
        public:
            bool cacheable() const override;
            const game::static_component<game::mesh_render_cache>& cache() const override;
            game::static_component<game::mesh_render_cache>& cache() override;
        };

        template <typename... Systems>
        std::shared_ptr<ecs> make_ecs(ecs_flags aCreationFlags = ecs_flags::Default)
        {
            auto newEcs = std::make_shared<ecs>(aCreationFlags);
            (newEcs->system<Systems>(), ...);
            return newEcs;
        }
    }
}