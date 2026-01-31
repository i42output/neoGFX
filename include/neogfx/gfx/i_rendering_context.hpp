// i_rendering_context.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/hid/i_display.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/graphics_operations.hpp>
#include <neogfx/gfx/i_render_target.hpp>

namespace neogfx
{
    class i_rendering_engine;
    class i_gradient_shader;

    class i_rendering_context
    {
    public:
        struct texture_not_resident : std::runtime_error { texture_not_resident() : std::runtime_error("neogfx::i_rendering_context::texture_not_resident") {} };
    public:
        virtual ~i_rendering_context() = default;
        virtual std::unique_ptr<i_rendering_context> clone() const = 0;
    public:
        virtual i_rendering_engine& rendering_engine() const = 0;
        virtual const i_render_target& render_target() const = 0;
        virtual rect rendering_area(bool aConsiderScissor = true) const = 0;
        virtual graphics_operation::queue& queue() const = 0;
        virtual void enqueue(const graphics_operation::operation& aOperation) = 0;
        virtual void flush() = 0;
    public:
        virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
        virtual neogfx::logical_coordinates logical_coordinates() const = 0;
        virtual vec2 offset() const = 0;
        virtual void set_offset(const optional_vec2& aOffset) = 0;
        virtual void blit(const rect& aDestinationRect, const i_texture& aTexture, const rect& aSourceRect, blending_mode aBlendingMode = blending_mode::Blit) = 0;
        virtual bool gradient_set() const = 0;
        virtual void apply_gradient(i_gradient_shader& aShader) = 0;
    public:
        virtual neogfx::subpixel_format subpixel_format() const = 0;
    };
}