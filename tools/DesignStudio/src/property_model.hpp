// property_model.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2023 Leigh Johnston
  
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <limits>
#include <bit>
#include <algorithm>
#include <vector>
#include <variant>
#include <sstream>
#include <typeinfo>
#include <typeindex>
#include <map>
#include <functional>
#include <cmath>
#include <iterator>
#include <charconv>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/core/units.hpp>
#include <neogfx/core/i_property.hpp>
#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/gradient.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/app/i_style.hpp>
#include <neogfx/app/palette.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>
#include <neogfx/gui/widget/i_text_widget.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/widget/item_view.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/tools/DesignStudio/i_property_component_registry.hpp>

namespace neogfx::DesignStudio
{
    // a component of a composite property's value (e.g. a size's width), shown as a child of the property's row
    struct property_component
    {
        ng::i_property* property;
        std::uint32_t index;
        auto operator<=>(property_component const&) const = default;
    };

    // an .nrc attribute of a member element (e.g. ".text_widget"'s "text"): a member of the element's (member element) attribute's object value
    struct member_attribute
    {
        std::string member;
        std::string name;
        auto operator<=>(member_attribute const&) const = default;
    };

    // row: std::monostate: class (group) node; std::uint32_t: .nrc attribute index (new_property_row: an attribute that can be added, see the "Attributes" node); i_property*: object property;
    // property_component: a component of a composite object property; member_attribute: an .nrc attribute of a member element (that it has or can have)
    typedef std::variant<std::monostate, std::uint32_t, ng::i_property*, property_component, member_attribute> property_model_item;
    typedef ng::basic_item_tree_model<property_model_item, 2> property_model;

    inline bool property_value_editable(ng::property_variant const& aValue)
    {
        return std::visit([](auto const& aValue)
        {
            using type = std::decay_t<decltype(aValue)>;
            return std::is_same_v<type, bool> || std::is_same_v<type, char> || std::is_same_v<type, std::int32_t> || std::is_same_v<type, std::uint32_t> ||
                std::is_same_v<type, std::int64_t> || std::is_same_v<type, std::uint64_t> || std::is_same_v<type, float> || std::is_same_v<type, double> ||
                std::is_same_v<type, ng::string>;
        }, aValue);
    }

    inline std::string property_value_to_string(ng::property_variant const& aValue)
    {
        return std::visit([](auto const& aValue) -> std::string
        {
            using type = std::decay_t<decltype(aValue)>;
            if constexpr (std::is_same_v<type, bool>)
                return aValue ? "true" : "false";
            else if constexpr (std::is_same_v<type, char>)
                return std::string(1u, aValue);
            else if constexpr (std::is_same_v<type, ng::string>)
                return aValue.to_std_string();
            else if constexpr (requires(std::ostream& aStream) { aStream << aValue; })
            {
                std::ostringstream result;
                result << aValue;
                return result.str();
            }
            else
                return {};
        }, aValue);
    }

    // parse aText as a value of the same type as aCurrent; nullopt if not possible
    inline std::optional<ng::property_variant> property_value_from_string(ng::property_variant const& aCurrent, std::string const& aText)
    {
        try
        {
            return std::visit([&](auto const& aValue) -> std::optional<ng::property_variant>
            {
                using type = std::decay_t<decltype(aValue)>;
                if constexpr (std::is_same_v<type, bool>)
                {
                    if (aText == "true" || aText == "1")
                        return ng::property_variant{ true };
                    if (aText == "false" || aText == "0")
                        return ng::property_variant{ false };
                    return {};
                }
                else if constexpr (std::is_same_v<type, char>)
                    return aText.size() == 1u ? std::optional<ng::property_variant>{ ng::property_variant{ aText[0] } } : std::nullopt;
                else if constexpr (std::is_same_v<type, std::int32_t>)
                    return ng::property_variant{ static_cast<std::int32_t>(std::stol(aText)) };
                else if constexpr (std::is_same_v<type, std::uint32_t>)
                    return ng::property_variant{ static_cast<std::uint32_t>(std::stoul(aText)) };
                else if constexpr (std::is_same_v<type, std::int64_t>)
                    return ng::property_variant{ static_cast<std::int64_t>(std::stoll(aText)) };
                else if constexpr (std::is_same_v<type, std::uint64_t>)
                    return ng::property_variant{ static_cast<std::uint64_t>(std::stoull(aText)) };
                else if constexpr (std::is_same_v<type, float>)
                    return ng::property_variant{ std::stof(aText) };
                else if constexpr (std::is_same_v<type, double>)
                    return ng::property_variant{ std::stod(aText) };
                else if constexpr (std::is_same_v<type, ng::string>)
                    return ng::property_variant{ ng::string{ aText } };
                else
                    return {};
            }, aCurrent);
        }
        catch (...)
        {
            return {};
        }
    }

    // property-aware versions of the above. Unset optionals show as "(none)"; an empty value or "(none)" unsets an optional.
    // Non-simple types have a text form (see property_text/property_parse below); colors and fonts can also be edited with a dialog.
    inline bool property_value_unset(ng::property_variant const& aValue)
    {
        return std::holds_alternative<std::monostate>(aValue);
    }

    // a value of an unset optional property's underlying type (simple types only) so text entered for it can be parsed
    inline std::optional<ng::property_variant> property_unset_value_prototype(ng::i_property const& aProperty)
    {
        auto const& type = aProperty.type();
        if (same_type(type, typeid(ng::optional<bool>)))
            return ng::property_variant{ false };
        if (same_type(type, typeid(ng::optional<std::int32_t>)))
            return ng::property_variant{ std::int32_t{} };
        if (same_type(type, typeid(ng::optional<std::uint32_t>)))
            return ng::property_variant{ std::uint32_t{} };
        if (same_type(type, typeid(ng::optional<std::int64_t>)))
            return ng::property_variant{ std::int64_t{} };
        if (same_type(type, typeid(ng::optional<std::uint64_t>)))
            return ng::property_variant{ std::uint64_t{} };
        if (same_type(type, typeid(ng::optional<float>)))
            return ng::property_variant{ float{} };
        if (same_type(type, typeid(ng::optional<double>)))
            return ng::property_variant{ double{} };
        if (same_type(type, typeid(ng::optional<ng::string>)))
            return ng::property_variant{ ng::string{} };
        return {};
    }

    // does the property hold a T (or an optional T)?
    template <typename T>
    inline bool property_has_type(ng::i_property const& aProperty)
    {
        return same_type(aProperty.type(), typeid(T)) || same_type(aProperty.type(), typeid(ng::optional<T>));
    }

