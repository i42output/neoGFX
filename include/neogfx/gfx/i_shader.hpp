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

    enum class shader_variable_qualifier : uint32_t
    {
        In,
        Out
    };

    enum class shader_language : uint32_t
    {
        Glsl
    };

    enum class shader_data_type : uint32_t
    {
        Boolean,
        Float,
        Double,
        Int,
        Vec2,
        DVec2,
        Vec3,
        DVec3,
        Vec4,
        DVec4,
        Mat4,
        DMat4,
        FloatArray,
        DoubleArray,
        Sampler2D,
        Sampler2DMS,
        Sampler2DRect
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::shader_variable_qualifier> neolib::enum_enumerators_v<neogfx::shader_variable_qualifier>
{
    declare_enum_string(neogfx::shader_variable_qualifier, In)
    declare_enum_string(neogfx::shader_variable_qualifier, Out)
};

template <>
const neolib::enum_enumerators_t<neogfx::shader_language> neolib::enum_enumerators_v<neogfx::shader_language>
{
    declare_enum_string(neogfx::shader_language, Glsl)
};

template <>
const neolib::enum_enumerators_t<neogfx::shader_data_type> neolib::enum_enumerators_v<neogfx::shader_data_type>
{
    declare_enum_string(neogfx::shader_data_type, Boolean)
    declare_enum_string(neogfx::shader_data_type, Float)
    declare_enum_string(neogfx::shader_data_type, Double)
    declare_enum_string(neogfx::shader_data_type, Int)
    declare_enum_string(neogfx::shader_data_type, Vec2)
    declare_enum_string(neogfx::shader_data_type, DVec2)
    declare_enum_string(neogfx::shader_data_type, Vec3)
    declare_enum_string(neogfx::shader_data_type, DVec3)
    declare_enum_string(neogfx::shader_data_type, Vec4)
    declare_enum_string(neogfx::shader_data_type, DVec4)
    declare_enum_string(neogfx::shader_data_type, Mat4)
    declare_enum_string(neogfx::shader_data_type, DMat4)
    declare_enum_string(neogfx::shader_data_type, FloatArray)
    declare_enum_string(neogfx::shader_data_type, DoubleArray)
    declare_enum_string(neogfx::shader_data_type, Sampler2D)
    declare_enum_string(neogfx::shader_data_type, Sampler2DMS)
    declare_enum_string(neogfx::shader_data_type, Sampler2DRect)
};

namespace neogfx 
{
    typedef neolib::vector<float> shader_float_array;
    typedef neolib::vector<double> shader_double_array;

    template <shader_data_type HandleType>
    struct shader_handle
    {
        int handle;
        typedef shader_handle abstract_type;
    };

    typedef shader_handle<shader_data_type::Sampler2D> sampler2D;
    typedef shader_handle<shader_data_type::Sampler2DMS> sampler2DMS;
    typedef shader_handle<shader_data_type::Sampler2DRect> sampler2DRect;

    typedef neolib::plugin_variant<shader_data_type, bool, float, double, int, vec2f, vec2, vec3f, vec3, vec4f, vec4, mat4f, mat4, shader_float_array, shader_double_array, sampler2D, sampler2DMS, sampler2DRect> shader_value_type;

    typedef uint32_t shader_variable_location;
    typedef neolib::pair<shader_variable_location, enum_t<shader_variable_qualifier>> shader_variable_lq;
    typedef neolib::pair<shader_variable_lq, enum_t<shader_data_type>> shader_variable;

    class i_rendering_context;
    class i_shader_program;

    struct shader_variable_not_found : std::logic_error { shader_variable_not_found() : std::logic_error{ "neogfx::shader_variable_not_found" } {} };
    struct invalid_shader_variable_type : std::logic_error { invalid_shader_variable_type() : std::logic_error{ "neogfx::invalid_shader_variable_type" } {} };
    struct unsupported_shader_language : std::logic_error { unsupported_shader_language() : std::logic_error{ "neogfx::unsupported_shader_language" } {} };

    class i_shader : public neolib::i_reference_counted
    {
        typedef i_shader self_type;
    public:
        typedef self_type asbtract_type;
        typedef abstract_t<shader_value_type> value_type;
        typedef neolib::i_map<i_string, neolib::i_pair<value_type, bool>> uniform_map;
        typedef neolib::i_map<i_string, abstract_t<shader_variable>> variable_map;
    public:
        virtual ~i_shader() {}
    public:
        virtual shader_type type() const = 0;
        virtual const i_string& name() const = 0;
        virtual void* handle(const i_shader_program& aProgram) const = 0;
        virtual bool enabled() const = 0;
        virtual bool disabled() const = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;
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
        virtual const variable_map& variables() const = 0;
        virtual void clear_variable(const i_string& aName) = 0;
        virtual void add_variable(const i_string& aName, const abstract_t<shader_variable>& aVariable) = 0;
        template <typename T>
        void add_in_variable(const i_string& aName, shader_variable_location aLocation)
        {
            add_variable(aName, shader_variable{ shader_variable_lq{ aLocation, shader_variable_qualifier::In }, static_cast<shader_data_type>(neolib::index_of<T, shader_value_type>()) });
        }
        template <typename T>
        void add_out_variable(const i_string& aName, shader_variable_location aLocation)
        {
            add_variable(aName, shader_variable{ shader_variable_lq{ aLocation, shader_variable_qualifier::Out }, static_cast<shader_data_type>(neolib::index_of<T, shader_value_type>()) });
        }
    public:
        virtual void prepare_uniforms(const i_rendering_context& aRenderingContext, i_shader_program& aProgram) = 0;
        virtual void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const = 0;
    };

    template <typename Shader, typename... Args>
    inline neolib::ref_ptr<i_shader> make_shader(Args&&... aArgs)
    {
        return neolib::ref_ptr<i_shader>(new Shader{ std::forward<Args>(aArgs)... });
    }
}