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
		iCollisionMask{ 0ull }, iKilled { false }
	{
	}

	sprite::sprite(const colour& aColour) :
		shape{ aColour },
		iCollisionMask{ 0ull }, iKilled{ false }
	{
	}

	sprite::sprite(const i_texture& aTexture, const optional_animation_info& aAnimationInfo) :
		shape{ aTexture, aAnimationInfo },
		iCollisionMask{ 0ull }, iKilled{ false }
	{
	}

	sprite::sprite(const i_image& aImage, const optional_animation_info& aAnimationInfo) :
		shape{ aImage, aAnimationInfo },
		iCollisionMask{ 0ull }, iKilled{ false }
	{
	}

	sprite::sprite(const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		shape{ aTexture, aTextureRect, aAnimationInfo },
		iCollisionMask{ 0ull }, iKilled{ false }
	{
	}

	sprite::sprite(const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		shape{ aImage, aTextureRect, aAnimationInfo },
		iCollisionMask{ 0ull }, iKilled{ false }
	{
	}

	sprite::sprite(const sprite& aOther) :
		shape{ aOther },
		physical_object{ aOther },
		iPath{ aOther.iPath },
		iCollisionMask{ 0ull }, iKilled{ false }
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
		return iKilled;
	}

	void sprite::kill()
	{
		iKilled = true;
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
		shape::set_origin(aOrigin);
		physical_object::set_origin(aOrigin);
	}
	
	void sprite::set_position(const vec3& aPosition)
	{
		shape::set_position(aPosition);
		physical_object::set_position(aPosition);
	}

	void sprite::set_path(const optional_path& aPath)
	{
		iPath = aPath;
	}

	void sprite::clear_vertices_cache()
	{
		shape::clear_vertices_cache();
		clear_aabb_cache();
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

	sprite::aabb_type sprite::aabb() const
	{
		if (iAabb == boost::none)
		{
			auto tvs = transformed_vertices();
			auto iv = tvs[0];
			coordinate xMin = iv.coordinates.x;
			coordinate yMin = iv.coordinates.y;
			coordinate zMin = iv.coordinates.z;
			coordinate xMax = iv.coordinates.x;
			coordinate yMax = iv.coordinates.y;
			coordinate zMax = iv.coordinates.z;
			for (const auto& v : tvs)
			{
				xMin = std::min<coordinate>(xMin, v.coordinates.x);
				yMin = std::min<coordinate>(yMin, v.coordinates.y);
				zMin = std::min<coordinate>(zMin, v.coordinates.z);
				xMax = std::max<coordinate>(xMax, v.coordinates.x);
				yMax = std::max<coordinate>(yMax, v.coordinates.y);
				zMax = std::max<coordinate>(zMax, v.coordinates.z);
			}
			iAabb = aabb_type{ vec3{ xMin, yMin, zMin }, vec3{ xMax, yMax, zMax } };
		}
		return *iAabb;
	}

	void sprite::clear_aabb_cache()
	{
		iAabb = boost::none;
	}
}