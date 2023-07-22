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
        {
            static const string code =
            {
                1 + R"glsl(
vec4 gradient_color(in float n)
{
    n = max(min(n, 1.0), 0.0) * float(uGradientColorCount - 1);
    vec4 firstColor = texelFetch(uGradientColors, ivec2(floor(n), uGradientColorRow));
    vec4 secondColor = texelFetch(uGradientColors, ivec2(ceil(n), uGradientColorRow));
    return mix(firstColor, secondColor, n - floor(n));
}
                
float ellipse_radius(vec2 ab, vec2 center, vec2 pt)
{
    vec2 d = pt - center;
    float angle = 0;
    vec2 ratio = vec2(1.0, 1.0);
    if (ab.x >= ab.y)
        ratio.y = ab.x / ab.y;
    else
        ratio.x = ab.y / ab.x;
    angle = atan(d.y * ratio.y, d.x * ratio.x);
    float x = pow(abs(cos(angle)), 2.0 / uGradientExponents.x) * sign(cos(angle)) * ab.x;
    float y = pow(abs(sin(angle)), 2.0 / uGradientExponents.y) * sign(sin(angle)) * ab.y;
    return sqrt(x * x + y * y);
}
                
vec4 color_at(vec2 viewPos, vec4 boundingBox)
{
    vec2 s = boundingBox.zw - boundingBox.xy;
    vec2 pos = viewPos - boundingBox.xy;
    if (uGradientTile)
    {
        if (uGradientDirection != 1) /* vertical */
        {
            float adjust = 0;
            if (uGradientTileParams.z == 1)
                adjust = float(int(boundingBox.y) % uGradientTileParams.y);
            float frac = 1 / float(uGradientTileParams.y);
            pos.y = floor((float(int(pos.y + adjust) % uGradientTileParams.y) * frac + frac / 2) * s.y);
        }
        if (uGradientDirection != 0) /* horizontal */
        {
            float adjust = 0;
            if (uGradientTileParams.z == 1)
                adjust = float(int(boundingBox.x) % uGradientTileParams.x);
            float frac = 1 / float(uGradientTileParams.x);
            pos.x = floor((float(int(pos.x + adjust) % uGradientTileParams.x) * frac + frac / 2) * s.x);
        }
    }
    pos.x = max(min(pos.x, s.x - 1.0), 0.0);
    pos.y = max(min(pos.y, s.y - 1.0), 0.0);
    float gradientPos;
    if (uGradientDirection == 0) /* vertical */
    {
        gradientPos = pos.y / s.y;
        if (!uGradientGuiCoordinates)
            gradientPos = 1.0 - gradientPos;
    }
    else if (uGradientDirection == 1) /* horizontal */
        gradientPos = pos.x / s.x;
    else if (uGradientDirection == 2) /* diagonal */
    {
        vec2 center = s / 2.0;
        float angle;
        switch (uGradientStartFrom)
        {
        case 0:
            angle = atan(center.y, -center.x);
            break;
        case 1:
            angle = atan(-center.y, -center.x);
            break;
        case 2:
            angle = atan(-center.y, center.x);
            break;
        case 3:
            angle = atan(center.y, center.x);
            break;
        default:
            angle = uGradientAngle;
            break;
        }
        pos.y = s.y - pos.y;
        pos = pos - center;
        mat2 rot = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
        pos = rot * pos;
        pos = pos + center;
        gradientPos = pos.y / s.y;
    }
    else if (uGradientDirection == 3) /* rectangular */
    {
        float vert = pos.y / s.y;
        if (vert > 0.5)
            vert = 1.0 - vert;
        float horz = pos.x / s.x;
        if (horz > 0.5)
            horz = 1.0 - horz;
        gradientPos = min(vert, horz) * 2.0;
    }
    else /* radial */
    {
        vec2 ab = s / 2.0;
        pos -= ab;
        vec2 center = ab * uGradientCenter;
        float d = distance(center, pos);
        vec2 c1 = boundingBox.xy - boundingBox.xy - ab;
        vec2 c2 = boundingBox.xw - boundingBox.xy - ab;
        vec2 c3 = boundingBox.zw - boundingBox.xy - ab;
        vec2 c4 = boundingBox.zy - boundingBox.xy - ab;
        vec2 cc = c1;
        if (distance(center, c2) < distance(center, cc))
            cc = c2;
        if (distance(center, c3) < distance(center, cc))
            cc = c3;
        if (distance(center, c4) < distance(center, cc))
            cc = c4;
        vec2 fc = c1;
        if (distance(center, c2) > distance(center, fc))
            fc = c2;
        if (distance(center, c3) > distance(center, fc))
            fc = c3;
        if (distance(center, c4) > distance(center, fc))
            fc = c4;
        vec2 cs = vec2(min(abs(-ab.x + center.x), abs(ab.x + center.x)), min(abs(-ab.y + center.y), abs(ab.y + center.y)));
        vec2 fs = vec2(max(abs(-ab.x + center.x), abs(ab.x + center.x)), max(abs(-ab.y + center.y), abs(ab.y + center.y)));
        float r;
        if (uGradientShape == 0) // Ellipse
        {
            switch (uGradientSize)
            {
            default:
            case 0: // ClosestSide
                r = ellipse_radius(cs, center, pos);
                break;
            case 1: // FarthestSide
                r = ellipse_radius(fs, center, pos);
                break;
            case 2: // ClosestCorner
                r = ellipse_radius(abs(cc - center), center, pos);
                break;
            case 3: // FarthestCorner
                r = ellipse_radius(abs(fc - center), center, pos);
                break;
            }
        }
        else if (uGradientShape == 1) // Circle
        {
            switch (uGradientSize)
            {
            default:
            case 0:
                r = min(cs.x, cs.y);
                break;
            case 1:
                r = max(fs.x, fs.y);
                break;
            case 2:
                r = distance(cc, center);
                break;
            case 3:
                r = distance(fc, center);
                break;
            }
        }
        if (d < r)
            gradientPos = d / r;
        else
            gradientPos = 1.0;
    }
    return gradient_color(gradientPos);
}
                
void standard_gradient_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uGradientEnabled)
    {
        int d = uGradientFilterSize / 2;
        if (texelFetch(uGradientFilter, ivec2(d, d)).r == 1.0)
        {
            color = color_at(Coord.xy, function0) * vec4(1.0, 1.0, 1.0, color.a);  
        }
        else
        {
            vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
            for (int fy = -d; fy <= d; ++fy)
            {
                for (int fx = -d; fx <= d; ++fx)
                {
                    sum += (color_at(Coord.xy + vec2(fx, fy), function0) * texelFetch(uGradientFilter, ivec2(fx + d, fy + d)).r);
                }
            }
            color = sum * vec4(1.0, 1.0, 1.0, color.a); 
        }
    }
}
                )glsl"
            };
            aOutput += code;
        }
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
            uGradientTileParams = vec3{ aGradient.tile()->extents.cx, aGradient.tile()->extents.cy, aGradient.tile()->aligned ? 1.0 : 0.0 }.as<int32_t>();
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
        {
            static const string code 
            {
                1 + R"glsl(
vec4 texel_at(vec2 texCoord, int sampleId)
{
    vec4 texel = vec4(0.0);
    if (uTextureMultisample < 5) // Scaled
    {
        texel = texture(tex, texCoord).rgba;
    }
    else
    {
        texel = texelFetch(texMS, ivec2(texCoord * uTextureExtents), sampleId).rgba;
    }
    switch(uTextureDataFormat)
    {
    case 1: // RGBA
    default:
        break;
    case 2: // Red
        texel = vec4(1.0, 1.0, 1.0, texel.r);
        break;
    case 3: // SubPixel
        texel = vec4(1.0, 1.0, 1.0, (texel.r + texel.g + texel.b) / 3.0);
        break;
    }
    return texel;
}
vec4 texel_at(vec2 texCoord)
{
    return texel_at(texCoord, gl_SampleID);
}
vec4 combined_texel_at(vec2 texCoord)
{
    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < gl_NumSamples; ++i)
    {
        sum += texel_at(texCoord, i);
    }
    return sum / float(gl_NumSamples);
}
void standard_texture_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uTextureEnabled)
    {
        vec4 texel = texel_at(TexCoord);
        switch(uTextureEffect)
        {
        case 0: // effect: None
            color = texel.rgba * color;
            break;
        case 1: // effect: Colorize, ColorizeAverage
            {
                float avg = (texel.r + texel.g + texel.b) / 3.0;
                color = vec4(avg, avg, avg, texel.a) * color;
            }
            break;
        case 2: // effect: ColorizeMaximum
            {
                float maxChannel = max(texel.r, max(texel.g, texel.b));
                color = vec4(maxChannel, maxChannel, maxChannel, texel.a) * color;
            }
            break;
        case 3: // effect: ColorizeSpot
            color = vec4(1.0, 1.0, 1.0, texel.a) * color;
            break;
        case 4: // effect: ColorizeAlpha
            {
                float avg = (texel.r + texel.g + texel.b) / 3.0;
                color = vec4(1.0, 1.0, 1.0, texel.a * avg) * color;
            }
            break;
        case 5: // effect: Monochrome
            {
                float gray = dot(color.rgb * texel.rgb, vec3(0.299, 0.587, 0.114));
                color = vec4(gray, gray, gray, texel.a) * color;
            }
            break;
        case 10: // effect: Filter
            break;
        case 99: // effect: Ignore
            break;
        }
    }
}
                )glsl"
            };
            aOutput += code;
        }
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
        {
            static const string code
            {
                1 + R"glsl(
void standard_filter_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uFilterEnabled)
    {
        switch(uFilterType)
        {
        case 0: // filter: None
            break;
        case 1: // effect: GaussianBlur
            {
                int d = uFilterKernelSize / 2;
                if (texelFetch(uFilterKernel, ivec2(d, d)).r != 1.0)
                {
                    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
                    for (int fy = -d; fy <= d; ++fy)
                    {
                        for (int fx = -d; fx <= d; ++fx)
                        {
                            vec4 texel = texel_at(TexCoord + vec2(fx, fy) / uTextureExtents); 
                            sum += (texel * texelFetch(uFilterKernel, ivec2(fx + d, fy + d)).r);
                        }
                    }
                    color = sum;
                }
            }
            break;
        }
    }
}
                )glsl"
            };
            aOutput += code;
        }
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
            aArgument1 += (1u - static_cast<uint32_t>(aArgument1) % 2u);
        auto const arguments = vec4{ aArgument1, aArgument2, aArgument3, aArgument4 };
        uFilterArguments = arguments.as<float>();
        auto kernel = iFilterKernel.find(std::make_pair(aFilter, arguments));
        if (kernel == iFilterKernel.end())
        {
            if (aFilter == shader_filter::GaussianBlur)
            {
                kernel = iFilterKernel.emplace(std::make_pair(aFilter, arguments), std::optional<shader_array<float>>{}).first;
                auto const kernelValues = dynamic_gaussian_filter<float>(static_cast<uint32_t>(aArgument1), static_cast<float>(aArgument2));
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
        {
            static const string code
            {
                1 + R"glsl(
ivec2 render_position()
{
    if (uGlyphGuiCoordinates)
        return ivec2(Coord.x, uGlyphRenderTargetExtents.y - Coord.y);
    else
        return ivec2(Coord.xy);
}
                
vec3 output_pixel()
{
    return texelFetch(uGlyphRenderOutput, render_position(), 0).rgb;
}
                
void standard_glyph_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uGlyphEnabled)
    {
        float a = 0.0;
        if (uGlyphSubpixel)
        {
            vec4 aaaAlpha = texture(tex, TexCoord);
            if (aaaAlpha.rgb == vec3(1.0, 1.0, 1.0))
                return;
            else if (aaaAlpha.rgb == vec3(0.0, 0.0, 0.0))
                discard;
            else
            {
                switch(uGlyphSubpixelFormat)
                {
                default:
                    a = (aaaAlpha.r + aaaAlpha.g + aaaAlpha.b) / 3.0;
                    color = vec4(color.xyz, color.a * a);
                    break;
                case 1: // RGBHorizontal
                    color = vec4(color.rgb * aaaAlpha.rgb * color.a + output_pixel() * (vec3(1.0, 1.0, 1.0) - aaaAlpha.rgb * color.a), 1.0);
                    break;
                case 2: // BGRHorizontal
                    color = vec4(color.rgb * aaaAlpha.bgr * color.a + output_pixel() * (vec3(1.0, 1.0, 1.0) - aaaAlpha.bgr * color.a), 1.0);
                    break;
                }
            }
        }
        else
        {
            a = texture(tex, TexCoord).r;
            if (a == 0)
                discard;
            color = vec4(color.xyz, color.a * a);
        }
    }
}
                )glsl"
            };
            aOutput += code;
        }
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
        uGlyphRenderTargetExtents = aContext.render_target().extents().to_vec2().as<int32_t>();
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
        {
            static const string code
            {
                1 + R"glsl(
void standard_stipple_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uStippleEnabled)
    {
        float d = 0.0;
        if (!uShapeEnabled)
            d = distance(uStippleVertex, Coord);
        else if (uShape == SHAPE_Circle)
        {
            float y = Coord.y - function1.y;
            float x = Coord.x - function1.x;
            d = (atan(y, x) + PI) * function1.z;
        }
        uint patternBit = uint((d + uStipplePosition) / uStippleFactor) % 16;
        if ((uStipplePattern & (1 << patternBit)) == 0)
            discard;
    }
}
                )glsl"
            };
            aOutput += code;
        }
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

    void standard_stipple_shader::set_stipple(scalar aFactor, uint16_t aPattern, scalar aPosition)
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
        {
            static const string code
            {
                // Parts hereof...
                // The MIT License
                // Copyright © 2019 Inigo Quilez
                // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the Software), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

                1 + R"glsl(
void draw_line(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float dy = function1.y - function1.w;
    float dx = function1.x - function1.z;
    float m = dy / dx; // GLSL allows divide-by-zero, we won't use the Inf
    float c = function1.y - m * function1.x;
    float d = dx != 0.0 ? (abs(m) < 1.0 ? distance(vec2(Coord.x, m * Coord.x + c), Coord.xy) : distance(vec2((Coord.y - c) / m, Coord.y), Coord.xy)) : abs(Coord.x - function1.x);
    color = vec4(color.xyz, color.a * (1.0 - smoothstep(function2.x / 2.0 - 0.5, function2.x / 2.0 + 0.5, d)));
}
// https://www.shadertoy.com/view/XdVBWd
// The MIT License
// Copyright © 2018 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \Software\), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED \AS IS\, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
                
float bezier_sdSegmentSq(in vec2 p, in vec2 a, in vec2 b)
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa,ba) / dot(ba,ba), 0.0, 1.0);
    return dot(pa - ba * h, pa - ba * h);
}
                
