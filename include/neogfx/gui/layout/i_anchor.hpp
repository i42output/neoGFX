// i_anchor.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <functional>

#include <neolib/core/i_enum.hpp>

#include <neogfx/core/i_property.hpp>
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    enum class anchor_constraint_function : std::uint32_t
    {
        Invalid         = 0x00000000,

        Identity        = 0x00000001,
        Equal           = 0x00000002,
        Min             = 0x00000003,
        Max             = 0x00000004,
        Custom          = 0x00001000, // todo

        X               = 0x00010000,
        Y               = 0x00020000,
        Z               = 0x00040000,
        W               = 0x00080000,

        CX              = X,
        CY              = Y,

        IdentityX       = Identity | X,
        IdentityY       = Identity | Y,
        IdentityZ       = Identity | Z,
        IdentityW       = Identity | W,
        EqualX          = Equal | X,
        EqualY          = Equal | Y,
        EqualZ          = Equal | Z,
        EqualW          = Equal | W,
        MinX            = Min | X,
        MinY            = Min | Y,
        MinZ            = Min | Z,
        MinW            = Min | W,
        MaxX            = Max | X,
        MaxY            = Max | Y,
        MaxZ            = Max | Z,
        MaxW            = Max | W,

        IdentityCX      = Identity | CX,
        IdentityCY      = Identity | CY,
        EqualCX         = Equal | CX,
        EqualCY         = Equal | CY,
        MinCX           = Min | CX,
        MinCY           = Min | CY,
        MaxCX           = Max | CX,
        MaxCY           = Max | CY,

        FUNCTION_MASK   = 0x0000FFFF,
        ARGUMENT_MASK   = 0x00FF0000
    };

    inline constexpr anchor_constraint_function operator~(anchor_constraint_function lhs)
    {
        return static_cast<anchor_constraint_function>(~static_cast<std::uint32_t>(lhs));
    }

    inline constexpr anchor_constraint_function operator&(anchor_constraint_function lhs, anchor_constraint_function rhs)
    {
        return static_cast<anchor_constraint_function>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
    }

    inline constexpr anchor_constraint_function operator|(anchor_constraint_function lhs, anchor_constraint_function rhs)
    {
        return static_cast<anchor_constraint_function>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
    }
}

begin_declare_enum(neogfx::anchor_constraint_function)
declare_enum_string(neogfx::anchor_constraint_function, Identity)
declare_enum_string(neogfx::anchor_constraint_function, Equal)
declare_enum_string(neogfx::anchor_constraint_function, Min)
declare_enum_string(neogfx::anchor_constraint_function, Max)
declare_enum_string(neogfx::anchor_constraint_function, Custom)
end_declare_enum(neogfx::anchor_constraint_function)

namespace neogfx
{
    template <typename T>
    class anchor_constraint : public std::function<T(const T&, const T&)>
    {
        typedef std::function<T(const T&, const T&)> base_type;
    public:
        typedef T value_type;
    public:
        using base_type::base_type;
    public:
        static anchor_constraint<T> identity;
        static anchor_constraint<T> identity_x;
        static anchor_constraint<T> identity_y;
        static anchor_constraint<T> equal;
        static anchor_constraint<T> equal_x;
        static anchor_constraint<T> equal_y;
        static anchor_constraint<T> min;
        static anchor_constraint<T> min_x;
        static anchor_constraint<T> min_y;
        static anchor_constraint<T> max;
        static anchor_constraint<T> max_x;
        static anchor_constraint<T> max_y;
    };

    template <typename T>
    inline T constraint_x(const T& value)
    {
        return value;
    }

    template <typename T>
    inline T constraint_y(const T& value)
    {
        return value;
    }

    template <typename T>
    inline T constraint_x(const basic_point<T>& value)
    {
        return value.x;
    }

    template <typename T>
    inline T constraint_y(const basic_point<T>& value)
    {
        return value.y;
    }

    template <typename T>
    inline T constraint_x(const basic_size<T>& value)
    {
        return value.cx;
    }

    template <typename T>
    inline T constraint_y(const basic_size<T>& value)
    {
        return value.cy;
    }

    template <typename T>
    inline T constraint_x(const basic_box_areas<T>& value)
    {
        return value.size().cx;
    }