    // (property_value_as and property_value_for: see i_property_component_registry.hpp)

    inline std::vector<std::string> property_text_tokens(std::string const& aText, char aSeparator)
    {
        std::vector<std::string> result;
        std::string token;
        auto add = [&]()
        {
            auto const first = token.find_first_not_of(" \t");
            auto const last = token.find_last_not_of(" \t");
            result.push_back(first == std::string::npos ? std::string{} : token.substr(first, last - first + 1));
            token.clear();
        };
        for (auto ch : aText)
            if (ch == aSeparator)
                add();
            else
                token += ch;
        add();
        return result;
    }

    // a length: a number of pixels or a number with units (e.g. "42 px", "10mm", "12pt", "2em", "1.5 dip") converted to pixels using the 
    // current units context (see scoped_units_context: the object whose property it is)
    inline std::optional<double> property_text_length(std::string const& aText)
    {
        if (aText == "max" || aText == "inf")
            return std::numeric_limits<double>::infinity(); // (unbounded, e.g. a maximum size)
        double number = 0.0;
        auto const [end, error] = std::from_chars(aText.data(), aText.data() + aText.size(), number);
        if (aText.empty() || error != std::errc{})
            return {};
        auto const unitsText = aText.substr(static_cast<std::size_t>(end - aText.data()));
        auto const first = unitsText.find_first_not_of(" \t");
        if (first == std::string::npos)
            return number; // pixels
        try
        {
            return ng::length::from_string(ng::string{ aText.substr(0u, static_cast<std::size_t>(end - aText.data())) + " " + unitsText.substr(first) }).value();
        }
        catch (...)
        {
            return {};
        }
    }

    // numbers separated by commas; each can have units (see property_text_length)
    inline std::optional<std::vector<double>> property_text_numbers(std::string const& aText)
    {
        std::vector<double> result;
        for (auto const& token : property_text_tokens(aText, ','))
        {
            auto const number = property_text_length(token);
            if (!number)
                return {};
            result.push_back(*number);
        }
        return result;
    }

    inline std::string property_text_numbers(std::initializer_list<double> aNumbers)
    {
        std::ostringstream result;
        bool first = true;
        for (auto n : aNumbers)
        {
            if (!first)
                result << ", ";
            result << n;
            first = false;
        }
        return result.str();
    }

    // enumerator names: from neolib enum declarations where there are some, otherwise listed here
    template <typename Enum>
    inline std::vector<std::pair<std::uint64_t, std::string>> property_enum_names()
    {
        std::vector<std::pair<std::uint64_t, std::string>> result;
        for (auto const& e : neolib::enum_enumerators<Enum>())
            result.emplace_back(static_cast<std::uint64_t>(e.first()), e.second().to_std_string());
        return result;
    }

    template <>
    inline std::vector<std::pair<std::uint64_t, std::string>> property_enum_names<ng::font_role>()
    {
        return {
            { static_cast<std::uint64_t>(ng::font_role::Caption), "Caption" },
            { static_cast<std::uint64_t>(ng::font_role::Menu), "Menu" },
            { static_cast<std::uint64_t>(ng::font_role::Toolbar), "Toolbar" },
            { static_cast<std::uint64_t>(ng::font_role::StatusBar), "StatusBar" },
            { static_cast<std::uint64_t>(ng::font_role::Widget), "Widget" } };
    }

    template <>
    inline std::vector<std::pair<std::uint64_t, std::string>> property_enum_names<ng::logical_coordinate_system>()
    {
        return {
            { static_cast<std::uint64_t>(ng::logical_coordinate_system::Specified), "Specified" },
            { static_cast<std::uint64_t>(ng::logical_coordinate_system::AutomaticGui), "AutomaticGui" },
            { static_cast<std::uint64_t>(ng::logical_coordinate_system::AutomaticGame), "AutomaticGame" } };
    }

    template <>
    inline std::vector<std::pair<std::uint64_t, std::string>> property_enum_names<ng::text_widget_flags>()
    {
        return {
            { static_cast<std::uint64_t>(ng::text_widget_flags::None), "None" },
            { static_cast<std::uint64_t>(ng::text_widget_flags::HideOnEmpty), "HideOnEmpty" },
            { static_cast<std::uint64_t>(ng::text_widget_flags::TakesSpaceWhenEmpty), "TakesSpaceWhenEmpty" },
            { static_cast<std::uint64_t>(ng::text_widget_flags::CutOff), "CutOff" },
            { static_cast<std::uint64_t>(ng::text_widget_flags::UseEllipsis), "UseEllipsis" },
            { static_cast<std::uint64_t>(ng::text_widget_flags::CacheToTexture), "CacheToTexture" },
            { static_cast<std::uint64_t>(ng::text_widget_flags::UseFade), "UseFade" } };
    }

    // enums whose values are combinations of their enumerators (flags): edited as text (e.g. "Left | Top") rather than chosen from a list
    template <typename Enum>
    inline constexpr bool property_enum_is_flags = false;
    template <>
    inline constexpr bool property_enum_is_flags<ng::alignment> = true;
    template <>
    inline constexpr bool property_enum_is_flags<ng::focus_policy> = true;
    template <>
    inline constexpr bool property_enum_is_flags<ng::text_widget_flags> = true;

    // the enumerator names an enum's value is chosen from (empty if it is flags)
    template <typename Enum>
    inline std::vector<std::string> property_enum_choices()
    {
        std::vector<std::string> result;
        if constexpr (!property_enum_is_flags<Enum>)
            for (auto const& name : property_enum_names<Enum>())
                if (std::find(result.begin(), result.end(), name.second) == result.end())
                    result.push_back(name.second);
        return result;
    }

    // the individual flags (single bit enumerators, the first name of each) of a flags enum (empty if it isn't flags)
    template <typename Enum>
    inline std::vector<std::pair<std::uint64_t, std::string>> property_enum_flags()
    {
        std::vector<std::pair<std::uint64_t, std::string>> result;
        if constexpr (property_enum_is_flags<Enum>)
            for (auto const& name : property_enum_names<Enum>())
                if (std::popcount(name.first) == 1 && std::find_if(result.begin(), result.end(), [&](auto const& f) { return f.first == name.first; }) == result.end())
                    result.push_back(name);
        return result;
    }

