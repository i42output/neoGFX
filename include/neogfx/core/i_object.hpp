// i_object.hpp
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

#include <neolib/i_lifetime.hpp>

#include <neogfx/core/i_event.hpp>
#include <neogfx/core/i_property.hpp>

namespace neogfx
{
    enum class object_type : uint64_t;

    class i_object : public i_property_owner
    {
    public:
        declare_event(destroying);
        declare_event(destroyed);
    public:
        virtual ~i_object() = default;
    public:
        virtual neogfx::object_type object_type() const = 0;
    };

    template <typename Object>
    inline bool is_alive(Object& aObject)
    {
        if constexpr (std::is_base_of_v<neolib::i_lifetime, Object>)
            return static_cast<neolib::i_lifetime&>(aObject).is_alive();
        else
            return dynamic_cast<neolib::i_lifetime&>(aObject).is_alive();
    }

    template <typename Object, typename Handler>
    inline auto destroying(Object& aObject, const Handler aHandler)
    {
        if constexpr (std::is_base_of_v<i_object, Object>)
            return static_cast<i_object&>(aObject).destroying(aHandler);
        else
            return dynamic_cast<i_object&>(aObject).destroying(aHandler);
    }

    template <typename Object, typename Handler>
    inline auto destroyed(Object& aObject, const Handler aHandler)
    {
        if constexpr (std::is_base_of_v<i_object, Object>)
            return static_cast<i_object&>(aObject).destroyed(aHandler);
        else
            return dynamic_cast<i_object&>(aObject).destroyed(aHandler);
    }
}
