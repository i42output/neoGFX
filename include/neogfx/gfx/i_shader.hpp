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
#include <neolib/jar.hpp>
#include <neolib/reference_counted.hpp>
#include <neolib/i_vector.hpp>
#include <neolib/i_set.hpp>
#include <neolib/i_string.hpp>
#include <neogfx/core/numerical.hpp>

namespace neogfx
{
    enum class shader_type : uint32_t
    {
        Compute,
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment,
        COUNT
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
        Uint,
        Vec2,
        DVec2,
        IVec2,
        UVec2,
        Vec3,
        DVec3,
        IVec3,
        UVec3,
        Vec4,
        DVec4,
        IVec4,
        UVec4,
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
    declare_enum_string_explicit(neogfx::shader_variable_qualifier, In, in)
    declare_enum_string_explicit(neogfx::shader_variable_qualifier, Out, out)
};

template <>
const neolib::enum_enumerators_t<neogfx::shader_language> neolib::enum_enumerators_v<neogfx::shader_language>
{
    declare_enum_string(neogfx::shader_language, Glsl)
};

template <>
const neolib::enum_enumerators_t<neogfx::shader_data_type> neolib::enum_enumerators_v<neogfx::shader_data_type>
{
    declare_enum_string_explicit(neogfx::shader_data_type, Boolean, bool)
    declare_enum_string_explicit(neogfx::shader_data_type, Float, float)
    declare_enum_string_explicit(neogfx::shader_data_type, Double, double)
    declare_enum_string_explicit(neogfx::shader_data_type, Int, int)
    declare_enum_string_explicit(neogfx::shader_data_type, Uint, uint)
    declare_enum_string_explicit(neogfx::shader_data_type, Vec2, vec2)
    declare_enum_string_explicit(neogfx::shader_data_type, DVec2, dvec2)
    declare_enum_string_explicit(neogfx::shader_data_type, IVec2, ivec2)
    declare_enum_string_explicit(neogfx::shader_data_type, UVec2, uvec2)
    declare_enum_string_explicit(neogfx::shader_data_type, Vec3, vec3)
    declare_enum_string_explicit(neogfx::shader_data_type, DVec3, dvec3)
    declare_enum_string_explicit(neogfx::shader_data_type, IVec3, ivec3)
    declare_enum_string_explicit(neogfx::shader_data_type, UVec3, uvec3)
    declare_enum_string_explicit(neogfx::shader_data_type, Vec4, vec4)
    declare_enum_string_explicit(neogfx::shader_data_type, DVec4, dvec4)
    declare_enum_string_explicit(neogfx::shader_data_type, IVec4, ivec4)
    declare_enum_string_explicit(neogfx::shader_data_type, UVec4, uvec4)
    declare_enum_string_explicit(neogfx::shader_data_type, Mat4, mat4)
    declare_enum_string_explicit(neogfx::shader_data_type, DMat4, dmat4)
    declare_enum_string(neogfx::shader_data_type, FloatArray)
    declare_enum_string(neogfx::shader_data_type, DoubleArray)
    declare_enum_string_explicit(neogfx::shader_data_type, Sampler2D, sampler2D)
    declare_enum_string_explicit(neogfx::shader_data_type, Sampler2DMS, sampler2DMS)
    declare_enum_string_explicit(neogfx::shader_data_type, Sampler2DRect, sampler2DRect)
};

namespace neogfx 
{
    struct unknown_uniform_location : std::logic_error { unknown_uniform_location() : std::logic_error{ "neogfx::unknown_uniform_location" } {} };
    struct shader_variable_not_linked : std::logic_error { shader_variable_not_linked() : std::logic_error{ "neogfx::shader_variable_not_linked" } {} };
    struct shader_variable_not_found : std::logic_error { shader_variable_not_found() : std::logic_error{ "neogfx::shader_variable_not_found" } {} };
    struct invalid_shader_variable_type : std::logic_error { invalid_shader_variable_type() : std::logic_error{ "neogfx::invalid_shader_variable_type" } {} };
    struct unsupported_shader_language : std::logic_error { unsupported_shader_language() : std::logic_error{ "neogfx::unsupported_shader_language" } {} };

    typedef neolib::vector<float> shader_float_array;
    typedef neolib::vector<double> shader_double_array;

    template <shader_data_type HandleType>
    struct shader_handle
    {
        int handle;
        typedef shader_handle<HandleType> self_type;
        typedef self_type abstract_type;
        bool operator==(const self_type& aRhs) const { return handle == aRhs.handle; }
    };

    typedef shader_handle<shader_data_type::Sampler2D> sampler2D;
    typedef shader_handle<shader_data_type::Sampler2DMS> sampler2DMS;
    typedef shader_handle<shader_data_type::Sampler2DRect> sampler2DRect;

    typedef neolib::plugin_variant<shader_data_type, bool, float, double, int32_t, uint32_t, vec2f, vec2, vec2i32, vec2u32, vec3f, vec3, vec3i32, vec3u32, vec4f, vec4, vec4i32, vec4u32, mat4f, mat4, shader_float_array, shader_double_array, sampler2D, sampler2DMS, sampler2DRect> shader_value_type;

