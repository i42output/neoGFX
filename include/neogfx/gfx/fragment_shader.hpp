// fragment_shader.hpp
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

#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_gradient_manager.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/shader_array.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gfx/i_shader_program.hpp>
#include <neogfx/gfx/shader.hpp>
#include <neogfx/gfx/i_fragment_shader.hpp>
#include "standard.frag.hpp"

namespace neogfx
{
    template <typename Base = i_fragment_shader>
    class fragment_shader : public shader<Base>
    {
        typedef shader<Base> base_type;
    public:
        fragment_shader(std::string const& aName) :
            base_type{ shader_type::Fragment, aName }
        {
        }
    };

    template <typename Base = i_fragment_shader>
    class standard_fragment_shader : public fragment_shader<Base>
    {
        typedef fragment_shader<Base> base_type;
    public:
        using base_type::add_in_variable;
        using base_type::add_out_variable;
    public:
        standard_fragment_shader(std::string const& aName = "standard_fragment_shader") :
            fragment_shader<Base>{ aName }
        {
            add_in_variable<vec3f>("Coord"_s, 0u);
            auto& fragColor = add_in_variable<vec4f>("Color"_s, 1u);
            add_in_variable<vec2f>("TexCoord"_s, 2u);
            auto& fragFunction0 = add_in_variable<vec4f>("Function0"_s, 3u, true);
            auto& fragFunction1 = add_in_variable<vec4f>("Function1"_s, 4u, true);
            auto& fragFunction2 = add_in_variable<vec4f>("Function2"_s, 5u, true);
            auto& fragFunction3 = add_in_variable<vec4f>("Function3"_s, 6u, true);
            auto& fragFunction4 = add_in_variable<vec4f>("Function4"_s, 7u, true);
            auto& fragFunction5 = add_in_variable<vec4f>("Function5"_s, 8u, true);
            auto& fragFunction6 = add_in_variable<vec4f>("Function6"_s, 9u, true);
            add_out_variable<vec4f>("FragColor"_s, 0u).link(fragColor);
            add_out_variable<vec4f>("FragFunction0"_s, 1u).link(fragFunction0);
            add_out_variable<vec4f>("FragFunction1"_s, 2u).link(fragFunction1);
            add_out_variable<vec4f>("FragFunction2"_s, 3u).link(fragFunction2);
            add_out_variable<vec4f>("FragFunction3"_s, 4u).link(fragFunction3);
            add_out_variable<vec4f>("FragFunction4"_s, 5u).link(fragFunction4);
            add_out_variable<vec4f>("FragFunction5"_s, 6u).link(fragFunction5);
            add_out_variable<vec4f>("FragFunction6"_s, 7u).link(fragFunction6);
        }
    public:
        bool supports(vertex_buffer_type aBufferType) const override
        {
            return (aBufferType & (
                vertex_buffer_type::Vertices | 
                vertex_buffer_type::Color | 
                vertex_buffer_type::Function0 | 
                vertex_buffer_type::Function1 | 
                vertex_buffer_type::Function2 | 
                vertex_buffer_type::Function3 | 
                vertex_buffer_type::Function4 |
                vertex_buffer_type::Function5 |
                vertex_buffer_type::Function6 )) != vertex_buffer_type::Invalid;
        }
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override
        {
            fragment_shader<Base>::generate_code(aProgram, aLanguage, aOutput);
            if (aProgram.is_first_in_stage(*this))
            {
                if (aLanguage == shader_language::Glsl)
                    aOutput += string{ glsl::StandardFragmentShader };
                else
                    throw unsupported_shader_language();
            }
        }
    };

