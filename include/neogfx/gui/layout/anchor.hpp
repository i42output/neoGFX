// anchor.hpp
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
#include <neolib/optional.hpp>
#include <neogfx/core/property.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gui/layout/i_anchorable.hpp>
#include <neogfx/gui/layout/i_anchor.hpp>

namespace neogfx
{
    template <typename T, typename PVT, class Context, class C, typename... CalculatorArgs>
    class anchor : public i_calculating_anchor<T, PVT, CalculatorArgs...>
    {
        typedef i_calculating_anchor<T, PVT, CalculatorArgs...> base_type;
    public:
        using typename base_type::abstract_type;
        using typename base_type::value_type;
        using typename base_type::property_value_type;
        using typename base_type::constraint;
        typedef Context context_type;
    private:
        typedef T(C::* calculator_function_type)(CalculatorArgs...) const;
        typedef std::pair<constraint, std::shared_ptr<i_anchor>> constraint_entry_t;
        typedef std::vector<constraint_entry_t> constraint_entries_t;
    public:
        anchor(i_anchorable& aOwner, i_property& aProperty) :
            iOwner{ aOwner }, iProperty{ aProperty }
        {
            iOwner.anchors()[name()] = this;
        }
        ~anchor()
        {
            auto iter = iOwner.anchors().find(name());
            if (iter != iOwner.anchors().end())
                iOwner.anchors().erase(iter);
        }
    public:
        const i_string& name() const override
        {
            return property().name();
        }
        const i_property& property() const override
        {
            return iProperty;
        }
        i_property& property() override
        {
            return iProperty;
        }
    public:
        void constrain(i_anchor& aRhs, anchor_constraint_function aLhsFunction, anchor_constraint_function aRhsFunction) override
        {
            constrain(aRhs, aRhsFunction);
            aRhs.constrain(*this, aLhsFunction);
        }
        void constrain(i_anchor& aOther, anchor_constraint_function aOtherFunction) override
        {
            switch (aOtherFunction)
            {
            case anchor_constraint_function::Identity:
                add_constraint(constraint::identity, static_cast<abstract_type&>(aOther));
                break;
            case anchor_constraint_function::Equal:
                add_constraint(constraint::equal, static_cast<abstract_type&>(aOther));
                break;
            case anchor_constraint_function::Min:
                add_constraint(constraint::min, static_cast<abstract_type&>(aOther));
                break;
            case anchor_constraint_function::Max:
                add_constraint(constraint::max, static_cast<abstract_type&>(aOther));
                break;
            case anchor_constraint_function::Custom:
                // todo
                break;
            default:
                break;
            }
        }
    public:
        bool property_set() const
        {
            if constexpr (!neolib::is_optional_v<property_value_type>)
                return true;
            else
                return property().get<property_value_type>() != std::nullopt;
        }
        const value_type& property_value() const override
        {
            if constexpr (!neolib::is_optional_v<property_value_type>)
                return property().get<property_value_type>();
            else if (property_set())
                return *property().get<property_value_type>();
            else
                throw anchor_property_has_no_value();
        }
        value_type& property_value() override
        {
            return const_cast<value_type&>(to_const(*this).property_value());
        }
        void add_constraint(const constraint& aConstraint, abstract_type& aOtherAnchor) override
        {
            add_constraint(aConstraint, std::shared_ptr<abstract_type>{ std::shared_ptr<abstract_type>{}, &aOtherAnchor });
        }
        void add_constraint(const constraint& aConstraint, std::shared_ptr<abstract_type> aOtherAnchor) override
        {
            iConstraints.push_back(constraint_entry_t{ aConstraint, aOtherAnchor });
        }
        value_type evaluate_constraints(const CalculatorArgs&... aArgs) const override
        {
            auto result = (property_set() ? property_value() : property().calculate<context_type, calculator_function_type>(aArgs...));
            for (auto const& c : iConstraints)
            {
                auto const& otherAnchor = static_cast<abstract_type&>(*c.second);
                result = c.first(result, (otherAnchor.property_set() ? otherAnchor.property_value() : otherAnchor.property().calculate<context_type, calculator_function_type>(aArgs...)));
            }
            return result;
        }
    private:
        i_anchorable& iOwner;
        i_property& iProperty;
        constraint_entries_t iConstraints;
    };

    namespace detail
    {
        template <template<typename, typename, typename, typename...> class Anchor, class Ctx, typename PVT, typename Callable>
        struct anchor_callable_function_cracker;
        template <template<typename, typename, typename, typename...> class Anchor, class Ctx, typename PVT, typename R, typename C, typename... Args>
        struct anchor_callable_function_cracker<Anchor, Ctx, PVT, R(C::*)(Args...) const>
        {
            typedef Anchor<R, PVT, std::add_const_t<Ctx>, C, Args...> type;
            typedef R(C::* callable_type)(Args...) const;
            typedef PVT property_value_type;
            typedef R value_type;
            typedef C class_type;
        };
        template <template<typename, typename, typename, typename...> class Anchor, class Ctx, typename PVT, typename R, typename C, typename... Args>
        struct anchor_callable_function_cracker<Anchor, Ctx, PVT, R(C::*)(Args...)>
        {
            typedef Anchor<R, PVT, Ctx, C, Args...> type;
            typedef R(C::* callable_type)(Args...);
            typedef PVT property_value_type;
            typedef R value_type;
            typedef C class_type;
        };
    }

    template <typename Context, typename PVT, typename Callable>
    using anchor_t = typename detail::anchor_callable_function_cracker<anchor, Context, PVT, Callable>::type;

    #define define_anchor( name ) neogfx::anchor_t<property_context_type, typename decltype(name)::value_type, typename decltype(name)::calculator_function_type> Anchor_##name = { *this, name };
}
