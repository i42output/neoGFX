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
#include <neogfx/core/transition_animator.hpp>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702 ) // unreachable code
#endif

namespace neogfx
{
    template <typename T, typename Category, class Context, typename Calculator>
    class property;

    template <typename T, typename Category, class Context, typename Calculator>
    class property_transition : public transition
    {
    public:
        typedef property<T, Category, Context, Calculator> property_type;
        typedef T value_type;
    public:
        property_transition(i_animator& aAnimator, property_type& aProperty, easing aEasingFunction, double aDuration, bool aEnabled = true) :
            transition{ aAnimator, aEasingFunction, aDuration, aEnabled },
            iProperty{ aProperty }
        {
        }
    public:
        void start_if()
        {
            if (iProperty.iPreviousValue != std::nullopt)
            {
                bool alreadyActive = active();
                sync();
                reset(true, disable_when_finished(), !alreadyActive);
                neolib::scoped_flag sf{ iUpdatingProperty };
                iProperty = *iFrom;
            }
        }
        bool started() const
        {
            return iFrom != std::nullopt;
        }
        value_type const& from() const
        {
            return iFrom.value();
        }
        value_type const& to() const
        {
            return iTo.value();
        }
        value_type const& mix() const
        {
            return iMix.value();
        }
        bool updating_property() const
        {
            return iUpdatingProperty;
        }
    public:
        bool property_transition::can_apply() const final
        {
            return !finished() && enabled() && !paused();
        }
        void apply() final
        {
            if (!can_apply())
                throw cannot_apply();
            if (!animation_finished())
            {
                auto const mixValue = mix_value();
                iMix = neogfx::mix(iFrom, iTo, mixValue);
                neolib::scoped_flag sf{ iUpdatingProperty };
                iProperty = mix();
            }
            else
            {
                iMix = (easing_function() != easing::Zero ? *iTo : *iFrom);
                neolib::scoped_flag sf{ iUpdatingProperty };
                iProperty = mix();
                clear();
                if (disable_when_finished())
                    disable();
            }
        }
        bool finished() const final
        {
            return iFrom == std::nullopt;
        }
    public:
        void clear() final
        {
            iFrom = std::nullopt;
            iTo = std::nullopt;
            iMix = std::nullopt;
        }
        void sync(bool aIgnorePrevious = false) final
        {
            iFrom = aIgnorePrevious ? iProperty.iValue : iProperty.iPreviousValue;
            iTo = iProperty.iValue;
        }
    private:
        property_type& iProperty;
        std::optional<value_type> iFrom;
        std::optional<value_type> iTo;
        std::optional<value_type> iMix;
        bool iUpdatingProperty = false;
    };

