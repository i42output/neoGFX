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
	sprite::sprite(i_shape_container& aContainer) :
		shape(aContainer)
	{
	}

	sprite::sprite(i_shape_container& aContainer, const colour& aColour) :
		shape(aContainer, aColour)
	{
	}

	sprite::sprite(i_shape_container& aContainer, const i_texture& aTexture, const optional_rect& aTextureRect) :
		shape(aContainer, aTexture, aTextureRect)
	{
	}

	sprite::sprite(i_shape_container& aContainer, const i_image& aImage, const optional_rect& aTextureRect) :
		shape(aContainer, aImage, aTextureRect)
	{
	}

	sprite::sprite(const sprite& aOther) :
		shape(aOther),
		iPath(aOther.iPath),
		iObject(aOther.iObject)
	{
	}

	point sprite::origin() const
	{
		return point{ physics().origin()[0], physics().origin()[1] };
	}

	point sprite::position() const
	{
		return point{ physics().position()[0], physics().position()[1] };
	}

	vec3 sprite::position_3D() const
	{
		return physics().position();
	}

	mat33 sprite::transformation_matrix() const
	{
		if (shape::has_transformation_matrix())
			return shape::transformation_matrix();
		auto az = physics().angle_radians()[2];
		auto pos = physics().position();
		return mat33{ { std::cos(az), -std::sin(az), 0.0 },{ std::sin(az), std::cos(az), 0.0 },{ pos[0], pos[1], 1.0 } };
	}

	const optional_path& sprite::path() const
	{
		return iPath;
	}
	
	void sprite::set_origin(const point& aOrigin)
	{
		shape::set_origin(aOrigin);
		physics().set_origin(aOrigin.to_vector3());
	}
	
	void sprite::set_position(const point& aPosition)
	{
		shape::set_position(aPosition);
		physics().set_position(aPosition.to_vector3());
	}
	
	void sprite::set_position_3D(const vec3& aPosition3D)
	{
		shape::set_position_3D(aPosition3D);
		physics().set_position(aPosition3D);
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

	std::size_t sprite::vertex_count(bool aIncludeCentre) const
	{
		return aIncludeCentre ? 5 : 4;
	}

	vec3_list sprite::vertices(bool aIncludeCentre) const
	{
		vec3_list result = shape::vertices(aIncludeCentre);
		auto r = bounding_box();
		result.push_back(r.top_left().to_vector3());
		result.push_back(r.top_right().to_vector3());
		result.push_back(r.bottom_right().to_vector3());
		result.push_back(r.bottom_left().to_vector3());
		return result;
	}
}