    class standard_gradient_shader : public standard_fragment_shader<i_gradient_shader>
    {
    public:
        standard_gradient_shader(std::string const& aName = "standard_gradient_shader");
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    public:
        void clear_gradient() final;
        void set_gradient(i_rendering_context& aContext, const gradient& aGradient) final;
        void set_gradient(i_rendering_context& aContext, const game::gradient& aGradient) final;
    private:
        cache_uniform(uGradientGuiCoordinates)
        cache_uniform(uGradientDirection)
        cache_uniform(uGradientAngle)
        cache_uniform(uGradientStartFrom)
        cache_uniform(uGradientSize)
        cache_uniform(uGradientShape)
        cache_uniform(uGradientExponents)
        cache_uniform(uGradientCenter)
        cache_uniform(uGradientTile)
        cache_uniform(uGradientTileParams)
        cache_uniform(uGradientColorCount)
        cache_uniform(uGradientColorRow)
        cache_uniform(uGradientColors)
        cache_uniform(uGradientFilterSize)
        cache_uniform(uGradientFilter)
        cache_uniform(uGradientEnabled)
    };

    class standard_texture_shader : public standard_fragment_shader<i_texture_shader>
    {
    public:
        standard_texture_shader(std::string const& aName = "standard_texture_shader");
    public:
        bool supports(vertex_buffer_type aBufferType) const override;
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    public:
        void clear_texture() final;
        void set_texture(const i_texture& aTexture) final;
        void set_effect(shader_effect aEffect) final;
    private:
        cache_uniform(uTextureEnabled)
        cache_uniform(uTextureDataFormat)
        cache_uniform(uTextureMultisample)
        cache_uniform(uTextureExtents)
        cache_uniform(uTextureEffect)
    };

    class standard_filter_shader : public standard_fragment_shader<i_filter_shader>
    {
    public:
        standard_filter_shader(std::string const& aName = "standard_filter_shader");
    public:
        bool supports(vertex_buffer_type aBufferType) const override;
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    public:
        void clear_filter() final;
        void set_filter(shader_filter aFilter, scalar aArgument1 = 0.0, scalar aArgument2 = 0.0, scalar aArgument3 = 0.0, scalar aArgument4 = 0.0) final;
    private:
        cache_uniform(uFilterEnabled)
        cache_uniform(uFilterType)
        cache_uniform(uFilterArguments)
        cache_uniform(uFilterKernelSize)
        cache_uniform(uFilterKernel)
        std::map<std::pair<shader_filter, vec4>, std::optional<shader_array<float>>> iFilterKernel;
    };

    class standard_glyph_shader : public standard_fragment_shader<i_glyph_shader>
    {
    public:
        standard_glyph_shader(std::string const& aName = "standard_glyph_shader");
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    public:
        void clear_glyph() final;
        void set_first_glyph(const i_rendering_context& aContext, const glyph_text& aText, const glyph_char& aGlyphChar) final;
    private:
        cache_uniform(uGlyphRenderTargetExtents)
        cache_uniform(uGlyphGuiCoordinates)
        cache_uniform(uGlyphRenderOutput)
        cache_uniform(uGlyphSubpixel)
        cache_uniform(uGlyphSubpixelFormat)
        cache_uniform(uGlyphEnabled)
    };

    class standard_stipple_shader : public standard_fragment_shader<i_stipple_shader>
    {
    public:
        standard_stipple_shader(std::string const& aName = "standard_stipple_shader");
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    public:
        bool stipple_active() const final;
        void clear_stipple() final;
        void set_stipple(stipple const& aStipple) final;
        void start(const i_rendering_context& aContext, const vec3& aFrom) final;
        void next(const i_rendering_context& aContext, const vec3& aFrom, scalar aPositionOffset) final;
    private:
        scalar iPosition;
    private:
        cache_uniform(uStipplePattern)
        cache_uniform(uStipplePatternSize)
        cache_uniform(uStipplePatternLength)
        cache_uniform(uStipplePosition)
        cache_uniform(uStippleVertex)
        cache_uniform(uStippleEnabled)
    };

    class standard_shape_shader : public standard_fragment_shader<i_shape_shader>
    {
    public:
        standard_shape_shader(i_shader_program& aShaderProgram, std::string const& aName = "standard_shape_shader");
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    public:
        bool shape_active() const final;
        void clear_shape() final;
        void set_shape(shader_shape aShape) final;
        i_ssbo& shape_vertices() final;
    private:
        ref_ptr<i_ssbo> iShapeVertices;
        cache_shared_uniform(uShapeEnabled)
        cache_shared_uniform(uShape)
    };
}