    template <typename T, typename Category, class Context, typename Calculator = T(*)()>
    class property : public i_property, public neolib::lifetime<>
    {
        typedef property<T, Category, Context, Calculator> self_type;
        template <typename, typename, class, typename>
        friend class property_transition;
    public:
        typedef T value_type;
        typedef Context context_type;
        typedef property_transition<T, Category, Context, Calculator> transition_type;
    public:
        define_declared_event(PropertyChanged, property_changed, const property_variant&)
        define_declared_event(PropertyChangedFromTo, property_changed_from_to, const property_variant&, const property_variant&)
        define_event(Changed, changed, value_type const&)
        define_event(ChangedFromTo, changed_from_to, value_type const&, value_type const&)
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
        property(i_property_owner& aOwner, std::string const& aName) : iOwner{ aOwner }, iName{ aName }, iValue{}
        {
            aOwner.properties().register_property(*this);
        }
        property(i_property_owner& aOwner, std::string const& aName, calculator_function_type aCalculator) : iOwner{ aOwner }, iName{ aName }, iCalculator{ aCalculator }, iValue{}
        {
            aOwner.properties().register_property(*this);
        }
        property(i_property_owner& aOwner, std::string const& aName, const T& aValue) : iOwner{ aOwner }, iName{ aName }, iValue { aValue }
        {
            aOwner.properties().register_property(*this);
        }
        property(i_property_owner& aOwner, std::string const& aName, calculator_function_type aCalculator, const T& aValue) : iOwner{ aOwner }, iName{ aName }, iCalculator{ aCalculator }, iValue{ aValue }
        {
            aOwner.properties().register_property(*this);
        }
    public:
        property_variant get(const i_property& aProperty) const final
        {
            return get_as_variant();
        }
    public:
        i_property_owner& owner() const final
        {
            return iOwner;
        }
    public:
        const string& name() const final
        {
            return iName;
        }
        const std::type_info& type() const final
        {
            return typeid(value_type);
        }
        const std::type_info& category() const final
        {
            return typeid(category_type);
        }
        bool optional() const final
        {
            return neolib::is_optional_v<T>;
        }
        property_variant get_as_variant() const final
        {
            if constexpr (neolib::is_optional_v<T>)
            {
                if (value() != std::nullopt)
                    return *value();
                else
                    return neolib::none;
            }
            else
                return value();
        }
        void set_from_variant(const property_variant& aValue) final
        {
            std::visit([this](auto&& arg)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>)
                    *this = value_type{};
                else
                    *this = std::forward<decltype(arg)>(arg);
            }, aValue);
        }
        bool read_only() const final
        {
            return iReadOnly;
        }
        void set_read_only(bool aReadOnly) final
        {
            iReadOnly = aReadOnly;
        }
        bool transition_set() const final
        {
            return iTransition != nullptr;
        }
        transition_type& transition() const final
        {
            if (iTransition != nullptr)
                return *iTransition;
            throw std::logic_error( "neogfx::property: no transition!" );
        }
        void set_transition(i_animator& aAnimator, easing aEasingFunction, double aDuration, bool aEnabled = true) final
        {
            iTransition = std::make_unique<transition_type>(aAnimator, *this, aEasingFunction, aDuration, aEnabled);
        }
        void clear_transition() final
        {
            iTransition = nullptr;
        }
        bool transition_suppressed() const final
        {
            return iTransitionSuppressed;
        }
        void suppress_transition(bool aSuppress) final
        {
            iTransitionSuppressed = aSuppress;
            if (aSuppress)
            {
                if (transition_set() && transition().started())
                    assign(effective_value(), true, true);
            }
        }
        bool has_delegate() const final
        {
            return iDelegate != nullptr;
        }   
        i_property_delegate const& delegate() const final
        {
            if (has_delegate())
                return *iDelegate;
            throw no_delegate();
        }
        i_property_delegate& delegate() final
        {
            return const_cast<i_property_delegate&>(to_const(*this).delegate());
        }
        void set_delegate(i_property_delegate& aDelegate) final
        {
            iDelegate = &aDelegate;
        }
        void unset_delegate() final
        {
            iDelegate = nullptr;
        }
    public:
        value_type const& value() const
        {
            if (has_delegate())
            {
                std::visit([this](auto&& arg)
                {
                    typedef std::decay_t<decltype(arg)> try_type;
                    if constexpr (std::is_same_v<try_type, value_type>)
                        iValue = arg;
                    else if constexpr (std::is_same_v<try_type, custom_type>)
                    {
                        if constexpr (!neolib::is_optional_v<value_type>)
                            iValue = neolib::any_cast<value_type const&>(arg);
                        else
                            iValue = neolib::any_cast<const neolib::optional_t<value_type>&>(arg);
                    }
                    else if constexpr (std::is_same_v<try_type, neolib::none_t> || std::is_same_v<try_type, std::monostate>)
                    {
                        if constexpr (!neolib::is_optional_v<value_type>)
                            iValue = {};
                        else
                            iValue = std::nullopt;
                    }
                    else
                    {
                        // [[unreachable]]
                        throw invalid_type();
                    }
                }, delegate().get(*this));
            }
            return iValue;
        }
        value_type const& effective_value() const
        {
            if (!transition_set() || !transition().started())
                return value();
            else
                return transition().to();
        }
        template <typename T2>
        self_type& assign(T2&& aValue, bool aOwnerNotify = true, bool aDiscardPreviousValue = false)
        {
            typedef std::decay_t<decltype(aValue)> try_type;
            if constexpr (std::is_same_v<try_type, value_type> || std::is_same_v<neolib::optional<try_type>, value_type>)
                return do_assign(std::forward<T2>(aValue), aOwnerNotify, aDiscardPreviousValue);
            else if constexpr (std::is_same_v<try_type, custom_type>)
                return do_assign(neolib::any_cast<value_type>(std::forward<T2>(aValue)), aOwnerNotify, aDiscardPreviousValue);
            else if constexpr (std::is_same_v<try_type, neolib::none_t> || std::is_same_v<try_type, std::monostate>)
                return do_assign(value_type{}, aOwnerNotify, aDiscardPreviousValue);
            else if constexpr (std::is_arithmetic_v<value_type> && std::is_convertible_v<try_type, value_type> && std::is_integral_v<try_type> == std::is_integral_v<value_type>)
                return do_assign(static_cast<value_type>(std::forward<T2>(aValue)), aOwnerNotify, aDiscardPreviousValue);
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
        operator value_type const&() const
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
        template <typename SFINAE = optional_proxy<self_type>>
        typename std::enable_if<neolib::is_optional_v<T>, SFINAE>::type operator->()
        {
            return optional_proxy<self_type>{ *this };
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
        bool operator==(const neolib::optional<T>& aRhs) const
        {
            return value() == aRhs;
        }
        template <typename T>
        bool operator!=(const neolib::optional<T>& aRhs) const
        {
            return value() != aRhs;
        }
    protected:
        const void* data() const final
        {
            if (!has_delegate())
                return &value();
            else
                return delegate().data();
        }
        void* data() final
        {
            if (!has_delegate())
                return &mutable_value();
            else
                return delegate().data();
        }
        void*const* calculator_function() const final
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
        self_type& do_assign(T2&& aValue, bool aOwnerNotify = true, bool aDiscardPreviousValue = false)
        {
            if (read_only())
                return *this;

            if (transition_set() && !transition_suppressed() &&
                !transition().updating_property() &&
                transition().started() && aValue == effective_value())
                return *this;

            if (mutable_value() != aValue)
            {
                if (!transition_set())
                {
                    iPreviousValue = !aDiscardPreviousValue ? value() : aValue;
                    mutable_value() = aValue;
                }
                else if (transition().updating_property())
                {
                    mutable_value() = aValue;
                }
                else 
                {
                    iPreviousValue = !aDiscardPreviousValue ? effective_value() : aValue;
                    mutable_value() = aValue;
                    if (!transition_suppressed())
                        transition().start_if();
                    else
                        transition().clear();
                }
                update(aOwnerNotify);
            }

            return *this;
        }
        void update(bool aOwnerNotify = true)
        {
            destroyed_flag destroyed{ *this };

            if (aOwnerNotify)
                iOwner.property_changed(*this);
            if (destroyed)
                return;

            bool const discardChanged = !PropertyChanged.trigger(get_as_variant());
            if (destroyed)
                return;

            bool discardChangedFromTo = false;
            if constexpr (!neolib::is_optional_v<T>)
                discardChangedFromTo = !PropertyChangedFromTo.trigger(property_variant{ *iPreviousValue }, get_as_variant());
            else
                discardChangedFromTo = !PropertyChangedFromTo.trigger(*iPreviousValue != std::nullopt ? property_variant{ **iPreviousValue } : property_variant{ neolib::none }, get_as_variant());
            if (destroyed)
                return;

            if (!discardChanged && !Changed.trigger(value()))
                return;
            if (destroyed)
                return;

            if (!discardChangedFromTo && !ChangedFromTo.trigger(*iPreviousValue, value()))
                return;
            if (destroyed)
                return;
        }
    private:
        i_property_owner& iOwner;
        string iName;
        calculator_function_type iCalculator;
        mutable value_type iValue;
        std::optional<value_type> iPreviousValue;
        bool iReadOnly = false;
        std::unique_ptr<transition_type> iTransition;
        bool iTransitionSuppressed = false;
        i_property_delegate* iDelegate = nullptr;
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
