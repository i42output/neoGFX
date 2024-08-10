// fragment_shader.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/gfx/fragment_shader.hpp>
#include "standard-gradient.frag.hpp"
#include "standard-texture.frag.hpp"
#include "standard-filter.frag.hpp"
#include "standard-glyph.frag.hpp"
#include "standard-stipple.frag.hpp"
#include "standard-shape.frag.hpp"

namespace neogfx
{ 
    standard_gradient_shader::standard_gradient_shader(std::string const& aName) :
        standard_fragment_shader{ aName }
    {
        disable();
    }

    void standard_gradient_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_gradient_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardGradientFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_gradient_shader::clear_gradient()
    {
        uGradientEnabled = false;
    }

    void standard_gradient_shader::set_gradient(i_rendering_context& aContext, const gradient& aGradient)
    {
        enable();
        uGradientGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uGradientDirection = aGradient.direction();
        uGradientAngle = std::holds_alternative<double>(aGradient.orientation()) ? static_cast<float>(static_variant_cast<double>(aGradient.orientation())) : 0.0f;
        uGradientStartFrom = std::holds_alternative<corner>(aGradient.orientation()) ? static_cast<int>(static_variant_cast<corner>(aGradient.orientation())) : -1;
        uGradientSize = aGradient.size();
        uGradientShape = aGradient.shape();
        basic_vector<float, 2> gradientExponents = (aGradient.exponents() != std::nullopt ? *aGradient.exponents() : vec2{ 2.0, 2.0 });
        uGradientExponents = vec2f{ gradientExponents.x, gradientExponents.y };
        basic_point<float> gradientCenter = (aGradient.center() != std::nullopt ? *aGradient.center() : point{});
        uGradientCenter = vec2f{ gradientCenter.x, gradientCenter.y };
        uGradientTile = (aGradient.tile() != std::nullopt);
        if (aGradient.tile() != std::nullopt)
            uGradientTileParams = vec3{ aGradient.tile()->extents.cx, aGradient.tile()->extents.cy, aGradient.tile()->aligned ? 1.0 : 0.0 }.as<std::int32_t>();
        else
            uGradientTileParams = vec3i32{};
        uGradientColorCount = static_cast<int>(aGradient.colors().sampler().data().extents().cx);
        uGradientColorRow = static_cast<int>(aGradient.colors().sampler_row());
        uGradientFilterSize = static_cast<int>(aGradient.filter().sampler().data().extents().cx);
        aGradient.colors().sampler().data().bind(3);
        aGradient.filter().sampler().data().bind(4);
        uGradientColors = sampler2DRect{ 3 };
        uGradientFilter = sampler2DRect{ 4 };
        uGradientEnabled = true;
    }

    void standard_gradient_shader::set_gradient(i_rendering_context& aContext, const game::gradient& aGradient)
    {
        gradient g = service<i_gradient_manager>().find_gradient(aGradient.id.cookie());
        set_gradient(aContext, g);
    }

    standard_texture_shader::standard_texture_shader(std::string const& aName) :
        standard_fragment_shader<i_texture_shader>{ aName }
    {
        disable();
        set_uniform("tex"_s, sampler2D{1});
        set_uniform("texMS"_s, sampler2DMS{2});
        uTextureEffect = shader_effect::None;
    }

    bool standard_texture_shader::supports(vertex_buffer_type aBufferType) const
    {
        return enabled() && (aBufferType & vertex_buffer_type::UV) != vertex_buffer_type::Invalid;
    }

    void standard_texture_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_texture_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardTextureFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_texture_shader::clear_texture()
    {
        enable();
        uTextureEnabled = false;
        uTextureEffect = shader_effect::None;
        uTextureDataFormat = texture_data_format::RGBA;
        uTextureMultisample = texture_sampling::Normal;
        uTextureExtents = vec2f{};
    }

    void standard_texture_shader::set_texture(const i_texture& aTexture)
    {
        enable();
        uTextureEnabled = true;
        uTextureDataFormat = aTexture.data_format();
        uTextureMultisample = aTexture.sampling();
        uTextureExtents = aTexture.storage_extents().to_vec2().as<float>();
    }

    void standard_texture_shader::set_effect(shader_effect aEffect)
    {
        uTextureEffect = aEffect;
        if (aEffect == shader_effect::Ignore)
            uTextureEnabled = false;
    }

    standard_filter_shader::standard_filter_shader(std::string const& aName) :
        standard_fragment_shader<i_filter_shader>{ aName }
    {
        disable();
    }

    bool standard_filter_shader::supports(vertex_buffer_type aBufferType) const
    {
        return enabled() && (aBufferType & vertex_buffer_type::UV) != vertex_buffer_type::Invalid;
    }

    void standard_filter_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_filter_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardFilterFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_filter_shader::clear_filter()
    {
        uFilterEnabled = false;
    }

