// i_native_graphics_context.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/geometry.hpp>
#include <neogfx/core/path.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/graphics_operations.hpp>

namespace neogfx
{
	class i_native_surface;

	class i_native_graphics_context
	{
	public:
		struct texture_not_resident : std::runtime_error { texture_not_resident() : std::runtime_error("neogfx::i_native_graphics_context::texture_not_resident") {} };
	public:
		virtual ~i_native_graphics_context() {}
		virtual std::unique_ptr<i_native_graphics_context> clone() const = 0;
	public:
		virtual const i_native_surface& surface() const = 0;
		virtual void enqueue(const graphics_operation::operation& aOperation) = 0;
		virtual void flush() = 0;
	public:
		virtual const std::pair<vec2, vec2>& logical_coordinates() const = 0;
	};
}