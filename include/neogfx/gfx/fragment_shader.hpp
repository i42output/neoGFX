// fragment_shader.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/shader_array.hpp>
#include <neogfx/gfx/i_shader_program.hpp>
#include <neogfx/gfx/shader.hpp>

namespace neogfx
{
    static const uint32_t GRADIENT_FILTER_SIZE = 15;
    struct gradient_shader_data
    {
        //todo: use a mini atlas for the this
        uint32_t stopCount;
        shader_array<float> stops = { size_u32{gradient::MaxStops, 1} };
        shader_array<std::array<float, 4>> stopColours = { size_u32{gradient::MaxStops, 1} };
        shader_array<float> filter = { size_u32{GRADIENT_FILTER_SIZE, GRADIENT_FILTER_SIZE} };
    };

    class fragment_shader : public shader<i_shader>
    {
        typedef shader<i_shader> base_type;
    public:
        fragment_shader(const std::string& aName) :
            base_type{ shader_type::Fragment, aName }
        {
        }
    };

    class standard_fragment_shader : public fragment_shader
    {
    private:
        typedef std::list<neogfx::gradient_shader_data> gradient_data_cache_t;
        typedef std::map<gradient, gradient_data_cache_t::iterator> gradient_data_cache_map_t;
        typedef std::deque<gradient_data_cache_map_t::iterator> gradient_data_cache_queue_t;
    public:
        standard_fragment_shader(const std::string& aName = "standard_fragment_shader") :
            fragment_shader{ aName }
        {
            add_in_variable<vec2f>("OutputCoord"_s, 0u);
            add_in_variable<vec4f>("Color"_s, 1u);
            add_out_variable<vec4f>("FragColor"_s, 0u);
        }
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override
        {
            if (aLanguage == shader_language::Glsl)
            {
                fragment_shader::generate_code(aProgram, aLanguage, aOutput);
                aOutput.replace_all("%PARAMETERS%"_s, "inout vec4 color"_s);
                aOutput.replace_all("%FIRST_ARGS%"_s, "Color"_s);
                aOutput.replace_all("%ARGS%"_s, "color"_s);
                aOutput.replace_all("%CODE"_s,
                    "void %NAME%(inout vec4 color)\n"
                    "{\n"
                    "%CODE%"
                    "%INVOKE_NEXT%"
                    "}\n"_s);
            }
            else
                throw unsupported_language();
        }
    private:
        std::vector<float> iGradientStopPositions;
        std::vector<std::array<float, 4>> iGradientStopColours;
        gradient_data_cache_t iGradientDataCache;
        gradient_data_cache_map_t iGradientDataCacheMap;
        gradient_data_cache_queue_t iGradientDataCacheQueue;
        std::optional<neogfx::gradient_shader_data> iUncachedGradient;
    };

    class standard_texture_fragment_shader : public standard_fragment_shader
    {
    public:
        standard_texture_fragment_shader(const std::string& aName = "standard_texture_fragment_shader") :
            standard_fragment_shader{ aName }
        {
            add_in_variable<vec2f>("TexCoord"_s, 2u);
            set_uniform("multisample"_s, false);
            set_uniform("texDataFormat"_s, texture_data_format::RGBA);
            set_uniform("effect"_s, shader_effect::None);
            set_uniform("tex"_s, sampler2D{ 1 });
            set_uniform("texMS"_s, sampler2DMS{ 2 });
        }
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override
        {
                if (aLanguage == shader_language::Glsl)
                {
                    standard_fragment_shader::generate_code(aProgram, aLanguage, aOutput);
                    static const string code 
                    {
                        "    vec4 texel = vec4(0.0);\n"
                        "    if (!multisample)\n"
                        "    {\n"
                        "        texel = texture(tex, TexCoord).rgba;\n"
                        "    }\n"
                        "    else\n"
                        "    {\n"
                        "        ivec2 texCoord = ivec2(TexCoord * texExtents);\n"
                        "        texel = texelFetch(texMS, texCoord, gl_SampleID).rgba;\n"
                        "    }\n"
                        "    switch(texDataFormat)\n"
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
                        "    switch(effect)\n"
                        "    {\n"
                        "    case 0:\n" // effect: None
                        "        fragColour = texel.rgba * Color;\n"
                        "        break;\n"
                        "    case 1:\n" // effect: Colourize, ColourizeAverage
                        "        {\n"
                        "            float avg = (texel.r + texel.g + texel.b) / 3.0;\n"
                        "            fragColour = vec4(avg, avg, avg, texel.a) * Color;\n"
                        "        }\n"
                        "        break;\n"
                        "    case 2:\n" // effect: ColourizeMaximum
                        "        {\n"
                        "            float maxChannel = max(texel.r, max(texel.g, texel.b));\n"
                        "            fragColour = vec4(maxChannel, maxChannel, maxChannel, texel.a) * Color;\n"
                        "        }\n"
                        "        break;\n"
                        "    case 3:\n" // effect: ColourizeSpot
                        "        fragColour = vec4(1.0, 1.0, 1.0, texel.a) * Color;\n"
                        "        break;\n"
                        "    case 4:\n" // effect: Monochrome
                        "        {\n"
                        "            float gray = dot(Color.rgb * texel.rgb, vec3(0.299, 0.587, 0.114));\n"
                        "            fragColour = vec4(gray, gray, gray, texel.a) * Color;\n"
                        "        }\n"
                        "        break;\n"
                        "    }\n"
                        "%CODE%"
                    };
                    aOutput.replace_all("%CODE%"_s, code);
                    aOutput.replace_all("%NAME%"_s, name());
                }
                else
                    throw unsupported_language();
            }
        }
    };
}