// i_shape.hpp
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

#include <neogfx/neogfx.hpp>
#include <chrono>
#include <boost/optional.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometry.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/game/i_mesh.hpp>

namespace neogfx
{
	class i_frame
	{
	public:
		virtual bool has_extents() const = 0;
		virtual size extents() const = 0;
		virtual const optional_colour& colour() const = 0;
		virtual void set_colour(const optional_colour& aColour) = 0;
		virtual const optional_texture& texture() const = 0;
		virtual void set_texture(const optional_texture& aTexture) = 0;
		virtual const optional_rect& texture_rect() const = 0;
		virtual void set_texture_rect(const optional_rect& aTextureRect) = 0;
		virtual const optional_mat33& transformation() const = 0;
		virtual void set_transformation(const optional_mat33& aTransformation) = 0;
	};

	class i_widget;
	class i_shape;

	class i_shape_container
	{
	public:
		virtual const i_widget& as_widget() const = 0;
		virtual i_widget& as_widget() = 0;
	public:
		virtual bool has_buddy(const i_shape& aShape) const = 0;
		virtual i_shape& buddy(const i_shape& aShape) const = 0;
		virtual void set_buddy(const i_shape& aShape, i_shape& aBuddy, const vec3& aBuddyOffset = vec3{}) = 0;
		virtual const vec3& buddy_offset(const i_shape& aShape) const = 0;
		virtual void set_buddy_offset(const i_shape& aShape, const vec3& aBuddyOffset) = 0;
		virtual void unset_buddy(const i_shape& aShape) = 0;
	};

	class i_shape : public i_mesh
	{
		// types
	public:
		typedef std::size_t frame_index;
		typedef scalar time_interval;
		typedef std::vector<std::pair<frame_index, time_interval>> animation_frames;
		typedef boost::optional<time_interval> optional_time_interval;
		// exceptions
	public:
		struct no_buddy : std::logic_error { no_buddy() : std::logic_error("neogfx::i_shape::no_buddy") {} };
		struct bad_frame_index : std::logic_error { bad_frame_index() : std::logic_error("neogfx::i_shape::bad_frame_index") {} };
		// construction
	public:
		virtual ~i_shape() {}
		// container/buddy
	public:
		virtual i_shape_container& container() const = 0;
		virtual bool has_buddy() const = 0;
		virtual i_shape& buddy() const = 0;
		virtual void set_buddy(i_shape& aBuddy, const vec3& aBuddyOffset = vec3{}) = 0;
		virtual const vec3& buddy_offset() const = 0;
		virtual void set_buddy_offset(const vec3& aBuddyOffset) = 0;
		virtual void unset_buddy() = 0;
		// animation
	public:
		virtual frame_index frame_count() const = 0;
		virtual const i_frame& frame(frame_index aFrameIndex) const = 0;
		virtual i_frame& frame(frame_index aFrameIndex) = 0;
		virtual void add_frame(i_frame& aFrame) = 0;
		virtual void add_frame(std::shared_ptr<i_frame> aFrame) = 0;
		virtual void replace_frame(frame_index aFrameIndex, i_frame& aFrame) = 0;
		virtual void replace_frame(frame_index aFrameIndex, std::shared_ptr<i_frame> aFrame) = 0;
		virtual void remove_frame(frame_index aFrameIndex) = 0;
		virtual void set_texture_rect_for_all_frames(const optional_rect& aTextureRect) = 0;
		// geometry
	public:
		virtual const animation_frames& animation() const = 0;
		virtual const i_frame& current_frame() const = 0;
		virtual i_frame& current_frame() = 0;
		virtual point origin() const = 0;
		virtual point position() const = 0;
		virtual vec3 position_3D() const = 0;
		virtual rect bounding_box() const = 0;
		virtual const vec2& scale() const = 0;
		virtual bool has_transformation_matrix() const = 0;
		virtual mat33 transformation_matrix() const = 0;
		virtual void set_animation(const animation_frames& aAnimation) = 0;
		virtual void set_current_frame(frame_index aFrameIndex) = 0;
		virtual void set_origin(const point& aOrigin) = 0;
		virtual void set_position(const point& aPosition) = 0;
		virtual void set_position_3D(const vec3& aPosition3D) = 0;
		virtual void set_bounding_box(const optional_rect& aBoundingBox) = 0;
		virtual void set_scale(const vec2& aScale) = 0;
		virtual void set_transformation_matrix(const optional_mat33& aTransformationMatrix) = 0;
		// rendering
	public:
		virtual bool update(const optional_time_interval& aNow = optional_time_interval{}) = 0;
		virtual void paint(graphics_context& aGraphicsContext) const = 0;
		// helpers
	public:
		void set_size(const size& aSize)
		{
			set_bounding_box(rect{ origin() - (aSize / size{ 2.0 }), aSize });
		}
	};
}