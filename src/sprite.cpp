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

#include "neogfx.hpp"
#include <boost/math/constants/constants.hpp>
#include "sprite.hpp"

namespace neogfx
{
	sprite::sprite()
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

	sprite::frame_index sprite::frame_count() const
	{
		return shape::frame_count();
	}

	const i_frame& sprite::frame(frame_index aFrameIndex) const
	{
		return shape::frame(aFrameIndex);
	}

	i_frame& sprite::frame(frame_index aFrameIndex)
	{
		return shape::frame(aFrameIndex);
	}

	void sprite::add_frame(i_frame& aFrame)
	{
		shape::add_frame(aFrame);
	}

	void sprite::add_frame(std::shared_ptr<i_frame> aFrame)
	{
		shape::add_frame(aFrame);
	}

	void sprite::replace_frame(frame_index aFrameIndex, i_frame& aFrame)
	{
		shape::replace_frame(aFrameIndex, aFrame);
	}

	void sprite::replace_frame(frame_index aFrameIndex, std::shared_ptr<i_frame> aFrame)
	{
		shape::replace_frame(aFrameIndex, aFrame);
	}

	void sprite::remove_frame(frame_index aFrameIndex)
	{
		shape::remove_frame(aFrameIndex);
	}

	void sprite::set_texture_rect_for_all_frames(const optional_rect& aTextureRect)
	{
		shape::set_texture_rect_for_all_frames(aTextureRect);
	}

	const sprite::animation_frames& sprite::animation() const
	{
		return shape::animation();
	}

	const i_frame& sprite::current_frame() const
	{
		return shape::current_frame();
	}

	i_frame& sprite::current_frame()
	{
		return shape::current_frame();
	}

	point sprite::origin() const
	{
		return point{ physics().origin()[0], physics().origin()[1] };
	}

	point sprite::position() const
	{
		return point{ physics().position()[0], physics().position()[1] };
	}

	rect sprite::bounding_box() const
	{
		return shape::bounding_box();
	}

	const vec2& sprite::scale() const
	{
		return shape::scale();
	}

	bool sprite::has_transformation_matrix() const
	{
		return shape::has_transformation_matrix();
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
	
	void sprite::set_animation(const animation_frames& aAnimation)
	{
		shape::set_animation(aAnimation);
	}

	void sprite::set_current_frame(frame_index aFrameIndex)
	{
		shape::set_current_frame(aFrameIndex);
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
	
	void sprite::set_bounding_box(const optional_rect& aBoundingBox)
	{
		shape::set_bounding_box(aBoundingBox);
	}

	void sprite::set_scale(const vec2& aScale)
	{
		shape::set_scale(aScale);
	}

	void sprite::set_transformation_matrix(const optional_mat33& aTransformationMatrix)
	{
		shape::set_transformation_matrix(aTransformationMatrix);
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

	bool sprite::update(const optional_time_point& aNow, const vec3& aForce)
	{
		bool updated = shape::update(aNow);
		return physics().update(aNow, aForce) || updated;
	}

	bool sprite::update(const optional_time_point& aNow)
	{
		return update(aNow, vec3{});
	}

	void sprite::paint(graphics_context& aGraphicsContext) const
	{
		shape::paint(aGraphicsContext);
	}
}