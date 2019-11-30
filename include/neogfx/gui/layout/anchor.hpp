// anchor.hpp
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
#include <neolib/optional.hpp>
#include <neogfx/core/property.hpp>
#include <neogfx/gui/layout/i_anchorable_object.hpp>
#include <neogfx/gui/layout/i_anchor.hpp>

namespace neogfx
{
    template <typename Property>
    class anchor : public i_calculating_anchor<neolib::optional_t<typename Property::value_type>, typename Property::calculator_function_type>
    {
        typedef i_calculating_anchor<neolib::optional_t<typename Property::value_type>, typename Property::calculator_function_type> base_type;
    public:
        typedef Property property_type;
        using typename base_type::abstract_type;
        using typename base_type::value_type;
        using typename base_type::constraint;
    private:
        typedef typename property_type::calculator_function_type calculator_function_type;
        typedef std::pair<constraint, std::shared_ptr<i_anchor>> constraint_entry_t;
        typedef std::vector<constraint_entry_t> constraint_entries_t;
    public:
        anchor(i_anchorable_object& aOwner, i_property& aProperty) :
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
        const value_type& value() const override
        {
            auto const& v = property().get<value_type>();
            if constexpr (!neolib::is_optional_v<value_type>)
                return v;
            else if (v != std::nullopt)
                return v;
            else
                throw anchor_property_has_no_value();
        }
        value_type& value() override
        {
            return property().get<value_type>();
        }
        void add_constraint(const constraint& aConstraint, abstract_type& aOtherAnchor) override
        {
            add_constraint(aConstraint, std::shared_ptr<abstract_type>{ std::shared_ptr<abstract_type>{}, &aOtherAnchor });
        }
        void add_constraint(const constraint& aConstraint, std::shared_ptr<abstract_type> aOtherAnchor) override
        {
            iConstraints.push_back(constraint_entry_t{ aConstraint, aOtherAnchor });
        }
        neolib::optional_t<value_type> evaluate_constraints() const override
        {
            auto result = value();
            for (auto const& c : iConstraints)
                result = c.first(result, static_cast<abstract_type&>(*c.second).value());
            return result;
        }
        neolib::optional_t<value_type> evaluate_constraints(const neolib::i_callable<calculator_function_type>& aCallable) const override
        {
            auto result = property().calculate(aCallable);
            for (auto const& c : iConstraints)
                result = c.first(result, static_cast<abstract_type&>(*c.second).property().calculate(aCallable));
            return result;
        }
    private:
        i_anchorable_object& iOwner;
        i_property& iProperty;
        constraint_entries_t iConstraints;
    };

    #define define_anchor( name ) neogfx::anchor<decltype(name)> Anchor_##name = { *this, name, #name ##s };
}
