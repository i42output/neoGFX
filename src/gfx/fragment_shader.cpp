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
                "    if (uGradientTile)\n"
                "    {\n"
                "        if (uGradientDirection != 1)\n" /* vertical */
                "        {\n"
                "            float adjust = 0;\n"
                "            if (uGradientTileParams.z == 1)\n"
                "                adjust = float(int(boundingBox.y) % uGradientTileParams.y);\n"
                "            float frac = 1 / float(uGradientTileParams.y);\n"
                "            pos.y = floor((float(int(pos.y + adjust) % uGradientTileParams.y) * frac + frac / 2) * s.y);\n"
                "        }\n"
                "        if (uGradientDirection != 0)\n" /* horizontal */
                "        {\n"
                "            float adjust = 0;\n"
                "            if (uGradientTileParams.z == 1)\n"
                "                adjust = float(int(boundingBox.x) % uGradientTileParams.x);\n"
                "            float frac = 1 / float(uGradientTileParams.x);\n"
                "            pos.x = floor((float(int(pos.x + adjust) % uGradientTileParams.x) * frac + frac / 2) * s.x);\n"
                "        }\n"
                "    }\n"
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
                "void standard_gradient_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    if (uGradientEnabled)\n"
                "    {\n"
                "        int d = uGradientFilterSize / 2;\n"
                "        if (texelFetch(uGradientFilter, ivec2(d, d)).r == 1.0)\n"
                "        {\n"
                "            color = color_at(Coord.xy, function0);\n"  
                "        }\n"
                "        else\n"
                "        {\n"
                "            vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "            for (int fy = -d; fy <= d; ++fy)\n"
                "            {\n"
                "                for (int fx = -d; fx <= d; ++fx)\n"
                "                {\n"
                "                    sum += (color_at(Coord.xy + vec2(fx, fy), function0) * texelFetch(uGradientFilter, ivec2(fx + d, fy + d)).r);\n"
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

    void standard_gradient_shader::set_gradient(i_rendering_context& aContext, const gradient& aGradient, double aOpacity)
    {
        enable();
        gradient const adjustedGradient = (aOpacity == 1.0 ? aGradient : aGradient.with_combined_alpha(aOpacity));
        uGradientGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uGradientDirection = adjustedGradient.direction();
        uGradientAngle = std::holds_alternative<double>(adjustedGradient.orientation()) ? static_cast<float>(static_variant_cast<double>(adjustedGradient.orientation())) : 0.0f;
        uGradientStartFrom = std::holds_alternative<corner>(adjustedGradient.orientation()) ? static_cast<int>(static_variant_cast<corner>(adjustedGradient.orientation())) : -1;
        uGradientSize = adjustedGradient.size();
        uGradientShape = adjustedGradient.shape();
        basic_vector<float, 2> gradientExponents = (adjustedGradient.exponents() != std::nullopt ? *adjustedGradient.exponents() : vec2{ 2.0, 2.0 });
        uGradientExponents = vec2f{ gradientExponents.x, gradientExponents.y };
        basic_point<float> gradientCenter = (adjustedGradient.center() != std::nullopt ? *adjustedGradient.center() : point{});
        uGradientCenter = vec2f{ gradientCenter.x, gradientCenter.y };
        uGradientTile = (adjustedGradient.tile() != std::nullopt);
        if (adjustedGradient.tile() != std::nullopt)
            uGradientTileParams = vec3{ adjustedGradient.tile()->extents.cx, adjustedGradient.tile()->extents.cy, adjustedGradient.tile()->aligned ? 1.0 : 0.0 }.as<int32_t>();
        else
            uGradientTileParams = vec3i32{};
        uGradientColorCount = static_cast<int>(adjustedGradient.colors().sampler().data().extents().cx);
        uGradientColorRow = static_cast<int>(adjustedGradient.colors().sampler_row());
        uGradientFilterSize = static_cast<int>(adjustedGradient.filter().sampler().data().extents().cx);
        adjustedGradient.colors().sampler().data().bind(3);
        adjustedGradient.filter().sampler().data().bind(4);
        uGradientColors = sampler2DRect{ 3 };
        uGradientFilter = sampler2DRect{ 4 };
        uGradientEnabled = true;
    }

    void standard_gradient_shader::set_gradient(i_rendering_context& aContext, const game::gradient& aGradient, double aOpacity)
    {
        gradient g = service<i_gradient_manager>().find_gradient(aGradient.id.cookie());
        set_gradient(aContext, g, aOpacity);
    }

    standard_texture_shader::standard_texture_shader(std::string const& aName) :
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
                "void standard_texture_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
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
                "        case 4:\n" // effect: ColorizeAlpha
                "            {\n"
                "                float avg = (texel.r + texel.g + texel.b) / 3.0;\n"
                "                color = vec4(1.0, 1.0, 1.0, texel.a * avg) * color;\n"
                "            }\n"
                "            break;\n"
                "        case 5:\n" // effect: Monochrome
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
                "void standard_filter_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
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
                "void standard_glyph_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
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

    void standard_glyph_shader::set_first_glyph(const i_rendering_context& aContext, const glyph_text& aText, const glyph& aGlyph)
    {
        enable();
        bool subpixelRender = subpixel(aGlyph) && aText.glyph_texture(aGlyph).subpixel();
        if (subpixelRender)
            aContext.render_target().target_texture().bind(7);
        uGlyphRenderTargetExtents = aContext.render_target().extents().to_vec2().as<int32_t>();
        uGlyphGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uGlyphRenderOutput = sampler2DMS{ 7 };
        uGlyphSubpixel = aText.glyph_texture(aGlyph).subpixel();
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
                "void standard_stipple_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    if (uStippleEnabled)\n"
                "    {\n"
                "        float d = 0.0;\n"
                "        if (!uShapeEnabled)\n"
                "            d = distance(uStippleVertex, Coord);\n"
                "        else if (uShape == 3)\n" // circle
                "            d = (atan(Coord.y - function1.y, Coord.x - function1.x) + PI) * function1.z;\n"
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
                // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

                "void draw_line(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    float dy = function1.y - function1.w;\n"
                "    float dx = function1.x - function1.z;\n"
                "    float m = dy / dx;\n" // GLSL allows divide-by-zero, we won't use the Inf
                "    float c = function1.y - m * function1.x;\n"
                "    float d = dx != 0.0 ? (abs(m) < 1.0 ? distance(vec2(Coord.x, m * Coord.x + c), Coord.xy) : distance(vec2((Coord.y - c) / m, Coord.y), Coord.xy)) : abs(Coord.x - function1.x);\n"
                "    color = vec4(color.xyz, color.a * (1.0 - smoothstep(function2.x / 2.0 - 0.5, function2.x / 2.0 + 0.5, d)));\n"
                "}\n"
                "// https://www.shadertoy.com/view/XdVBWd\n"
                "// The MIT License\n"
                "// Copyright © 2018 Inigo Quilez\n"
                "// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
                "\n"
                "float bezier_sdSegmentSq(in vec2 p, in vec2 a, in vec2 b)\n"
                "{\n"
                "    vec2 pa = p - a, ba = b - a;\n"
                "    float h = clamp(dot(pa,ba) / dot(ba,ba), 0.0, 1.0);\n"
                "    return dot(pa - ba * h, pa - ba * h);\n"
                "}\n"
                "\n"
                "float bezier_sdSegment(in vec2 p, in vec2 a, in vec2 b)\n"
                "{\n"
                "    return sqrt(bezier_sdSegmentSq(p,a,b));\n"
                "}\n"
                "\n"
                "// todo: research a non-iterative approach\n"
                "float bezier_udBezier(vec2 p0, vec2 p1, vec2 p2, in vec2 p3, vec2 pos)\n"
                "{\n"
                "    const int kNum = 50;\n"
                "    float res = 1e10;\n"
                "    vec2 a = p0;\n"
                "    for (int i = 1; i < kNum; i++)\n"
                "    {\n"
                "        float t = float(i) / float(kNum - 1);\n"
                "        float s = 1.0 - t;\n"
                "        vec2 b = p0 * s * s * s + p1 * 3.0 * s * s * t + p2 * 3.0 * s * t * t + p3 * t * t * t;\n"
                "        float d = bezier_sdSegmentSq(pos, a, b);\n"
                "        if (d < res) res = d;\n"
                "        a = b;\n"
                "    }\n"
                "    return sqrt(res);\n"
                "}\n"
                "void draw_cubic_bezier(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    float d = bezier_udBezier(function1.xy, function1.zw, function2.xy, function2.zw, Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5)));\n"
                "    color = vec4(color.xyz, color.a * (1.0 - smoothstep(function3.x / 2.0 - 0.5, function3.x / 2.0 + 0.5, d)));\n"
                "}\n"
                "void draw_circle(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    float d = distance(function1.xy, Coord.xy) - function1.z;\n"
                "    if (function2.x == 1.0)\n"
                "        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function1.w / 2.0 - 0.5, function1.w / 2.0 + 0.5, d)));\n"
                "    else if (abs(d) >= function1.w / 2.0 - 0.5)\n"
                "        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function1.w / 2.0 - 0.5, function1.w / 2.0 + 0.5, abs(d))));\n"
                "}\n"
                "float sdPie(in vec2 p, in vec2 c, in float r)\n"
                "{\n"
                "    p.x = abs(p.x);\n"
                "    float l = length(p) - r;\n"
                "    float m = length(p - c * clamp(dot(p,c),0.0,r));\n"
                "    return max(l,m * sign(c.y * p.x - c.x * p.y));\n"
                "}\n"
                "void draw_pie(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    float a0 = PI * 0.5 + function2.x * 0.5;"
                "    vec2 p0 = (Coord.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));\n"
                "    float d0 = sdPie(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);\n"
                "    if (function2.z == 1.0)\n"
                "        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));\n"
                "    else\n"
                "        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.y / 2.0 - 0.5, function2.y / 2.0 + 0.5, abs(d0))));\n"
                "}\n"
                "float sdArc(in vec2 p, in vec2 c, in float r)\n"
                "{\n"
                "    p.x = abs(p.x);\n"
                "    float l = length(p) - r;\n"
                "    float m = length(p - c * clamp(dot(p,c),0.0,r));\n"
                "    return max(-l,m * sign(c.y * p.x - c.x * p.y));\n"
                "}\n"
                "void draw_arc(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    float a0 = PI * 0.5 + function2.x * 0.5;"
                "    vec2 p0 = (Coord.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));\n"
                "    if (function2.z == 1.0)\n"
                "    {\n"
                "        float d0 = sdPie(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);\n"
                "        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        float d0 = sdArc(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);\n"
                "        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.y / 2.0 - 0.5, function2.y / 2.0 + 0.5, abs(d0))));\n"
                "    }\n"
                "}\n"
                "\n"
                "void standard_shape_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)\n"
                "{\n"
                "    if (uShapeEnabled)\n"
                "    {\n"
                "        switch(uShape)\n"
                "        {\n"
                "        case 1:\n" // line
                "            draw_line(color, function1, function2, function3);\n"
                "            break;\n"
                "        case 2:\n" // cubic bezier
                "            draw_cubic_bezier(color, function1, function2, function3);\n"
                "            break;\n"
                "        case 3:\n" // circle
                "            draw_circle(color, function1, function2, function3);\n"
                "            break;\n"
                "        case 4:\n" // pie
                "            draw_pie(color, function1, function2, function3);\n"
                "            break;\n"
                "        case 5:\n" // arc
                "            draw_arc(color, function1, function2, function3);\n"
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