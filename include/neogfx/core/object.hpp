// object.hpp
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
#include <neolib/lifetime.hpp>

#include <neogfx/core/event.hpp>
#include <neogfx/core/i_object.hpp>
#include <neogfx/core/i_properties.hpp>
#include <neogfx/core/i_property.hpp>

namespace neogfx
{
    template <typename Base>
    class object : public Base, public i_properties, protected virtual neolib::lifetime
    {
    public:
        define_declared_event(Destroying, destroying);
        define_declared_event(Destroyed, destroyed);
    public:
        ~object()
        {
            set_destroyed();
        }
        // i_lifetime
    public:
        void set_destroying() override
        {
            if (is_alive())
            {
                Destroying.trigger();
                neolib::lifetime::set_destroying();
            }
        }
        void set_destroyed() override
        {
            if (!is_destroyed())
            {
                Destroyed.trigger();
                neolib::lifetime::set_destroyed();
            }
        }
    public:
        // i_object
    public:
        neolib::i_lifetime& as_lifetime() override
        {
            return *this;
        }
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
        neogfx::property_map iProperties;
    };
}
