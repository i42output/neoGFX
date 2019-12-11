// i_shader.hpp
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
#include <neolib/plugin_variant.hpp>
#include <neolib/vector.hpp>
#include <neolib/reference_counted.hpp>
#include <neolib/i_map.hpp>
#include <neolib/i_string.hpp>
#include <neogfx/core/numerical.hpp>

namespace neogfx
{
    enum class shader_type
    {
        Compute,
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment
    };

    enum class shader_variable_qualifier
    {
        In,
        Out
    };

    enum class shader_language : uint32_t
    {
        Glsl
    };

    typedef neolib::vector<float> shader_float_array;
    typedef neolib::vector<double> shader_double_array;
    
    enum class shader_data_type : uint32_t
    {
        Boolean,
        Float,
        Double,
        Int,
        Vec2f,
        Vec2,
        Vec3f,
        Vec3,
        Vec4f,
        Vec4,
        Mat44f,
        Mat44,
        FloatArray,
        DoubleArray,
        Sampler2D,
        Sampler2DMS
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::shader_data_type> neolib::enum_enumerators_v<neogfx::shader_data_type>
{
    declare_enum_string(neogfx::shader_data_type, Boolean)
    declare_enum_string(neogfx::shader_data_type, Float)
    declare_enum_string(neogfx::shader_data_type, Double)
    declare_enum_string(neogfx::shader_data_type, Int)
    declare_enum_string(neogfx::shader_data_type, Vec2f)
    declare_enum_string(neogfx::shader_data_type, Vec2)
    declare_enum_string(neogfx::shader_data_type, Vec3f)
    declare_enum_string(neogfx::shader_data_type, Vec3)
    declare_enum_string(neogfx::shader_data_type, Vec4f)
    declare_enum_string(neogfx::shader_data_type, Vec4)
    declare_enum_string(neogfx::shader_data_type, Mat44f)
    declare_enum_string(neogfx::shader_data_type, Mat44)
    declare_enum_string(neogfx::shader_data_type, FloatArray)
    declare_enum_string(neogfx::shader_data_type, DoubleArray)
    declare_enum_string(neogfx::shader_data_type, Sampler2D)
    declare_enum_string(neogfx::shader_data_type, Sampler2DMS)
};

namespace neogfx 
{
    template <shader_data_type HandleType>
    struct shader_handle
    {
        int handle;
        typedef shader_handle abstract_type;
    };

    typedef shader_handle<shader_data_type::Sampler2D> sampler2D;
    typedef shader_handle<shader_data_type::Sampler2DMS> sampler2DMS;

    typedef neolib::plugin_variant<shader_data_type, bool, float, double, int, vec2f, vec2, vec3f, vec3, vec4f, vec4, mat44f, mat44, shader_float_array, shader_double_array, sampler2D, sampler2DMS> shader_value_type;

    typedef uint32_t shader_variable_location;
    typedef neolib::pair<neolib::pair<shader_variable_location, shader_variable_qualifier>, shader_value_type::index_type> shader_variable;

    class i_rendering_context;
    class i_shader_program;

    class i_shader : public neolib::i_reference_counted
    {
        typedef i_shader self_type;
    public:
        typedef self_type asbtract_type;
        typedef abstract_t<shader_value_type> value_type;
        typedef neolib::i_map<i_string, neolib::i_pair<value_type, bool>> uniform_map;
    public:
        struct variable_not_found : std::logic_error { variable_not_found() : std::logic_error{ "neogfx::i_shader::variable_not_found" } {} };
        struct unsupported_language : std::logic_error { unsupported_language() : std::logic_error{ "neogfx::i_shader::unsupported_language" } {} };
    public:
        virtual ~i_shader() {}
    public:
        virtual shader_type type() const = 0;
        virtual const i_string& name() const = 0;
        virtual void* handle() const = 0;
        virtual bool enabled() const = 0;
        virtual void enable() const = 0;
        virtual void disable() const = 0;
        virtual bool dirty() const = 0;
        virtual void set_dirty() = 0;
        virtual void set_clean() = 0;
    public:
        virtual const uniform_map& uniforms() const = 0;
        virtual void clear_uniform(const i_string& aName) = 0;
        virtual void set_uniform(const i_string& aName, const value_type& aValue) = 0;
        template <typename T>
        std::enable_if_t<!std::is_enum_v<T>, void> set_uniform(const i_string& aName, const T& aValue)
        {
            set_uniform(aName, shader_value_type{ aValue });
        }
        template <typename Enum>
        std::enable_if_t<std::is_enum_v<Enum>, void> set_uniform(const i_string& aName, Enum aEnum)
        {
            set_uniform(aName, static_cast<int>(aEnum));
        }
        template <std::size_t ArraySize>
        void set_uniform(const i_string& aName, const float(&aArray)[ArraySize])
        {
            set_uniform(aName, shader_float_array{ &aArray[0], &aArray[0] + ArraySize });
        }
        template <std::size_t ArraySize>
        void set_uniform(const i_string& aName, const double(&aArray)[ArraySize])
        {
            set_uniform(aName, shader_double_array{ &aArray[0], &aArray[0] + ArraySize });
        }
        void set_uniform(const i_string& aName, const float* aArray, std::size_t aArraySize)
        {
            set_uniform(aName, shader_float_array{ aArray, aArray + aArraySize });
        }
        void set_uniform(const i_string& aName, const double* aArray, std::size_t aArraySize)
        {
            set_uniform(aName, shader_double_array{ aArray, aArray + aArraySize });
        }
        virtual void clear_variable(const i_string& aName) = 0;
        virtual void add_variable(const i_string& aName, const abstract_t<shader_variable>& aVariable) = 0;
        template <typename T>
        void add_in_variable(const i_string& aName, shader_variable_location aLocation)
        {
            add_variable(aName, shader_variable{ { aLocation, shader_variable_qualifier::In }, neolib::index_of<T, shader_value_type>() });
        }
        template <typename T>
        void add_out_variable(const i_string& aName, shader_variable_location aLocation)
        {
            add_variable(aName, shader_variable{ { aLocation, shader_variable_qualifier::Out }, neolib::index_of<T, shader_value_type>() });
        }
    public:
        virtual void prepare_uniforms(const i_rendering_context& aRenderingContext, i_shader_program& aProgram) = 0;
        virtual const i_string& generate_code(i_shader_program& aProgram, shader_language aLanguage) const = 0;
    protected:
        virtual void replace_tokens(i_shader_program& aProgram, shader_language aLanguage, i_string& aSource) const = 0;
    };

    template <typename Shader, typename... Args>
    inline neolib::ref_ptr<i_shader> make_shader(Args&&... aArgs)
    {
        return neolib::ref_ptr<i_shader>(new Shader{ std::forward<Args>(aArgs)... });
    }
}