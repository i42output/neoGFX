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
#include <neogfx/gui/layout/i_anchorable_object.hpp>
#include <neogfx/gui/layout/i_anchor.hpp>

namespace neogfx
{
    template <typename T, typename... GetterArgs>
    class anchor : public i_anchor<T, GetterArgs...>
    {
        typedef i_anchor<T, GetterArgs...> base_type;
    public:
        using typename base_type::abstract_type;
        using typename base_type::value_type;
        using typename base_type::constraint;
    private:
        typedef std::function<value_type(GetterArgs...)> value_getter_t;
        typedef std::pair<constraint, std::shared_ptr<i_anchor_base>> constraint_entry_t;
        typedef std::vector<constraint_entry_t> constraint_entries_t;
    public:
        anchor(i_anchorable_object& aOwner, const std::string& aName, value_getter_t aValueGetter) :
            iOwner{ aOwner }, iName { aName }, iValueGetter{ aValueGetter }
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
        const neolib::string& name() const override
        {
            return iName;
        }
    public:
        void constrain(i_anchor_base& aRhs, anchor_constraint_function aLhsFunction, anchor_constraint_function aRhsFunction) override
        {
            constrain(aRhs, aRhsFunction);
            aRhs.constrain(*this, aLhsFunction);
        }
        void constrain(i_anchor_base& aOther, anchor_constraint_function aOtherFunction) override
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
        value_type value(GetterArgs... aArguments) const override
        {
            return iValueGetter(std::forward<GetterArgs>(aArguments)...);
        }
        void add_constraint(const constraint& aConstraint, abstract_type& aOtherAnchor) override
        {
            add_constraint(aConstraint, std::shared_ptr<base_type>{ std::shared_ptr<abstract_type>{}, &aOtherAnchor });
        }
        void add_constraint(const constraint& aConstraint, std::shared_ptr<abstract_type> aOtherAnchor) override
        {
            iConstraints.push_back(constraint_entry_t{ aConstraint, aOtherAnchor });
        }
        value_type evaluate_constraints(GetterArgs... aArguments) const override
        {
            auto result = value(std::forward<GetterArgs>(aArguments)...);
            for (auto const& c : iConstraints)
                result = c.first(result, static_cast<base_type&>(*c.second).value(std::forward<GetterArgs>(aArguments)...));
            return result;
        }
    private:
        i_anchorable_object& iOwner;
        neolib::string iName;
        value_getter_t iValueGetter;
        constraint_entries_t iConstraints;
    };

    #define define_anchor( name, getter, ... ) neogfx::anchor<__VA_ARGS__> Anchor_##name = { *this, #name ##s, getter };
}