    template <typename T>
    inline T constraint_y(const basic_box_areas<T>& value)
    {
        return value.size().cy;
    }

    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::identity = [](const T& lhs, const T&) -> T 
    { 
        return lhs; 
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::identity_x = [](const T& lhs, const T& rhs) -> T
    {
        return T{ constraint_x(lhs), constraint_y(rhs) };
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::identity_y = [](const T& lhs, const T& rhs) -> T
    {
        return T{ constraint_x(rhs), constraint_y(lhs) };
    };

    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::equal = [](const T&, const T& rhs) -> T 
    { 
        return rhs; 
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::equal_x = [](const T& lhs, const T& rhs) -> T
    {
        return T{ constraint_x(rhs), constraint_y(lhs) };
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::equal_y = [](const T& lhs, const T& rhs) -> T
    {
        return T{ constraint_x(lhs), constraint_y(rhs) };
    };

    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::min = [](const T& lhs, const T& rhs) -> T 
    { 
        return std::min(lhs, rhs); 
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::min_x = [](const T& lhs, const T& rhs) -> T
    {
        return T{ std::min(constraint_x(lhs), constraint_x(rhs)), constraint_y(lhs) };
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::min_y = [](const T& lhs, const T& rhs) -> T
    {
        return T{ constraint_x(lhs), std::min(constraint_y(lhs), constraint_y(rhs)) };
    };

    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::max = [](const T& lhs, const T& rhs) -> T 
    { 
        return std::max(lhs, rhs);
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::max_x = [](const T& lhs, const T& rhs) -> T
    {
        return T{ std::max(constraint_x(lhs), constraint_x(rhs)), constraint_y(lhs) };
    };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::max_y = [](const T& lhs, const T& rhs) -> T
    {
        return T{ constraint_x(lhs), std::max(constraint_y(lhs), constraint_y(rhs)) };
    };

    struct anchor_property_has_no_value : std::logic_error { anchor_property_has_no_value() : std::logic_error{ "neogfx::anchor_property_has_no_value" } {} };

    class i_anchorable;

    class i_anchor
    {
        // construction
    public:
        virtual ~i_anchor() = default;
        // meta
    public:
        virtual i_anchorable& owner() const = 0;
        virtual const i_string& name() const = 0;
        virtual const i_property& property() const = 0;
        virtual i_property& property() = 0;
        virtual bool active() const noexcept = 0;
        virtual bool calculator_overriden() const noexcept = 0;
        virtual bool calculating() const noexcept = 0;
        // operations
    public:
        virtual void constrain(i_anchor& aRhs, anchor_constraint_function aLhsFunction, anchor_constraint_function aRhsFunction) = 0;
        virtual void constrain(i_anchor& aOther, anchor_constraint_function aOtherFunction) = 0;
    };

    template <typename T, typename PVT, typename... CalculatorArgs>
    class i_calculating_anchor : public i_anchor
    {
    public:
        typedef i_calculating_anchor abstract_type;
        typedef T value_type;
        typedef PVT property_value_type;
        typedef anchor_constraint<value_type> constraint;
    public:
        virtual bool property_set() const = 0;
        virtual value_type const& property_value() const = 0;
        virtual value_type& property_value() = 0;
        virtual void add_constraint(const constraint& aConstraint, abstract_type& aOtherAnchor) = 0;
        virtual void add_constraint(const constraint& aConstraint, std::shared_ptr<abstract_type> aOtherAnchor) = 0;
    public:
        virtual value_type evaluate_constraints(const CalculatorArgs&... aArgs) const = 0;
        virtual value_type calculate(const CalculatorArgs&... aArgs) const = 0;
    };

    namespace detail
    {
        template <template<typename, typename, typename...> class Anchor, typename PVT, typename Callable>
        struct abstract_anchor_callable_function_cracker;
        template <template<typename, typename, typename...> class Anchor, typename PVT, typename R, typename C, typename... Args>
        struct abstract_anchor_callable_function_cracker<Anchor, PVT, R(C::*)(Args...) const>
        {
            typedef Anchor<R, PVT, Args...> type;
            typedef R(C::* callable_type)(Args...) const;
            typedef PVT property_value_type;
            typedef R value_type;
            typedef C class_type;
        };
        template <template<typename, typename, typename...> class Anchor, typename PVT, typename R, typename C, typename... Args>
        struct abstract_anchor_callable_function_cracker<Anchor, PVT, R(C::*)(Args...)>
        {
            typedef Anchor<R, PVT, Args...> type;
            typedef R(C::* callable_type)(Args...);
            typedef PVT property_value_type;
            typedef R value_type;
            typedef C class_type;
        };
    }

    template <typename Property>
    using i_anchor_t = typename detail::abstract_anchor_callable_function_cracker<i_calculating_anchor, typename Property::value_type, typename Property::calculator_function_type>::type;
}
