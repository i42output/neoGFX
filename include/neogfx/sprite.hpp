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
		// animation
	public:
		virtual frame_index frame_count() const;
		virtual const i_frame& frame(frame_index aFrameIndex) const;
		virtual i_frame& frame(frame_index aFrameIndex);
		virtual void add_frame(i_frame& aFrame);
		virtual void add_frame(std::shared_ptr<i_frame> aFrame);
		virtual void replace_frame(frame_index aFrameIndex, i_frame& aFrame);
		virtual void replace_frame(frame_index aFrameIndex, std::shared_ptr<i_frame> aFrame);
		virtual void remove_frame(frame_index aFrameIndex);
		virtual void set_texture_rect_for_all_frames(const optional_rect& aTextureRect);
		// geometry
	public:
		virtual const animation_frames& animation() const;
		virtual const i_frame& current_frame() const;
		virtual i_frame& current_frame();
		virtual point origin() const;
		virtual point position() const;
		virtual rect bounding_box() const;
		virtual const vec2& scale() const;
		virtual bool has_transformation_matrix() const;
		virtual mat33 transformation_matrix() const;
		virtual const optional_path& path() const;
		virtual void set_animation(const animation_frames& aAnimation);
		virtual void set_current_frame(frame_index aFrameIndex);
		virtual void set_origin(const point& aOrigin);
		virtual void set_position(const point& aPosition);
		virtual void set_bounding_box(const optional_rect& aBoundingBox);
		virtual void set_scale(const vec2& aScale);
		virtual void set_transformation_matrix(const optional_mat33& aTransformationMatrix);
		virtual void set_path(const optional_path& aPath);
		// physics
	public:
		virtual const i_physical_object& physics() const;
		virtual i_physical_object& physics();
		virtual bool update(const optional_time_point& aNow, const vec3& aForce);
		// rendering
	public:
		virtual bool update(const optional_time_point& aNow);
		virtual void paint(graphics_context& aGraphicsContext) const;
		// attributes
	private:
		optional_path iPath;
		physical_object iObject;
	};
}