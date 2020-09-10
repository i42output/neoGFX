// object.hpp
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

#include <neolib/neolib.hpp>
#include <neolib/core/lifetime.hpp>
#include <neolib/core/map.hpp>
#include <neolib/app/object.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/i_properties.hpp>
#include <neogfx/core/i_property.hpp>

namespace neogfx
{
    template <typename Base = i_object>
    class object : public neolib::object<Base>, public i_properties
    {
        typedef neolib::object<Base> base_type;
    public:
        using base_type::base_type;
    public:
        // meta
        i_object& as_object() override
        {
            return *this;
        }
        // type
    public:
        neogfx::object_type object_type() const override
        {
            return neogfx::object_type{};
        }
        // i_property_owner
    public:
        void property_changed(i_property&) override
        {
            // default is to do nothing
        }
    public:
        const i_properties& properties() const override
        {
            return *this;
        }
        i_properties& properties() override
        {
            return *this;
        }
        // i_properties
    public:
        void register_property(i_property& aProperty) override
        {
            iProperties.emplace(aProperty.name(), &aProperty);
        }
        const neogfx::property_map& property_map() const override
        {
            return iProperties;
        }
        // state
    private:
        neolib::map<neolib::string, i_property*> iProperties;
    };
}
