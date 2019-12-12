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
#include <neogfx/gfx/i_shader_program.hpp>
#include <neogfx/gfx/shader.hpp>

namespace neogfx
{
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
    public:
        standard_fragment_shader(const std::string& aName = "standard_fragment_shader") :
            fragment_shader{ aName }
        {
            add_in_variable<vec2f>("OutputCoord"_s, 0u);
            add_in_variable<vec4f>("Color"_s, 1u);
            add_out_variable<vec4f>("FragColor"_s, 0u);
        }
    public:
        void generate_code(i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override
        {
            if (aLanguage == shader_language::Glsl)
            {
                fragment_shader::generate_code(aProgram, aLanguage, aOutput);
                if (aProgram.is_last_in_stage(*this))
                {
                    aOutput.replace_all("%INVOKE_FIRST_SHADER%"_s, "    %FIRST_SHADER_NAME%(Color);\n"_s);
                    aOutput.replace_all("%FIRST_SHADER_NAME%"_s, aProgram.first_in_stage(*this).name());
                }
                aOutput.replace_all("%INVOKE_NEXT_SHADER%"_s, "    %SHADER_NAME%(color);\n"_s);
            }
            else
                throw unsupported_language();
        }
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
        void generate_code(i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override
        {
                if (aLanguage == shader_language::Glsl)
                {
                    standard_fragment_shader::generate_code(aProgram, aLanguage, aOutput);
                    aOutput.replace_all("%CODE%"_s,
                        "%SHADER_NAME%(inout vec4 color)\n"
                        "{\n"
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
                        "%INVOKE_NEXT_SHADER%"
                        "}\n"_s);
                    aOutput.replace_all("%SHADER_NAME%"_s, name());
                }
                else
                    throw unsupported_language();
            }
        }
    };
}