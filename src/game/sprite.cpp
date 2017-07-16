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
	sprite::sprite()
	{
	}

	sprite::sprite(const colour& aColour) :
		shape(aColour)
	{
	}

	sprite::sprite(const i_texture& aTexture, const optional_rect& aTextureRect) :
		shape(aTexture, aTextureRect)
	{
	}

	sprite::sprite(const i_image& aImage, const optional_rect& aTextureRect) :
		shape(aImage, aTextureRect)
	{
	}

	sprite::sprite(const sprite& aOther) :
		shape(aOther),
		iPath(aOther.iPath),
		iObject(aOther.iObject)
	{
	}

	vec3 sprite::origin() const
	{
		return physics().origin();
	}

	vec3 sprite::position() const
	{
		return physics().position();
	}

	mat44 sprite::transformation_matrix() const
	{
		if (shape::has_transformation_matrix())
			return shape::transformation_matrix();
		auto az = physics().angle_radians().z;
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
		physics().set_origin(aOrigin);
	}
	
	void sprite::set_position(const vec3& aPosition)
	{
		shape::set_position(aPosition);
		physics().set_position(aPosition);
	}

	void sprite::set_path(const optional_path& aPath)
	{
		iPath = aPath;
	}

	const i_physical_object& sprite::physics() const
	{
		return iObject;
	}

	i_physical_object& sprite::physics()
	{
		return iObject;
	}

	bool sprite::update(const optional_time_interval& aNow, const vec3& aForce)
	{
		bool updated = shape::update(aNow);
		return physics().update(aNow, aForce) || updated;
	}

	const sprite::optional_time_interval& sprite::update_time() const
	{
		return physics().update_time();
	}

	void sprite::set_update_time(const optional_time_interval& aLastUpdateTime)
	{
		physics().set_update_time(aLastUpdateTime);
	}

	bool sprite::update(const optional_time_interval& aNow)
	{
		return update(aNow, vec3{});
	}
}