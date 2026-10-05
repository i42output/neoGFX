// i_property_component_registry.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2026 Leigh Johnston
  
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

#include <typeinfo>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#include <optional>

#include <neolib/core/reference_counted.hpp>
#include <neolib/core/i_vector.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/string.hpp>
#include <neogfx/core/i_property.hpp>

namespace neogfx::DesignStudio
{
    // std::type_info objects can't be compared directly across module (application/plugin) boundaries so types are identified by name
    inline bool same_type(std::type_info const& aLhs, std::type_info const& aRhs)
    {
        return &aLhs == &aRhs || std::strcmp(aLhs.name(), aRhs.name()) == 0;
    }

    // a component of a composite property type's value (e.g. a size's "Width" is its cx): shown beneath the property in the 
    // Properties toolbox and edited individually (the property's own row is read only)
    class i_property_component : public i_reference_counted
    {
    public:
        typedef i_property_component abstract_type;
    public:
        virtual i_string const& name() const = 0;
        /// The component's value as text (empty if the property is unset).
        virtual void get(i_property const& aProperty, i_string& aText) const = 0;
        /// The property's new value (to pass to i_property::set_from_variant) with the component changed; false if aText can't be parsed.
        virtual bool set(i_property const& aProperty, i_string const& aText, property_variant& aNewValue) const = 0;
        /// Is the component chosen from a drop-down list (of choices())?
        virtual bool has_choices() const = 0;
        virtual void choices(i_property const& aProperty, neolib::i_vector<i_string>& aChoices) const = 0;
    };

    // the components of composite property types: Design Studio's own (e.g. size, point, padding, font, size_policy) and those 
    // registered by plugins (discover it from the application, e.g. ref_ptr<i_property_component_registry> registry{ aApplication }); 
    // property types are identified by name (std::type_info::name() of the type i_property::type() returns) as std::type_info objects 
    // can't be compared across module boundaries
    class i_property_component_registry : public i_reference_counted
    {
    public:
        typedef i_property_component_registry abstract_type;
    public:
        /// Appends a component to those of properties of the named type (std::type_info::name() of the type i_property::type() returns).
        virtual void register_component(i_string const& aPropertyTypeName, i_property_component& aComponent) = 0;
        virtual std::uint32_t component_count(i_string const& aPropertyTypeName) const = 0;
        virtual i_property_component& component(i_string const& aPropertyTypeName, std::uint32_t aComponentIndex) const = 0;
        // helpers
    public:
        /// Appends a component to those of properties of type T and of optional T.
        template <typename T>
        void register_component(i_property_component& aComponent)
        {
            register_component(string{ typeid(T).name() }, aComponent);
            register_component(string{ typeid(optional<T>).name() }, aComponent);
        }
        std::uint32_t component_count(i_property const& aProperty) const
        {
            return component_count(string{ aProperty.type().name() });
        }
        i_property_component& component(i_property const& aProperty, std::uint32_t aComponentIndex) const
        {
            return component(string{ aProperty.type().name() }, aComponentIndex);
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0x1c4d26f2, 0xa550, 0x42b7, 0xb4fc, { 0xc3, 0x8f, 0x1e, 0x3f, 0x16, 0x94 } }; return sIid; }
    };

    // the T held by a property value (either a property_variant alternative or a custom_type), or nullptr
    template <typename T>
    inline T const* property_value_as(property_variant const& aValue)
    {
        return std::visit([](auto const& aAlternative) -> T const*
        {
            using type = std::decay_t<decltype(aAlternative)>;
            if constexpr (std::is_same_v<type, T>)
                return &aAlternative;
            else if constexpr (std::is_same_v<type, custom_type>)
                return same_type(aAlternative.type(), typeid(T)) ? &neolib::any_cast<T const&>(aAlternative) : nullptr;
            else
                return nullptr;
        }, aValue);
    }

    // the value to pass to i_property::set_from_variant for a T (property_variant alternatives are passed as is, 
    // otherwise as a custom_type holding exactly the property's type)
    template <typename T>
    inline property_variant property_value_for(i_property const& aProperty, T const& aValue)
    {
        auto as_custom = [&]()
        {
            if (same_type(aProperty.type(), typeid(optional<T>)))
                return property_variant{ custom_type{ optional<T>{ aValue } } };
            return property_variant{ custom_type{ aValue } };
        };
        if constexpr (neolib::is_variant_v<T>)
            return as_custom();
        else
        {
            property_variant result{ aValue };
            if (std::holds_alternative<custom_type>(result))
                return as_custom();
            return result;
        }
    }

    // a component of a T property implemented with functions getting it as, and setting it from, text
    template <typename T>
    class basic_property_component : public reference_counted<i_property_component>
    {
    public:
        typedef std::function<std::string(T const&)> get_function;
        typedef std::function<bool(T&, std::string const&)> set_function; // false if the text can't be parsed
        typedef std::function<std::vector<std::string>(T const*)> choices_function; // (nullptr if the property is unset)
    public:
        basic_property_component(std::string const& aName, get_function aGet, set_function aSet, choices_function aChoices = {}) :
            iName{ aName }, iGet{ aGet }, iSet{ aSet }, iChoices{ aChoices }
        {
        }
    public:
        i_string const& name() const override
        {
            return iName;
        }
        void get(i_property const& aProperty, i_string& aText) const override
        {
            if (auto const value = property_value_as<T>(aProperty.get_as_variant()))
                aText = string{ iGet(*value) };
            else
                aText = string{};
        }
        bool set(i_property const& aProperty, i_string const& aText, property_variant& aNewValue) const override
        {
            auto const current = property_value_as<T>(aProperty.get_as_variant());
            std::optional<T> value;
            if (current)
                value.emplace(*current);
            else if constexpr (std::is_default_constructible_v<T>)
                value.emplace(); // (an unset optional property)
            else
                return false;
            if (!iSet(*value, aText.to_std_string()))
                return false;
            aNewValue = property_value_for(aProperty, *value);
            return true;
        }
        bool has_choices() const override
        {
            return !!iChoices;
        }
        void choices(i_property const& aProperty, neolib::i_vector<i_string>& aChoices) const override
        {
            aChoices.clear();
            if (iChoices)
                for (auto const& choice : iChoices(property_value_as<T>(aProperty.get_as_variant())))
                    aChoices.push_back(string{ choice });
        }
    private:
        string iName;
        get_function iGet;
        set_function iSet;
        choices_function iChoices;
    };
}