float bezier_sdSegment(in vec2 p, in vec2 a, in vec2 b)
{
    return sqrt(bezier_sdSegmentSq(p,a,b));
}
                
// todo: research a non-iterative approach
float bezier_udBezier(vec2 p0, vec2 p1, vec2 p2, in vec2 p3, vec2 pos)
{
    const int kNum = 50;
    float res = 1e10;
    vec2 a = p0;
    for (int i = 1; i < kNum; i++)
    {
        float t = float(i) / float(kNum - 1);
        float s = 1.0 - t;
        vec2 b = p0 * s * s * s + p1 * 3.0 * s * s * t + p2 * 3.0 * s * t * t + p3 * t * t * t;
        float d = bezier_sdSegmentSq(pos, a, b);
        if (d < res) res = d;
        a = b;
    }
    return sqrt(res);
}
void draw_cubic_bezier(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d = bezier_udBezier(function1.xy, function1.zw, function2.xy, function2.zw, Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5)));
    color = vec4(color.xyz, color.a * (1.0 - smoothstep(function3.x / 2.0 - 0.5, function3.x / 2.0 + 0.5, d)));
}
float sdTriangle(in vec2 p, in vec2 p0, in vec2 p1, in vec2 p2)
{
    vec2 e0 = p1 - p0, e1 = p2 - p1, e2 = p0 - p2;
    vec2 v0 = p - p0, v1 = p - p1, v2 = p - p2;
    vec2 pq0 = v0 - e0 * clamp(dot(v0,e0) / dot(e0,e0), 0.0, 1.0);
    vec2 pq1 = v1 - e1 * clamp(dot(v1,e1) / dot(e1,e1), 0.0, 1.0);
    vec2 pq2 = v2 - e2 * clamp(dot(v2,e2) / dot(e2,e2), 0.0, 1.0);
    float s = sign(e0.x * e2.y - e0.y * e2.x);
    vec2 d = min(min(vec2(dot(pq0,pq0), s * (v0.x * e0.y - v0.y * e0.x)),
                        vec2(dot(pq1,pq1), s * (v1.x * e1.y - v1.y * e1.x))),
                        vec2(dot(pq2,pq2), s * (v2.x * e2.y - v2.y * e2.x)));
    return -sqrt(d.x) * sign(d.y);
}
void draw_triangle(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d0 = sdTriangle(Coord.xy, function1.xy, function1.zw, function2.xy);
    if (function2.w == 1.0)
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    else
        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.z / 2.0 - 0.5, function2.z / 2.0 + 0.5, abs(d0))));
}
void draw_circle(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d = distance(function1.xy, Coord.xy) - function1.z;
    if (function2.x == 1.0)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function1.w / 2.0 - 0.5, function1.w / 2.0 + 0.5, d)));
    else if (abs(d) >= function1.w / 2.0 - 0.5)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function1.w / 2.0 - 0.5, function1.w / 2.0 + 0.5, abs(d))));
}
float sdEllipse(in vec2 p, in vec2 ab)
{
    p = abs(p); if (p.x > p.y) { p = p.yx; ab = ab.yx; }
    float l = ab.y * ab.y - ab.x * ab.x;
    float m = ab.x * p.x / l;      float m2 = m * m;
    float n = ab.y * p.y / l;      float n2 = n * n;
    float c = (m2 + n2 - 1.0) / 3.0; float c3 = c * c * c;
    float q = c3 + m2 * n2 * 2.0;
    float d = c3 + m2 * n2;
    float g = m + m * n2;
    float co;
    if (d < 0.0)
    {
        float h = acos(q / c3) / 3.0;
        float s = cos(h);
        float t = sin(h) * sqrt(3.0);
        float rx = sqrt(-c * (s + t + 2.0) + m2);
        float ry = sqrt(-c * (s - t + 2.0) + m2);
        co = (ry + sign(l) * rx + abs(g) / (rx * ry) - m) / 2.0;
    }
    else
    {
        float h = 2.0 * m * n * sqrt(d);
        float s = sign(q + h) * pow(abs(q + h), 1.0 / 3.0);
        float u = sign(q - h) * pow(abs(q - h), 1.0 / 3.0);
        float rx = -s - u - c * 4.0 + 2.0 * m2;
        float ry = (s - u) * sqrt(3.0);
        float rm = sqrt(rx * rx + ry * ry);
        co = (ry / sqrt(rm - rx) + 2.0 * g / rm - m) / 2.0;
    }
    vec2 r = ab * vec2(co, sqrt(1.0 - co * co));
    return length(r - p) * sign(p.y - r.y);
}
void draw_ellipse(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d = sdEllipse(Coord.xy - function1.xy, function1.zw);
    if (function2.y == 1.0)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function2.x / 2.0 - 0.5, function2.x / 2.0 + 0.5, d)));
    else if (abs(d) >= function2.x / 2.0 - 0.5)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function2.x / 2.0 - 0.5, function2.x / 2.0 + 0.5, abs(d))));
}
float sdPie(in vec2 p, in vec2 c, in float r)
{
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p,c),0.0,r));
    return max(l,m * sign(c.y * p.x - c.x * p.y));
}
void draw_pie(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float a0 = PI * 0.5 + function2.x * 0.5;
    vec2 p0 = (Coord.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));
    float d0 = sdPie(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
    if (function2.z == 1.0)
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    else
        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.y / 2.0 - 0.5, function2.y / 2.0 + 0.5, abs(d0))));
}
float sdArc(in vec2 p, in vec2 c, in float r)
{
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p,c),0.0,r));
    return max(-l,m * sign(c.y * p.x - c.x * p.y));
}
void draw_arc(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float a0 = PI * 0.5 + function2.x * 0.5;
    vec2 p0 = (Coord.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));
    if (function2.z == 1.0)
    {
        float d0 = sdPie(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    }
    else
    {
        float d0 = sdArc(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.y / 2.0 - 0.5, function2.y / 2.0 + 0.5, abs(d0))));
    }
}
                
