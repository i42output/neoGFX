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
#include <neolib/ecs/ecs.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_vertex_provider.hpp>

namespace neogfx
{
    using namespace neolib::ecs;

    namespace game
    {
        class ecs : public neolib::ecs::ecs, public i_vertex_provider
        {
            typedef neolib::ecs::ecs base_type;
        public:
            ecs(ecs_flags aCreationFlags = ecs_flags::Default) : base_type{ aCreationFlags }
            {
                service<i_rendering_engine>().allocate_vertex_buffer(*this, vertex_buffer_type::DefaultECS);
            }
            ~ecs()
            {
                service<i_rendering_engine>().deallocate_vertex_buffer(*this);
            }
        };
    }
}