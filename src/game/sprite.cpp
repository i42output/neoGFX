// sprite.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>
#include <boost/math/constants/constants.hpp>
#include <neogfx/game/sprite.hpp>

namespace neogfx
{
	sprite::sprite() :
		iCollisionMask{ 0ull }
	{
	}

	sprite::sprite(const colour& aColour) :
		shape{ aColour },
		iCollisionMask{ 0ull }
	{
	}

	sprite::sprite(const i_texture& aTexture, const optional_animation_info& aAnimationInfo) :
		shape{ aTexture, aAnimationInfo },
		iCollisionMask{ 0ull }
	{
	}

	sprite::sprite(const i_image& aImage, const optional_animation_info& aAnimationInfo) :
		shape{ aImage, aAnimationInfo },
		iCollisionMask{ 0ull }
	{
	}

	sprite::sprite(const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		shape{ aTexture, aTextureRect, aAnimationInfo },
		iCollisionMask{ 0ull }
	{
	}

	sprite::sprite(const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		shape{ aImage, aTextureRect, aAnimationInfo },
		iCollisionMask{ 0ull }
	{
	}

	sprite::sprite(const sprite& aOther) :
		shape{ aOther },
		physical_object{ aOther },
		iPath{ aOther.iPath },
		iCollisionMask{ 0ull }
	{
	}

	object_category sprite::category() const
	{
		return object_category::Sprite;
	}

	const i_shape& sprite::as_shape() const
	{
		return *this;
	}

	i_shape& sprite::as_shape()
	{
		return *this;
	}

	const i_collidable& sprite::as_collidable() const
	{
		return *this;
	}

	i_collidable& sprite::as_collidable()
	{
		return *this;
	}

	const i_physical_object& sprite::as_physical_object() const
	{
		return *this;
	}

	i_physical_object& sprite::as_physical_object()
	{
		return *this;
	}

	uint64_t sprite::collision_mask() const
	{
		return iCollisionMask;
	}

	void sprite::set_collision_mask(uint64_t aMask)
	{
		iCollisionMask = aMask;
	}

	bool sprite::killed() const
	{
		return physical_object::killed() || shape<i_sprite>::killed();
	}

	void sprite::kill()
	{
		physical_object::kill();
		shape<i_sprite>::kill();
	}

	void sprite::animation_finished()
	{
		kill();
	}

	vec3 sprite::origin() const
	{
		return physical_object::origin();
	}

	vec3 sprite::position() const
	{
		return physical_object::position();
	}

	mat44 sprite::transformation_matrix() const
	{
		if (shape::has_transformation_matrix())
			return shape::transformation_matrix();
		auto az = physical_object::angle_radians().z;
		// todo: following rotation is 2D, make it 3D...
		return mat44{ { std::cos(az), -std::sin(az), 0.0, 0.0 }, { std::sin(az), std::cos(az), 0.0, 0.0 }, { 0.0, 0.0, 1.0, 0.0 }, { position().x, position().y, position().z, 1.0 } };
	}

	const optional_path& sprite::path() const
	{
		return iPath;
	}
	
	void sprite::set_origin(const vec3& aOrigin)
	{
		physical_object::set_origin(aOrigin);
		shape::set_origin(aOrigin);
	}
	
	void sprite::set_position(const vec3& aPosition)
	{
		physical_object::set_position(aPosition);
		shape::set_position(aPosition);
	}

	void sprite::set_path(const optional_path& aPath)
	{
		iPath = aPath;
	}

	void sprite::clear_vertices_cache()
	{
		physical_object::clear_vertices_cache();
		shape::clear_vertices_cache();
	}

	void sprite::clear_aabb_cache()
	{
		iAabb = boost::none;
	}

	const i_physical_object& sprite::physics() const
	{
		return *this;
	}

	i_physical_object& sprite::physics()
	{
		return *this;
	}

	bool sprite::update(const optional_time_interval& aNow, const vec3& aForce)
	{
		return physical_object::update(aNow, aForce);
	}

	const sprite::optional_time_interval& sprite::update_time() const
	{
		return physical_object::update_time();
	}

	void sprite::set_update_time(const optional_time_interval& aLastUpdateTime)
	{
		physical_object::set_update_time(aLastUpdateTime);
	}

	const aabb& sprite::aabb() const
	{
		if (iAabb == boost::none)
		{
			const auto& tvs = transformed_vertices();
			const auto& iv = tvs[0];
			vec3 min = iv.coordinates;
			vec3 max = min;
			for (const auto& v : tvs)
			{
				min = min.min(v.coordinates);
				max = max.max(v.coordinates);
			}
			iAabb.emplace(min, max);
		}
		return *iAabb;
	}
}