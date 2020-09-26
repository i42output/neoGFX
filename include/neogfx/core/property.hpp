// property.hpp
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
#include <string>
#include <neolib/core/optional.hpp>
#include <neogfx/core/i_object.hpp>
#include <neogfx/core/i_property.hpp>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702 ) // unreachable code
#endif

namespace neogfx
{
    template <typename T, typename Category, class Context, typename Calculator = T(*)()>
    class property : public i_property, public neolib::lifetime<>
    {
        typedef property<T, Category, Context, Calculator> self_type;
    public:
        typedef T value_type;
        typedef Context context_type;
    public:
        define_declared_event(PropertyChanged, property_changed, const property_variant&)
        define_declared_event(PropertyChangedFromTo, property_changed_from_to, const property_variant&, const property_variant&)
        define_event(Changed, changed, const value_type&)
        define_event(ChangedFromTo, changed_from_to, const value_type&, const value_type&)
    public:
        struct invalid_type : std::logic_error { invalid_type() : std::logic_error("neogfx::property::invalid_type") {} };
    public:
        typedef Category category_type;
        typedef Calculator calculator_function_type;
    public:
        template <typename ParentType>
        class optional_proxy
        {
        public:
            typedef ParentType parent_type;
        public:
            optional_proxy(parent_type& aParent) : iParent{ aParent }
            {
            }
        public:
            template <typename SFINAE = neolib::optional_t<T>>
            operator const typename std::enable_if<neolib::is_optional_v<T>, SFINAE>::type&() const
            {
                return *iParent.value();
            }
            template <typename T2, typename SFINAE = optional_proxy<parent_type>>
            typename std::enable_if<!std::is_const<parent_type>::value, SFINAE>::type& operator=(const T2& aValue)
            {
                iParent.assign(aValue);
                return *this;
            }
            template <typename SFINAE = const neolib::optional_t<T>*>
            const typename std::enable_if<neolib::is_optional_v<T>, SFINAE>::type operator->() const
            {
                return &*iParent.value();
            }
        private:
            parent_type& iParent;
        };
    public:
        property(i_property_owner& aOwner, const std::string& aName) : iOwner{ aOwner }, iName{ aName }, iValue{}
        {
            aOwner.properties().register_property(*this);
        }
        property(i_property_owner& aOwner, const std::string& aName, calculator_function_type aCalculator) : iOwner{ aOwner }, iName{ aName }, iCalculator{ aCalculator }, iValue{}
        {
            aOwner.properties().register_property(*this);
        }
        property(i_property_owner& aOwner, const std::string& aName, const T& aValue) : iOwner{ aOwner }, iName{ aName }, iValue { aValue }
        {
            aOwner.properties().register_property(*this);
        }
        property(i_property_owner& aOwner, const std::string& aName, calculator_function_type aCalculator, const T& aValue) : iOwner{ aOwner }, iName{ aName }, iCalculator{ aCalculator }, iValue{ aValue }
        {
            aOwner.properties().register_property(*this);
        }
    public:
        property_variant get(const i_property& aProperty) const override
        {
            return get_as_variant();
        }
    public:
        i_property_owner& owner() const override
        {
            return iOwner;
        }
    public:
        const string& name() const override
        {
            return iName;
        }
        const std::type_info& type() const override
        {
            return typeid(value_type);
        }
        const std::type_info& category() const override
        {
            return typeid(category_type);
        }
        bool optional() const override
        {
            return neolib::is_optional_v<T>;
        }
        property_variant get_as_variant() const override
        {
            if constexpr (neolib::is_optional_v<T>)
            {
                if (value() != std::nullopt)
                    return *value();
                else
                    return neolib::none;
            }
            return value();
        }
        property_variant get_new_as_variant() const override
        {
            if (iNewValue != std::nullopt)
            {
                if constexpr (neolib::is_optional_v<T>)
                {
                    if (*iNewValue != std::nullopt)
                        return **iNewValue;
                    else
                        return neolib::none;
                }
                return *iNewValue;
            }
            throw no_new_value();
        }
        void set_from_variant(const property_variant& aValue) override
        {
            std::visit([this](auto&& arg)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>)
                    *this = value_type{};
                else
                    *this = std::forward<decltype(arg)>(arg);
            }, aValue);
        }
        bool has_delegate() const override
        {
            return iDelegate != nullptr;
        }   
        i_property_delegate const& delegate() const override
        {
            if (has_delegate())
                return *iDelegate;
            throw no_delegate();
        }
        i_property_delegate& delegate() override
        {
            return const_cast<i_property_delegate&>(to_const(*this).delegate());
        }
        void set_delegate(i_property_delegate& aDelegate) override
        {
            iDelegate = &aDelegate;
        }
        void unset_delegate() override
        {
            iDelegate = nullptr;
        }
        void discard_change_events() override
        {
            iDiscardChangeEvents = true;
        }
    public:
        const value_type& value() const
        {
            if (!has_delegate())
                return iValue;
            const value_type* dptr = nullptr;
            std::visit([this, &dptr](auto&& arg)
            {
                typedef std::decay_t<decltype(arg)> try_type;
                if constexpr (std::is_same_v<try_type, value_type>)
                    dptr = &arg;
                else if constexpr (std::is_same_v<try_type, custom_type>)
                    dptr = &neolib::any_cast<const value_type&>(arg);
                else if constexpr (std::is_same_v<try_type, neolib::none_t>)
                    dptr = nullptr;
                else
                {
                    // [[unreachable]]
                    throw invalid_type();
                }
            }, delegate().get(*this));
            return (iValue = *dptr);
        }
        template <typename T2>
        self_type& assign(T2&& aValue, bool aOwnerNotify = true)
        {
            typedef std::decay_t<decltype(aValue)> try_type;
            if constexpr (std::is_same_v<try_type, value_type> || std::is_same_v<std::optional<try_type>, value_type>)
                return do_assign(std::forward<T2>(aValue), aOwnerNotify);
            else if constexpr (std::is_same_v<try_type, custom_type>)
                return do_assign(neolib::any_cast<value_type>(std::forward<T2>(aValue)), aOwnerNotify);
            else if constexpr (std::is_same_v<try_type, neolib::none_t>)
                return do_assign(value_type{}, aOwnerNotify);
            else if constexpr (std::is_arithmetic_v<value_type> && std::is_convertible_v<try_type, value_type> && std::is_integral_v<try_type> == std::is_integral_v<value_type>)
                return do_assign(static_cast<value_type>(std::forward<T2>(aValue)), aOwnerNotify);
            else
            {
                // [[unreachable]]
                (void)aValue;
                (void)aOwnerNotify;
                throw invalid_type();
            }
        }
        template <typename T2>
        self_type& operator=(T2&& aValue)
        {
            return assign(std::forward<T2>(aValue));
        }
        operator const value_type&() const
        {
            return value();
        }
        template <typename SFINAE = optional_proxy<const self_type>>
        const typename std::enable_if<neolib::is_optional_v<T>, SFINAE>::type operator*() const
        {
            return optional_proxy<const self_type>{ *this };
        }
        template <typename SFINAE = optional_proxy<self_type>>
        typename std::enable_if<neolib::is_optional_v<T>, SFINAE>::type operator*()
        {
            return optional_proxy<self_type>{ *this };
        }
        template <typename SFINAE = optional_proxy<const self_type>>
        const typename std::enable_if<neolib::is_optional_v<T>, SFINAE>::type operator->() const
        {
            return optional_proxy<const self_type>{ *this };
        }
        template <typename T>
        bool operator==(const T& aRhs) const
        {
            return value() == aRhs;
        }
        template <typename T>
        bool operator!=(const T& aRhs) const
        {
            return value() != aRhs;
        }
        template <typename T>
        bool operator==(const std::optional<T>& aRhs) const
        {
            return value() == aRhs;
        }
        template <typename T>
        bool operator!=(const std::optional<T>& aRhs) const
        {
            return value() != aRhs;
        }
    protected:
        const void* data() const override
        {
            if (!has_delegate())
                return &value();
            else
                return delegate().data();
        }
        void* data() override
        {
            if (!has_delegate())
                return &mutable_value();
            else
                return delegate().data();
        }
        void*const* calculator_function() const override
        {
            // why? because we have to type-erase to support plugins and std::function can't be passed across a plugin boundary.
            if (iCalculator != nullptr)
                return reinterpret_cast<void*const*>(&iCalculator);
            throw no_calculator();
        }
    private:
        value_type& mutable_value()
        {
            return const_cast<value_type&>(to_const(*this).value());
        }
        template <typename T2>
        self_type& do_assign(T2&& aValue, bool aOwnerNotify = true)
        {
            if (mutable_value() != aValue)
            {
                iNewValue = std::forward<T2>(aValue);
                destroyed_flag destroyed{ *this };
                try
                {
                    PropertyChanged.pre_trigger();
                    if (destroyed)
                        return *this;
                    PropertyChangedFromTo.pre_trigger();
                    if (destroyed)
                        return *this;
                    Changed.pre_trigger();
                    if (destroyed)
                        return *this;
                    ChangedFromTo.pre_trigger();
                    if (destroyed)
                        return *this;
                }
                catch (...)
                {
                    iDiscardChangeEvents = false;
                    throw;
                }
                auto const previousValue = mutable_value();
                mutable_value() = std::forward<T2>(aValue);
                iNewValue = std::nullopt;
                if (iDiscardChangeEvents)
                {
                    iDiscardChangeEvents = false;
                    return *this;
                }
                if (aOwnerNotify)
                    iOwner.property_changed(*this);
                if (destroyed)
                    return *this;
                bool const discardChanged = !PropertyChanged.trigger(get_as_variant());
                if (destroyed)
                    return *this;
                bool const discardChangedFromTo = !PropertyChangedFromTo.trigger(property_variant{ previousValue }, get_as_variant());
                if (destroyed)
                    return *this;
                if (!discardChanged && !Changed.trigger(value()))
                    return *this;
                if (destroyed)
                    return *this;
                if (!discardChangedFromTo && !ChangedFromTo.trigger(previousValue, value()))
                    return *this;
                if (destroyed)
                    return *this;
            }
            return *this;
        }
    private:
        i_property_owner& iOwner;
        string iName;
        calculator_function_type iCalculator;
        mutable value_type iValue;
        std::optional<value_type> iNewValue;
        i_property_delegate* iDelegate = nullptr;
        bool iDiscardChangeEvents = false;
    };

    namespace property_category
    {
        struct soft_geometry {};
        struct hard_geometry {};
        struct font {};
        struct color {};
        struct other_appearance {};
        struct interaction {};
        struct other {};
    };

    #define define_property( category, type, name, calculator, ... ) neogfx::property<type, category, property_context_type, decltype(&property_context_type::##calculator)> name = { *this, #name ##s, &property_context_type::##calculator, __VA_ARGS__ };
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif
