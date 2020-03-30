// i_plugin_property.hpp
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

namespace neogfx
{
    namespace plugin
    {
        enum property_type : uint32_t
        {
            Pointer,
            Boolean,
            Char,
            Int32,
            Uint32,
            Int64,
            Uint64,
            Float,
            Double,
            String,
            Size,
            Point,
            Rect,
            Custom
        };

        class i_property
        {
        public:
            declare_event(changed, const property_variant&)
            declare_event(changed_from_to, const property_variant&, const property_variant&)
        public:
            virtual ~i_property() = default;
        public:
            virtual neolib::i_lifetime& as_lifetime() = 0;
        public:
            virtual const std::string& name() const = 0;
            virtual const std::type_info& type() const = 0;
            virtual const std::type_info& category() const = 0;
            virtual bool optional() const = 0;
            virtual property_variant get() const = 0;
            virtual void set(const property_variant& aValue) = 0;
            virtual bool has_delegate() const = 0;
            virtual i_property_delegate& delegate() const = 0;
            virtual void set_delegate(i_property_delegate& aDelegate) = 0;
            virtual void unset_delegate() = 0;
        };
    }
}