    void standard_filter_shader::set_filter(shader_filter aFilter, scalar aArgument1, scalar aArgument2, scalar aArgument3, scalar aArgument4)
    {
        enable();
        uFilterEnabled = true;
        uFilterType = aFilter;
        if (aFilter == shader_filter::GaussianBlur)
            aArgument1 += (1u - static_cast<std::uint32_t>(aArgument1) % 2u);
        auto const arguments = vec4{ aArgument1, aArgument2, aArgument3, aArgument4 };
        uFilterArguments = arguments.as<float>();
        auto kernel = iFilterKernel.find(std::make_pair(aFilter, arguments));
        if (kernel == iFilterKernel.end())
        {
            if (aFilter == shader_filter::GaussianBlur)
            {
                kernel = iFilterKernel.emplace(std::make_pair(aFilter, arguments), std::optional<shader_array<float>>{}).first;
                auto const kernelValues = dynamic_gaussian_filter<float>(static_cast<std::uint32_t>(aArgument1), static_cast<float>(aArgument2));
                kernel->second.emplace(size{ aArgument1, aArgument1 });
                kernel->second->data().set_pixels(rect{ point{0.0, 0.0}, size{aArgument1, aArgument1} }, &kernelValues[0][0]);
            }
        }
        uFilterKernelSize = static_cast<i32>(kernel->second->data().extents().cx);
        kernel->second->data().bind(5);
        uFilterKernel = sampler2DRect{ 5 };
    }

    standard_glyph_shader::standard_glyph_shader(std::string const& aName) :
        standard_fragment_shader<i_glyph_shader>{ aName }
    {
        disable();
    }

    void standard_glyph_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_glyph_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardGlyphFragmentShader };
        else
            throw unsupported_shader_language();
    }

    void standard_glyph_shader::clear_glyph()
    {
        uGlyphEnabled = false;
    }

    void standard_glyph_shader::set_first_glyph(const i_rendering_context& aContext, const glyph_text& aText, const glyph_char& aGlyphChar)
    {
        enable();
        bool subpixelRender = subpixel(aGlyphChar) && aText.glyph(aGlyphChar).subpixel();
        if (subpixelRender)
            aContext.render_target().target_texture().bind(7);
        uGlyphRenderTargetExtents = aContext.render_target().extents().to_vec2().as<std::int32_t>();
        uGlyphGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uGlyphRenderOutput = sampler2DMS{ 7 };
        uGlyphSubpixel = aText.glyph(aGlyphChar).subpixel();
        uGlyphSubpixelFormat = subpixelRender ? aContext.subpixel_format() : subpixel_format::None;
        uGlyphEnabled = true;
    }

    standard_stipple_shader::standard_stipple_shader(std::string const& aName) :
        standard_fragment_shader<i_stipple_shader>{ aName }, iPosition{ 0.0 }
    {
        disable();
    }

    void standard_stipple_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_stipple_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardStippleFragmentShader };
        else
            throw unsupported_shader_language();
    }

    bool standard_stipple_shader::stipple_active() const
    {
        return !uStippleEnabled.uniform().value().empty() && 
            uStippleEnabled.uniform().value().get<bool>();
    }

    void standard_stipple_shader::clear_stipple()
    {
        iPosition = 0.0;
        uStippleEnabled = false;
    }

    void standard_stipple_shader::set_stipple(std::uint16_t aPattern, scalar aFactor, scalar aPosition)
    {
        enable();
        iPosition = aPosition;
        uStippleFactor = static_cast<float>(aFactor);
        uStipplePattern = aPattern;
        uStipplePosition = static_cast<float>(iPosition);
        uStippleVertex = vec3f{};
        uStippleEnabled = true;
    }

    void standard_stipple_shader::start(const i_rendering_context& aContext, const vec3& aFrom)
    {
        next(aContext, aFrom, 0.0);
    }
    
    void standard_stipple_shader::next(const i_rendering_context& aContext, const vec3& aFrom, scalar aPositionOffset)
    {
        uStipplePosition = static_cast<float>(iPosition + aPositionOffset);
        uStippleVertex = aFrom.as<float>();
    }

    standard_shape_shader::standard_shape_shader(std::string const& aName) :
        standard_fragment_shader<i_shape_shader>{ aName }
    {
        uShape = shader_shape::None;
        uShapeEnabled = false;
        disable();
    }

    void standard_shape_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_shape_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardShapeFragmentShader };
        else
            throw unsupported_shader_language();
    }

    bool standard_shape_shader::shape_active() const
    {
        return !uShapeEnabled.uniform().value().empty() &&
            uShapeEnabled.uniform().value().get<bool>();
    }

    void standard_shape_shader::clear_shape()
    {
        uShapeEnabled = false;
    }

    void standard_shape_shader::set_shape(shader_shape aShape)
    {
        enable();
        uShape = aShape;
        uShapeEnabled = true;
    }
}