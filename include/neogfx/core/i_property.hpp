// i_property.hpp
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
#include <type_traits>
#include <neolib/core/any.hpp>
#include <neolib/plugin/plugin_variant.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/core/i_enum.hpp>
#include <neogfx/core/i_object.hpp>
#include <neogfx/core/i_properties.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>

namespace neogfx
{
    template <typename R, typename... Args>
    struct callable_function_cracker;
    template <typename R, class C, typename... Args>
    struct callable_function_cracker<R(C::*)(Args...) const>
    {
        typedef R(C::* callable_type)(Args...) const;
        typedef R return_type;
        typedef C class_type;
    };
    template <typename R, class C, typename... Args>
    struct callable_function_cracker<R(C::*)(Args...)>
    {
        typedef R(C::* callable_type)(Args...);
        typedef R return_type;
        typedef C class_type;
    };

    typedef neolib::any custom_type;

    // todo: move to neolib::plugin_variant when geometry types are abstractable
    using property_variant = neolib::variant<
        void*,
        bool,
        char,
        int32_t,
        uint32_t,
        int64_t,
        uint64_t,
        float,
        double,
        std::string,
        size,
        point,
        rect,
        custom_type>;

    template <typename T>
    struct property_value_type_checker
    {
        static constexpr bool is_custom_type = false;
    };

    template <typename T> struct variant_type_for { typedef custom_type type; };
    template <typename T> struct variant_type_for<T*> { typedef void* type; };

    class i_property;

    class i_property_delegate
    {
    public:
        virtual ~i_property_delegate() = default;
    public:
        virtual property_variant get(const i_property& aProperty) const = 0;
        virtual void set(const i_property& aProperty) = 0;
    };

    class i_property_owner : public i_object
    {
    public:
        virtual ~i_property_owner() = default;
    public:
        virtual void property_changed(i_property& aProperty) = 0;
    public:
        virtual const i_properties& properties() const = 0;
        virtual i_properties& properties() = 0;
    };

    class i_property : public i_property_delegate
    {
        // types
    public:
        typedef i_property abstract_type;
        // events
    public:
        declare_event(property_changed, const property_variant&)
        declare_event(property_changed_from_to, const property_variant&, const property_variant&)
        // exceptions
    public:
        struct no_calculator : std::logic_error { no_calculator() : std::logic_error("neogfx::i_property::no_calculator") {} };
        struct no_delegate : std::logic_error { no_delegate() : std::logic_error("neogfx::i_property::no_delegate") {} };
        struct no_new_value : std::logic_error { no_new_value() : std::logic_error("neogfx::i_property::no_new_value") {} };
        // construction
    public:
        virtual ~i_property() = default;
        // object
    public:
        virtual i_property_owner& owner() const = 0;
        // operations
    public:
        virtual const i_string& name() const = 0;
        virtual const std::type_info& type() const = 0;
        virtual const std::type_info& category() const = 0;
        virtual bool optional() const = 0;
        virtual property_variant get_as_variant() const = 0;
        virtual property_variant get_new_as_variant() const = 0;
        virtual void set_from_variant(const property_variant& aValue) = 0;
        virtual bool has_delegate() const = 0;
        virtual i_property_delegate& delegate() const = 0;
        virtual void set_delegate(i_property_delegate& aDelegate) = 0;
        virtual void unset_delegate() = 0;
        virtual void discard_change_events() = 0;
        // implementation
    protected:
        virtual const void* data() const = 0;
        virtual void* data() = 0;
        virtual void*const* calculator_function() const = 0;
        // helpers
    public:
        template <typename T>
        const T& get() const
        {
            return *static_cast<const T*>(data());
        }
        template <typename T>
        T& get()
        {
            return *static_cast<T*>(data());
        }
        template <typename Context, typename Callable, typename... Args>
        auto calculate(Args&&... aArgs) const
        {
            // why? because we have to type-erase to support plugins and std::function can't be passed across a plugin boundary.
            auto const calculator = *reinterpret_cast<Callable const*>(calculator_function());
            if constexpr(std::is_convertible_v<const Context&, const i_property_owner&>)
                return (static_cast<const Context&>(owner()).*calculator)(std::forward<Args>(aArgs)...);
            else
                return (dynamic_cast<const Context&>(owner()).*calculator)(std::forward<Args>(aArgs)...);
        }
    };

    template <typename T, typename Getter = std::function<T()>, typename Setter = std::function<void(T const&)>>
    class property_delegate : public i_property_delegate
    {
    public:
        typedef T value_type;
    public:
        property_delegate(i_property& aSubject, Getter aGetter, Setter aSetter) :
            iSubject{ aSubject }, iGetter{ aGetter }, iSetter{ aSetter }
        {
            subject().set_delegate(*this);
        }
        ~property_delegate()
        {
            subject().unset_delegate();
        }
    public:
        property_variant get(i_property const&) const override
        {
            return iGetter();
        }
        void set(const i_property& aProperty) override
        {
            return iSetter(aProperty.get<value_type>());
        }
    public:
        i_property& subject()
        {
            return iSubject;
        }
    private:
        i_property& iSubject;
        Getter iGetter;
        Setter iSetter;
    };

    template <typename PropertyOwner>
    inline i_property& get_property(PropertyOwner& Owner, const std::string& aPropertyName)
    {
        return *static_cast<i_property_owner&>(Owner).properties().property_map().find(string{ aPropertyName })->second();
    }
}

