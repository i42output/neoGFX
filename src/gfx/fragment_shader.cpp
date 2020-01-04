// fragment_shader.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.
  
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
                "vec4 gradient_colour(in float n)\n"
                "{\n"
                "    int l = 0;\n"
                "    int r = uGradientStopCount - 1;\n"
                "    int found = -1;\n"
                "    float pos = 0.0;\n"
                "    if (n < 0.0)\n"
                "        n = 0.0;\n"
                "    if (n > 1.0)\n"
                "        n = 1.0;\n"
                "    while (found == -1)\n"
                "    {\n"
                "        int m = (l + r) / 2;\n"
                "        pos = texelFetch(uGradientStopPositions, ivec2(m, 0)).r;\n"
                "        if (l > r)\n"
                "            found = r;\n"
                "        else\n"
                "        {\n"
                "            if (pos < n)\n"
                "                l = m + 1;\n"
                "            else if (pos > n)\n"
                "                r = m - 1;\n"
                "            else\n"
                "                found = m;\n"
                "        }\n"
                "    }\n"
                "    if (pos >= n && found != 0)\n"
                "        --found;\n"
                "    float firstPos = texelFetch(uGradientStopPositions, ivec2(found, 0)).r;\n"
                "    float secondPos = texelFetch(uGradientStopPositions, ivec2(found + 1, 0)).r;\n"
                "    vec4 firstColour = texelFetch(uGradientStopColours, ivec2(found, 0));\n"
                "    vec4 secondColour = texelFetch(uGradientStopColours, ivec2(found + 1, 0));\n"
                "    return mix(firstColour, secondColour, (n - firstPos) / (secondPos - firstPos));\n"
                "}\n"
                "\n"
                "float ellipse_radius(vec2 ab, vec2 centre, vec2 pt)\n"
                "{\n"
                "    vec2 d = pt - centre;\n"
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
                "vec4 colour_at(vec2 viewPos)\n"
                "{\n"
                "    vec2 s = uGradientBottomRight - uGradientTopLeft;\n"
                "    vec2 pos = viewPos - uGradientTopLeft;\n"
                "    pos.x = max(min(pos.x, s.x - 1.0), 0.0);\n"
                "    pos.y = max(min(pos.y, s.y - 1.0), 0.0);\n"
                "    float uGradientPos;\n"
                "    if (uGradientDirection == 0)\n" /* vertical */
                "        uGradientPos = pos.y / s.y;\n"
                "    else if (uGradientDirection == 1)\n" /* horizontal */
                "        uGradientPos = pos.x / s.x;\n"
                "    else if (uGradientDirection == 2)\n" /* diagonal */
                "    {\n"
                "        vec2 centre = s / 2.0;\n"
                "        float angle;\n"
                "        switch (uGradientStartFrom)\n"
                "        {\n"
                "        case 0:\n"
                "            angle = atan(centre.y, -centre.x);\n"
                "            break;\n"
                "        case 1:\n"
                "            angle = atan(-centre.y, -centre.x);\n"
                "            break;\n"
                "        case 2:\n"
                "            angle = atan(-centre.y, centre.x);\n"
                "            break;\n"
                "        case 3:\n"
                "            angle = atan(centre.y, centre.x);\n"
                "            break;\n"
                "        default:\n"
                "            angle = uGradientAngle;\n"
                "            break;\n"
                "        }\n"
                "        pos.y = s.y - pos.y;\n"
                "        pos = pos - centre;\n"
                "        mat2 rot = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));\n"
                "        pos = rot * pos;\n"
                "        pos = pos + centre;\n"
                "        uGradientPos = pos.y / s.y;\n"
                "    }\n"
                "    else if (uGradientDirection == 3)\n" /* rectangular */
                "    {\n"
                "        float vert = pos.y / s.y;\n"
                "        if (vert > 0.5)\n"
                "            vert = 1.0 - vert;\n"
                "        float horz = pos.x / s.x;\n"
                "        if (horz > 0.5)\n"
                "            horz = 1.0 - horz;\n"
                "        uGradientPos = min(vert, horz) * 2.0;\n"
                "    }\n"
                "    else\n" /* radial */
                "    {\n"
                "        vec2 ab = s / 2.0;\n"
                "        pos -= ab;\n"
                "        vec2 centre = ab * uGradientCentre;\n"
                "        float d = distance(centre, pos);\n"
                "        vec2 c1 = uGradientTopLeft - uGradientTopLeft - ab;\n"
                "        vec2 c2 = vec2(uGradientTopLeft.x, uGradientBottomRight.y) - uGradientTopLeft - ab;\n"
                "        vec2 c3 = uGradientBottomRight - uGradientTopLeft - ab;\n"
                "        vec2 c4 = vec2(uGradientBottomRight.x, uGradientTopLeft.y) - uGradientTopLeft - ab;\n"
                "        vec2 cc = c1;\n"
                "        if (distance(centre, c2) < distance(centre, cc))\n"
                "            cc = c2;\n"
                "        if (distance(centre, c3) < distance(centre, cc))\n"
                "            cc = c3;\n"
                "        if (distance(centre, c4) < distance(centre, cc))\n"
                "            cc = c4;\n"
                "        vec2 fc = c1;\n"
                "        if (distance(centre, c2) > distance(centre, fc))\n"
                "            fc = c2;\n"
                "        if (distance(centre, c3) > distance(centre, fc))\n"
                "            fc = c3;\n"
                "        if (distance(centre, c4) > distance(centre, fc))\n"
                "            fc = c4;\n"
                "        vec2 cs = vec2(min(abs(-ab.x + centre.x), abs(ab.x + centre.x)), min(abs(-ab.y + centre.y), abs(ab.y + centre.y)));\n"
                "        vec2 fs = vec2(max(abs(-ab.x + centre.x), abs(ab.x + centre.x)), max(abs(-ab.y + centre.y), abs(ab.y + centre.y)));\n"
                "        float r;\n"
                "        if (uGradientShape == 0)\n" // Ellipse
                "        {\n"
                "            switch (uGradientSize)\n"
                "            {\n"
                "            default:\n"
                "            case 0:\n" // ClosestSide
                "                r = ellipse_radius(cs, centre, pos);\n"
                "                break;\n"
                "            case 1:\n" // FarthestSide
                "                r = ellipse_radius(fs, centre, pos);\n"
                "                break;\n"
                "            case 2:\n" // ClosestCorner
                "                r = ellipse_radius(abs(cc - centre), centre, pos);\n"
                "                break;\n"
                "            case 3:\n" // FarthestCorner
                "                r = ellipse_radius(abs(fc - centre), centre, pos);\n"
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
                "                r = distance(cc, centre);\n"
                "                break;\n"
                "            case 3:\n"
                "                r = distance(fc, centre);\n"
                "                break;\n"
                "            }\n"
                "        }\n"
                "        if (d < r)\n"
                "            uGradientPos = d / r;\n"
                "        else\n"
                "            uGradientPos = 1.0;\n"
                "    }\n"
                "    return gradient_colour(uGradientPos);\n"
                "}\n"
                "\n"
                "void standard_gradient_shader(inout vec4 color)\n"
                "{\n"
                "    if (uGradientEnabled)\n"
                "    {\n"
                "        vec2 viewPos = gl_FragCoord.xy;\n"
                "        viewPos.y = uGradientViewportTop - viewPos.y;\n"
                "        int d = uGradientFilterSize / 2;\n"
                "        if (texelFetch(uGradientFilter, ivec2(d, d)).r == 1.0)\n"
                "        {\n"
                "            color = colour_at(viewPos);\n"  
                "        }\n"
                "        else\n"
                "        {\n"
                "            vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "            for (int fy = -d; fy <= d; ++fy)\n"
                "            {\n"
                "                for (int fx = -d; fx <= d; ++fx)\n"
                "                {\n"
                "                    sum += (colour_at(viewPos + vec2(fx, fy)) * texelFetch(uGradientFilter, ivec2(fx + d, fy + d)).r);\n"
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

    void standard_gradient_shader::set_gradient(i_rendering_context& aContext, const gradient& aGradient, const rect& aBoundingBox)
    {
        enable();
        basic_rect<float> boundingBox{ aBoundingBox };
        uGradientViewportTop = static_cast<float>(aContext.logical_coordinates().bottomLeft.y);
        uGradientTopLeft = vec2f{ boundingBox.top_left().x, boundingBox.top_left().y };
        uGradientBottomRight = vec2f{ boundingBox.bottom_right().x, boundingBox.bottom_right().y };
        uGradientDirection = aGradient.direction();
        uGradientAngle = std::holds_alternative<double>(aGradient.orientation()) ? static_cast<float>(static_variant_cast<double>(aGradient.orientation())) : 0.0f;
        uGradientStartFrom = std::holds_alternative<corner>(aGradient.orientation()) ? static_cast<int>(static_variant_cast<corner>(aGradient.orientation())) : -1;
        uGradientSize = aGradient.size();
        uGradientShape = aGradient.shape();
        basic_vector<float, 2> gradientExponents = (aGradient.exponents() != std::nullopt ? *aGradient.exponents() : vec2{ 2.0, 2.0 });
        uGradientExponents = vec2f{ gradientExponents.x, gradientExponents.y };
        basic_point<float> gradientCentre = (aGradient.centre() != std::nullopt ? *aGradient.centre() : point{});
        uGradientCentre = vec2f{ gradientCentre.x, gradientCentre.y };
        auto& gradientArrays = gradient_shader_data(aGradient);
        uGradientFilterSize = static_cast<int>(gradientArrays.filter.data().extents().cx);
        uGradientStopCount = static_cast<int>(gradientArrays.stopCount);
        gradientArrays.stops.data().bind(3);
        gradientArrays.stopColours.data().bind(4);
        gradientArrays.filter.data().bind(5);
        uGradientStopPositions = sampler2DRect{ 3 };
        uGradientStopColours = sampler2DRect{ 4 };
        uGradientFilter = sampler2DRect{ 5 };
        uGradientEnabled = true;
    }

    gradient_shader_data& standard_gradient_shader::gradient_shader_data(const gradient& aGradient)
    {
        auto instantiate_gradient = [this, &aGradient](neogfx::gradient_shader_data& aData)
        {
            auto combinedStops = aGradient.combined_stops();
            iGradientStopPositions.reserve(combinedStops.size());
            iGradientStopColours.reserve(combinedStops.size());
            iGradientStopPositions.clear();
            iGradientStopColours.clear();
            for (const auto& stop : combinedStops)
            {
                iGradientStopPositions.push_back(static_cast<float>(stop.first));
                iGradientStopColours.push_back(std::array<float, 4>{ {stop.second.red<float>(), stop.second.green<float>(), stop.second.blue<float>(), stop.second.alpha<float>()}});
            }
            aData.stopCount = static_cast<uint32_t>(combinedStops.size());
            aData.stops.data().set_pixels(rect{ point{}, size_u32{ static_cast<uint32_t>(iGradientStopPositions.size()), 1u } }, & iGradientStopPositions[0]);
            aData.stopColours.data().set_pixels(rect{ point{}, size_u32{ static_cast<uint32_t>(iGradientStopColours.size()), 1u } }, & iGradientStopColours[0]);
            auto filter = static_gaussian_filter<float, GRADIENT_FILTER_SIZE>(static_cast<float>(aGradient.smoothness() * 10.0));
            aData.filter.data().set_pixels(rect{ point(), size_u32{ GRADIENT_FILTER_SIZE, GRADIENT_FILTER_SIZE } }, & filter[0][0]);
        };
        if (aGradient.use_cache())
        {
            auto mapResult = iGradientDataCacheMap.try_emplace(aGradient, iGradientDataCache.end());
            auto mapEntry = mapResult.first;
            bool newGradient = mapResult.second;
            if (!newGradient)
            {
                auto queueEntry = std::find(iGradientDataCacheQueue.begin(), iGradientDataCacheQueue.end(), mapEntry);
                if (queueEntry != std::prev(iGradientDataCacheQueue.end()))
                {
                    iGradientDataCacheQueue.erase(queueEntry);
                    iGradientDataCacheQueue.push_back(mapEntry);
                }
            }
            else
            {
                if (iGradientDataCache.size() < GRADIENT_DATA_CACHE_QUEUE_SIZE)
                {
                    iGradientDataCache.emplace_back();
                    mapEntry->second = std::prev(iGradientDataCache.end());
                }
                else
                {
                    auto data = iGradientDataCacheQueue.front()->second;
                    iGradientDataCacheMap.erase(iGradientDataCacheQueue.front());
                    iGradientDataCacheQueue.pop_front();
                    mapEntry->second = data;
                }
                iGradientDataCacheQueue.push_back(mapEntry);
            }
            if (newGradient)
                instantiate_gradient(*mapEntry->second);
            return *mapEntry->second;
        }
        else
        {
            if (iUncachedGradient == std::nullopt)
                iUncachedGradient.emplace();
            instantiate_gradient(*iUncachedGradient);
            return *iUncachedGradient;
        }
    }

    standard_texture_shader::standard_texture_shader(const std::string& aName) :
        standard_fragment_shader<i_texture_shader>{ aName }
    {
        disable();
        add_in_variable<vec2f>("TexCoord"_s, 2u);
        set_uniform("tex"_s, sampler2D{ 1 });
        set_uniform("texMS"_s, sampler2DMS{ 2 });
        uTextureEffect = shader_effect::None;
    }

    void standard_texture_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_texture_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
        {
            static const string code 
            {
                "void standard_texture_shader(inout vec4 color)\n"
                "{\n"
                "    if (uTextureEnabled)\n"
                "    {\n"
                "        vec4 texel = vec4(0.0);\n"
                "        if (uTextureMultisample < 5)\n" // Scaled
                "        {\n"
                "            texel = texture(tex, TexCoord).rgba;\n"
                "        }\n"
                "        else\n"
                "        {\n"
                "            ivec2 TexCoord = ivec2(TexCoord * uTextureExtents);\n"
                "            texel = texelFetch(texMS, TexCoord, gl_SampleID).rgba;\n"
                "        }\n"
                "        switch(uTextureDataFormat)\n"
                "        {\n"
                "        case 1:\n" // RGBA
                "        default:\n"
                "            break;\n"
                "        case 2:\n" // Red
                "            texel = vec4(1.0, 1.0, 1.0, texel.r);\n"
                "            break;\n"
                "        case 3:\n" // SubPixel
                "            texel = vec4(1.0, 1.0, 1.0, (texel.r + texel.g + texel.b) / 3.0);\n"
                "            break;\n"
                "        }\n"
                "        switch(uTextureEffect)\n"
                "        {\n"
                "        case 0:\n" // effect: None
                "            color = texel.rgba * color;\n"
                "            break;\n"
                "        case 1:\n" // effect: Colourize, ColourizeAverage
                "            {\n"
                "                float avg = (texel.r + texel.g + texel.b) / 3.0;\n"
                "                color = vec4(avg, avg, avg, texel.a) * color;\n"
                "            }\n"
                "            break;\n"
                "        case 2:\n" // effect: ColourizeMaximum
                "            {\n"
                "                float maxChannel = max(texel.r, max(texel.g, texel.b));\n"
                "                color = vec4(maxChannel, maxChannel, maxChannel, texel.a) * color;\n"
                "            }\n"
                "            break;\n"
                "        case 3:\n" // effect: ColourizeSpot
                "            color = vec4(1.0, 1.0, 1.0, texel.a) * color;\n"
                "            break;\n"
                "        case 4:\n" // effect: Monochrome
                "            {\n"
                "                float gray = dot(color.rgb * texel.rgb, vec3(0.299, 0.587, 0.114));\n"
                "                color = vec4(gray, gray, gray, texel.a) * color;\n"
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

    void standard_texture_shader::clear_texture()
    {
        uTextureEnabled = false;
        uTextureEffect = shader_effect::None;
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
                "void standard_glyph_shader(inout vec4 color)\n"
                "{\n"
                "    if (uGlyphEnabled)\n"
                "    {\n"
                "        float a = 0.0;\n"
                "        if (uGlyphSubpixel)\n"
                "        {\n"
                "            vec4 aaaAlpha = texture(uGlyphTexture, TexCoord);\n"
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
                "            a = texture(uGlyphTexture, TexCoord).r;\n"
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
        aGlyph.glyph_texture().texture().bind(6);
        bool subpixelRender = aGlyph.subpixel() && aGlyph.glyph_texture().subpixel();
        if (subpixelRender)
            aContext.render_target().target_texture().bind(7);
        uGlyphRenderTargetExtents = aContext.render_target().extents().to_vec2().as<int32_t>();
        uGlyphGuiCoordinates = aContext.logical_coordinates().is_gui_orientation();
        uGlyphTexture = sampler2D{ 6 };
        uGlyphRenderOutput = sampler2DMS{ 7 };
        uGlyphSubpixel = aGlyph.glyph_texture().subpixel();
        uGlyphSubpixelFormat = subpixelRender ? aContext.subpixel_format() : subpixel_format::None;
        uGlyphEnabled = true;
    }

    standard_stipple_shader::standard_stipple_shader(const std::string& aName) :
        standard_fragment_shader<i_stipple_shader>{ aName }
    {
        disable();
    }

    void standard_stipple_shader::prepare_uniforms(const i_rendering_context& aRenderingContext, i_shader_program& aProgram)
    {
        if (stipple_active() && uStippleVertex.uniform().value().empty())
            throw no_stipple_vertex();
    }

    void standard_stipple_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_fragment_shader<i_stipple_shader>::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
        {
            static const string code
            {
                "void standard_stipple_shader(inout vec4 color)\n"
                "{\n"
                "    if (uStippleEnabled)\n"
                "    {\n"
                "        uint d = uint(distance(uStippleVertex, Coord));\n"
                "        uint patternBit = ((d + uStippleCounter) / uStippleFactor) % 16;\n"
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
        uStippleEnabled = false;
    }

    void standard_stipple_shader::set_stipple(uint32_t aFactor, uint16_t aPattern)
    {
        enable();
        uStippleFactor = aFactor;
        uStipplePattern = aPattern;
        uStippleCounter = 0u;
        uStippleVertex = vec3f{};
        uStippleEnabled = true;
    }

    void standard_stipple_shader::first_vertex(const vec3& aVertex)
    {
        uStippleCounter = 0u;
        uStippleVertex = aVertex.as<float>();
    }
    
    void standard_stipple_shader::next_vertex(const vec3& aVertex, scalar aCounterUpdateOffset)
    {
        uStippleCounter = uStippleCounter.uniform().value().get<uint32_t>() +
            static_cast<uint32_t>(uStippleVertex.uniform().value().get<vec3f>().distance(aVertex.as<float>()) + aCounterUpdateOffset);
        uStippleVertex = aVertex.as<float>();
    }
}