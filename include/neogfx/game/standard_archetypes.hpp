// entity_archetype.hpp
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
#include <neogfx/game/entity_archetype.hpp>
#include <neogfx/game/rigid_body.hpp>
#include <neogfx/game/collider.hpp>
#include <neogfx/game/sprite.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>

namespace neogfx::game
{
	class sprite_archetype : public entity_archetype
	{
	public:
		sprite_archetype(const entity_archetype_id& aId, const std::string& aName) :
			entity_archetype{ aId, aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::mesh_filter::meta::id(), game::rigid_body::meta::id(), box_collider::meta::id() } }
		{
		}
		sprite_archetype(const std::string& aName) :
			entity_archetype{ aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::mesh_filter::meta::id(), game::rigid_body::meta::id(), box_collider::meta::id() } }
		{
		}
	};

}