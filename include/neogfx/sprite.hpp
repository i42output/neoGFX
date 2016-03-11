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
	public:
		typedef i_physical_object::time_point time_point;
		typedef i_physical_object::optional_time_point optional_time_point;
	public:
		sprite();
		sprite(const i_texture& aTexture, const optional_rect& aTextureRect);
		sprite(const i_image& aImage, const optional_rect& aTextureRect);
		sprite(const sprite& aOther);
	public:
		virtual point origin() const;
		virtual point position() const;
		virtual const optional_path& path() const;
		virtual mat33 transformation_matrix() const;
		virtual void set_origin(const point& aOrigin);
		virtual void set_position(const point& aPosition);
		virtual void set_path(const optional_path& aPath);
	public:
		virtual const i_shape& as_shape() const;
		virtual i_shape& as_shape();
	public:
		virtual const i_physical_object& physics() const;
		virtual i_physical_object& physics();
	public:
		virtual bool update(const optional_time_point& aNow = optional_time_point(), const vec3& aForce = vec3{});
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		optional_path iPath;
		physical_object iObject;
	};
}