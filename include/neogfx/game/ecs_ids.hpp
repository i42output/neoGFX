// ecs_ids.hpp
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
#include <neolib/uuid.hpp>
#include <neolib/cookie_jar.hpp>

namespace neogfx::game
{
    typedef neolib::uuid entity_archetype_id;
    typedef neolib::uuid component_id;
    typedef neolib::uuid system_id;
    typedef void* handle_t;
    typedef neolib::cookie id_t;
    constexpr id_t null_id = 0;
    typedef id_t handle_id;
    typedef id_t entity_id;
    constexpr entity_id null_entity = 0;
}
