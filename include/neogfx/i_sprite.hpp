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
	class i_sprite : public i_shape
	{
		// types
	public:
		typedef i_physical_object::time_point time_point;
		typedef i_physical_object::optional_time_point optional_time_point;
		// physics
	public:
		virtual const i_physical_object& physics() const = 0;
		virtual i_physical_object& physics() = 0;
		virtual bool update(const optional_time_point& aNow = optional_time_point(), const vec3& aForce = vec3{}) = 0;
		// geometry
	public:
		virtual const optional_path& path() const = 0;
		virtual void set_path(const optional_path& aPath) = 0;
		// rendering
	public:
		virtual void paint(graphics_context& aGraphicsContext) const = 0;
		// helpers
	public:
		void set_size(const size& aSize) { set_bounding_box(rect{ origin() - (aSize / size{ 2.0 }), aSize }); }
	};
}