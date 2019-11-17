// i_property.hpp
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
#include <type_traits>
#include <neolib/any.hpp>
#include <neolib/simple_variant.hpp>
#include <neolib/variant.hpp>
#include <neolib/i_enum.hpp>
#include <neolib/i_lifetime.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>

namespace neogfx
{
    typedef neolib::any custom_type;

    typedef neolib::variant<
        void*,
        bool,
        char,
        int32_t,
        uint32_t,
        int64_t,
        uint64_t,
        float,
        double,
        std::string,
        size,
        point,
        rect,
        custom_type> property_variant_t;

    class property_variant : public property_variant_t
    {
    public:
        property_variant() : 
            property_variant_t{}
        {
        }
        property_variant(const property_variant& other) :
            property_variant_t{ static_cast<const property_variant_t&>(other) }
        {
        }
        property_variant(property_variant&& other) :
            property_variant_t{ static_cast<property_variant_t&&>(std::move(other)) }
        {
        }
        template <typename T>
        property_variant(T&& aValue) :
            property_variant_t{ std::forward<T>(aValue) }
        {
        }
    public:
        using property_variant_t::operator=;
        property_variant& operator=(const property_variant& other)
        {
            property_variant_t::operator=(static_cast<const property_variant_t&>(other));
            return *this;
        }
        property_variant& operator=(property_variant&& other)
        {
            property_variant_t::operator=(static_cast<property_variant_t&&>(std::move(other)));
            return *this;
        }
    public:
        using property_variant_t::operator==;
        using property_variant_t::operator!=;
    };

    template <typename T> struct variant_type_for { typedef custom_type type; };
    template <typename T> struct variant_type_for<T*> { typedef void* type; };

    class i_property;

    class i_property_delegate
    {
    public:
        virtual const property_variant& get(const i_property& aProperty) const = 0;
        virtual void set(const i_property& aProperty) const = 0;
    };

    class i_property
    {
    public:
        declare_event(changed, const property_variant&)
        declare_event(changed_from_to, const property_variant&, const property_variant&)
    public:
        struct no_delegate : std::logic_error { no_delegate() : std::logic_error("neogfx::i_property::no_delegate") {} };
    public:
        virtual ~i_property() {}
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

