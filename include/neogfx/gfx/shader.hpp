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
        typedef neolib::map<string, shader_variable> variable_map;
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
        void* handle(const i_shader_program& aProgram) const override
        {
            if (!aProgram.is_first_in_stage(*this))
                return aProgram.first_in_stage(*this)
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
        bool disabled() const override
        {
            return !iEnabled;
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
            {
                if ((iUniforms[aName].first().which() == shader_data_type::FloatArray &&
                    iUniforms[aName].first().get<shader_float_array>().size() != aValue.get<shader_float_array>().size()) ||
                    (iUniforms[aName].first().which() == shader_data_type::DoubleArray &&
                    iUniforms[aName].first().get<shader_double_array>().size() != aValue.get<shader_double_array>().size()))
                    set_dirty();
                iUniforms[aName] = neolib::make_pair(aValue, true);
            }
        }
        const variable_map& variables() const override
        {
            return iVariables;
        }
        void clear_variable(const i_string& aName) override
        {
            auto v = iVariables.find(aName);
            if (v != iVariables.end())
            {
                iVariables.erase(v);
                set_dirty();
            }
        }
        void add_variable(const i_string& aName, const abstract_t<shader_variable>& aVariable) override
        {
            if (iVariables.find(aName) == iVariables.end() || iVariables[aName].second() != aVariable.second().second())
            {
                iVariables.emplace(aName, aVariable);
                set_dirty();
            }
        }
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
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
                        "%INVOKE_DECLARATIONS%"
                        "%CODE%"
                    };
                    static const string sOpenGlEsSource =
                    {
                        "#version 110\n"
                        "precision mediump float;\n"
                        "\n"
                        "%UNIFORMS%"
                        "%VARIABLES%"
                        "%INVOKE_DECLARATIONS%"
                        "%FUNCTIONS%"
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
                    throw unsupported_shader_language();
            }
            if (aProgram.is_last_in_stage(*this))
            {
                if (aLanguage == shader_language::Glsl)
                {
                    aOutput.replace_all("%INVOKE_NEXT%"_s, ""_s);
                    static const string source =
                    {
                        "%CODE%"
                        "void main()\n"
                        "{\n"
                        "%INVOKE_FIRST%"
                        "}\n"
                    };
                    if (aProgram.first_in_stage(*this))
                        aOutput.replace_all("%CODE%"_s, source);
                    else
                        aOutput += source;
                }
                else
                    throw unsupported_shader_language();
            }
            if (!aProgram.first_in_stage(*this) && !aProgram.last_in_stage(*this))
            {
                if (aLanguage == shader_language::Glsl)
                    aOutput += "%CODE%"_s;
                else
                    throw unsupported_shader_language();
            }
            if (aLanguage == shader_language::Glsl)
            {
                string uniformDefinitions;
                for (auto const& u : uniforms())
                {
                    string uniformDefinition;
                    switch (u.second().first().which())
                    {
                    case shader_data_type::Boolean:
                        uniformDefinition = "uniform bool %I%;\n"_s;
                        break;
                    case shader_data_type::Float:
                        uniformDefinition = "uniform float %I%;\n"_s;
                        break;
                    case shader_data_type::Double:
                        uniformDefinition = "uniform double %I%;\n"_s;
                        break;
                    case shader_data_type::Int:
                        uniformDefinition = "uniform int %I%;\n"_s;
                        break;
                    case shader_data_type::Vec2:
                        uniformDefinition = "uniform vec2 %I%;\n"_s;
                        break;
                    case shader_data_type::DVec2:
                        uniformDefinition = "uniform dvec2 %I%;\n"_s;
                        break;
                    case shader_data_type::Vec3:
                        uniformDefinition = "uniform vec3 %I%;\n"_s;
                        break;
                    case shader_data_type::DVec3:
                        uniformDefinition = "uniform dvec3 %I%;\n"_s;
                        break;
                    case shader_data_type::Vec4:
                        uniformDefinition = "uniform vec4 %I%;\n"_s;
                        break;
                    case shader_data_type::DVec4:
                        uniformDefinition = "uniform dvec4 %I%;\n"_s;
                        break;
                    case shader_data_type::Mat44:
                        uniformDefinition = "uniform mat4 %I%;\n"_s;
                        break;
                    case shader_data_type::DMat44:
                        uniformDefinition = "uniform dmat4 %I%;\n"_s;
                        break;
                    case shader_data_type::FloatArray:
                        uniformDefinition = "uniform float %I%["_s + string{ std::to_string(u.second().first().get<shader_float_array>().size()) } + "];\n"_s;
                        break;
                    case shader_data_type::DoubleArray:
                        uniformDefinition = "uniform double %I%["_s + string{ std::to_string(u.second().first().get<shader_double_array>().size()) } +"];\n"_s;
                        break;
                    case shader_data_type::Sampler2D:
                        uniformDefinition = "uniform sampler2D %I%;\n"_s;
                        break;
                    case shader_data_type::Sampler2DMS:
                        uniformDefinition = "uniform sampler2DMS %I%;\n"_s;
                        break;
                    }
                    uniformDefinition.replace_all("%I%"_s, u.first());
                    uniformDefinitions += uniformDefinition;
                }
                // use of replace_all instead of boost::format? this code will not be critical path as shader source code will not need to change that often
                aOutput.replace_all("%UNIFORMS%"_s, uniformDefinitions);
                string variableDefinitions;
                for (auto const& v : variables())
                {
                    string variableDefinition;
                    switch (v.second())
                    {
                    case shader_data_type::Boolean:
                        variableDefinition = "%L%%Q% mediump bool %I%;\n"_s;
                        break;
                    case shader_data_type::Float:
                        variableDefinition = "%L%%Q% mediump float %I%;\n"_s;
                        break;
                    case shader_data_type::Double:
                        variableDefinition = "%L%%Q% mediump double %I%;\n"_s;
                        break;
                    case shader_data_type::Int:
                        variableDefinition = "%L%%Q% mediump int %I%;\n"_s;
                        break;
                    case shader_data_type::Vec2f:
                        variableDefinition = "%L%%Q% mediump vec2 %I%;\n"_s;
                        break;
                    case shader_data_type::Vec2:
                        variableDefinition = "%L%%Q% mediump dvec2 %I%;\n"_s;
                        break;
                    case shader_data_type::Vec3f:
                        variableDefinition = "%L%%Q% mediump vec3 %I%;\n"_s;
                        break;
                    case shader_data_type::Vec3:
                        variableDefinition = "%L%%Q% mediump dvec3 %I%;\n"_s;
                        break;
                    case shader_data_type::Vec4f:
                        variableDefinition = "%L%%Q% mediump vec4 %I%;\n"_s;
                        break;
                    case shader_data_type::Vec4:
                        variableDefinition = "%L%%Q% mediump dvec4 %I%;\n"_s;
                        break;
                    case shader_data_type::Mat44f:
                        variableDefinition = "%L%%Q% mediump mat4 %I%;\n"_s;
                        break;
                    case shader_data_type::Mat44:
                        variableDefinition = "%L%%Q% mediump dmat4 %I%;\n"_s;
                        break;
                    case shader_data_type::FloatArray:
                        variableDefinition = "%L%%Q% mediump float %I%["_s + string{ std::to_string(u.second().first().get<shader_float_array>().size()) } +"];\n"_s;
                        break;
                    case shader_data_type::DoubleArray:
                        variableDefinition = "%L%%Q% mediump double %I%["_s + string{ std::to_string(u.second().first().get<shader_double_array>().size()) } +"];\n"_s;
                        break;
                    default:
                        throw invalid_variable_type();
                    }
                    variableDefinition.replace_all("%Q%"_s, neolib::enum_to_string<string>(v.second().first().second()));
                    variableDefinition.replace_all("%L%"_s, "layout (location = %L%) "_s);
                    variableDefinition.replace_all("%L%"_s, std::to_string(v.second().first().first()));
                    variableDefinition.replace_all("%I%"_s, v.first());
                    variableDefinitions += variableDefinition;
                }
                aOutput.replace_all("%VARIABLES%"_s, variableDefinitions);
                aOutput.replace_all("%INVOKE_DECLARATIONS%"_s, "void %NAME%(%PARAMETERS%);%INVOKE_DECLARATIONS%"_s);
                if (aProgram.is_last_in_stage(*this))
                {
                    aOutput.replace_all("%INVOKE_DECLARATIONS%"_s, ""_s);
                    aOutput.replace_all("%INVOKE_FIRST%"_s, "    %FIRST_NAME%(%FIRST_ARGS%);\n"_s);
                    aOutput.replace_all("%FIRST_NAME%"_s, aProgram.first_in_stage(*this).name());
                }
                aOutput.replace_all("%INVOKE_NEXT%"_s, "    %NAME%(%ARGS%);\n"_s);
            }
            else
                throw unsupported_shader_language();
        }
    private:
        shader_type iType;
        string iName;
        mutable std::optional<void*> iHandle;
        bool iEnabled;
        bool iDirty;
        uniform_map iUniforms;
        variable_map iVariables;
    };
}