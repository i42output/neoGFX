// i_render_target.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/primitives.hpp>

namespace neogfx
{
	class i_texture;

	enum class render_target_type
	{
		Surface,
		Texture
	};

	class i_render_target
	{
	public:
		virtual render_target_type target_type() const = 0;
		virtual void* target_handle() const = 0;
		virtual const i_texture& target_texture() const = 0;
		virtual size target_extents() const = 0;
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) = 0;
		virtual const neogfx::logical_coordinates& logical_coordinates() const = 0;
		virtual void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) = 0;
	public:
		virtual bool activate_target() const = 0;
		virtual bool deactivate_target() const = 0;
	};
}