// i_fragment_shader.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/color.hpp>
#include <neogfx/hid/i_display.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/text/glyph_text.hpp>
#include <neogfx/gfx/i_shader.hpp>

namespace neogfx
{
    struct no_stipple_vertex : std::logic_error { no_stipple_vertex() : std::logic_error{ "neogfx::no_stipple_vertex" } {} };

    class i_rendering_context;

    class i_fragment_shader : public i_shader
    {
    public:
        typedef i_fragment_shader abstract_type;
    };

    class i_gradient_shader : public i_fragment_shader
    {
    public:
        typedef i_gradient_shader abstract_type;
    public:
        virtual void clear_gradient() = 0;
        virtual void set_gradient(i_rendering_context& aContext, const gradient& aGradient) = 0;
        virtual void set_gradient(i_rendering_context& aContext, const game::gradient& aGradient) = 0;
    };

    class i_texture_shader : public i_fragment_shader
    {
    public:
        typedef i_texture_shader abstract_type;
    public:
        virtual void clear_texture() = 0;
        virtual void set_texture(const i_texture& aTexture) = 0;
        virtual void set_effect(shader_effect aEffect) = 0;
    };

    class i_filter_shader : public i_fragment_shader
    {
    public:
        typedef i_filter_shader abstract_type;
    public:
        virtual void clear_filter() = 0;
        virtual void set_filter(shader_filter aFilter, scalar aArgument1 = 0.0, scalar aArgument2 = 0.0, scalar aArgument3 = 0.0, scalar aArgument4 = 0.0) = 0;
    };

    class i_glyph_shader : public i_fragment_shader
    {
    public:
        typedef i_glyph_shader abstract_type;
    public:
        virtual void clear_glyph() = 0;
        virtual void set_first_glyph(const i_rendering_context& aContext, const glyph_text& aText, const glyph_char& aGlyphChar) = 0;
    };

    class i_stipple_shader : public i_fragment_shader
    {
    public:
        typedef i_stipple_shader abstract_type;
    public:
        virtual bool stipple_active() const = 0;
        virtual void clear_stipple() = 0;
        virtual void set_stipple(stipple const& aStipple) = 0;
        virtual void start(const i_rendering_context& aContext, const vec3& aFrom) = 0;
        virtual void next(const i_rendering_context& aContext, const vec3& aFrom, scalar aPositionOffset) = 0;
    };

    class i_shape_shader : public i_fragment_shader
    {
    public:
        typedef i_shape_shader abstract_type;
    public:
        virtual bool shape_active() const = 0;
        virtual void clear_shape() = 0;
        virtual void set_shape(shader_shape aShape) = 0;
    };
}