// shader.hpp
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
#include <neolib/map.hpp>
#include <neogfx/gfx/i_shader.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
    template <typename Base>
    class shader : public neolib::reference_counted<Base>
    {
        typedef shader<Base> self_type;
        typedef neolib::reference_counted<Base> base_type;
    public:
        using typename base_type::abstract_type;
        typedef i_shader::value_type abstract_value_type;
        typedef shader_value_type value_type;
    private:
        typedef neolib::map<string, neolib::pair<value_type, bool>> uniform_map;
    public:
        shader(shader_type aType, const std::string& aName, bool aEnabled = true) : 
            iType{ aType },
            iName{ aName },
            iEnabled{ aEnabled },
            iDirty{ true }
        {
        }
        ~shader()
        {
            if (iHandle != std::nullopt)
                service<i_rendering_engine>().destroy_shader_object(*iHandle);
        }
    public:
        shader_type type() const override
        {
            return iType;
        }
        const i_string& name() const override
        {
            return iName;
        }
        void* handle() const override
        {
            if (iHandle == std::nullopt)
                iHandle = service<i_rendering_engine>().create_shader_object(type());
            if (*iHandle == nullptr)
                throw failed_to_create_shader_program("Failed to create shader object");
            return *iHandle;
        }
        bool enabled() const override
        {
            return iEnabled;
        }
        void enable() override
        {
            if (!iEnabled)
            {
                iEnabled = true;
                set_dirty();
            }
        }
        void disable() override
        {
            if (iEnabled)
            {
                iEnabled = false;
                set_dirty();
            }
        }
        bool dirty() const override
        {
            return iDirty;
        }
        void set_dirty() override
        {
            iDirty = true;
        }
        void set_clean() override
        {
            iDirty = false;
            for (auto& u : iUniforms)
                u.second().second() = false;
        }
    public:
        const uniform_map& uniforms() const override
        {
            return iUniforms;
        }
        void clear_uniform(const i_string& aName) override
        {
            auto u = iUniforms.find(aName);
            if (u != iUniforms.end())
            {
                iUniforms.erase(u);
                set_dirty();
            }
        }
        using i_shader::set_uniform;
        void set_uniform(const i_string& aName, const abstract_value_type& aValue) override
        {
            if (iUniforms.find(aName) == iUniforms.end() || iUniforms[aName].which() != aValue.which())
            {
                iUniforms.emplace(aName, neolib::make_pair(aValue, true));
                set_dirty();
            }
            else if (iUniforms[aName].first != aValue)
                iUniforms[aName] = neolib::make_pair(aValue, true);
        }
    public:
        void generate_code(i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
        {
            if (aProgram.is_first_in_stage(*this))
            {
                if (aLanguage == shader_language::Glsl)
                {
                    static const string sOpenGlSource =
                    {
                        "#version 400\n"
                        "precision mediump float;\n"
                        "\n"
                        "%UNIFORMS%"
                        "%VARIABLES%"
                        "%CODE%"
                    };
                    static const string sOpenGlEsSource =
                    {
                        "#version 110\n"
                        "precision mediump float;\n"
                        "\n"
                        "%UNIFORMS%"
                        "%VARIABLES%"
                        "%CODE%"
                    };
                    switch (service<i_rendering_engine>().renderer())
                    {
                    case renderer::OpenGL:
                    case renderer::Software:
                    default:
                        aOutput = sOpenGlSource;
                        break;
                    case renderer::DirectX:
                        aOutput = sOpenGlEsSource;
                        break;
                    }
                }
                else
                    throw unsupported_language();
            }
            else if (aProgram.is_last_in_stage(*this))
            {
                if (aLanguage == shader_language::Glsl)
                {
                    aOutput.replace_all("%INVOKE_NEXT_SHADER%"_s, ""_s);
                    static const string source =
                    {
                        "%CODE%"
                        "void main()\n"
                        "{\n"
                        "%INVOKE_FIRST_SHADER%"
                        "}\n"
                    };
                    aOutput += source;
                }
                else
                    throw unsupported_language();
            }
            else
            {
                if (aLanguage == shader_language::Glsl)
                    aOutput += "%CODE%"_s;
                else
                    throw unsupported_language();
            }
        }
    private:
        shader_type iType;
        string iName;
        mutable std::optional<void*> iHandle;
        bool iEnabled;
        bool iDirty;
        uniform_map iUniforms;
    };
}