// i_sprite.hpp
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
#include <chrono>
#include <boost/optional.hpp>
#include "geometry.hpp"
#include "graphics_context.hpp"
#include "i_shape.hpp"
#include "i_physical_object.hpp"

namespace neogfx
{
	class i_sprite
	{
	public:
		typedef i_physical_object::time_point time_point;
		typedef i_physical_object::optional_time_point optional_time_point;
	public:
		virtual ~i_sprite() {}
	public:
		virtual const optional_path& path() const = 0;
		virtual void set_path(const optional_path& aPath) = 0;
	public:
		virtual const i_shape& as_shape() const = 0;
		virtual i_shape& as_shape() = 0;
	public:
		virtual const i_physical_object& physics() const = 0;
		virtual i_physical_object& physics() = 0;
	public:
		virtual bool update(const optional_time_point& aNow = optional_time_point(), const vec3& aForce = vec3{}) = 0;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const = 0;
		// convenience helpers
	public:
		void set_animation(const i_shape::animation_frames& aAnimation) { as_shape().set_animation(aAnimation); }
		void set_current_frame(i_shape::frame_index aFrameIndex) { as_shape().set_current_frame(aFrameIndex); }
		void set_origin(const point& aOrigin) { as_shape().set_origin(aOrigin); }
		void set_position(const point& aPosition) { as_shape().set_position(aPosition); }
		void set_bounding_box(const optional_rect& aBoundingBox) { as_shape().set_bounding_box(aBoundingBox); }
		void set_scale(const vec2& aScale) { as_shape().set_scale(aScale); }
		void set_transformation_matrix(const optional_mat33& aTransformationMatrix) { as_shape().set_transformation_matrix(aTransformationMatrix); }
		void set_size(const size& aSize) { as_shape().set_bounding_box(rect{ as_shape().origin() - (aSize / size{ 2.0 }), aSize }); }
	};
}