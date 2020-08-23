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
    standard_gradient_shader::standard_gradient_shader(const std::string& aName) :
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
                "vec4 gradient_color(in float n)\n"
                "{\n"
                "    n = max(min(n, 1.0), 0.0) * float(uGradientColorCount - 1);"
                "    vec4 firstColor = texelFetch(uGradientColors, ivec2(floor(n), uGradientColorRow));\n"
                "    vec4 secondColor = texelFetch(uGradientColors, ivec2(ceil(n), uGradientColorRow));\n"
                "    return mix(firstColor, secondColor, n - floor(n));\n"
                "}\n"
                "\n"
                "float ellipse_radius(vec2 ab, vec2 center, vec2 pt)\n"
                "{\n"
                "    vec2 d = pt - center;\n"
                "    float angle = 0;\n"
                "    vec2 ratio = vec2(1.0, 1.0);\n"
                "    if (ab.x >= ab.y)\n"
                "        ratio.y = ab.x / ab.y;\n"
                "    else\n"
                "        ratio.x = ab.y / ab.x;\n"
                "    angle = atan(d.y * ratio.y, d.x * ratio.x);\n"
                "    float x = pow(abs(cos(angle)), 2.0 / uGradientExponents.x) * sign(cos(angle)) * ab.x;\n"
                "    float y = pow(abs(sin(angle)), 2.0 / uGradientExponents.y) * sign(sin(angle)) * ab.y;\n"
                "    return sqrt(x * x + y * y);\n"
                "}\n"
                "\n"
                "vec4 color_at(vec2 viewPos, vec4 boundingBox)\n"
                "{\n"
                "    vec2 s = boundingBox.zw - boundingBox.xy;\n"
                "    vec2 pos = viewPos - boundingBox.xy;\n"
                "    pos.x = max(min(pos.x, s.x - 1.0), 0.0);\n"
                "    pos.y = max(min(pos.y, s.y - 1.0), 0.0);\n"
                "    float gradientPos;\n"
                "    if (uGradientDirection == 0)\n" /* vertical */
                "    {\n"
                "        gradientPos = pos.y / s.y;\n"
                "        if (!uGradientGuiCoordinates)\n"
                "            gradientPos = 1.0 - gradientPos;\n"
                "    }\n"
                "    else if (uGradientDirection == 1)\n" /* horizontal */
                "        gradientPos = pos.x / s.x;\n"
                "    else if (uGradientDirection == 2)\n" /* diagonal */
                "    {\n"
                "        vec2 center = s / 2.0;\n"
                "        float angle;\n"
                "        switch (uGradientStartFrom)\n"
                "        {\n"
                "        case 0:\n"
                "            angle = atan(center.y, -center.x);\n"
                "            break;\n"
                "        case 1:\n"
                "            angle = atan(-center.y, -center.x);\n"
                "            break;\n"
                "        case 2:\n"
                "            angle = atan(-center.y, center.x);\n"
                "            break;\n"
                "        case 3:\n"
                "            angle = atan(center.y, center.x);\n"
                "            break;\n"
                "        default:\n"
                "            angle = uGradientAngle;\n"
                "            break;\n"
                "        }\n"
                "        pos.y = s.y - pos.y;\n"
                "        pos = pos - center;\n"
                "        mat2 rot = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));\n"
                "        pos = rot * pos;\n"
                "        pos = pos + center;\n"
                "        gradientPos = pos.y / s.y;\n"
                "    }\n"
                "    else if (uGradientDirection == 3)\n" /* rectangular */
                "    {\n"
                "        float vert = pos.y / s.y;\n"
                "        if (vert > 0.5)\n"
                "            vert = 1.0 - vert;\n"
                "        float horz = pos.x / s.x;\n"
                "        if (horz > 0.5)\n"
                "            horz = 1.0 - horz;\n"
                "        gradientPos = min(vert, horz) * 2.0;\n"
                "    }\n"
                "    else\n" /* radial */
                "    {\n"
                "        vec2 ab = s / 2.0;\n"
                "        pos -= ab;\n"
                "        vec2 center = ab * uGradientCenter;\n"
                "        float d = distance(center, pos);\n"
                "        vec2 c1 = boundingBox.xy - boundingBox.xy - ab;\n"
                "        vec2 c2 = boundingBox.xw - boundingBox.xy - ab;\n"
                "        vec2 c3 = boundingBox.zw - boundingBox.xy - ab;\n"
                "        vec2 c4 = boundingBox.zy - boundingBox.xy - ab;\n"
                "        vec2 cc = c1;\n"
                "        if (distance(center, c2) < distance(center, cc))\n"
                "            cc = c2;\n"
                "        if (distance(center, c3) < distance(center, cc))\n"
                "            cc = c3;\n"
                "        if (distance(center, c4) < distance(center, cc))\n"
                "            cc = c4;\n"
                "        vec2 fc = c1;\n"
                "        if (distance(center, c2) > distance(center, fc))\n"
                "            fc = c2;\n"
                "        if (distance(center, c3) > distance(center, fc))\n"
                "            fc = c3;\n"
                "        if (distance(center, c4) > distance(center, fc))\n"
                "            fc = c4;\n"
                "        vec2 cs = vec2(min(abs(-ab.x + center.x), abs(ab.x + center.x)), min(abs(-ab.y + center.y), abs(ab.y + center.y)));\n"
                "        vec2 fs = vec2(max(abs(-ab.x + center.x), abs(ab.x + center.x)), max(abs(-ab.y + center.y), abs(ab.y + center.y)));\n"
                "        float r;\n"
                "        if (uGradientShape == 0)\n" // Ellipse
                "        {\n"
                "            switch (uGradientSize)\n"
                "            {\n"
                "            default:\n"
                "            case 0:\n" // ClosestSide
                "                r = ellipse_radius(cs, center, pos);\n"
                "                break;\n"
                "            case 1:\n" // FarthestSide
                "                r = ellipse_radius(fs, center, pos);\n"
                "                break;\n"
                "            case 2:\n" // ClosestCorner
                "                r = ellipse_radius(abs(cc - center), center, pos);\n"
                "                break;\n"
                "            case 3:\n" // FarthestCorner
                "                r = ellipse_radius(abs(fc - center), center, pos);\n"
                "                break;\n"
                "            }\n"
                "        }\n"
                "        else if (uGradientShape == 1)\n" // Circle
                "        {\n"
                "            switch (uGradientSize)\n"
                "            {\n"
                "            default:\n"
                "            case 0:\n"
                "                r = min(cs.x, cs.y);\n"
                "                break;\n"
                "            case 1:\n"
                "                r = max(fs.x, fs.y);\n"
                "                break;\n"
                "            case 2:\n"
                "                r = distance(cc, center);\n"
                "                break;\n"
                "            case 3:\n"
                "                r = distance(fc, center);\n"
                "                break;\n"
                "            }\n"
                "        }\n"
                "        if (d < r)\n"
                "            gradientPos = d / r;\n"
                "        else\n"
                "            gradientPos = 1.0;\n"
                "    }\n"
                "    return gradient_color(gradientPos);\n"
                "}\n"
                "\n"
                "void standard_gradient_shader(inout vec4 color, inout vec4 function)\n"
                "{\n"
                "    if (uGradientEnabled)\n"
                "    {\n"
                "        int d = uGradientFilterSize / 2;\n"
                "        if (texelFetch(uGradientFilter, ivec2(d, d)).r == 1.0)\n"
                "        {\n"
                "            color = color_at(Coord.xy, function);\n"  
                "        }\n"
                "        else\n"
                "        {\n"
                "            vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "            for (int fy = -d; fy <= d; ++fy)\n"
                "            {\n"
                "                for (int fx = -d; fx <= d; ++fx)\n"
                "                {\n"
                "                    sum += (color_at(Coord.xy + vec2(fx, fy), function) * texelFetch(uGradientFilter, ivec2(fx + d, fy + d)).r);\n"
                "                }\n"
                "            }\n"
                "            color = sum;\n" 
                "        }\n"
                "    }\n"
                "}\n"_s
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

    standard_texture_shader::standard_texture_shader(const std::string& aName) :
        standard_fragment_shader<i_texture_shader>{ aName }
    {
        disable();
        set_uniform("tex"_s, sampler2D{ 1 });
        set_uniform("texMS"_s, sampler2DMS{ 2 });
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
                "vec4 texel_at(vec2 texCoord, int sampleId)\n"
                "{\n"
                "    vec4 texel = vec4(0.0);\n"
                "    if (uTextureMultisample < 5)\n" // Scaled
                "    {\n"
                "        texel = texture(tex, texCoord).rgba;\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        texel = texelFetch(texMS, ivec2(texCoord * uTextureExtents), sampleId).rgba;\n"
                "    }\n"
                "    switch(uTextureDataFormat)\n"
                "    {\n"
                "    case 1:\n" // RGBA
                "    default:\n"
                "        break;\n"
                "    case 2:\n" // Red
                "        texel = vec4(1.0, 1.0, 1.0, texel.r);\n"
                "        break;\n"
                "    case 3:\n" // SubPixel
                "        texel = vec4(1.0, 1.0, 1.0, (texel.r + texel.g + texel.b) / 3.0);\n"
                "        break;\n"
                "    }\n"
                "    return texel;\n"
                "}\n"
                "vec4 texel_at(vec2 texCoord)\n"
                "{\n"
                "    return texel_at(texCoord, gl_SampleID);\n"
                "}\n"
                "vec4 combined_texel_at(vec2 texCoord)\n"
                "{\n"
                "    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "    for (int i = 0; i < gl_NumSamples; ++i)\n"
                "    {\n"
                "        sum += texel_at(texCoord, i);\n"
                "    }\n"
                "    return sum / float(gl_NumSamples);\n"
                "}\n"
                "void standard_texture_shader(inout vec4 color, inout vec4 function)\n"
                "{\n"
                "    if (uTextureEnabled)\n"
                "    {\n"
                "        vec4 texel = texel_at(TexCoord);\n"
                "        switch(uTextureEffect)\n"
                "        {\n"
                "        case 0:\n" // effect: None
                "            color = texel.rgba * color;\n"
                "            break;\n"
                "        case 1:\n" // effect: Colorize, ColorizeAverage
                "            {\n"
                "                float avg = (texel.r + texel.g + texel.b) / 3.0;\n"
                "                color = vec4(avg, avg, avg, texel.a) * color;\n"
                "            }\n"
                "            break;\n"
                "        case 2:\n" // effect: ColorizeMaximum
                "            {\n"
                "                float maxChannel = max(texel.r, max(texel.g, texel.b));\n"
                "                color = vec4(maxChannel, maxChannel, maxChannel, texel.a) * color;\n"
                "            }\n"
                "            break;\n"
                "        case 3:\n" // effect: ColorizeSpot
                "            color = vec4(1.0, 1.0, 1.0, texel.a) * color;\n"
                "            break;\n"
                "        case 4:\n" // effect: Monochrome
                "            {\n"
                "                float gray = dot(color.rgb * texel.rgb, vec3(0.299, 0.587, 0.114));\n"
                "                color = vec4(gray, gray, gray, texel.a) * color;\n"
                "            }\n"
                "            break;\n"
                "        case 10:\n" // effect: Filter
                "            break;\n"
                "        case 99:\n" // effect: Ignore
                "            break;\n"
                "        }\n"
                "    }\n"
                "}\n"_s
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

    standard_filter_shader::standard_filter_shader(const std::string& aName) :
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
                "void standard_filter_shader(inout vec4 color, inout vec4 function)\n"
                "{\n"
                "    if (uFilterEnabled)\n"
                "    {\n"
                "        switch(uFilterType)\n"
                "        {\n"
                "        case 0:\n" // filter: None
                "            break;\n"
                "        case 1:\n" // effect: GaussianBlur
                "           {\n"
                "               int d = uFilterKernelSize / 2;\n"
                "               if (texelFetch(uFilterKernel, ivec2(d, d)).r != 1.0)\n"
                "               {\n"
                "                   vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "                   for (int fy = -d; fy <= d; ++fy)\n"
                "                   {\n"
                "                       for (int fx = -d; fx <= d; ++fx)\n"
                "                       {\n"
                "                           vec4 texel = texel_at(TexCoord + vec2(fx, fy) / uTextureExtents);\n" 
                "                           sum += (texel * texelFetch(uFilterKernel, ivec2(fx + d, fy + d)).r);\n"
                "                       }\n"
                "                   }\n"
                "                   color = sum;\n"
                "               }\n"
                "            }\n"
                "            break;\n"
                "        }\n"
                "    }\n"
                "}\n"_s
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

    standard_glyph_shader::standard_glyph_shader(const std::string& aName) :
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
                "ivec2 render_position()\n"
                "{\n"
                "    if (uGlyphGuiCoordinates)\n"
                "        return ivec2(Coord.x, uGlyphRenderTargetExtents.y - Coord.y);\n"
                "    else\n"
                "        return ivec2(Coord.xy);\n"
                "}\n"
                "\n"
                "vec3 output_pixel()\n"
                "{\n"
                "    return texelFetch(uGlyphRenderOutput, render_position(), 0).rgb;\n"
                "}\n"
                "\n"
                "\n"
                "void standard_glyph_shader(inout vec4 color, inout vec4 function)\n"
                "{\n"
                "    if (uGlyphEnabled)\n"
                "    {\n"
                "        float a = 0.0;\n"
                "        if (uGlyphSubpixel)\n"
                "        {\n"
                "            vec4 aaaAlpha = texture(tex, TexCoord);\n"
                "            if (aaaAlpha.rgb == vec3(1.0, 1.0, 1.0))\n"
                "                return;\n"
                "            else if (aaaAlpha.rgb == vec3(0.0, 0.0, 0.0))\n"
                "                discard;\n"
                "            else\n"
                "            {\n"
                "                switch(uGlyphSubpixelFormat)\n"
                "                {\n"
                "                default:\n"
                "                    a = (aaaAlpha.r + aaaAlpha.g + aaaAlpha.b) / 3.0;\n"
                "                    color = vec4(color.xyz, color.a * a);\n"
                "                    break;\n"
                "                case 1:\n" // RGBHorizontal
                "                    color = vec4(color.rgb * aaaAlpha.rgb * color.a + output_pixel() * (vec3(1.0, 1.0, 1.0) - aaaAlpha.rgb * color.a), 1.0);\n"
                "                    break;\n"
                "                case 2:\n" // BGRHorizontal
                "                    color = vec4(color.rgb * aaaAlpha.bgr * color.a + output_pixel() * (vec3(1.0, 1.0, 1.0) - aaaAlpha.bgr * color.a), 1.0);\n"
                "                    break;\n"
                "                }\n"
                "            }\n"
                "        }\n"
                "        else\n"
                "        {\n"
                "            a = texture(tex, TexCoord).r;\n"
                "            if (a == 0)\n"
                "                discard;\n"
                "            color = vec4(color.xyz, color.a * a);\n"
                "        }\n"
                "    }\n"
                "}\n"_s
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

    void standard_glyph_shader::set_first_glyph(const i_rendering_context& aContext, const glyph& aGlyph)
    {
        enable();
        bool subpixelRender = aGlyph.subpixel() && aGlyph.glyph_texture().subpixel();
        if (subpixelRender)
            aContext.render_target().target_texture().bind(7);
        uGlyphRenderTargetExtents = aContext.render_target().extents().to_vec2().as<int32_t>();
        uGlyphGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uGlyphRenderOutput = sampler2DMS{ 7 };
        uGlyphSubpixel = aGlyph.glyph_texture().subpixel();
        uGlyphSubpixelFormat = subpixelRender ? aContext.subpixel_format() : subpixel_format::None;
        uGlyphEnabled = true;
    }

    standard_stipple_shader::standard_stipple_shader(const std::string& aName) :
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
                "void standard_stipple_shader(inout vec4 color, inout vec4 function)\n"
                "{\n"
                "    if (uStippleEnabled)\n"
                "    {\n"
                "        float d = distance(uStippleVertex, Coord);\n"
                "        uint patternBit = uint((d + uStipplePosition) / uStippleFactor) % 16;\n"
                "        if ((uStipplePattern & (1 << patternBit)) == 0)\n"
                "            discard;\n"
                "    }\n"
                "}\n"_s
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
        uStipplePosition = 0.0f;
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
}