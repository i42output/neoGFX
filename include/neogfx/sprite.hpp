// sprite.hpp
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
#pragma once

#include "neogfx.hpp"
#include "shape.hpp"
#include "physical_object.hpp"
#include "i_sprite.hpp"

namespace neogfx
{
	class sprite : public shape, public i_sprite
	{
		// types
	public:
		using i_sprite::time_point;
		using i_sprite::optional_time_point;
		// construction
	public:
		sprite();
		sprite(const i_texture& aTexture, const optional_rect& aTextureRect);
		sprite(const i_image& aImage, const optional_rect& aTextureRect);
		sprite(const sprite& aOther);
		// geometry
	public:
		virtual point origin() const;
		virtual point position() const;
		virtual const optional_path& path() const;
		virtual mat33 transformation_matrix() const;
		virtual void set_animation(const animation_frames& aAnimation);
		virtual void set_current_frame(frame_index aFrameIndex);
		virtual void set_origin(const point& aOrigin);
		virtual void set_position(const point& aPosition);
		virtual void set_bounding_box(const optional_rect& aBoundingBox);
		virtual void set_scale(const vec2& aScale);
		virtual void set_transformation_matrix(const optional_mat33& aTransformationMatrix);
		virtual void set_path(const optional_path& aPath);
		// geometry/rendering
	public:
		virtual const i_shape& as_shape() const;
		virtual i_shape& as_shape();
		// physics
	public:
		virtual const i_physical_object& physics() const;
		virtual i_physical_object& physics();
		virtual bool update(const optional_time_point& aNow = optional_time_point(), const vec3& aForce = vec3{});
		// rendering
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
		// attributes
	private:
		optional_path iPath;
		physical_object iObject;
	};
}