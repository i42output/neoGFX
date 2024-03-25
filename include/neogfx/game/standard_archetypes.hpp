// entity_archetype.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/game/renderable_entity_archetype.hpp>
#include <neogfx/game/rigid_body.hpp>
#include <neogfx/game/box_collider.hpp>
#include <neogfx/game/sprite.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/animation_filter.hpp>

namespace neogfx::game
{
    class sprite_archetype : public renderable_entity_archetype
    {
    public:
        sprite_archetype(const entity_archetype_id& aId, std::string const& aName) :
            renderable_entity_archetype{ aId, aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::mesh_filter::meta::id(), game::rigid_body::meta::id(), box_collider::meta::id() } }
        {
        }
        sprite_archetype(std::string const& aName) :
            renderable_entity_archetype{ aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::mesh_filter::meta::id(), game::rigid_body::meta::id(), box_collider::meta::id() } }
        {
        }
    };

    class sprite_2d_archetype : public renderable_entity_archetype
    {
    public:
        sprite_2d_archetype(const entity_archetype_id& aId, std::string const& aName) :
            renderable_entity_archetype{ aId, aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::mesh_filter::meta::id(), game::rigid_body::meta::id(), box_collider_2d::meta::id() } }
        {
        }
        sprite_2d_archetype(std::string const& aName) :
            renderable_entity_archetype{ aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::mesh_filter::meta::id(), game::rigid_body::meta::id(), box_collider_2d::meta::id() } }
        {
        }
    };

    class animated_sprite_archetype : public renderable_entity_archetype
    {
    public:
        animated_sprite_archetype(const entity_archetype_id& aId, std::string const& aName) :
            renderable_entity_archetype{ aId, aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::animation_filter::meta::id(), game::rigid_body::meta::id(), box_collider::meta::id() } }
        {
        }
        animated_sprite_archetype(std::string const& aName) :
            renderable_entity_archetype{ aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::animation_filter::meta::id(), game::rigid_body::meta::id(), box_collider::meta::id() } }
        {
        }
    };

    class animated_sprite_2d_archetype : public renderable_entity_archetype
    {
    public:
        animated_sprite_2d_archetype(const entity_archetype_id& aId, std::string const& aName) :
            renderable_entity_archetype{ aId, aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::animation_filter::meta::id(), game::rigid_body::meta::id(), box_collider_2d::meta::id() } }
        {
        }
        animated_sprite_2d_archetype(std::string const& aName) :
            renderable_entity_archetype{ aName, { game::sprite::meta::id(), game::mesh_renderer::meta::id(), game::animation_filter::meta::id(), game::rigid_body::meta::id(), box_collider_2d::meta::id() } }
        {
        }
    };

    class animation_archetype : public renderable_entity_archetype
    {
    public:
        animation_archetype(const entity_archetype_id& aId, std::string const& aName) :
            renderable_entity_archetype{ aId, aName, { game::mesh_renderer::meta::id(), game::animation_filter::meta::id() } }
        {
        }
        animation_archetype(std::string const& aName) :
            renderable_entity_archetype{ aName, { game::mesh_renderer::meta::id(), game::animation_filter::meta::id() } }
        {
        }
    };
}