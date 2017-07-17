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

#include <neogfx/neogfx.hpp>
#include "shape.hpp"
#include "physical_object.hpp"
#include "i_sprite.hpp"

namespace neogfx
{
	class sprite : public physical_object, public shape<i_sprite>
	{
		// types
	public:
		using i_sprite::time_interval;
		using i_sprite::optional_time_interval;
		using i_sprite::step_time_interval;
		// construction
	public:
		sprite();
		sprite(const colour& aColour);
		sprite(const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		sprite(const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
		sprite(const sprite& aOther);
		// object
	public:
		object_category category() const override;
		bool destroyed() const override;
		// geometry
	public:
		vec3 origin() const override;
		vec3 position() const override;
		mat44 transformation_matrix() const override;
		const optional_path& path() const override;
		void set_origin(const vec3& aOrigin) override;
		using i_shape::set_origin;
		void set_position(const vec3& aPosition) override;
		using i_shape::set_position;
		void set_path(const optional_path& aPath) override;
		// udates
	public:
		void clear_vertices_cache() override;
		// physics
	public:
		const i_physical_object& physics() const override;
		i_physical_object& physics() override;
		bool update(const optional_time_interval& aNow, const vec3& aForce) override;
		const optional_time_interval& update_time() const override;
		void set_update_time(const optional_time_interval& aLastUpdateTime) override;
		// physical object
	public:
		aabb_type aabb() const override;
		// own
	public:
		void destroy();
		// attributes
	private:
		optional_path iPath;
		mutable boost::optional<aabb_type> iAabb;
		bool iDestroyed;
	};
}