    typedef uint32_t shader_variable_location;
    typedef int32_t shader_uniform_location;

    typedef neolib::cookie shader_uniform_id;
    constexpr shader_uniform_id no_uniform = shader_uniform_id{};

    class i_shader_uniform
    {
    public:
        virtual shader_uniform_id id() const = 0;
        virtual const i_string& name() const = 0;
        virtual bool has_location() const = 0;
        virtual shader_uniform_location location() const = 0;
        virtual void set_location(shader_uniform_location aLocation) = 0;
        virtual void clear_location() = 0;
        virtual const abstract_t<shader_value_type>& value() const = 0;
        virtual void set_value(const abstract_t<shader_value_type>& aValue) = 0;
        virtual bool is_dirty() const = 0;
        virtual void clean() const = 0;
        virtual bool different_type_to(const abstract_t<shader_value_type>& aValue) const = 0;
    public:
        template <typename T>
        void set_value(const T& aValue)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_same_v<T, bool>)
                    set_value(to_abstract(shader_value_type{ aValue }));
                else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>)
                    set_value(to_abstract(shader_value_type{ aValue }));
                else if constexpr (std::is_signed_v<T>)
                    set_value(to_abstract(shader_value_type{ static_cast<int32_t>(aValue) }));
                else if constexpr (std::is_unsigned_v<T>)
                    set_value(to_abstract(shader_value_type{ static_cast<uint32_t>(aValue) }));
                else
                    set_value(to_abstract(shader_value_type{ static_cast<int32_t>(aValue) }));
            }
            else if constexpr (std::is_enum_v<T>)
                set_value(to_abstract(shader_value_type{ static_cast<int32_t>(aValue) }));
            else
                set_value(to_abstract(shader_value_type{ aValue }));
        }
    };

    class shader_uniform : public i_shader_uniform
    {
        typedef i_shader_uniform base_type;
    public:
        typedef base_type abstract_type;
    public:
        template <typename T>
        shader_uniform(shader_uniform_id aId, const string& aName, const T& aValue) :
            iId { aId },
            iName{ aName }, 
            iValue{ aValue },
            iDirty{ true }
        {
        }
        shader_uniform(const shader_uniform& aOther) :
            iId{ aOther.iId },
            iName{ aOther.iName },
            iValue{ aOther.iValue },
            iDirty{ true }
        {
        }
        shader_uniform(shader_uniform&& aOther) noexcept :
            iId{ std::move(aOther.iId) },
            iName{ std::move(aOther.iName) },
            iValue{ std::move(aOther.iValue) },
            iDirty{ true }
        {
        }
        shader_uniform(const i_shader_uniform& aOther) :
            iId{ aOther.id() },
            iName{ aOther.name() },
            iValue{ aOther.value() },
            iDirty{ true }
        {
        }
    public:
        shader_uniform& operator=(const shader_uniform& aOther)
        {
            if (&aOther == this)
                return *this;
            this->~shader_uniform();
            new (this) shader_uniform{ aOther };
            return *this;
        }
        shader_uniform& operator=(shader_uniform&& aOther) noexcept
        {
            if (&aOther == this)
                return *this;
            iId = std::move(aOther.iId);
            iName = std::move(aOther.iName);
            iValue = std::move(aOther.iValue);
            iDirty = true;
            return *this;
        }
    public:
        shader_uniform_id id() const override
        {
            return iId;
        }
        const i_string& name() const override 
        { 
            return iName; 
        }
        bool has_location() const override
        {
            return iLocation != std::nullopt;
        }
        shader_uniform_location location() const override
        {
            if (has_location())
                return *iLocation;
            throw unknown_uniform_location();
        }
        void set_location(shader_uniform_location aLocation) override
        {
            iLocation = aLocation;
        }
        void clear_location() override
        {
            iLocation = std::nullopt;
        }
        const abstract_t<shader_value_type>& value() const override 
        { 
            return iValue; 
        }
        void set_value(const abstract_t<shader_value_type>& aValue) override 
        { 
            if (iValue != aValue)
            {
                iValue = aValue;
                iDirty = true;
            }
        }
        bool is_dirty() const override 
        { 
            return iDirty; 
        }
        void clean() const override 
        { 
            iDirty = false; 
        }
        bool different_type_to(const abstract_t<shader_value_type>& aValue) const
        {
            if (value().which() != aValue.which())
                return true;
            switch (value().which())
            {
            case shader_data_type::FloatArray:
                return value().get<abstract_t<shader_float_array>>().size() !=
                    aValue.get<abstract_t<shader_float_array>>().size();
            case shader_data_type::DoubleArray:
                return value().get<abstract_t<shader_double_array>>().size() !=
                    aValue.get<abstract_t<shader_double_array>>().size();
            default:
                return false;
            }
        }
    public:
        shader_uniform_id iId;
        string iName;
        mutable std::optional<shader_uniform_location> iLocation;
        shader_value_type iValue;
        mutable bool iDirty;
    };

    class i_shader_variable
    {
    public:
        virtual const i_string& name() const = 0;
        virtual shader_variable_location location() const = 0;
        virtual const i_enum_t<shader_variable_qualifier>& qualifier() const = 0;
        virtual const i_enum_t<shader_data_type>& type() const = 0;
        virtual bool has_link() const = 0;
        virtual const i_shader_variable& link() const = 0;
        virtual void link(const i_shader_variable& aOther) = 0;
        virtual void reset_link() = 0;
    };

    class shader_variable : public i_shader_variable
    {
        typedef i_shader_variable base_type;
    public:
        typedef base_type abstract_type;
    public:
        shader_variable(
                const string& aName,
                shader_variable_location aLocation,
                shader_variable_qualifier aQualifier,
                shader_data_type aType) :
            iName{ aName },
            iLocation{ aLocation },
            iQualifier{ aQualifier },
            iType{ aType },
            iLink{ nullptr }
        {
        }
        shader_variable(const i_shader_variable& aOther) :
            iName{ aOther.name() },
            iLocation{ aOther.location() },
            iQualifier{ aOther.qualifier() },
            iType{ aOther.type() },
            iLink{ nullptr }
        {
            if (aOther.has_link())
                link(aOther.link());
        }
    public:
        const i_string& name() const override 
        { 
            return iName; 
        }
        shader_variable_location location() const override 
        { 
            return iLocation; 
        }
        const i_enum_t<shader_variable_qualifier>& qualifier() const override 
        { 
            return iQualifier; 
        }
        const i_enum_t<shader_data_type>& type() const override 
        { 
            return iType; 
        }
        bool has_link() const override 
        { 
            return iLink != nullptr; 
        }
        const i_shader_variable& link() const override 
        { 
            if (has_link()) 
                return *iLink; 
            throw shader_variable_not_linked(); 
        }
        void link(const i_shader_variable& aOther) override 
        { 
            iLink = &aOther; 
        }
        void reset_link() override 
        { 
            iLink = nullptr; 
        }
    public:
        bool operator<(const shader_variable& aRhs) const
        {
            return location() < aRhs.location();
        }
    public:
        string iName;
        shader_variable_location iLocation;
        enum_t<shader_variable_qualifier> iQualifier;
        enum_t<shader_data_type> iType;
        const i_shader_variable* iLink;
    };

    class i_rendering_context;
    class i_shader_program;

    class i_shader : public neolib::i_reference_counted
    {
        typedef i_shader self_type;
    public:
        typedef self_type asbtract_type;
        typedef abstract_t<shader_value_type> value_type;
        typedef neolib::i_vector<i_shader_uniform> uniform_list;
        typedef neolib::i_set<i_shader_variable> variable_list;
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
        virtual const uniform_list& uniforms() const = 0;
        virtual void clear_uniform(shader_uniform_id aUniform) = 0;
        virtual shader_uniform_id create_uniform(const i_string& aName) = 0;
        virtual shader_uniform_id find_uniform(const i_string& aName) const = 0;
        virtual void set_uniform(shader_uniform_id aUniform, const value_type& aValue) = 0;
        virtual void clear_uniform_location(shader_uniform_id aUniform) = 0;
        virtual void update_uniform_location(shader_uniform_id aUniform, shader_uniform_location aLocation) = 0;
        virtual const variable_list& in_variables() const = 0;
        virtual const variable_list& out_variables() const = 0;
        virtual void clear_variable(const i_string& aName) = 0;
        virtual i_shader_variable& add_variable(const i_shader_variable& aVariable) = 0;
    public:
        virtual void prepare_uniforms(const i_rendering_context& aContext, i_shader_program& aProgram) = 0;
        virtual void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const = 0;
        virtual void generate_invoke(const i_shader_program& aProgram, shader_language aLanguage, i_string& aInvokes) const = 0;
    public:
        void set_uniform(const i_string& aName, const value_type& aValue)
        {
            auto existing = find_uniform(aName);
            if (existing == no_uniform)
                existing = create_uniform(aName);
            set_uniform(existing, aValue);
        }
        template <typename T>
        void set_uniform(const i_string& aName, const T& aValue)
        {
            if constexpr (!std::is_enum_v<T>)
                set_uniform(aName, to_abstract(shader_value_type{ aValue }));
            else
                set_uniform(aName, to_abstract(shader_value_type{ static_cast<int>(aValue) }));
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
        template <typename T>
        i_shader_variable& add_in_variable(const i_string& aName, shader_variable_location aLocation)
        {
            return add_variable(shader_variable{ aName, aLocation, shader_variable_qualifier::In,  static_cast<shader_data_type>(neolib::index_of<T, shader_value_type>()) });
        }
        template <typename T>
        i_shader_variable& add_out_variable(const i_string& aName, shader_variable_location aLocation)
        {
            return add_variable(shader_variable{ aName, aLocation, shader_variable_qualifier::Out,  static_cast<shader_data_type>(neolib::index_of<T, shader_value_type>()) });
        }
    };

    template <typename Shader, typename... Args>
    inline neolib::ref_ptr<i_shader> make_shader(Args&&... aArgs)
    {
        return neolib::ref_ptr<i_shader>(new Shader{ std::forward<Args>(aArgs)... });
    }
}