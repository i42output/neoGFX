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
#include <neolib/jar.hpp>
#include <neolib/set.hpp>
#include <neogfx/gfx/i_shader.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
    #define cache_uniform( uniformName ) cached_uniform uniformName = { *this, #uniformName };

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
        typedef neolib::polymorphic_jar<shader_uniform> uniform_list;
        typedef neolib::set<shader_variable> variable_list;
        class cached_uniform
        {
        public:
            cached_uniform(shader<Base>& aParent, const char* const aName) :
                iParent{ aParent }, iName{ aName }
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
                        iId = iParent.create_uniform(iName);
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
            mutable std::optional<shader_uniform_id> iId;
        };
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
                return aProgram.first_in_stage(type()).handle(aProgram);
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
            for (auto& u : uniforms())
                u.clear_location();
        }
        void set_clean() override
        {
            iDirty = false;
        }
        bool uniforms_changed() const override
        {
            for (auto& u : uniforms())
                if (u.is_dirty())
                    return true;
            return false;
        }
    public:
        const i_shader::uniform_list& uniforms() const override
        {
            return iUniforms.items();
        }
    protected:
        uniform_list& uniforms()
        {
            return iUniforms;
        }
    public:
        void clear_uniform(shader_uniform_id aUniform) override
        {
            iUniforms.remove(aUniform);
            set_dirty();
        }
        shader_uniform_id create_uniform(const i_string& aName) override
        {
            auto id = uniforms().next_cookie();
            uniforms().add(id, id, aName, value_type{});
            set_dirty();
            return id;
        }
        shader_uniform_id find_uniform(const i_string& aName) const override
        {
            for (auto const& u : uniforms())
                if (u.name() == aName)
                    return u.id();
            return no_uniform;
        }
        using i_shader::set_uniform;
        void set_uniform(shader_uniform_id aUniform, const abstract_value_type& aValue) override
        {
            auto& u = uniforms()[aUniform];
            if (u.value().empty() != aValue.empty() || (!u.value().empty() && u.different_type_to(aValue)))
                set_dirty();
            u.set_value(aValue);
        }
        void clear_uniform_location(shader_uniform_id aUniform) override
        {
            uniforms()[aUniform].clear_location();
        }
        void update_uniform_location(shader_uniform_id aUniform, shader_uniform_location aLocation) override
        {
            uniforms()[aUniform].set_location(aLocation);
        }
        const variable_list& in_variables() const override
        {
            return iInVariables;
        }
        const variable_list& out_variables() const override
        {
            return iOutVariables;
        }
        void clear_variable(const i_string& aName) override
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
        i_shader_variable& add_variable(const i_shader_variable& aVariable) override
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
                        "#version 410\n"
                        "precision mediump float;\n"
                        "\n"
                        "%UNIFORMS%"
                        "%VARIABLES%"
                        "\n"_s
                    };
                    static const string sOpenGlEsSource =
                    {
                        "#version 110\n"
                        "precision mediump float;\n"
                        "\n"
                        "%UNIFORMS%"
                        "%VARIABLES%"
                        "\n"_s
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
                    std::map<std::pair<shader_variable_qualifier, shader_variable_location>, string> variableDefinitions;
                    for (auto const& s : aProgram.stage(type()))
                    {
                        if (s->disabled())
                            continue;
                        for (auto const& u : s->uniforms())
                        {
                            string uniformDefinition;
                            switch (u.value().which())
                            {
                            case shader_data_type::FloatArray:
                                uniformDefinition = "uniform float %I%["_s + to_string(u.value().get<abstract_t<shader_float_array>>().size()) +"];\n"_s;
                                break;
                            case shader_data_type::DoubleArray:
                                uniformDefinition = "uniform double %I%["_s + to_string(u.value().get<abstract_t<shader_double_array>>().size()) +"];\n"_s;
                                break;
                            default:
                                uniformDefinition = "uniform %T% %I%;\n"_s;
                                break;
                            }
                            uniformDefinition.replace_all("%T%"_s, enum_to_string(u.value().which()));
                            uniformDefinition.replace_all("%I%"_s, u.name());
                            uniformDefinitions[u.name()] = uniformDefinition;
                        }
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
                    string vdefs;
                    for (auto const& vdef : variableDefinitions)
                        vdefs += vdef.second;
                    aOutput.replace_all("%UNIFORMS%"_s, udefs);
                    aOutput.replace_all("%VARIABLES%"_s, vdefs);
                }
                else
                    throw unsupported_shader_language();
            }
        }
        void generate_invoke(const i_shader_program& aProgram, shader_language aLanguage, i_string& aInvokes) const override
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
        variable_list iInVariables;
        variable_list iOutVariables;
    };
}