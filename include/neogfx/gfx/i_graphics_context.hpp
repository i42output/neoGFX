// i_graphics_context.hpp
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
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/graphics_operations.hpp>
#include <neogfx/gfx/i_render_target.hpp>

namespace neogfx
{
    class i_rendering_engine;

    class i_graphics_context
    {
    public:
        struct texture_not_resident : std::runtime_error { texture_not_resident() : std::runtime_error("neogfx::i_graphics_context::texture_not_resident") {} };
    public:
        virtual ~i_graphics_context() {}
        virtual std::unique_ptr<i_graphics_context> clone() const = 0;
    public:
        virtual i_rendering_engine& rendering_engine() = 0;
        virtual const i_render_target& render_target() const = 0;
        virtual const i_render_target& render_target() = 0;
        virtual rect rendering_area(bool aConsiderScissor = true) const = 0;
        virtual void enqueue(const graphics_operation::operation& aOperation) = 0;
        virtual void flush() = 0;
    public:
        virtual neogfx::logical_coordinates logical_coordinates() const = 0;
    };
}