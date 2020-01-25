// i_object.hpp
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

#include <neolib/i_lifetime.hpp>

#include <neogfx/core/i_event.hpp>
#include <neogfx/core/i_properties.hpp>

namespace neogfx
{
    enum class object_type : uint64_t;

    class i_object
    {
    public:
        declare_event(destroying);
        declare_event(destroyed);
    public:
        virtual ~i_object() {}
    public:
        virtual neogfx::object_type object_type() const = 0;
    public:
        virtual neolib::i_lifetime& as_lifetime() = 0;
    public:
        virtual void property_changed(i_property& aProperty) = 0;
    public:
        virtual const i_properties& properties() const = 0;
        virtual i_properties& properties() = 0;
    };
}