float sdRoundedBox( in vec2 p, in vec2 b, in vec4 r )
{
    r.xy = (p.x>0.0)?r.xy : r.zw;
    r.x  = (p.y>0.0)?r.x  : r.y;
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}
                
void draw_rounded_rect(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    vec2 p0 = (Coord.xy - function1.xy);
    float d0 = sdRoundedBox(p0, function1.zw * 0.5, function2);
    if (function3.y == 1.0)
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    else
        color = vec4(color.xyz, color.a * (1 - smoothstep(function3.x / 2.0 - 0.5, function3.x / 2.0 + 0.5, abs(d0))));
}
void standard_shape_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uShapeEnabled)
    {
        switch(uShape)
        {
        case SHAPE_Line:
            draw_line(color, function1, function2, function3);
            break;
        case SHAPE_CubicBezier:
            draw_cubic_bezier(color, function1, function2, function3);
            break;
        case SHAPE_Triangle:
            draw_triangle(color, function1, function2, function3);
            break;
        case SHAPE_Circle:
            draw_circle(color, function1, function2, function3);
            break;
        case SHAPE_Ellipse:
            draw_ellipse(color, function1, function2, function3);
            break;
        case SHAPE_Pie:
            draw_pie(color, function1, function2, function3);
            break;
        case SHAPE_Arc:
            draw_arc(color, function1, function2, function3);
            break;
        case SHAPE_RoundedRect:
            draw_rounded_rect(color, function1, function2, function3);
            break;
        }
    }
}
                )glsl"
            };
            aOutput += code;
        }
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