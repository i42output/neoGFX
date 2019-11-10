// property.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/i_object.hpp>
#include <neogfx/core/i_property.hpp>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702 ) // unreachable code
#endif

namespace neogfx
{
    namespace detail
    {
        template <typename T>
        struct property_optional_type_cracker 
        { 
            typedef T type;
            static constexpr bool optional = false;
        };
        template <typename T>
        struct property_optional_type_cracker<std::optional<T>>
        {
            typedef T type;
            static constexpr bool optional = true;
        };
    }

    template <typename T, typename Category>
    class property : public i_property, public neolib::lifetime
    {
    public:
        typedef T value_type;
    public:
        define_declared_event(VariantChanged, changed, const property_variant&)
        define_declared_event(VariantChangedFromTo, changed_from_to, const property_variant&, const property_variant&)
        define_event_2(Changed, changed, const value_type&)
        define_event_2(ChangedFromTo, changed_from_to, const value_type&, const value_type&)
    public:
        struct invalid_type : std::logic_error { invalid_type() : std::logic_error("neogfx::property::invalid_type") {} };
    public:
        typedef property<T, Category> self_type;
        typedef Category category_type;
    private:
        typedef detail::property_optional_type_cracker<T> cracker;
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
            template <typename SFINAE = cracker::type>
            operator const typename std::enable_if<cracker::optional, SFINAE>::type&() const
            {
                return *iParent.value();
            }
            template <typename T2, typename SFINAE = optional_proxy<parent_type>>
            typename std::enable_if<!std::is_const<parent_type>::value, SFINAE>::type& operator=(const T2& aValue)
            {
                iParent.assign(aValue);
                return *this;
            }
            template <typename SFINAE = const cracker::type*>
            const typename std::enable_if<cracker::optional, SFINAE>::type operator->() const
            {
                return &*iParent.value();
            }
        private:
            parent_type& iParent;
        };
    public:
        property(i_object& aOwner, const std::string& aName) : iOwner{ aOwner }, iName { aName }, iValue{}
        {
            aOwner.properties().register_property(*this);
        }
        property(i_object& aOwner, const std::string& aName, const T& aValue) : iOwner{ aOwner }, iName{ aName }, iValue { aValue }
        {
            aOwner.properties().register_property(*this);
        }
    public:
        neolib::i_lifetime& as_lifetime() override
        {
            return *this;
        }
    public:
        const std::string& name() const override
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
            return cracker::optional;
        }
        property_variant get() const override
        {
            return iValue;
        }
        void set(const property_variant& aValue) override
        {
            std::visit([this](auto&& arg)
            {
                *this = std::forward<decltype(arg)>(arg);
            }, aValue.for_visitor());
        }
        bool has_delegate() const override
        {
            return iDelegate != nullptr;
        }
        i_property_delegate& delegate() const override
        {
            if (has_delegate())
                return *iDelegate;
            throw no_delegate();
        }
        void set_delegate(i_property_delegate& aDelegate) override
        {
            iDelegate = &aDelegate;
        }
        void unset_delegate() override
        {
            iDelegate = nullptr;
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
            }, delegate().get(*this).for_visitor());
            return *dptr;
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
        const typename std::enable_if<cracker::optional, SFINAE>::type operator*() const
        {
            return optional_proxy<const self_type>{ *this };
        }
        template <typename SFINAE = optional_proxy<self_type>>
        typename std::enable_if<cracker::optional, SFINAE>::type operator*()
        {
            return optional_proxy<self_type>{ *this };
        }
        template <typename SFINAE = optional_proxy<const self_type>>
        const typename std::enable_if<cracker::optional, SFINAE>::type operator->() const
        {
            return optional_proxy<const self_type>{ *this };
        }
        template <typename T>
        bool operator==(const T& aRhs) const
        {
            return iValue == aRhs;
        }
        template <typename T>
        bool operator!=(const T& aRhs) const
        {
            return iValue != aRhs;
        }
        template <typename T>
        bool operator==(const std::optional<T>& aRhs) const
        {
            return iValue == aRhs;
        }
        template <typename T>
        bool operator!=(const std::optional<T>& aRhs) const
        {
            return iValue != aRhs;
        }
    private:
        template <typename T2>
        self_type& do_assign(T2&& aValue, bool aOwnerNotify = true)
        {
            if (iValue != aValue)
            {
                auto previousValue = iValue;
                iValue = std::forward<T2>(aValue);
                neolib::destroyed_flag destroyed{ *this };
                if (aOwnerNotify)
                    iOwner.property_changed(*this);
                if (destroyed)
                    return *this;
                VariantChanged.trigger(get());
                if (destroyed)
                    return *this;
                VariantChangedFromTo.trigger(property_variant{ previousValue }, get());
                if (destroyed)
                    return *this;
                Changed.trigger(value());
                if (destroyed)
                    return *this;
                ChangedFromTo.trigger(previousValue, value());
                if (destroyed)
                    return *this;
                if (has_delegate())
                    delegate().set(*this);
            }
            return *this;
        }
    private:
        i_object& iOwner;
        std::string iName;
        mutable value_type iValue;
        i_property_delegate* iDelegate = nullptr;
    };

    #define define_property( category, type, name, ... ) neogfx::property<type, category> name = { *this, #name ##s, __VA_ARGS__ };
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif
