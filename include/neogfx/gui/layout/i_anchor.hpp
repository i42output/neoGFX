// i_anchor.hpp
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
#include <functional>
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    template <typename T>
    class anchor_constraint : public std::function<T(const T&, const T&)>
    {
    public:
        typedef T value_type;
    public:
        using function::function;
    public:
        static anchor_constraint<T> equal_to;
        static anchor_constraint<T> min;
        static anchor_constraint<T> max;
    };

    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::equal_to = [](const T&, const T& rhs) -> T { return rhs; };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::min = [](const T& lhs, const T& rhs) -> T { return std::min(lhs, rhs); };
    template <typename T>
    anchor_constraint<T> anchor_constraint<T>::max = [](const T& lhs, const T& rhs) -> T { return std::max(lhs, rhs); };

    template <typename T, typename... GetterArgs>
    class i_anchor;

    class i_anchor_base
    {
    public:
        virtual ~i_anchor_base() {}
    public:
        virtual const std::string& name() const = 0;
        // helpers
    public:
        template <typename T, typename... GetterArgs>
        const i_anchor<T, GetterArgs...>& as() const;
        template <typename T, typename... GetterArgs>
        i_anchor<T, GetterArgs...>& as();
    };

    template <typename T, typename... GetterArgs>
    class i_anchor : public i_anchor_base
    {
    public:
        typedef i_anchor<T, GetterArgs...> self_type;
        typedef T value_type;
        typedef anchor_constraint<value_type> constraint;
    public:
        virtual value_type value(GetterArgs&&... aArguments) const = 0;
        virtual void add_constraint(constraint aConstraint, self_type& aOtherAnchor) = 0;
        virtual void add_constraint(constraint aConstraint, std::shared_ptr<self_type> aOtherAnchor) = 0;
        virtual value_type evaluate_constraints(GetterArgs&&... aArguments) const = 0;
    };

    template <typename T, typename... GetterArgs>
    inline const i_anchor<T, GetterArgs...>& i_anchor_base::as() const
    {
        if constexpr (ndebug)
            return static_cast<const i_anchor<T, GetterArgs...>&>(*this);
        else
            return dynamic_cast<const i_anchor<T, GetterArgs...>&>(*this);
    }

    template <typename T, typename... GetterArgs>
    inline i_anchor<T, GetterArgs...>& i_anchor_base::as()
    {
        if constexpr (ndebug)
            return static_cast<i_anchor<T, GetterArgs...>&>(*this);
        else
            return dynamic_cast<i_anchor<T, GetterArgs...>&>(*this);
    };
}
