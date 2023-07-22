// shader.hpp
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
#include <neolib/core/jar.hpp>
#include <neolib/core/set.hpp>
#include <neogfx/gfx/i_shader.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
    #define cache_uniform( uniformName ) cached_uniform uniformName = { *this, #uniformName };
    #define cache_shared_uniform( uniformName ) cached_uniform uniformName = { *this, #uniformName, true };

    template <typename Base>
    class shader : public reference_counted<Base>
    {
        typedef shader<Base> self_type;
        typedef reference_counted<Base> base_type;
    public:
        using typename base_type::abstract_type;
        typedef i_shader::value_type abstract_value_type;
        typedef shader_value_type value_type;
    protected:
        typedef neolib::jar<shader_uniform> uniform_list;
        typedef neolib::set<shader_variable> variable_list;
        class cached_uniform
        {
        public:
            cached_uniform(shader<Base>& aParent, const char* const aName, bool aShared = false) :
                iParent{ aParent }, iName{ aName }, iShared{ aShared }
            {
            }
        public:
            const i_shader_uniform& uniform() const
            {
                if (iId == std::nullopt)
                {
                    auto existing = iParent.find_uniform(iName);
                    if (existing != no_uniform)
                        iId = existing;
                    if (iId == std::nullopt)
                        iId = iParent.create_uniform(iName, iShared);
                }
                return iParent.uniforms()[*iId];
            }
            i_shader_uniform& uniform()
            {
                return const_cast<i_shader_uniform&>(to_const(*this).uniform());
            }
            template <typename T>
            i_shader_uniform& operator=(const T& aValue)
            {
                uniform().set_value(aValue);
                return uniform();
            }
        private:
            shader<Base>& iParent;
            string iName;
            bool iShared;
            mutable std::optional<shader_uniform_id> iId;
        };
    public:
        shader(shader_type aType, std::string const& aName, bool aEnabled = true) : 
            iType{ aType },
            iName{ aName },
            iEnabled{ aEnabled },
            iDirty{ true },
            iHasSharedUniforms{ false }
        {
        }
        ~shader()
        {
            if (iHandle != std::nullopt)
                service<i_rendering_engine>().destroy_shader_object(*iHandle);
        }
    public:
        shader_type type() const final
        {
            return iType;
        }
        const i_string& name() const final
        {
            return iName;
        }
        bool supports(vertex_buffer_type aBufferType) const override
        {
            return false;
        }
        void* handle(const i_shader_program& aProgram) const final
        {
            if (!aProgram.is_first_in_stage(*this))
                return aProgram.first_in_stage(type()).handle(aProgram);
            if (iHandle == std::nullopt)
                iHandle = service<i_rendering_engine>().create_shader_object(type());
            if (*iHandle == nullptr)
                throw failed_to_create_shader_program("Failed to create shader object");
            return *iHandle;
        }
        bool enabled() const final
        {
            return iEnabled;
        }
        bool disabled() const final
        {
            return !iEnabled;
        }
        void enable() final
        {
            if (!iEnabled)
            {
                iEnabled = true;
                set_dirty();
            }
        }
        void disable() final
        {
            if (iEnabled)
            {
                iEnabled = false;
                set_dirty();
            }
        }
        bool dirty() const final
        {
            return iDirty;
        }
        void set_dirty() final
        {
            iDirty = true;
            for (auto& u : uniforms())
                u.clear_storage();
        }
        void set_clean() final
        {
            iDirty = false;
        }
        bool uniforms_changed() const final
        {
            for (auto& u : uniforms())
                if (u.is_dirty())
                    return true;
            return false;
        }
    public:
        const i_shader::uniform_list& uniforms() const final
        {
            return iUniforms.items();
        }
        bool has_shared_uniforms() const final
        {
            return iHasSharedUniforms;
        }
    protected:
        uniform_list& uniforms()
        {
            return iUniforms;
        }
    public:
        void clear_uniform(shader_uniform_id aUniform) final
        {
            iUniforms.remove(aUniform);
            set_dirty();
        }
        shader_uniform_id create_uniform(const i_string& aName, bool aShared = false) final
        {
            auto id = uniforms().next_cookie();
            uniforms().add(id, id, aName, aShared, value_type{});
            iHasSharedUniforms = iHasSharedUniforms || aShared;
            set_dirty();
            return id;
        }
        shader_uniform_id find_uniform(const i_string& aName) const final
        {
            for (auto const& u : uniforms())
                if (u.name() == aName)
                    return u.id();
            return no_uniform;
        }
        using i_shader::set_uniform;
        void set_uniform(shader_uniform_id aUniform, const abstract_value_type& aValue) final
        {
            auto& u = uniforms()[aUniform];
            if (u.value().empty() != aValue.empty() || (!u.value().empty() && u.different_type_to(aValue)))
                set_dirty();
            u.set_value(aValue);
        }
        void clear_uniform_storage(shader_uniform_id aUniform) final
        {
            uniforms()[aUniform].clear_storage();
        }
        void update_uniform_storage(shader_uniform_id aUniform, shader_uniform_storage aStorage) final
        {
            uniforms()[aUniform].set_storage(aStorage);
        }
        void clear_uniform_location(shader_uniform_id aUniform) final
        {
            uniforms()[aUniform].clear_location();
        }
        void update_uniform_location(shader_uniform_id aUniform, shader_uniform_location aLocation) final
        {
            uniforms()[aUniform].set_location(aLocation);
        }
        const variable_list& in_variables() const final
        {
            return iInVariables;
        }
        const variable_list& out_variables() const final
        {
            return iOutVariables;
        }
        void clear_variable(const i_string& aName) final
        {
            for (auto v = in_variables().begin(); v != in_variables().end(); ++v)
                if (v->name() == aName)
                {
                    in_variables().erase(v);
                    return;
                }
            for (auto v = out_variables().begin(); v != out_variables().end(); ++v)
                if (v->name() == aName)
                {
                    out_variables().erase(v);
                    return;
                }
        }
        i_shader_variable& add_variable(const i_shader_variable& aVariable) final
        {
            auto& variableList = (aVariable.qualifier().value<shader_variable_qualifier>() == shader_variable_qualifier::In ? 
                iInVariables : iOutVariables);
            auto existing = variableList.find(aVariable);
            if (existing == variableList.end())
            {
                auto& v = *variableList.insert(aVariable);
                set_dirty();
                return v;
            }
            if (existing->name() != aVariable.name())
                set_dirty();
            *existing = aVariable;
            return *existing;
        }
    public:
        void prepare_uniforms(const i_rendering_context&, i_shader_program&) override
        {
        }
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override
        {
            if (aProgram.is_first_in_stage(*this))
            {
                if (aLanguage == shader_language::Glsl)
                {
                    static const string sOpenGlSource =
                    {
                        1 + R"glsl(
#version 420
precision mediump float;

layout (binding = %UNIFORM_BLOCK_INDEX%) uniform %UNIFORM_BLOCK_NAME% 
{
%UNIFORMS%
};
%SINGULAR_UNIFORMS%
%VARIABLES%

                        )glsl"
                    };
                    static const string sOpenGlEsSource =
                    {
                        1 + R"glsl(
#version 110
precision mediump float;

layout (binding = %UNIFORM_BLOCK_INDEX%) uniform %UNIFORM_BLOCK_NAME% 
{
%UNIFORMS%
};
%SINGULAR_UNIFORMS%
%VARIABLES%

                        )glsl"
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
                    std::map<string, string> uniformDefinitions;
                    std::map<string, string> singularUniformDefinitions;
                    std::map<std::pair<shader_variable_qualifier, shader_variable_location>, string> variableDefinitions;
                    for (auto const& s : aProgram.stage(type())->shaders())
                    {
                        for (auto const& u : s->uniforms())
                        {
                            if (s->disabled() && !u.shared())
                                continue;
                            string uniformDefinition;
                            switch (u.value().which())
                            {
                            case shader_data_type::FloatArray:
                                uniformDefinition = "    float %I%["_s + to_string(u.value().get<abstract_t<shader_float_array>>().size()) +"];\n"_s;
                                break;
                            case shader_data_type::DoubleArray:
                                uniformDefinition = "    double %I%["_s + to_string(u.value().get<abstract_t<shader_double_array>>().size()) +"];\n"_s;
                                break;
                            default:
                                if (!u.singular())
                                    uniformDefinition = "    %T% %I%;\n"_s;
                                else
                                    uniformDefinition = "uniform %T% %I%;\n"_s;
                                break;
                            }
                            uniformDefinition.replace_all("%T%"_s, enum_to_string(u.value().which()));
                            uniformDefinition.replace_all("%I%"_s, u.name());
                            if (!u.singular())
                                uniformDefinitions[u.name()] = uniformDefinition;
                            else
                                singularUniformDefinitions[u.name()] = uniformDefinition;
                        }
                        if (s->disabled())
                            continue;
                        for (auto const& v : s->in_variables())
                        {
                            string variableDefinition = "layout (location = %L%) in %T% %I%;\n"_s;
                            variableDefinition.replace_all("%T%"_s, enum_to_string<shader_data_type>(v.type()));
                            variableDefinition.replace_all("%L%"_s, to_string(v.location()));
                            variableDefinition.replace_all("%I%"_s, v.name());                            
                            variableDefinitions[std::make_pair(v.qualifier().value<shader_variable_qualifier>(), v.location())] = variableDefinition;
                        };
                        for (auto const& v : s->out_variables())
                        {
                            string variableDefinition = "layout (location = %L%) out %T% %I%;\n"_s;
                            variableDefinition.replace_all("%T%"_s, enum_to_string<shader_data_type>(v.type()));
                            variableDefinition.replace_all("%L%"_s, to_string(v.location()));
                            variableDefinition.replace_all("%I%"_s, v.name());
                            variableDefinitions[std::make_pair(v.qualifier().value<shader_variable_qualifier>(), v.location())] = variableDefinition;
                        };                        
                    }
                    string udefs;
                    for (auto const& udef : uniformDefinitions)
                        udefs += udef.second;
                    string sudefs;
                    for (auto const& sudef : singularUniformDefinitions)
                        sudefs += sudef.second;
                    string vdefs;
                    for (auto const& vdef : variableDefinitions)
                        vdefs += vdef.second;
                    aOutput.replace_all("%UNIFORM_BLOCK_INDEX%"_s, neolib::string{ std::to_string(static_cast<std::uint32_t>(type())) });
                    aOutput.replace_all("%UNIFORM_BLOCK_NAME%"_s, enum_to_string(type()) + "Uniforms");
                    aOutput.replace_all("%UNIFORMS%"_s, udefs);
                    aOutput.replace_all("%SINGULAR_UNIFORMS%"_s, sudefs);
                    aOutput.replace_all("%VARIABLES%"_s, vdefs);
                }
                else
                    throw unsupported_shader_language();
            }
        }
        void generate_invoke(const i_shader_program& aProgram, shader_language aLanguage, i_string& aInvokes) const final
        {
            aInvokes += "    "_s + name() + "("_s;
            bool first = true;
            for (auto const& out : out_variables())
            {
                if (!first)
                    aInvokes += ", "_s;
                else
                    first = false;
                aInvokes += ("arg"_s + out.name());
            }
            aInvokes += ");\n"_s;
        }
    protected:
        variable_list& in_variables()
        {
            return iInVariables;
        }
        variable_list& out_variables()
        {
            return iOutVariables;
        }
    private:
        shader_type iType;
        string iName;
        mutable std::optional<void*> iHandle;
        bool iEnabled;
        bool iDirty;
        uniform_list iUniforms;
        bool iHasSharedUniforms;
        variable_list iInVariables;
        variable_list iOutVariables;
    };
}