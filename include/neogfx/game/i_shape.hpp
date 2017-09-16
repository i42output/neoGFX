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
#include <neogfx/gfx/texture.hpp>
#include <neogfx/game/i_object.hpp>
#include <neogfx/game/i_mesh.hpp>

namespace neogfx
{
	class graphics_context;

	class i_frame
	{
	public:
		virtual bool has_extents() const = 0;
		virtual size extents() const = 0;
		virtual const optional_effect_colour& colour() const = 0;
		virtual void set_colour(const optional_effect_colour& aColour) = 0;
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
	};

	class i_shape : public i_object, public i_mesh
	{
		// types
	public:
		typedef std::size_t frame_index;
		typedef scalar time_interval;
		typedef std::pair<frame_index, time_interval> animation_frame;
		typedef std::vector<animation_frame> animation_frames;
		typedef boost::optional<time_interval> optional_time_interval;
		// exceptions
	public:
		struct no_shape_container : std::logic_error { no_shape_container() : std::logic_error("neogfx::i_shape::no_shape_container") {} };
		struct not_a_tag : std::logic_error { not_a_tag() : std::logic_error("neogfx::i_shape::not_a_tag") {} };
		struct bad_frame_index : std::logic_error { bad_frame_index() : std::logic_error("neogfx::i_shape::bad_frame_index") {} };
		// construction
	public:
		virtual ~i_shape() {}
		// container
	public:
		virtual const i_shape_container& container() const = 0;
		virtual i_shape_container& container() = 0;
		// tag
	public:
		virtual bool is_tag() const = 0;
		virtual i_shape& tag_of() const = 0;
		virtual void set_tag_of(i_shape& aTagOf, const vec3& aOffset = vec3{}) = 0;
		virtual const vec3& tag_offset() const = 0;
		virtual void set_tag_offset(const vec3& aOffset) = 0;
		virtual void unset_tag_of() = 0;
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
		virtual bool repeat_animation() const = 0;
		virtual const animation_frame& current_animation_frame() const = 0;
		virtual bool has_animation_finished() const = 0;
		virtual void animation_finished() = 0;
		virtual frame_index current_frame_index() const = 0;
		virtual const i_frame& current_frame() const = 0;
		virtual i_frame& current_frame() = 0;
		virtual vec3 origin() const = 0;
		virtual vec3 position() const = 0;
		virtual vec3 extents() const = 0;
		virtual rect bounding_box_2d(bool aWithPosition = true) const = 0;
		virtual void set_animation(const animation_frames& aAnimation) = 0;
		virtual void set_current_frame(frame_index aFrameIndex) = 0;
		virtual void set_origin(const vec3& aOrigin) = 0;
		virtual void set_position(const vec3& aPosition) = 0;
		virtual void clear_extents() = 0;
		virtual void set_extents(const vec3& aExtents) = 0;
		virtual bool has_transformation_matrix() const = 0;
		virtual void clear_transformation_matrix() = 0;
		virtual void set_transformation_matrix(const mat33& aTransformationMatrix) = 0;
		virtual void set_transformation_matrix(const mat44& aTransformationMatrix) = 0;
		// rendering
	public:
		virtual bool update(time_interval aNow) = 0;
		virtual void paint(graphics_context& aGraphicsContext) const = 0;
		// helpers
	public:
		void set_origin(const vec2& aOrigin)
		{
			set_origin(vec3{ aOrigin.x, aOrigin.y, 0.0 });
		}
		void set_origin(const point& aOrigin)
		{
			set_origin(vec3{ aOrigin.x, aOrigin.y, 0.0 });
		}
		void set_position(const vec2& aPosition)
		{
			set_position(vec3{ aPosition.x, aPosition.y, 0.0 });
		}
		void set_position(const point& aPosition)
		{
			set_position(vec3{ aPosition.x, aPosition.y, 0.0 });
		}
		void set_extents(const vec2& aExtents)
		{
			set_extents(vec3{ aExtents.x, aExtents.y, 0.0 });
		}
		void set_extents(const size& aExtents)
		{
			set_extents(vec3{ aExtents.cx, aExtents.cy, 0.0 });
		}
	};
}