    // an enumerator name, or for flags the contained enumerators joined with " | " (largest first)
    template <typename Enum>
    inline std::string property_enum_to_string(Enum aValue)
    {
        auto names = property_enum_names<Enum>();
        auto const value = static_cast<std::uint64_t>(aValue);
        for (auto const& name : names)
            if (name.first == value)
                return name.second;
        std::stable_sort(names.begin(), names.end(), [](auto const& lhs, auto const& rhs) { return std::popcount(lhs.first) > std::popcount(rhs.first); });
        std::string result;
        auto remaining = value;
        for (auto const& name : names)
            if (name.first != 0u && (name.first & remaining) == name.first)
            {
                result += (result.empty() ? "" : " | ") + name.second;
                remaining &= ~name.first;
            }
        if (remaining != 0u || result.empty())
        {
            std::ostringstream hex;
            hex << "0x" << std::uppercase << std::hex << remaining;
            result += (result.empty() ? "" : " | ") + hex.str();
        }
        return result;
    }

    // enumerator names (or numbers) separated by '|'
    template <typename Enum>
    inline std::optional<Enum> property_enum_from_string(std::string const& aText)
    {
        auto const names = property_enum_names<Enum>();
        std::uint64_t value = 0u;
        for (auto const& token : property_text_tokens(aText, '|'))
        {
            auto const existing = std::find_if(names.begin(), names.end(), [&](auto const& name) { return name.second == token; });
            if (existing != names.end())
                value |= existing->first;
            else
            {
                try
                {
                    std::size_t used = 0;
                    value |= std::stoull(token, &used, 0);
                    if (used != token.size())
                        return {};
                }
                catch (...)
                {
                    return {};
                }
            }
        }
        return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(value));
    }

    // text forms of non-simple property types
    inline std::string property_text(ng::color const& aValue) { return aValue.to_hex_string(); }
    inline std::optional<ng::color> property_parse(std::string const& aText, ng::color const*) { return ng::color{ aText }; }

    inline std::string property_text(ng::color_or_gradient const& aValue) 
    { 
        if (std::holds_alternative<ng::color>(aValue))
            return property_text(std::get<ng::color>(aValue));
        if (std::holds_alternative<ng::gradient>(aValue))
            return "(gradient)";
        return std::string{};
    }
    inline std::optional<ng::color_or_gradient> property_parse(std::string const& aText, ng::color_or_gradient const*)
    {
        if (aText == "(gradient)")
            return {}; // leave the gradient as is
        return ng::color_or_gradient{ ng::color{ aText } };
    }

    // "Family, Style, Size", e.g. "Segoe UI, Bold, 10"
    inline std::string property_text(ng::font const& aValue)
    {
        std::ostringstream result;
        result << aValue.family_name().to_std_string() << ", " << aValue.style_name().to_std_string() << ", " << aValue.size();
        return result.str();
    }
    inline std::optional<ng::font> property_parse(std::string const& aText, ng::font const*)
    {
        auto const tokens = property_text_tokens(aText, ',');
        if (tokens.size() != 3u)
            return {};
        try
        {
            std::size_t used = 0;
            auto const size = std::stod(tokens[2], &used);
            if (used != tokens[2].size())
                return {};
            return ng::font{ ng::string{ tokens[0] }, ng::string{ tokens[1] }, size };
        }
        catch (...)
        {
            return {};
        }
    }

    // "cx, cy"
    inline std::string property_text(ng::size const& aValue) { return property_text_numbers({ aValue.cx, aValue.cy }); }
    inline std::optional<ng::size> property_parse(std::string const& aText, ng::size const*)
    {
        auto const numbers = property_text_numbers(aText);
        if (!numbers || numbers->size() != 2u)
            return {};
        return ng::size{ (*numbers)[0], (*numbers)[1] };
    }

    // "x, y"
    inline std::string property_text(ng::point const& aValue) { return property_text_numbers({ aValue.x, aValue.y }); }
    inline std::optional<ng::point> property_parse(std::string const& aText, ng::point const*)
    {
        auto const numbers = property_text_numbers(aText);
        if (!numbers || numbers->size() != 2u)
            return {};
        return ng::point{ (*numbers)[0], (*numbers)[1] };
    }

    // margin, border and padding: "left, top, right, bottom" (or "all" or "left/right, top/bottom")
    inline std::string property_text(ng::padding const& aValue) { return property_text_numbers({ aValue.left, aValue.top, aValue.right, aValue.bottom }); }
    inline std::optional<ng::padding> property_parse(std::string const& aText, ng::padding const*)
    {
        auto const numbers = property_text_numbers(aText);
        if (!numbers)
            return {};
        switch (numbers->size())
        {
        case 1u:
            return ng::padding{ (*numbers)[0] };
        case 2u:
            return ng::padding{ (*numbers)[0], (*numbers)[1] };
        case 4u:
            return ng::padding{ (*numbers)[0], (*numbers)[1], (*numbers)[2], (*numbers)[3] };
        default:
            return {};
        }
    }

    // "Horizontal, Vertical[, Visibility[, AspectRatioCx, AspectRatioCy]]", e.g. "Expanding, Minimum"
    inline std::string property_text(ng::size_policy const& aValue)
    {
        std::string result = property_enum_to_string(aValue.horizontal_constraint(false)) + ", " + property_enum_to_string(aValue.vertical_constraint(false));
        if (aValue.visibility() != ng::visibility_constraint::Consider || aValue.maintain_aspect_ratio())
            result += ", " + property_enum_to_string(aValue.visibility());
        if (aValue.maintain_aspect_ratio())
            result += ", " + property_text(aValue.aspect_ratio());
        return result;
    }
    inline std::optional<ng::size_policy> property_parse(std::string const& aText, ng::size_policy const*)
    {
        auto const tokens = property_text_tokens(aText, ',');
        if (tokens.size() != 2u && tokens.size() != 3u && tokens.size() != 5u)
            return {};
        auto const horizontal = property_enum_from_string<ng::size_constraint>(tokens[0]);
        auto const vertical = property_enum_from_string<ng::size_constraint>(tokens[1]);
        if (!horizontal || !vertical)
            return {};
        ng::size_policy result{ *horizontal, *vertical };
        if (tokens.size() >= 3u)
        {
            auto const visibility = property_enum_from_string<ng::visibility_constraint>(tokens[2]);
            if (!visibility)
                return {};
            result.set_ignore_visibility(*visibility == ng::visibility_constraint::Ignore);
        }
        if (tokens.size() == 5u)
        {
            auto const aspectRatio = property_parse(tokens[3] + ", " + tokens[4], static_cast<ng::size const*>(nullptr));
            if (!aspectRatio)
                return {};
            result.set_aspect_ratio(*aspectRatio);
        }
        return result;
    }

    // enums
    template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
    inline std::string property_text(Enum aValue) { return property_enum_to_string(aValue); }
    template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
    inline std::optional<Enum> property_parse(std::string const& aText, Enum const*) { return property_enum_from_string<Enum>(aText); }

    template <typename... Types>
    struct property_type_list {};
    // the non-simple property types that have a text form
    using property_text_types = property_type_list<
        ng::color, ng::color_or_gradient, ng::font, ng::size, ng::point, ng::padding, ng::size_policy,
        ng::alignment, ng::label_placement, ng::cardinal, ng::aspect_ratio, ng::focus_policy, ng::font_role, ng::logical_coordinate_system, ng::text_widget_flags>;

    // calls aFunction with a null T const* for the type in aTypes the property holds; false if none
    template <typename Function, typename... Types>
    inline bool property_visit_text_type(ng::i_property const& aProperty, Function&& aFunction, property_type_list<Types...>)
    {
        return ((property_has_type<Types>(aProperty) ? (aFunction(static_cast<Types const*>(nullptr)), true) : false) || ...);
    }

    // the values an enum property (not flags) is chosen from (with "(none)" first if it is optional); empty if it isn't one
    inline std::vector<std::string> property_choices(ng::i_property const& aProperty)
    {
        std::vector<std::string> result;
        property_visit_text_type(aProperty, [&](auto const* aType)
        {
            using type = std::decay_t<decltype(*aType)>;
            if constexpr (std::is_enum_v<type>)
                result = property_enum_choices<type>();
        }, property_text_types{});
        if (!result.empty() && aProperty.optional())
            result.insert(result.begin(), "(none)");
        return result;
    }

    // the individual flags of a flags enum property (see property_enum_flags); empty if it isn't one
    inline std::vector<std::pair<std::uint64_t, std::string>> property_flags(ng::i_property const& aProperty)
    {
        std::vector<std::pair<std::uint64_t, std::string>> result;
        property_visit_text_type(aProperty, [&](auto const* aType)
        {
            using type = std::decay_t<decltype(*aType)>;
            if constexpr (std::is_enum_v<type>)
                result = property_enum_flags<type>();
        }, property_text_types{});
        return result;
    }

    // a flags enum property's value as bits (0 if it is unset) and its text for some bits ("Left | Top")
    inline std::uint64_t property_flags_value(ng::i_property const& aProperty, std::string const& aText)
    {
        std::uint64_t result = 0u;
        property_visit_text_type(aProperty, [&](auto const* aType)
        {
            using type = std::decay_t<decltype(*aType)>;
            if constexpr (std::is_enum_v<type>)
                if (auto const value = property_enum_from_string<type>(aText))
                    result = static_cast<std::uint64_t>(*value);
        }, property_text_types{});
        return result;
    }
    inline std::string property_flags_text(ng::i_property const& aProperty, std::uint64_t aValue)
    {
        std::string result;
        property_visit_text_type(aProperty, [&](auto const* aType)
        {
            using type = std::decay_t<decltype(*aType)>;
            if constexpr (std::is_enum_v<type>)
                result = property_enum_to_string(static_cast<type>(static_cast<std::underlying_type_t<type>>(aValue)));
        }, property_text_types{});
        return result;
    }

    inline bool property_has_text_type(ng::i_property const& aProperty)
    {
        return property_visit_text_type(aProperty, [](auto) {}, property_text_types{});
    }

    inline bool property_value_editable(ng::i_property const& aProperty)
    {
        if (aProperty.read_only())
            return false;
        if (property_has_text_type(aProperty))
            return true;
        auto const value = aProperty.get_as_variant();
        if (property_value_unset(value))
            return property_unset_value_prototype(aProperty).has_value();
        return property_value_editable(value);
    }

    inline std::string property_value_to_string(ng::i_property const& aProperty)
    {
        auto const value = aProperty.get_as_variant();
        if (property_value_unset(value))
            return aProperty.optional() ? "(none)" : std::string{};
        std::optional<std::string> result;
        property_visit_text_type(aProperty, [&](auto const* aType)
        {
            using type = std::decay_t<decltype(*aType)>;
            if (auto const v = property_value_as<type>(value))
                result = property_text(*v);
        }, property_text_types{});
        if (result)
            return *result;
        return property_value_to_string(value);
    }

    // returns the value to pass to i_property::set_from_variant (an empty variant unsets an optional); nullopt if aText can't be parsed
    inline std::optional<ng::property_variant> property_value_from_string(ng::i_property const& aProperty, std::string const& aText)
    {
        if (aProperty.optional() && (aText.empty() || aText == "(none)"))
            return ng::property_variant{};
        std::optional<ng::property_variant> result;
        if (property_visit_text_type(aProperty, [&](auto const* aType)
        {
            if (auto const parsed = property_parse(aText, aType))
                result = property_value_for(aProperty, *parsed);
        }, property_text_types{}))
            return result;
        auto current = aProperty.get_as_variant();
        if (property_value_unset(current))
        {
            auto const prototype = property_unset_value_prototype(aProperty);
            if (!prototype)
                return {};
            current = *prototype;
        }
        return property_value_from_string(current, aText);
    }

    // composite property types: their components are registered with Design Studio's property component registry (by Design Studio and 
    // by plugins); a composite property's own row is read only (it shows a summary of its value), its components are shown (indented) 
    // beneath it and edited individually
    template <typename Component>
    inline std::string property_component_text(Component const& aValue)
    {
        if constexpr (std::is_enum_v<Component>)
            return property_enum_to_string(aValue);
        else if constexpr (std::is_same_v<Component, std::string>)
            return aValue;
        else
        {
            std::ostringstream result;
            result << aValue;
            return result.str();
        }
    }

    template <typename Component>
    inline std::optional<Component> property_component_parse(std::string const& aText)
    {
        if constexpr (std::is_enum_v<Component>)
            return property_enum_from_string<Component>(aText);
        else if constexpr (std::is_same_v<Component, std::string>)
            return aText;
        else
        {
            try
            {
                std::size_t used = 0;
                auto const result = std::stod(aText, &used);
                if (used != aText.size())
                    return {};
                return static_cast<Component>(result);
            }
            catch (...)
            {
                return {};
            }
        }
    }

    // a component of a T property that is a Component (as text: see property_component_text and property_component_parse)
    template <typename T, typename Component>
    inline ng::ref_ptr<i_property_component> property_component_of(std::string const& aName, std::function<Component(T const&)> aGet, std::function<void(T&, Component const&)> aSet,
        typename basic_property_component<T>::choices_function aChoices = {})
    {
        if constexpr (std::is_enum_v<Component>)
            if (!aChoices && !property_enum_is_flags<Component>)
                aChoices = [](T const*) { return property_enum_choices<Component>(); }; // (an enum component is chosen from its enumerators)
        return ng::make_ref<basic_property_component<T>>(aName,
            [aGet](T const& aValue) { return property_component_text(aGet(aValue)); },
            [aSet](T& aValue, std::string const& aText)
            {
                auto const component = property_component_parse<Component>(aText);
                if (!component)
                    return false;
                aSet(aValue, *component);
                return true;
            },
            aChoices);
    }

    // a component of a T property that is a length (e.g. a size's width): its text can have units (see property_text_length)
    template <typename T>
    inline ng::ref_ptr<i_property_component> property_length_component_of(std::string const& aName, std::function<double(T const&)> aGet, std::function<void(T&, double const&)> aSet)
    {
        return ng::make_ref<basic_property_component<T>>(aName,
            [aGet](T const& aValue) { return property_component_text(aGet(aValue)); },
            [aSet](T& aValue, std::string const& aText)
            {
                auto const component = property_text_length(aText);
                if (!component)
                    return false;
                aSet(aValue, *component);
                return true;
            });
    }

    // installed font families and the styles of a font's family
    inline std::vector<std::string> property_font_families(ng::font const*)
    {
        std::vector<std::string> result;
        auto const& fm = ng::service<ng::i_font_manager>();
        for (std::uint32_t family = 0u; family < fm.font_family_count(); ++family)
            result.push_back(fm.font_family(family).to_std_string());
        return result;
    }
    inline std::vector<std::string> property_font_styles(ng::font const* aFont)
    {
        std::vector<std::string> result;
        if (!aFont)
            return result;
        auto const& fm = ng::service<ng::i_font_manager>();
        for (std::uint32_t family = 0u; family < fm.font_family_count(); ++family)
            if (fm.font_family(family).to_std_string_view() == aFont->family_name().to_std_string_view())
            {
                for (std::uint32_t style = 0u; style < fm.font_style_count(family); ++style)
                    result.push_back(fm.font_style_name(family, style).to_std_string());
                break;
            }
        return result;
    }

    class property_component_registry : public ng::reference_counted<i_property_component_registry>
    {
    public:
        using i_property_component_registry::register_component;
        using i_property_component_registry::component_count;
        using i_property_component_registry::component;
        void register_component(ng::i_string const& aPropertyTypeName, i_property_component& aComponent) override
        {
            iComponents[aPropertyTypeName.to_std_string()].push_back(ng::ref_ptr<i_property_component>{ &aComponent });
        }
        std::uint32_t component_count(ng::i_string const& aPropertyTypeName) const override
        {
            auto const existing = iComponents.find(aPropertyTypeName.to_std_string());
            return existing != iComponents.end() ? static_cast<std::uint32_t>(existing->second.size()) : 0u;
        }
        i_property_component& component(ng::i_string const& aPropertyTypeName, std::uint32_t aComponentIndex) const override
        {
            return *iComponents.at(aPropertyTypeName.to_std_string()).at(aComponentIndex);
        }
    private:
        std::map<std::string, std::vector<ng::ref_ptr<i_property_component>>> iComponents; // (by type name)
    };

    // a component of a palette property: one of its colours (as text: see property_text/property_parse; empty or "(none)": the colour is 
    // unset (the style's is used))
    inline ng::ref_ptr<i_property_component> property_palette_component_of(std::string const& aName, ng::color_role aRole)
    {
        return ng::make_ref<basic_property_component<ng::palette>>(aName,
            [aRole](ng::palette const& aValue) { return aValue.has_color(aRole) ? property_text(aValue.color(aRole)) : std::string{}; },
            [aRole](ng::palette& aValue, std::string const& aText)
            {
                ng::optional_color color;
                if (!aText.empty() && aText != "(none)")
                {
                    auto const parsed = property_parse(aText, static_cast<ng::color const*>(nullptr));
                    if (!parsed)
                        return false;
                    color = *parsed;
                }
                if (aValue.has_proxy())
                    aValue.set_color(aRole, color);
                else
                {
                    // (a new palette (the property was unset): its other colours are the current style's)
                    ng::palette result{ ng::current_style_palette_proxy() };
                    for (auto role = static_cast<std::uint32_t>(ng::color_role::Theme); role <= static_cast<std::uint32_t>(ng::color_role::Void); ++role)
                        result.set_color(static_cast<ng::color_role>(role), aValue.maybe_color(static_cast<ng::color_role>(role)));
                    result.set_color(aRole, color);
                    aValue = result;
                }
                return true;
            });
    }

    // Design Studio's property component registry (discoverable by plugins: see app::discover) with its own components registered
    inline property_component_registry& the_property_component_registry()
    {
        static property_component_registry sRegistry;
        static bool const sInitialized = []()
        {
            sRegistry.add_ref(); // (lives as long as the application)
            sRegistry.register_component<ng::size>(*property_length_component_of<ng::size>("Width", [](ng::size const& v) { return v.cx; }, [](ng::size& v, double const& c) { v.cx = c; }));
            sRegistry.register_component<ng::size>(*property_length_component_of<ng::size>("Height", [](ng::size const& v) { return v.cy; }, [](ng::size& v, double const& c) { v.cy = c; }));
            sRegistry.register_component<ng::point>(*property_length_component_of<ng::point>("X", [](ng::point const& v) { return v.x; }, [](ng::point& v, double const& c) { v.x = c; }));
            sRegistry.register_component<ng::point>(*property_length_component_of<ng::point>("Y", [](ng::point const& v) { return v.y; }, [](ng::point& v, double const& c) { v.y = c; }));
            sRegistry.register_component<ng::padding>(*property_length_component_of<ng::padding>("Left", [](ng::padding const& v) { return v.left; }, [](ng::padding& v, double const& c) { v.left = c; }));
            sRegistry.register_component<ng::padding>(*property_length_component_of<ng::padding>("Top", [](ng::padding const& v) { return v.top; }, [](ng::padding& v, double const& c) { v.top = c; }));
            sRegistry.register_component<ng::padding>(*property_length_component_of<ng::padding>("Right", [](ng::padding const& v) { return v.right; }, [](ng::padding& v, double const& c) { v.right = c; }));
            sRegistry.register_component<ng::padding>(*property_length_component_of<ng::padding>("Bottom", [](ng::padding const& v) { return v.bottom; }, [](ng::padding& v, double const& c) { v.bottom = c; }));
            sRegistry.register_component<ng::font>(*property_component_of<ng::font, std::string>("Family", [](ng::font const& v) { return v.family_name().to_std_string(); }, 
                [](ng::font& v, std::string const& c) { v = ng::font{ ng::string{ c }, ng::string{ v.style_name() }, v.size() }; }, property_font_families));
            sRegistry.register_component<ng::font>(*property_component_of<ng::font, std::string>("Style", [](ng::font const& v) { return v.style_name().to_std_string(); }, 
                [](ng::font& v, std::string const& c) { v = ng::font{ ng::string{ v.family_name() }, ng::string{ c }, v.size() }; }, property_font_styles));
            sRegistry.register_component<ng::font>(*property_component_of<ng::font, double>("Size", [](ng::font const& v) { return static_cast<double>(v.size()); }, 
                [](ng::font& v, double const& c) { v = ng::font{ ng::string{ v.family_name() }, ng::string{ v.style_name() }, c }; }));
            sRegistry.register_component<ng::size_policy>(*property_component_of<ng::size_policy, ng::size_constraint>("Horizontal", [](ng::size_policy const& v) { return v.horizontal_constraint(false); }, 
                [](ng::size_policy& v, ng::size_constraint const& c) { v.set_horizontal_constraint(c); }));
            for (auto const& [name, role] : std::initializer_list<std::pair<char const*, ng::color_role>>{
                { "Theme", ng::color_role::Theme }, { "Background", ng::color_role::Background }, { "Foreground", ng::color_role::Foreground }, 
                { "Base", ng::color_role::Base }, { "Alternate Base", ng::color_role::AlternateBase }, { "Text", ng::color_role::Text }, 
                { "Selection", ng::color_role::Selection }, { "Alternate Selection", ng::color_role::AlternateSelection }, 
                { "Selected Text", ng::color_role::SelectedText }, { "Focus", ng::color_role::Focus }, { "Hover", ng::color_role::Hover }, 
                { "Primary Accent", ng::color_role::PrimaryAccent }, { "Secondary Accent", ng::color_role::SecondaryAccent }, { "Void", ng::color_role::Void } })
                sRegistry.register_component<ng::palette>(*property_palette_component_of(name, role));
            sRegistry.register_component<ng::size_policy>(*property_component_of<ng::size_policy, ng::size_constraint>("Vertical", [](ng::size_policy const& v) { return v.vertical_constraint(false); }, 
                [](ng::size_policy& v, ng::size_constraint const& c) { v.set_vertical_constraint(c); }));
            return true;
        }();
        (void)sInitialized;
        return sRegistry;
    }

    // the names of a composite property's components (empty if it isn't composite)
    inline std::vector<std::string> property_components(ng::i_property const& aProperty)
    {
        std::vector<std::string> result;
        auto const& registry = the_property_component_registry();
        for (std::uint32_t component = 0u; component < registry.component_count(aProperty); ++component)
            result.push_back(registry.component(aProperty, component).name().to_std_string());
        return result;
    }

    inline std::string property_component_to_string(ng::i_property const& aProperty, std::uint32_t aIndex)
    {
        auto const& registry = the_property_component_registry();
        if (aIndex >= registry.component_count(aProperty))
            return {};
        ng::string text;
        registry.component(aProperty, aIndex).get(aProperty, text);
        return text.to_std_string();
    }

    // the property's value with one of its components changed (as property_value_from_string)
    inline std::optional<ng::property_variant> property_component_from_string(ng::i_property const& aProperty, std::uint32_t aIndex, std::string const& aText)
    {
        auto const& registry = the_property_component_registry();
        if (aIndex >= registry.component_count(aProperty))
            return {};
        ng::property_variant value;
        if (!registry.component(aProperty, aIndex).set(aProperty, ng::string{ aText }, value))
            return {};
        return value;
    }

    // which dialog (if any) the "..." button next to a property's editor opens (a gradient's: from a color property row's context menu)
    enum class property_dialog
    {
        None,
        Color,
        Gradient,
        Font
    };

    inline property_dialog property_dialog_for(ng::i_property const& aProperty)
    {
        if (property_has_type<ng::color>(aProperty) || property_has_type<ng::color_or_gradient>(aProperty))
            return property_dialog::Color;
        if (property_has_type<ng::font>(aProperty))
            return property_dialog::Font;
        return property_dialog::None;
    }

    // the rank of a property category: its position in declaration order (see property_category); any others (e.g. a plugin's) follow 
    // in the order they are first seen (categories are compared by name: see same_type)
    inline std::size_t property_category_rank(std::type_info const& aCategory)
    {
        static std::vector<std::string> sCategoryOrder =
        {
            typeid(ng::property_category::soft_geometry).name(),
            typeid(ng::property_category::hard_geometry).name(),
            typeid(ng::property_category::appearance).name(),
            typeid(ng::property_category::other_appearance).name(),
            typeid(ng::property_category::interaction).name(),
            typeid(ng::property_category::other).name()
        };
        auto existing = std::find(sCategoryOrder.begin(), sCategoryOrder.end(), std::string{ aCategory.name() });
        if (existing == sCategoryOrder.end())
            existing = sCategoryOrder.insert(sCategoryOrder.end(), aCategory.name());
        return static_cast<std::size_t>(std::distance(sCategoryOrder.begin(), existing));
    }

    // class name without template arguments, e.g. "neogfx::layout_item"
    inline std::string property_class_name(std::type_info const& aType)
    {
        std::string result = aType.name();
        for (std::string const prefix : { "class ", "struct " })
            for (auto pos = result.find(prefix); pos != std::string::npos; pos = result.find(prefix))
                result.erase(pos, prefix.size());
        std::size_t depth = 0;
        std::string stripped;
        for (auto ch : result)
        {
            if (ch == '<')
                ++depth;
            else if (ch == '>')
                --depth;
            else if (depth == 0)
                stripped += ch;
        }
        return stripped;
    }

    // class name of a property's owning (context) class
    inline std::string property_class_name(ng::i_property const& aProperty)
    {
        return property_class_name(aProperty.context());
    }

    class property_presentation_model : public ng::basic_item_presentation_model<property_model>
    {
    public:
        static constexpr std::uint32_t new_property_row = std::numeric_limits<std::uint32_t>::max();
    public:
        using ng::basic_item_presentation_model<property_model>::basic_item_presentation_model;
    public:
        ng::item_cell_flags cell_flags(ng::item_presentation_model_index const& aIndex) const override
        {
            auto const readOnly = ng::item_cell_flags::Default & ~ng::item_cell_flags::Editable;
            auto const& item = item_model().item(to_item_model_index(aIndex));
            if (std::holds_alternative<std::monostate>(item))
                return readOnly; // class node
            if (std::holds_alternative<std::uint32_t>(item) || std::holds_alternative<member_attribute>(item))
                return aIndex.column() == 1u ? ng::item_cell_flags::Default : readOnly; // (its value)
            if (std::holds_alternative<property_component>(item))
            {
                auto const& component = std::get<property_component>(item);
                if (has_choices(component))
                    return readOnly; // (chosen from its drop-down list, see cell_widget)
                return aIndex.column() == 1u && property_value_editable(*component.property) ? ng::item_cell_flags::Default : readOnly;
            }
            auto const& property = *std::get<ng::i_property*>(item);
            if (has_choices(property))
                return readOnly; // (chosen from its drop-down list, see cell_widget)
            if (aIndex.column() == 1u && property_value_editable(property) && property_components(property).empty())
                return ng::item_cell_flags::Default;
            return readOnly; // (a composite property's components are edited instead)
        }
        // room before a cell's text (or in-place editor) for a "..." button shown in it (see main_window_ex)
        ng::optional_size cell_image_size(ng::item_presentation_model_index const& aIndex) const override
        {
            if (auto const cell = iDialogButtonCells.find(aIndex); cell != iDialogButtonCells.end())
                return cell->second;
            return ng::basic_item_presentation_model<property_model>::cell_image_size(aIndex);
        }
        // the cells showing a "..." button and the buttons' sizes; true if changed
        bool set_dialog_button_cells(std::map<ng::item_presentation_model_index, ng::size> const& aCells)
        {
            if (iDialogButtonCells == aCells)
                return false;
            iDialogButtonCells = aCells;
            return true;
        }
        // each cell has a 1 pixel border: a colour between the current style's base and text colours
        ng::optional_color cell_border(ng::item_presentation_model_index const&) const override
        {
            auto const& palette = ng::service<ng::i_app>().current_style().palette();
            return palette.color(ng::color_role::Base).mid(palette.color(ng::color_role::Text));
        }
        ng::optional_color cell_color(ng::item_presentation_model_index const& aIndex, ng::color_role aColorRole) const override
        {
            // (class nodes have the default colours) a category node has its properties' shade (hue) but as if for the opposite theme (light if 
            // the theme is dark, dark if light), with ink to match; a property's rows (its own and its components') alternate between two shades of a hue particular to its category (from a 
            // hash of the category's name), dark shades for a dark theme and light shades for a light one; selected rows are left to the view 
            // (which shows the selection) and the attachment is the property table (an item_view)
            bool const selected = attached() && static_cast<ng::item_view const&>(attachment()).has_selection_model() &&
                static_cast<ng::item_view const&>(attachment()).selection_model().is_selected(aIndex);
            if (auto const category = category_of_node(aIndex); category && !selected)
            {
                auto const shade = category_shade(*category, false, true);
                if (aColorRole == ng::color_role::Background)
                    return shade;
                if (aColorRole == ng::color_role::Text)
                    return shade.light() ? ng::color::Black : ng::color::White;
            }
            if (aColorRole == ng::color_role::Background)
            {
                auto const& item = item_model().item(to_item_model_index(aIndex));
                ng::i_property const* property = std::holds_alternative<ng::i_property*>(item) ? std::get<ng::i_property*>(item) :
                    std::holds_alternative<property_component>(item) ? std::get<property_component>(item).property : nullptr;
                if (property != nullptr && !selected)
                    return category_shade(property->category(), aIndex.row() % 2u == 1u);
            }
            return ng::basic_item_presentation_model<property_model>::cell_color(aIndex, aColorRole);
        }
        // a drop-down list for an enum property (not flags) or a component that has choices (e.g. a font's family or a size policy's 
        // horizontal constraint)
        using ng::basic_item_presentation_model<property_model>::cell_widget;
        void cell_widget(ng::item_presentation_model_index const& aIndex, ng::i_ref_ptr<ng::i_widget>& aWidget) const override
        {
            aWidget.reset();
            if (aIndex.column() != 1u)
                return;
            auto const modelIndex = to_item_model_index(aIndex);
            auto const& item = item_model().item(modelIndex);
            if (!(std::holds_alternative<property_component>(item) && has_choices(std::get<property_component>(item))) &&
                !(std::holds_alternative<ng::i_property*>(item) && has_choices(*std::get<ng::i_property*>(item))))
                return;
            auto existing = iCellWidgets.find(item);
            if (existing == iCellWidgets.end())
            {
                auto dropList = ng::make_ref<ng::drop_list>();
                auto& dropListRef = *dropList;
                dropListRef.set_padding(ng::padding{});
                auto& inputButton = static_cast<ng::push_button&>(dropListRef.input_widget().as_widget());
                inputButton.set_size_policy(ng::size_policy{ ng::size_constraint::Expanding, ng::size_constraint::Expanding });
                inputButton.set_face_color(ng::color{});
                if (attached())
                {
                    auto const cellPadding = cell_padding(attachment());
                    inputButton.set_padding(ng::padding{ cellPadding.left * 2.0 + 1.0_dip, 0.0, cellPadding.right, 0.0 });
                }
                // (a flags property's flags are checked and unchecked: the checked ones are its new value)
                dropList->presentation_model().item_toggled([this, item, modelIndex, &dropListRef](ng::item_presentation_model_index const&)
                {
                    if (iSyncingCellWidgets || !std::holds_alternative<ng::i_property*>(item))
                        return;
                    auto const& property = *std::get<ng::i_property*>(item);
                    auto const flags = property_flags(property);
                    std::uint64_t value = 0u;
                    for (std::uint32_t row = 0u; row < dropListRef.model().rows() && row < flags.size(); ++row)
                        if (dropListRef.presentation_model().is_checked(dropListRef.presentation_model().from_item_model_index(ng::item_model_index{ row })))
                            value |= flags[row].first;
                    auto const text = property_flags_text(property, value);
                    dropListRef.input_widget().set_text(ng::string{ text });
                    item_model().update_cell_data(modelIndex, ng::string{ text });
                });
                dropList->SelectionChanged([this, modelIndex, &dropListRef](ng::optional_item_model_index const& aSelection)
                {
                    if (iSyncingCellWidgets || !aSelection)
                        return;
                    // (the choice is the cell's new text: committed as an edit of the cell's text is (the property (or component) and its .nrc 
                    // attribute are set: see main_window_ex))
                    auto const text = static_variant_cast<ng::string const&>(dropListRef.model().cell_data(*aSelection));
                    item_model().update_cell_data(modelIndex, text);
                });
                existing = iCellWidgets.emplace(item, ng::ref_ptr<ng::i_widget>{ dropList }).first;
                // (it must be in the view, so it has a root, before its own models are populated)
                if (attached())
                    attachment().add(existing->second);
                sync_cell_widget(item, dropListRef);
            }
            aWidget = existing->second;
        }
        // the cell widgets no longer apply (the model is being rebuilt)
        void clear_cell_widgets()
        {
            iCellWidgets.clear();
        }
        // show a property's new value (and, e.g. for a font's style, new choices) in its components' drop-down lists
        void sync_cell_widgets(ng::i_property const& aProperty)
        {
            for (auto& [item, widget] : iCellWidgets)
                if ((std::holds_alternative<ng::i_property*>(item) && std::get<ng::i_property*>(item) == &aProperty) ||
                    (std::holds_alternative<property_component>(item) && std::get<property_component>(item).property == &aProperty))
                    sync_cell_widget(item, static_cast<ng::drop_list&>(*widget));
        }
        ng::dimension indent(ng::item_presentation_model_index const& aIndex, ng::i_units_context const& aUnitsContext) const override
        {
            // all items are indented the same amount (room for a tree expander) whatever their depth except the components of 
            // composite properties which are indented beneath their property
            if (aIndex.column() != 0u)
                return 0.0;
            auto const levels = std::holds_alternative<property_component>(item_model().item(to_item_model_index(aIndex))) ? 2.0 : 1.0;
            return levels * cell_tree_expander_size(aIndex, aUnitsContext)->cx;
        }
    private:
        // the shades of a property category's rows: a hue from a hash of the category's name; dark shades for a dark theme, light for a light one
        static ng::color category_shade(std::type_info const& aCategory, bool aAlternate, bool aOppositeTheme = false)
        {
            auto const& palette = ng::service<ng::i_app>().current_style().palette();
            bool const darkTheme = (palette.color(ng::color_role::Base).dark() != aOppositeTheme);
            // categories (in the tree they are in rank order, some may be absent) have hues 150 degrees apart by rank: categories whose ranks 
            // differ by one are 150 degrees apart and the first twelve categories (any categories whose ranks differ by less than twelve) are at 
            // least 30 degrees apart
            auto const rank = property_category_rank(aCategory);
            auto const hue = std::fmod(30.0 + static_cast<double>(rank) * 150.0, 360.0);
            auto const lightness = darkTheme ? (aAlternate ? 0.20 : 0.15) : (aAlternate ? 0.87 : 0.93);
            return ng::color::from_hsl(hue, darkTheme ? 0.35 : 0.50, lightness);
        }
        // the category of a category node (a class node's child whose children are properties), if it is one
        std::type_info const* category_of_node(ng::item_presentation_model_index const& aIndex) const
        {
            auto const modelIndex = to_item_model_index(aIndex);
            if (!std::holds_alternative<std::monostate>(item_model().item(modelIndex)) || modelIndex.row() + 1u >= item_model().rows())
                return nullptr;
            ng::item_model_index const firstChild{ modelIndex.row() + 1u, 0u };
            if (!item_model().has_parent(firstChild) || item_model().parent(firstChild).row() != modelIndex.row())
                return nullptr;
            auto const& child = item_model().item(firstChild);
            return std::holds_alternative<ng::i_property*>(child) ? &std::get<ng::i_property*>(child)->category() : nullptr;
        }
    private:
        static bool has_choices(property_component const& aComponent)
        {
            auto const& registry = the_property_component_registry();
            return aComponent.index < registry.component_count(*aComponent.property) && 
                registry.component(*aComponent.property, aComponent.index).has_choices();
        }
        static bool has_choices(ng::i_property const& aProperty)
        {
            return !aProperty.read_only() && property_components(aProperty).empty() && 
                (!property_choices(aProperty).empty() || !property_flags(aProperty).empty());
        }
        void sync_cell_widget(property_model_item const& aItem, ng::drop_list& aDropList) const
        {
            neolib::scoped_flag sf{ iSyncingCellWidgets };
            std::vector<std::string> choices;
            std::string current;
            if (std::holds_alternative<property_component>(aItem))
            {
                auto const& component = std::get<property_component>(aItem);
                neolib::vector<ng::string> choiceList;
                the_property_component_registry().component(*component.property, component.index).choices(*component.property, choiceList);
                for (auto const& choice : choiceList)
                    choices.push_back(choice.to_std_string());
                current = property_component_to_string(*component.property, component.index);
            }
            else
            {
                auto const& property = *std::get<ng::i_property*>(aItem);
                current = property_value_to_string(property);
                if (auto const flags = property_flags(property); !flags.empty())
                {
                    // (a flags property: its flags, checked if set (see drop_list::checkable); the list's text is the property's)
                    if (aDropList.model().rows() != flags.size())
                    {
                        aDropList.model().clear();
                        for (std::uint32_t row = 0u; row < flags.size(); ++row)
                            aDropList.model().insert_item(ng::item_model_index{ row }, ng::string{ flags[row].second });
                    }
                    auto const value = property_flags_value(property, current);
                    for (std::uint32_t row = 0u; row < flags.size(); ++row)
                    {
                        auto const index = aDropList.presentation_model().from_item_model_index(ng::item_model_index{ row });
                        aDropList.presentation_model().set_cell_checkable(index);
                        aDropList.presentation_model().set_checked(index, (value & flags[row].first) == flags[row].first);
                    }
                    aDropList.input_widget().set_text(ng::string{ current });
                    return;
                }
                choices = property_choices(property);
            }
            std::vector<std::string> existingChoices;
            for (std::uint32_t row = 0u; row < aDropList.model().rows(); ++row)
                existingChoices.push_back(static_variant_cast<ng::string const&>(aDropList.model().cell_data(ng::item_model_index{ row })).to_std_string());
            if (existingChoices != choices)
            {
                aDropList.model().clear();
                for (std::uint32_t row = 0u; row < choices.size(); ++row)
                    aDropList.model().insert_item(ng::item_model_index{ row }, ng::string{ choices[row] });
            }
            auto const match = std::find(choices.begin(), choices.end(), current);
            if (match != choices.end())
            {
                aDropList.selection_model().set_current_index(aDropList.presentation_model().from_item_model_index(
                    ng::item_model_index{ static_cast<std::uint32_t>(std::distance(choices.begin(), match)) }));
                aDropList.accept_selection();
            }
        }
    private:
        mutable std::map<property_model_item, ng::ref_ptr<ng::i_widget>> iCellWidgets; // (a property's or a component's)
        mutable bool iSyncingCellWidgets = false;
        std::map<ng::item_presentation_model_index, ng::size> iDialogButtonCells;
    };
}
