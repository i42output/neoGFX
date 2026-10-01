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
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/core/i_property.hpp>
#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/gradient.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/app/i_style.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>
#include <neogfx/gui/widget/i_text_widget.hpp>
#include <neogfx/gui/widget/label.hpp>

namespace neogfx::DesignStudio
{
    // row: std::monostate: class (group) node; std::uint32_t: .nrc attribute index (new_property_row: add an attribute); i_property*: object property
    typedef std::variant<std::monostate, std::uint32_t, ng::i_property*> property_model_item;
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
        if (type == typeid(ng::optional<bool>))
            return ng::property_variant{ false };
        if (type == typeid(ng::optional<std::int32_t>))
            return ng::property_variant{ std::int32_t{} };
        if (type == typeid(ng::optional<std::uint32_t>))
            return ng::property_variant{ std::uint32_t{} };
        if (type == typeid(ng::optional<std::int64_t>))
            return ng::property_variant{ std::int64_t{} };
        if (type == typeid(ng::optional<std::uint64_t>))
            return ng::property_variant{ std::uint64_t{} };
        if (type == typeid(ng::optional<float>))
            return ng::property_variant{ float{} };
        if (type == typeid(ng::optional<double>))
            return ng::property_variant{ double{} };
        if (type == typeid(ng::optional<ng::string>))
            return ng::property_variant{ ng::string{} };
        return {};
    }

    // does the property hold a T (or an optional T)?
    template <typename T>
    inline bool property_has_type(ng::i_property const& aProperty)
    {
        return aProperty.type() == typeid(T) || aProperty.type() == typeid(ng::optional<T>);
    }

    // the T held by a property value (either a property_variant alternative or a custom_type), or nullptr
    template <typename T>
    inline T const* property_value_as(ng::property_variant const& aValue)
    {
        return std::visit([](auto const& aAlternative) -> T const*
        {
            using type = std::decay_t<decltype(aAlternative)>;
            if constexpr (std::is_same_v<type, T>)
                return &aAlternative;
            else if constexpr (std::is_same_v<type, ng::custom_type>)
                return aAlternative.type() == typeid(T) ? &neolib::any_cast<T const&>(aAlternative) : nullptr;
            else
                return nullptr;
        }, aValue);
    }

    // the value to pass to i_property::set_from_variant for a T (property_variant alternatives are passed as is, 
    // otherwise as a custom_type holding exactly the property's type)
    template <typename T>
    inline ng::property_variant property_value_for(ng::i_property const& aProperty, T const& aValue)
    {
        auto as_custom = [&]()
        {
            if (aProperty.type() == typeid(ng::optional<T>))
                return ng::property_variant{ ng::custom_type{ ng::optional<T>{ aValue } } };
            return ng::property_variant{ ng::custom_type{ aValue } };
        };
        if constexpr (neolib::is_variant_v<T>)
            return as_custom();
        else
        {
            ng::property_variant result{ aValue };
            if (std::holds_alternative<ng::custom_type>(result))
                return as_custom();
            return result;
        }
    }

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

    inline std::optional<std::vector<double>> property_text_numbers(std::string const& aText)
    {
        try
        {
            std::vector<double> result;
            for (auto const& token : property_text_tokens(aText, ','))
            {
                std::size_t used = 0;
                result.push_back(std::stod(token, &used));
                if (used != token.size())
                    return {};
            }
            return result;
        }
        catch (...)
        {
            return {};
        }
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

    // which dialog (if any) the "..." button next to a property's editor opens
    enum class property_dialog
    {
        None,
        Color,
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
            if (std::holds_alternative<std::uint32_t>(item))
                return aIndex.column() == 1u || std::get<std::uint32_t>(item) == new_property_row ? ng::item_cell_flags::Default : readOnly;
            auto const& property = *std::get<ng::i_property*>(item);
            if (aIndex.column() == 1u && property_value_editable(property))
                return ng::item_cell_flags::Default;
            return readOnly;
        }
    };
}