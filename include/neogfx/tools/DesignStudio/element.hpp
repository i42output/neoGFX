// element.hpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

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

#include <algorithm>
#include <charconv>
#include <concepts>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <map>
#include <set>
#include <cctype>
#include <functional>
#include <iostream>

#include <neolib/core/reference_counted.hpp>
#include <neolib/core/optional.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/pair.hpp>
#include <neolib/core/string.hpp>
#include <neolib/task/event.hpp>

#include <neogfx/core/units.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/app/i_resource_manager.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/progress_bar.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/tab_page.hpp>
#include <neogfx/gui/widget/text_field.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/tab_button.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/i_tab_page_container.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/tools/DesignStudio/symbol.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_property_component_registry.hpp>

namespace neogfx::DesignStudio
{
    struct user_interface {};

    template <typename Type>
    inline element_group default_element_group()
    {
        if constexpr (std::is_base_of_v<app, Type>)
            return element_group::App;
        else if constexpr (std::is_base_of_v<action, Type>)
            return element_group::Action;
        else if constexpr (std::is_base_of_v<i_menu, Type>)
            return element_group::Menu;
        else if constexpr (std::is_base_of_v<i_widget, Type>)
            return element_group::Widget;
        else if constexpr (std::is_base_of_v<i_layout, Type>)
            return element_group::Layout;
        else if constexpr (std::is_base_of_v<i_project, Type>)
            return element_group::Project;
        else if constexpr (std::is_base_of_v<user_interface, Type>)
            return element_group::UserInterface;
        else
            return element_group::Unknown;
    }

    // tab pages are created by (and owned by) their tab page container rather than wrapped in a caddy
    template <>
    struct element_traits<tab_page>
    {
        typedef i_element base;
        static constexpr bool needsCaddy = false;
    };

    // applying .nrc attributes (as stored in an element: RJSON text) to the element's live widget/layout so that the design surface 
    // shows what nrc would generate (see neogfx::nrc::ui_element::parse/emit_body for the attributes supported)
    namespace nrc_attributes
    {
        // the attribute associated with an object property: the property's name (e.g. "SizePolicy") except Size's (none: it is the extents)
        inline std::string attribute_name_of(std::string const& aPropertyName)
        {
            if (aPropertyName == "Size")
                return {};
            return aPropertyName;
        }

        // the object property associated with an attribute: the attribute's name or, for a component of a composite property (e.g. 
        // "MaximumSize.Width", as "MaximumSize: { Width: 100 }" is applied), its first part
        inline std::string property_name_of(std::string const& aAttribute)
        {
            return aAttribute.substr(0u, aAttribute.find('.'));
        }

        // a composite property's component's name as a member of its attribute's object value (e.g. Palette's "Alternate Base": 
        // "AlternateBase" as in "Palette: { AlternateBase: Red }")
        inline std::string member_name_of(std::string const& aComponentName)
        {
            std::string result;
            for (auto ch : aComponentName)
                if (ch != ' ')
                    result += ch;
            return result;
        }

        inline std::string trim(std::string_view aText)
        {
            auto const first = aText.find_first_not_of(" \t\r\n");
            if (first == std::string_view::npos)
                return {};
            auto const last = aText.find_last_not_of(" \t\r\n");
            return std::string{ aText.substr(first, last - first + 1u) };
        }

        inline std::string unquote(std::string_view aText)
        {
            auto const text = trim(aText);
            if (text.size() < 2u || text.front() != '"' || text.back() != '"')
                return text;
            std::string result;
            for (std::size_t i = 1u; i + 1u < text.size(); ++i)
            {
                char ch = text[i];
                if (ch == '\\' && i + 2u < text.size())
                {
                    ch = text[++i];
                    if (ch == 'n')
                        ch = '\n';
                    else if (ch == 't')
                        ch = '\t';
                    else if (ch == 'r')
                        ch = '\r';
                }
                result += ch;
            }
            return result;
        }

        // the items of an array value ("[ a b c ]") or the value itself
        inline std::vector<std::string> items(std::string_view aValue)
        {
            auto const value = trim(aValue);
            if (value.size() < 2u || value.front() != '[' || value.back() != ']')
                return { value };
            std::vector<std::string> result;
            std::string item;
            bool quoted = false;
            int depth = 0;
            for (std::size_t i = 1u; i + 1u < value.size(); ++i)
            {
                char const ch = value[i];
                if (ch == '"' && (i == 0u || value[i - 1u] != '\\'))
                    quoted = !quoted;
                if (!quoted && (ch == '[' || ch == '{'))
                    ++depth;
                else if (!quoted && (ch == ']' || ch == '}'))
                    --depth;
                if (!quoted && depth == 0 && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == ','))
                {
                    if (!item.empty())
                        result.push_back(std::move(item));
                    item.clear();
                }
                else
                    item += ch;
            }
            if (!item.empty())
                result.push_back(std::move(item));
            return result;
        }

        // the members of an object value ("{ name: value ... }", one member per line)
        inline std::vector<std::pair<std::string, std::string>> members(std::string_view aValue)
        {
            std::vector<std::pair<std::string, std::string>> result;
            auto const value = trim(aValue);
            if (value.size() < 2u || value.front() != '{' || value.back() != '}')
                return result;
            std::string member;
            bool quoted = false;
            int depth = 0;
            auto add = [&]()
            {
                auto const colon = member.find(':');
                if (colon != std::string::npos)
                    result.emplace_back(trim(std::string_view{ member }.substr(0u, colon)), trim(std::string_view{ member }.substr(colon + 1u)));
                member.clear();
            };
            for (std::size_t i = 1u; i + 1u < value.size(); ++i)
            {
                char const ch = value[i];
                if (ch == '"' && value[i - 1u] != '\\')
                    quoted = !quoted;
                if (!quoted && (ch == '[' || ch == '{'))
                    ++depth;
                else if (!quoted && (ch == ']' || ch == '}'))
                    --depth;
                if (!quoted && depth == 0 && ch == '\n')
                    add();
                else
                    member += ch;
            }
            add();
            return result;
        }

        // whether a value is an object ("{ name: value ... }": a composite property's components, e.g. "Palette: { Base: Red }")
        inline bool is_object(std::string_view aValue)
        {
            auto const value = trim(aValue);
            return value.size() >= 2u && value.front() == '{' && value.back() == '}';
        }

        // an object value's member's value (if it has the member)
        inline std::optional<std::string> member(std::string_view aValue, std::string const& aMember)
        {
            std::optional<std::string> result;
            for (auto const& [name, value] : members(aValue))
                if (name == aMember)
                    result = value;
            return result;
        }

        // an object value with a member set (an empty member value removes it; an object without members is empty)
        inline std::string with_member(std::string_view aValue, std::string const& aMember, std::string const& aMemberValue)
        {
            auto existing = members(aValue);
            std::erase_if(existing, [&](auto const& m) { return m.first == aMember; });
            if (!aMemberValue.empty())
                existing.emplace_back(aMember, aMemberValue);
            if (existing.empty())
                return {};
            std::string result = "{\n";
            for (auto const& [name, value] : existing)
                result += "    " + name + ": " + value + "\n";
            return result + "}";
        }

        // a palette colour role by name (as in the .nrc, e.g. "AlternateBase")
        inline std::optional<color_role> to_color_role(std::string_view aName)
        {
            static std::pair<std::string_view, color_role> const roles[] = {
                { "Theme", color_role::Theme }, { "Background", color_role::Background }, { "Foreground", color_role::Foreground }, 
                { "Base", color_role::Base }, { "AlternateBase", color_role::AlternateBase }, { "Text", color_role::Text }, 
                { "Selection", color_role::Selection }, { "AlternateSelection", color_role::AlternateSelection }, 
                { "SelectedText", color_role::SelectedText }, { "Focus", color_role::Focus }, { "Hover", color_role::Hover }, 
                { "PrimaryAccent", color_role::PrimaryAccent }, { "SecondaryAccent", color_role::SecondaryAccent }, { "Void", color_role::Void } };
            for (auto const& [name, role] : roles)
                if (name == aName)
                    return role;
            return {};
        }

        // a length (e.g. "32dip", "max" or a number of pixels) in device units
        inline std::optional<dimension> to_dimension(i_units_context const& aContext, std::string const& aText)
        {
            scoped_units_context suc{ aContext };
            try
            {
                return length::from_string(string{ aText }).value();
            }
            catch (...) {}
            try
            {
                std::size_t used = 0u;
                auto const result = std::stod(aText, &used);
                if (used == aText.size())
                    return result;
            }
            catch (...) {}
            return {};
        }

        inline std::vector<dimension> to_dimensions(i_units_context const& aContext, std::string_view aValue)
        {
            std::vector<dimension> result;
            for (auto const& item : items(aValue))
            {
                auto const d = to_dimension(aContext, item);
                if (!d)
                    return {};
                result.push_back(*d);
            }
            return result;
        }

        inline std::optional<size> to_size(i_units_context const& aContext, std::string_view aValue)
        {
            auto const d = to_dimensions(aContext, aValue);
            if (d.size() == 1u)
                return size{ d[0], d[0] };
            if (d.size() == 2u)
                return size{ d[0], d[1] };
            return {};
        }

        // an integer (decimal or 0x hex); std::from_chars rather than std::stoull so a non-numeric value (e.g. a colour name) doesn't throw
        inline std::optional<std::uint64_t> to_integer(std::string_view aText)
        {
            int base = 10;
            if (aText.size() > 2u && aText[0] == '0' && (aText[1] == 'x' || aText[1] == 'X'))
            {
                aText.remove_prefix(2u);
                base = 16;
            }
            std::uint64_t result = 0u;
            auto const [end, error] = std::from_chars(aText.data(), aText.data() + aText.size(), result, base);
            if (aText.empty() || error != std::errc{} || end != aText.data() + aText.size())
                return {};
            return result;
        }

        inline std::optional<color> to_color(std::string_view aValue)
        {
            auto const parts = items(aValue);
            try
            {
                if (parts.size() == 1u)
                {
                    if (auto const argb = to_integer(parts[0]))
                        return color{ static_cast<std::uint32_t>(*argb) };
                    return color{ unquote(parts[0]) };
                }
                if (parts.size() == 3u || parts.size() == 4u)
                {
                    std::uint8_t component[4] = { 0u, 0u, 0u, 0xFFu };
                    for (std::size_t i = 0u; i < parts.size(); ++i)
                    {
                        auto const c = to_integer(parts[i]);
                        if (!c)
                            return {};
                        component[i] = static_cast<std::uint8_t>(*c);
                    }
                    return color{ component[0], component[1], component[2], component[3] };
                }
            }
            catch (...) {}
            return {};
        }

        // an enumerator, or for flags several (in an array or separated by '|')
        template <typename Enum>
        inline std::optional<Enum> to_enum(std::string_view aValue)
        {
            std::underlying_type_t<Enum> result{};
            bool any = false;
            for (auto const& item : items(aValue))
                for (auto const& part : items("[" + [&]() { auto s = item; std::replace(s.begin(), s.end(), '|', ' '); return s; }() + "]"))
                {
                    auto const e = neolib::try_string_to_enum<Enum>(unquote(part));
                    if (!e)
                        return {};
                    result |= static_cast<std::underlying_type_t<Enum>>(*e);
                    any = true;
                }
            if (!any)
                return {};
            return static_cast<Enum>(result);
        }

        // how to put back what an applied attribute changed (the target's value before the attribute was first applied), by target and property
        using restorers = std::map<std::string, std::function<void()>>;

        inline void remember(restorers& aRestorers, std::string const& aKey, std::function<void()> aRestorer)
        {
            if (aRestorers.find(aKey) == aRestorers.end())
                aRestorers.emplace(aKey, std::move(aRestorer));
        }

        // put back the defaults (e.g. before applying the current attributes when one has been removed)
        inline void restore(restorers& aRestorers)
        {
            for (auto& restorer : aRestorers)
            {
                try
                {
                    restorer.second();
                }
                catch (...) {}
            }
            aRestorers.clear();
        }

        template <typename Target>
        void apply(Target& aTarget, std::vector<std::pair<std::string, std::string>> const& aAttributes, bool aIncludeText, restorers& aRestorers, std::string const& aPath = {});

        // a member element (e.g. ".label" or ".image_widget" of a button)
        template <typename Target>
        inline void apply_member(Target& aTarget, std::string const& aName, std::string const& aValue, restorers& aRestorers, std::string const& aPath)
        {
            auto const attributes = members(aValue);
            auto const path = aPath + aName;
            if (aName == ".label")
            {
                if constexpr (requires(Target& aT) { aT.label().text_widget(); })
                    apply(aTarget.label(), attributes, true, aRestorers, path);
            }
            else if (aName == ".image_widget")
            {
                if constexpr (requires(Target& aT) { aT.image_widget().set_aspect_ratio(neogfx::aspect_ratio::Keep); })
                    apply(aTarget.image_widget(), attributes, true, aRestorers, path);
            }
            else if (aName == ".text_widget")
            {
                if constexpr (requires(Target& aT) { aT.text_widget().set_size_hint(neogfx::size_hint{}); })
                    apply(aTarget.text_widget(), attributes, true, aRestorers, path);
            }
            else if (aName == ".input_box")
            {
                if constexpr (requires(Target& aT) { aT.input_box().set_text(string{}); })
                    apply(aTarget.input_box(), attributes, true, aRestorers, path);
            }
        }

        // the attributes apply() supports for a target (each with its alternative names, e.g. "uri" for "image"); those that are object 
        // properties have the property's name (a composite property's components are members of its object value, e.g. "MaximumSize: 
        // { Width: 100 }" or "Palette: { Base: Red }")
        template <typename Target>
        inline std::vector<std::vector<std::string>> supported()
        {
            std::vector<std::vector<std::string>> result;
            if constexpr (std::is_base_of_v<i_geometry, Target>)
                for (char const* name : { "SizePolicy", "FixedSize", "MinimumSize", "MaximumSize", "Weight", "Padding" })
                    result.push_back({ name });
            if constexpr (std::is_base_of_v<i_layout, Target>)
                result.push_back({ "spacing" });
            if constexpr (requires(Target& aT) { aT.set_alignment(neogfx::alignment::Left); { aT.alignment() } -> std::convertible_to<neogfx::alignment>; })
                result.push_back({ "Alignment" });
            if constexpr (std::is_base_of_v<i_widget, Target>)
            {
                result.push_back({ "Palette" });
                result.push_back({ "Opacity" });
                result.push_back({ "Enabled" });
            }
            if constexpr (requires(Target& aT) { aT.set_image(string{}); aT.set_image(neogfx::texture{}); })
                result.push_back({ "image", "uri" });
            if constexpr (requires(Target& aT) { aT.set_aspect_ratio(neogfx::aspect_ratio::Keep); { aT.aspect_ratio() } -> std::convertible_to<neogfx::aspect_ratio>; })
                result.push_back({ "AspectRatio" });
            if constexpr (requires(Target& aT) { aT.set_placement(neogfx::label_placement::ImageTextHorizontal); { aT.placement() } -> std::convertible_to<neogfx::label_placement>; } ||
                requires(Target& aT) { aT.set_placement(neogfx::cardinal::Center); { aT.placement() } -> std::convertible_to<neogfx::cardinal>; })
                result.push_back({ "Placement" });
            return result;
        }

        template <typename Target>
        inline void apply(Target& aTarget, std::vector<std::pair<std::string, std::string>> const& aAttributes, bool aIncludeText, restorers& aRestorers, std::string const& aPath)
        {
            auto key = [&](char const* aProperty) { return aPath + "/" + aProperty; };
            // (a composite property's object value's members are its components, e.g. "MaximumSize: { Width: 100 }": "MaximumSize.Width: 100")
            std::vector<std::pair<std::string, std::string>> attributes;
            for (auto const& [name, value] : aAttributes)
                if (!name.empty() && std::isupper(static_cast<unsigned char>(name[0])) && is_object(value))
                {
                    // (a geometry property's members (e.g. "FixedSize: { Width: 24dip }") are applied to its default (unset) value so that a 
                    // member not given (e.g. one cleared) is as it would be by default rather than as it last was)
                    if constexpr (std::is_base_of_v<i_geometry, Target>)
                    {
                        auto& geometry = static_cast<i_geometry&>(aTarget);
                        if (name == "MinimumSize")
                        {
                            remember(aRestorers, key("minimum_size"), [&geometry, previous = geometry.has_minimum_size() ? optional_size{ geometry.minimum_size() } : optional_size{}]() { geometry.set_minimum_size(previous); });
                            geometry.set_minimum_size(optional_size{});
                        }
                        else if (name == "MaximumSize")
                        {
                            remember(aRestorers, key("maximum_size"), [&geometry, previous = geometry.has_maximum_size() ? optional_size{ geometry.maximum_size() } : optional_size{}]() { geometry.set_maximum_size(previous); });
                            geometry.set_maximum_size(optional_size{});
                        }
                        else if (name == "FixedSize")
                        {
                            remember(aRestorers, key("fixed_size"), [&geometry, previous = geometry.has_fixed_size() ? optional_size{ geometry.fixed_size() } : optional_size{}]() { geometry.set_fixed_size(previous); });
                            geometry.set_fixed_size(optional_size{});
                        }
                        else if (name == "Weight")
                        {
                            remember(aRestorers, key("weight"), [&geometry, previous = geometry.has_weight() ? optional_size{ geometry.weight() } : optional_size{}]() { geometry.set_weight(previous); });
                            geometry.set_weight(optional_size{});
                        }
                        else if (name == "Padding")
                        {
                            remember(aRestorers, key("padding"), [&geometry, previous = geometry.has_padding() ? optional_padding{ geometry.padding() } : optional_padding{}]() { geometry.set_padding(previous); });
                            geometry.set_padding(optional_padding{});
                        }
                        else if (name == "SizePolicy")
                        {
                            remember(aRestorers, key("size_policy"), [&geometry, previous = geometry.has_size_policy() ? optional_size_policy{ geometry.size_policy() } : optional_size_policy{}]() { geometry.set_size_policy(previous); });
                            geometry.set_size_policy(optional_size_policy{});
                        }
                    }
                    for (auto const& [memberName, memberValue] : members(value))
                        attributes.emplace_back(name + "." + memberName, memberValue);
                }
                else
                    attributes.emplace_back(name, value);
            for (auto const& [name, value] : attributes)
            {
                try
                {
                    if (!name.empty() && name[0] == '.')
                        apply_member(aTarget, name, value, aRestorers, aPath);
                    else if (name == "text")
                    {
                        if constexpr (requires(Target& aT) { aT.set_text(string{}); string{ aT.text() }; })
                            if (aIncludeText)
                            {
                                remember(aRestorers, key("text"), [&aTarget, previous = string{ aTarget.text() }]() { aTarget.set_text(previous); });
                                aTarget.set_text(string{ unquote(value) });
                            }
                    }
                    else if (name == "SizePolicy")
                    {
                        if constexpr (std::is_base_of_v<i_geometry, Target>)
                        {
                            auto& geometry = static_cast<i_geometry&>(aTarget);
                            auto const parts = items(value);
                            if (!parts.empty())
                            {
                                remember(aRestorers, key("size_policy"), [&geometry, previous = geometry.has_size_policy() ? optional_size_policy{ geometry.size_policy() } : optional_size_policy{}]() { geometry.set_size_policy(previous); });
                                geometry.set_size_policy(neogfx::size_policy::from_string(parts[0], parts[std::min<std::size_t>(1u, parts.size() - 1u)]));
                            }
                        }
                    }
                    else if (name == "MinimumSize" || name == "MaximumSize" || name == "FixedSize" || name == "Weight" ||
                        name == "MinimumSize.Width" || name == "MinimumSize.Height" || name == "MaximumSize.Width" || name == "MaximumSize.Height")
                    {
                        if constexpr (std::is_base_of_v<i_geometry, Target>)
                        {
                            auto& geometry = static_cast<i_geometry&>(aTarget);
                            bool const isMinimum = name.starts_with("MinimumSize");
                            bool const isMaximum = name.starts_with("MaximumSize");
                            if (isMinimum)
                                remember(aRestorers, key("minimum_size"), [&geometry, previous = geometry.has_minimum_size() ? optional_size{ geometry.minimum_size() } : optional_size{}]() { geometry.set_minimum_size(previous); });
                            else if (isMaximum)
                                remember(aRestorers, key("maximum_size"), [&geometry, previous = geometry.has_maximum_size() ? optional_size{ geometry.maximum_size() } : optional_size{}]() { geometry.set_maximum_size(previous); });
                            else if (name == "FixedSize")
                                remember(aRestorers, key("fixed_size"), [&geometry, previous = geometry.has_fixed_size() ? optional_size{ geometry.fixed_size() } : optional_size{}]() { geometry.set_fixed_size(previous); });
                            else
                                remember(aRestorers, key("weight"), [&geometry, previous = geometry.has_weight() ? optional_size{ geometry.weight() } : optional_size{}]() { geometry.set_weight(previous); });
                            if (name.ends_with(".Width") || name.ends_with(".Height"))
                            {
                                auto const d = to_dimension(geometry, unquote(value));
                                if (d)
                                {
                                    if (name == "MinimumSize.Width")
                                        geometry.set_minimum_width(*d);
                                    else if (name == "MinimumSize.Height")
                                        geometry.set_minimum_height(*d);
                                    else if (name == "MaximumSize.Width")
                                        geometry.set_maximum_width(*d);
                                    else
                                        geometry.set_maximum_height(*d);
                                }
                            }
                            else
                            {
                                auto const s = name == "Weight" ? 
                                    [&]() -> std::optional<size> { auto const d = items(value); if (d.size() == 1u) return size{ std::stod(d[0]), std::stod(d[0]) }; if (d.size() == 2u) return size{ std::stod(d[0]), std::stod(d[1]) }; return {}; }() : 
                                    to_size(geometry, value);
                                if (s)
                                {
                                    if (isMinimum)
                                        geometry.set_minimum_size(optional_size{ *s });
                                    else if (isMaximum)
                                        geometry.set_maximum_size(optional_size{ *s });
                                    else if (name == "FixedSize")
                                        geometry.set_fixed_size(optional_size{ *s });
                                    else
                                        geometry.set_weight(optional_size{ *s });
                                }
                            }
                        }
                    }
                    else if (name == "FixedSize.Width" || name == "FixedSize.Height" || name == "Weight.Width" || name == "Weight.Height" || 
                        name == "Padding.Left" || name == "Padding.Top" || name == "Padding.Right" || name == "Padding.Bottom" || 
                        name == "SizePolicy.Horizontal" || name == "SizePolicy.Vertical")
                    {
                        // (a geometry property's component given on its own (e.g. "FixedSize: { Width: 24dip }"): the others are as they are)
                        if constexpr (std::is_base_of_v<i_geometry, Target>)
                        {
                            auto& geometry = static_cast<i_geometry&>(aTarget);
                            if (name.starts_with("SizePolicy."))
                            {
                                if (auto const constraint = to_enum<size_constraint>(value))
                                {
                                    remember(aRestorers, key("size_policy"), [&geometry, previous = geometry.has_size_policy() ? optional_size_policy{ geometry.size_policy() } : optional_size_policy{}]() { geometry.set_size_policy(previous); });
                                    auto policy = geometry.size_policy();
                                    if (name.ends_with(".Horizontal"))
                                        policy.set_horizontal_constraint(*constraint);
                                    else
                                        policy.set_vertical_constraint(*constraint);
                                    geometry.set_size_policy(optional_size_policy{ policy });
                                }
                            }
                            else if (name.starts_with("Weight."))
                            {
                                remember(aRestorers, key("weight"), [&geometry, previous = geometry.has_weight() ? optional_size{ geometry.weight() } : optional_size{}]() { geometry.set_weight(previous); });
                                auto weight = geometry.weight();
                                (name.ends_with(".Width") ? weight.cx : weight.cy) = std::stod(unquote(value));
                                geometry.set_weight(optional_size{ weight });
                            }
                            else if (auto const d = to_dimension(geometry, unquote(value)))
                            {
                                if (name.starts_with("FixedSize."))
                                {
                                    remember(aRestorers, key("fixed_size"), [&geometry, previous = geometry.has_fixed_size() ? optional_size{ geometry.fixed_size() } : optional_size{}]() { geometry.set_fixed_size(previous); });
                                    auto fixedSize = geometry.fixed_size();
                                    (name.ends_with(".Width") ? fixedSize.cx : fixedSize.cy) = *d;
                                    geometry.set_fixed_size(optional_size{ fixedSize });
                                }
                                else
                                {
                                    remember(aRestorers, key("padding"), [&geometry, previous = geometry.has_padding() ? optional_padding{ geometry.padding() } : optional_padding{}]() { geometry.set_padding(previous); });
                                    auto padding = geometry.padding();
                                    (name.ends_with(".Left") ? padding.left : name.ends_with(".Top") ? padding.top : name.ends_with(".Right") ? padding.right : padding.bottom) = *d;
                                    geometry.set_padding(optional_padding{ padding });
                                }
                            }
                        }
                    }
                    else if (name == "Padding")
                    {
                        if constexpr (std::is_base_of_v<i_geometry, Target>)
                        {
                            auto& geometry = static_cast<i_geometry&>(aTarget);
                            auto const d = to_dimensions(geometry, value);
                            std::optional<neogfx::padding> padding;
                            if (d.size() == 1u)
                                padding.emplace(d[0]);
                            else if (d.size() == 2u)
                                padding.emplace(d[0], d[1]);
                            else if (d.size() == 4u)
                                padding.emplace(d[0], d[1], d[2], d[3]);
                            if (padding)
                            {
                                remember(aRestorers, key("padding"), [&geometry, previous = geometry.has_padding() ? optional_padding{ geometry.padding() } : optional_padding{}]() { geometry.set_padding(previous); });
                                geometry.set_padding(optional_padding{ *padding });
                            }
                        }
                    }
                    else if (name == "spacing")
                    {
                        if constexpr (std::is_base_of_v<i_layout, Target>)
                        {
                            auto& layout = static_cast<i_layout&>(aTarget);
                            if (auto const s = to_size(static_cast<i_geometry const&>(layout), value))
                            {
                                remember(aRestorers, key("spacing"), [&layout, previous = layout.has_spacing() ? optional_size{ layout.spacing() } : optional_size{}]() { layout.set_spacing(previous); });
                                layout.set_spacing(optional_size{ *s });
                            }
                        }
                    }
                    else if (name == "Alignment")
                    {
                        if constexpr (requires(Target& aT) { aT.set_alignment(neogfx::alignment::Left); { aT.alignment() } -> std::convertible_to<neogfx::alignment>; })
                            if (auto const a = to_enum<neogfx::alignment>(value))
                            {
                                remember(aRestorers, key("alignment"), [&aTarget, previous = static_cast<neogfx::alignment>(aTarget.alignment())]() { aTarget.set_alignment(previous); });
                                aTarget.set_alignment(*a);
                            }
                    }
                    else if (name.starts_with("Palette."))
                    {
                        if constexpr (std::is_base_of_v<i_widget, Target>)
                            if (auto const role = to_color_role(std::string_view{ name }.substr(8u)))
                                if (auto const c = to_color(value))
                                {
                                    auto& widget = static_cast<i_widget&>(aTarget);
                                    remember(aRestorers, key(name.c_str()), [&widget, role = *role, previous = widget.has_palette_color(*role) ? optional_color{ widget.palette_color(*role) } : optional_color{}]() { widget.set_palette_color(role, previous); });
                                    widget.set_palette_color(*role, optional_color{ *c });
                                }
                    }
                    else if (name == "Opacity")
                    {
                        if constexpr (std::is_base_of_v<i_widget, Target>)
                        {
                            auto& widget = static_cast<i_widget&>(aTarget);
                            auto const opacity = std::stod(unquote(value));
                            remember(aRestorers, key("opacity"), [&widget, previous = widget.opacity()]() { widget.set_opacity(previous); });
                            widget.set_opacity(opacity);
                        }
                    }
                    else if (name == "Enabled")
                    {
                        if constexpr (std::is_base_of_v<i_widget, Target>)
                        {
                            auto& widget = static_cast<i_widget&>(aTarget);
                            auto const v = unquote(value);
                            if (v == "true" || v == "false")
                            {
                                remember(aRestorers, key("enabled"), [&widget, previous = widget.enabled()]() { widget.enable(previous); });
                                widget.enable(v == "true");
                            }
                        }
                    }
                    else if (name == "image" || name == "uri")
                    {
                        if constexpr (requires(Target& aT) { aT.set_image(string{}); aT.set_image(neogfx::texture{}); })
                        {
                            // (widgets created on the design surface have no image)
                            remember(aRestorers, key("image"), [&aTarget]() { aTarget.set_image(neogfx::texture{}); });
                            aTarget.set_image(string{ unquote(value) });
                        }
                    }
                    else if (name == "AspectRatio")
                    {
                        if constexpr (requires(Target& aT) { aT.set_aspect_ratio(neogfx::aspect_ratio::Keep); { aT.aspect_ratio() } -> std::convertible_to<neogfx::aspect_ratio>; })
                            if (auto const a = to_enum<neogfx::aspect_ratio>(value))
                            {
                                remember(aRestorers, key("aspect_ratio"), [&aTarget, previous = static_cast<neogfx::aspect_ratio>(aTarget.aspect_ratio())]() { aTarget.set_aspect_ratio(previous); });
                                aTarget.set_aspect_ratio(*a);
                            }
                    }
                    else if (name == "Placement")
                    {
                        if constexpr (requires(Target& aT) { aT.set_placement(neogfx::label_placement::ImageTextHorizontal); { aT.placement() } -> std::convertible_to<neogfx::label_placement>; })
                        {
                            if (auto const p = to_enum<neogfx::label_placement>(value))
                            {
                                remember(aRestorers, key("placement"), [&aTarget, previous = static_cast<neogfx::label_placement>(aTarget.placement())]() { aTarget.set_placement(previous); });
                                aTarget.set_placement(*p);
                            }
                        }
                        else if constexpr (requires(Target& aT) { aT.set_placement(neogfx::cardinal::Center); { aT.placement() } -> std::convertible_to<neogfx::cardinal>; })
                        {
                            if (auto const p = to_enum<neogfx::cardinal>(value))
                            {
                                remember(aRestorers, key("placement"), [&aTarget, previous = static_cast<neogfx::cardinal>(aTarget.placement())]() { aTarget.set_placement(previous); });
                                aTarget.set_placement(*p);
                            }
                        }
                    }
                }
                catch (std::exception const& e)
                {
                    // not a value the design surface can show (the attribute is still kept and saved)
                    std::cerr << "DesignStudio: cannot apply attribute '" << aPath << (aPath.empty() ? "" : ".") << name << ": " << value << "': " << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "DesignStudio: cannot apply attribute '" << aPath << (aPath.empty() ? "" : ".") << name << ": " << value << "'" << std::endl;
                }
            }
        }
    }

    // menu bars, menus and toolbars: their (live) items built from the element tree (as nrc would generate them); actions referenced 
    // (e.g. "appTest.actionFileSave") are copies (text, image etc.) of the referenced action so they do nothing on the design surface
    namespace nrc_items
    {
        using attributes_t = std::vector<std::pair<std::string, std::string>>;

        inline attributes_t attributes_of(i_element const& aElement)
        {
            attributes_t result;
            for (auto const& attribute : aElement.attributes())
                result.emplace_back(attribute.first().to_std_string(), attribute.second().to_std_string());
            return result;
        }

        inline std::string attribute(attributes_t const& aAttributes, std::string const& aName)
        {
            for (auto const& [name, value] : aAttributes)
                if (name == aName)
                    return nrc_attributes::unquote(value);
            return {};
        }

        // an item of a menu or toolbar: an action reference (or separator) or a child element (inline action or sub menu)
        struct item
        {
            std::string reference;
            i_element* element = nullptr;
        };

        // the items in document order ("#child" marks where each child element is)
        inline std::vector<item> items_of(i_element& aElement)
        {
            std::vector<item> result;
            auto nextChild = aElement.children().begin();
            for (auto const& attribute : aElement.attributes())
            {
                auto const name = attribute.first().to_std_string();
                if (name == "#child" && nextChild != aElement.children().end())
                    result.push_back(item{ {}, &**nextChild++ });
                else if (name == "action")
                    result.push_back(item{ nrc_attributes::unquote(attribute.second().to_std_string()) });
            }
            for (; nextChild != aElement.children().end(); ++nextChild)
                result.push_back(item{ {}, &**nextChild });
            return result;
        }

        // a signature of an element's subtree (to know when its items need rebuilding)
        inline void signature(i_element const& aElement, std::string& aResult)
        {
            aResult += aElement.type().to_std_string() + "{";
            for (auto const& attribute : aElement.attributes())
                aResult += attribute.first().to_std_string() + "=" + attribute.second().to_std_string() + ";";
            for (auto const& child : aElement.children())
                signature(*child, aResult);
            aResult += "}";
        }

        inline ref_ptr<i_action> make_action(attributes_t const& aAttributes)
        {
            auto const text = attribute(aAttributes, "text");
            auto const image = attribute(aAttributes, "image");
            ref_ptr<i_action> result;
            try
            {
                if (!image.empty())
                    result = make_ref<action>(string{ text }, neogfx::image{ string{ image } });
            }
            catch (std::exception const& e)
            {
                std::cerr << "DesignStudio: cannot load action image '" << image << "': " << e.what() << std::endl;
            }
            if (!result)
                result = make_ref<action>(string{ text });
            auto const checkable = attribute(aAttributes, "checkable");
            if (checkable == "true")
                result->set_checkable(true);
            if (attribute(aAttributes, "checked") == "true")
                result->set_checked(true);
            return result;
        }

        // an action referenced by e.g. "appTest.actionFileSave": an action element of the project or one of the standard (app) actions
        inline ref_ptr<i_action> referenced_action(i_project& aProject, std::string const& aReference)
        {
            auto const dot = aReference.rfind('.');
            auto const id = dot == std::string::npos ? aReference : aReference.substr(dot + 1u);
            i_element* actionElement = nullptr;
            aProject.root().visit([&](i_element& aElement)
            {
                if (actionElement == nullptr && aElement.group() == element_group::Action && aElement.id().to_std_string() == id)
                    actionElement = &aElement;
            });
            if (actionElement != nullptr)
                return make_action(attributes_of(*actionElement));
            static std::vector<std::pair<std::string, i_action& (i_app::*)()>> const sStandardActions =
            {
                { "actionFileNew", &i_app::action_file_new },
                { "actionFileOpen", &i_app::action_file_open },
                { "actionFileClose", &i_app::action_file_close },
                { "actionFileCloseAll", &i_app::action_file_close_all },
                { "actionFileSave", &i_app::action_file_save },
                { "actionFileSaveAll", &i_app::action_file_save_all },
                { "actionFileExit", &i_app::action_file_exit },
                { "actionUndo", &i_app::action_undo },
                { "actionRedo", &i_app::action_redo },
                { "actionCut", &i_app::action_cut },
                { "actionCopy", &i_app::action_copy },
                { "actionPaste", &i_app::action_paste },
                { "actionDelete", &i_app::action_delete },
                { "actionSelectAll", &i_app::action_select_all }
            };
            for (auto const& [name, standardAction] : sStandardActions)
                if (name == id)
                {
                    auto& source = (service<i_app>().*standardAction)();
                    return make_ref<action>(string{ source.text() }, source.image());
                }
            return make_ref<action>(string{ id }); // (unknown: show its name)
        }

        inline void populate_menu(i_menu& aMenu, i_element& aMenuElement)
        {
            for (auto const& item : items_of(aMenuElement))
            {
                try
                {
                    if (item.element == nullptr)
                    {
                        if (item.reference == "separator")
                            aMenu.add_separator();
                        else
                            aMenu.insert_action_at(aMenu.count(), referenced_action(aMenuElement.project(), item.reference));
                    }
                    else if (item.element->type().to_std_string() == "menu")
                        populate_menu(aMenu.add_sub_menu(string{ attribute(attributes_of(*item.element), "title") }), *item.element);
                    else if (item.element->group() == element_group::Action)
                        aMenu.insert_action_at(aMenu.count(), make_action(attributes_of(*item.element)));
                }
                catch (std::exception const& e)
                {
                    std::cerr << "DesignStudio: cannot add menu item: " << e.what() << std::endl;
                }
            }
        }

        inline void populate_toolbar(toolbar& aToolbar, i_element& aToolbarElement)
        {
            for (auto const& item : items_of(aToolbarElement))
            {
                try
                {
                    if (item.element == nullptr)
                    {
                        if (item.reference == "separator")
                            aToolbar.add_separator();
                        else
                            aToolbar.add_action(referenced_action(aToolbarElement.project(), item.reference));
                    }
                    else if (item.element->group() == element_group::Action)
                        aToolbar.add_action(make_action(attributes_of(*item.element)));
                }
                catch (std::exception const& e)
                {
                    std::cerr << "DesignStudio: cannot add toolbar item: " << e.what() << std::endl;
                }
            }
        }
    }

    template <typename BaseType>
    class element_variant
    {
    public:
        typedef BaseType base_type;
    };

    template <typename Type, typename Base = typename element_traits<Type>::base>
    class element : public neolib::reference_counted<Base>
    {
        typedef neolib::reference_counted<Base> base_type;
    public:
        define_declared_event(ModeChanged, mode_changed)
        define_declared_event(SelectionChanged, selection_changed)
        define_declared_event(ContextMenu, context_menu, i_menu&)
        define_declared_event(AttributesChanged, attributes_changed)
    public:
        using typename i_element::no_parent;
        using typename i_element::no_layout_item;
        using typename i_element::no_caddy;
        using typename i_element::no_child_layout;
    public:
        typedef maybe_abstract_t<base_type> abstract_type;
        typedef neolib::vector<ref_ptr<i_element>> children_t;
        typedef neolib::vector<neolib::pair<string, string>> attributes_t;
    public:
        element(i_element_library const& aLibrary, i_project& aProject, i_string const& aType, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aProject },
            iParent { nullptr }, 
            iGroup{ aGroup }, 
            iType{ aType }
        {
        }
        element(i_element_library const& aLibrary, i_project& aProject, i_string const& aType, i_string const& aId, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aProject },
            iParent{ nullptr },
            iGroup{ aGroup }, 
            iType{ aType }, 
            iId{ aId }
        {
        }
        element(i_element_library const& aLibrary, i_element& aParent, i_string const& aType, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aParent.project() },
            iParent{ &aParent },
            iGroup{ aGroup }, 
            iType{ aType }
        {
            parent().add_child(*this);
        }
        element(i_element_library const& aLibrary, i_element& aParent, i_string const& aType, i_string const& aId, element_group aGroup = default_element_group<Type>()) :
            iLibrary{ aLibrary }, 
            iProject{ aParent.project() },
            iParent{ &aParent },
            iGroup{ aGroup },
            iType{ aType }, 
            iId{ aId }
        {
            parent().add_child(*this);
        }
        ~element()
        {
        }
    public:
        i_element_library const& library() const override
        {
            return iLibrary;
        }
        i_project& project() const override
        {
            return iProject;
        }
        element_group group() const override
        {
            return iGroup;
        }
        neolib::i_string const& type() const override
        {
            return iType;
        }
        neolib::i_string const& id() const override
        {
            return iId;
        }
    public:
        i_element const& root() const override
        {
            i_element const* e = this;
            while (e->has_parent())
                e = &e->parent();
            return *e;
        }
        i_element& root() override
        {
            return const_cast<i_element&>(to_const(*this).root());
        }
        bool has_parent() const override
        {
            return iParent != nullptr;
        }
        i_element const& parent() const override
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }
        i_element& parent() override
        {
            return const_cast<i_element&>(to_const(*this).parent());
        }
        void set_parent(i_element& aParent) override
        {
            iParent = &aParent;
        }
        children_t const& children() const override
        {
            return iChildren;
        }
        children_t& children() override
        {
            return iChildren;
        }
        void add_child(i_element& aChild) override
        {
            if (aChild.group() != element_group::App || group() != element_group::UserInterface)
                children().push_back(neolib::ref_ptr<i_element>{ &aChild });
            else
            {
                aChild.set_parent(parent());
                parent().children().insert(std::prev(parent().children().end()), neolib::ref_ptr<i_element>{ &aChild });
            }
        }
        void remove_child(i_element& aChild) override
        {
            auto existing = std::find_if(children().begin(), children().end(), [&](auto&& e) { return &*e == &aChild; });
            if (existing != children().end())
                children().erase(existing);
        }
    public:
        attributes_t const& attributes() const override
        {
            return iAttributes;
        }
        attributes_t& attributes() override
        {
            return iAttributes;
        }
    public:
        void create_default_children() override
        {
            DesignStudio::create_default_children<Type>(*this);
        }
    public:
        bool needs_caddy() const override
        {
            return element_traits<Type>::needsCaddy;
        }
        bool has_caddy() const override
        {
            return iCaddy != nullptr;
        }
        i_element_caddy& caddy() const override
        {
            if (has_caddy())
                return *iCaddy;
            throw no_caddy();
        }
        void set_caddy(i_element_caddy& aCaddy) override
        {
            iCaddy = aCaddy;
        }
        bool has_layout_item() const override
        {
            return iLayoutItem != nullptr;
        }
        void create_layout_item(i_widget& aParent) override
        {
            if (!iLayoutItem)
            {
                if constexpr (std::is_base_of_v<i_tab_page, Type>)
                {
                    if (has_parent() && parent().has_layout_item())
                        parent().create_child_layout_item(*this, iLayoutItem);
                    if (iLayoutItem)
                    {
                        // clicking the page's tab on the design surface makes the page the current element
                        iSink = static_cast<Type&>(iLayoutItem->as_widget()).selected([this]()
                        {
                            if (!sRevealing)
                                set_mode(element_mode::Edit);
                        });
                        // double clicking the page's tab edits its text in place (tab pages have no caddy of their own so the container's is used)
                        iSink += static_cast<tab_button&>(static_cast<Type&>(iLayoutItem->as_widget()).tab()).DoubleClicked([this]()
                        {
                            if (has_parent() && parent().has_caddy())
                                parent().caddy().begin_text_edit(*this);
                        });
                    }
                }
                else if constexpr (std::is_base_of_v<i_widget, Type>)
                {
                    if constexpr (std::is_base_of_v<menu_bar, Type>)
                        iLayoutItem = make_ref<Type>(aParent); // (a menu bar needs a root when constructed)
                    else if constexpr (std::is_constructible_v<Type, i_element&>)
                        iLayoutItem = make_ref<Type>(*this);
                    else if constexpr (std::is_constructible_v<Type, i_widget&, window_style>)
                        iLayoutItem = make_ref<Type>(aParent, (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_element&, i_widget&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, aParent, (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_widget&, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(aParent, iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_element&, i_widget&, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, aParent, iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_widget&, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(aParent, iId.to_std_string());
                    else if constexpr (std::is_constructible_v<Type, i_element&, i_widget&, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(*this, aParent, iId.to_std_string());
                    else if constexpr (std::is_constructible_v<Type, window_style>)
                        iLayoutItem = make_ref<Type>((window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_element&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, i_widget&, neolib::i_string const&, window_style>)
                        iLayoutItem = make_ref<Type>(*this, iId.to_std_string(), (window_style::Default | window_style::Nested));
                    else if constexpr (std::is_constructible_v<Type, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(iId.to_std_string());
                    else if constexpr (std::is_constructible_v<Type, i_element&, neolib::i_string const&>)
                        iLayoutItem = make_ref<Type>(*this, iId.to_std_string());
                    else if constexpr (std::is_default_constructible_v<Type>)
                        iLayoutItem = make_ref<Type>();
                    else if constexpr (std::is_constructible_v<Type, i_standard_layout_container&>)
                    {
                        // e.g. a status bar: created for its (window) container which is its parent element's (nested, so root) widget
                        if (has_parent() && parent().has_layout_item() && parent().layout_item().is_widget() && parent().layout_item().as_widget().is_root())
                        {
                            i_standard_layout_container& container = parent().layout_item().as_widget().root();
                            iLayoutItem = make_ref<Type>(container);
                            // (it adds itself to the container's layout but on the design surface its caddy is what is in that layout)
                            if (iLayoutItem->has_parent_layout())
                                iLayoutItem->parent_layout().remove(*iLayoutItem);
                        }
                    }
                    else
                    {
                        // todo: widget creation for the other widget types
                    }
                    if (!iLayoutItem)
                        return;
                    if constexpr (std::is_base_of_v<i_tab_page_container, Type>)
                    {
                        // the element's widget ignores mouse events (the caddy handles them) but the tabs must remain clickable
                        static_cast<Type&>(iLayoutItem->as_widget()).tab_bar().as_widget().set_consider_ancestors_for_mouse_events(false);
                    }
                    if (std::is_same_v<Type, progress_bar>)
                    {
                        auto& progressBar = static_cast<progress_bar&>(iLayoutItem->as_widget());
                        progressBar.set_value(0.5);
                        progressBar.set_minimum(0.0);
                        progressBar.set_maximum(1.0);
                    }
                    if (iLayoutItem->is_widget() && iLayoutItem->as_widget().is_root() && iLayoutItem->as_widget().root().is_nested())
                        service<i_surface_manager>().nest_for(aParent, nest_type::Caddy).add(iLayoutItem->as_widget().root().native_window());
                }
                else if constexpr (std::is_base_of_v<i_layout, Type>)
                {
                    if constexpr (std::is_constructible_v<Type, i_element&>)
                        iLayoutItem = make_ref<Type>(*this);
                    else
                        iLayoutItem = make_ref<Type>();
                }
                else if constexpr (std::is_same_v<Type, spacer>)
                    iLayoutItem = make_ref<spacer>(static_cast<expansion_policy>(
                        static_cast<std::uint32_t>(expansion_policy::ExpandHorizontally) | static_cast<std::uint32_t>(expansion_policy::ExpandVertically)));
                else if constexpr (std::is_base_of_v<i_spacer, Type> && std::is_default_constructible_v<Type>)
                    iLayoutItem = make_ref<Type>();
            }
        }
        bool has_text() const override
        {
            // has visible text that can be edited in place
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return true; // its tab's text
            else if constexpr (std::is_base_of_v<i_widget, Type>)
                return requires(Type& aWidget, string const& aText) { aWidget.set_title_text(aText); } || 
                    requires(Type& aWidget, string const& aText) { aWidget.set_text(aText); };
            else
                return false;
        }
        i_string const& text_attribute() const override
        {
            static string const sTabText{ "tab_text" };
            static string const sTitle{ "title" };
            static string const sText{ "text" };
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return sTabText;
            else if constexpr (requires(Type& aWidget, string const& aText) { aWidget.set_title_text(aText); })
                return sTitle;
            else
                return sText;
        }
        i_widget& text_area() const override
        {
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return static_cast<tab_button&>(static_cast<Type&>(layout_item().as_widget()).tab()).label().text_widget(); // tab pages have tab_button tabs
            else if constexpr (std::is_base_of_v<i_widget, Type>)
            {
                auto& widget = static_cast<Type&>(layout_item().as_widget());
                if constexpr (requires(Type& aWidget) { aWidget.input_box(); })
                    return widget.input_box(); // text fields: their text is the input box's
                else if constexpr (requires(Type& aWidget) { aWidget.label().text_widget(); })
                    return widget.label().text_widget(); // e.g. buttons, group boxes
                else if constexpr (requires(Type& aWidget) { aWidget.text_widget(); })
                    return widget.text_widget(); // e.g. labels
                else if constexpr (requires(Type& aWidget) { aWidget.title_bar().title_widget(); })
                {
                    try
                    {
                        return widget.title_bar().title_widget(); // windows: their title
                    }
                    catch (std::logic_error const&)
                    {
                        return widget; // no title bar
                    }
                }
                else
                    return widget;
            }
            else
                return layout_item().as_widget();
        }
        neogfx::alignment text_alignment() const override
        {
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return static_cast<tab_button&>(static_cast<Type&>(layout_item().as_widget()).tab()).label().text_widget().alignment();
            else if constexpr (std::is_base_of_v<i_widget, Type>)
            {
                auto& widget = static_cast<Type&>(layout_item().as_widget());
                if constexpr (requires(Type& aWidget) { aWidget.input_box(); })
                    return widget.input_box().alignment();
                else if constexpr (requires(Type& aWidget) { aWidget.label().text_widget(); })
                    return widget.label().text_widget().alignment();
                else if constexpr (requires(Type& aWidget) { aWidget.text_widget(); })
                    return widget.text_widget().alignment();
                else if constexpr (requires(Type& aWidget) { aWidget.title_bar().title_widget(); })
                {
                    try
                    {
                        return widget.title_bar().title_widget().alignment();
                    }
                    catch (std::logic_error const&)
                    {
                        return neogfx::alignment::Left | neogfx::alignment::VCenter;
                    }
                }
                else if constexpr (requires(Type& aWidget) { { aWidget.alignment() } -> std::convertible_to<neogfx::alignment>; })
                    return widget.alignment(); // e.g. text widgets
                else
                    return neogfx::alignment::Left | neogfx::alignment::VCenter;
            }
            else
                return neogfx::alignment::Left | neogfx::alignment::VCenter;
        }
        void available_attributes(neolib::i_vector<i_string>& aResult) const override
        {
            // the attributes that can be added: those supported (see nrc_attributes::apply) not already added (by any of their names) 
            // except those associated with a property of the element's type (e.g. "SizePolicy"): editing the property (or one of its 
            // components, e.g. MaximumSize's Width or Palette's Base) sets those (see accepts_attribute)
            aResult.clear();
            std::set<std::string> associated;
            if constexpr (std::is_base_of_v<i_widget, Type> || std::is_base_of_v<i_layout, Type>)
            {
                std::vector<property_type_info> properties;
                collect_property_types<Type>(properties);
                for (auto const& property : properties)
                    associated.insert(nrc_attributes::attribute_name_of(property.name));
            }
            auto added = [&](std::string const& aName)
            {
                for (auto const& attribute : iAttributes)
                    if (attribute.first().to_std_string() == aName && !attribute.second().empty())
                        return true;
                return false;
            };
            std::vector<std::vector<std::string>> candidates;
            if (has_text())
                candidates.push_back({ text_attribute().to_std_string() });
            if constexpr (std::is_base_of_v<i_widget, Type> || std::is_base_of_v<i_layout, Type>)
            {
                auto const supported = nrc_attributes::supported<Type>();
                candidates.insert(candidates.end(), supported.begin(), supported.end());
            }
            for (auto const& names : candidates)
                if (std::none_of(names.begin(), names.end(), added) && 
                    std::none_of(names.begin(), names.end(), [&](std::string const& aName) { return associated.find(aName) != associated.end(); }))
                    aResult.push_back(string{ names[0] });
        }
        bool accepts_attribute(i_string const& aName) const override
        {
            // the attributes (with any of their names) that are applied (see nrc_attributes::apply), including those associated with a 
            // property (which aren't available to add, see available_attributes) so editing the property can set them
            auto const name = aName.to_std_string();
            if (has_text() && name == text_attribute().to_std_string())
                return true;
            if constexpr (std::is_base_of_v<i_widget, Type> || std::is_base_of_v<i_layout, Type>)
                for (auto const& names : nrc_attributes::supported<Type>())
                    if (std::find(names.begin(), names.end(), name) != names.end())
                        return true;
            return false;
        }
        void apply_attributes(bool aShowIds) override
        {
            // show the element's text (as in a running application) or, if requested, its id
            if (!has_layout_item())
                return;
            // visual attributes (colours, images, sizes, etc.); only when they have changed (e.g. not on every in-place text edit)
            std::vector<std::pair<std::string, std::string>> visual;
            for (auto const& attribute : iAttributes)
            {
                auto const name = attribute.first().to_std_string();
                if (!name.empty() && name[0] != '#' && name != "id" && name != "text" && name != "title" && name != "tab_text" && !attribute.second().empty())
                    visual.emplace_back(name, attribute.second().to_std_string());
            }
            if (iVisualDefaultsFor != &layout_item())
            {
                // (a different layout item: what was remembered about the previous one no longer applies)
                iVisualDefaults.clear();
                iAppliedVisualAttributes.clear();
                iVisualDefaultsFor = &layout_item();
            }
            // (also when resources have been added (e.g. by adding an .nrc file to the project) as images may now be available)
            auto const resourceCount = service<i_resource_manager>().resources().size();
            if (visual != iAppliedVisualAttributes || resourceCount != iAppliedResourceCount)
            {
                iAppliedResourceCount = resourceCount;
                iAppliedVisualAttributes = visual;
                // apply the current attributes then put back the defaults of those no longer applied (removed attributes); not putting back 
                // all the defaults first as that would change (and then change back) every applied attribute's value, e.g. laying out again 
                // when only a colour has changed
                nrc_attributes::restorers applied; // (what was applied; for those not applied before, how to put back their defaults)
                if constexpr (std::is_base_of_v<i_widget, Type>)
                    nrc_attributes::apply(static_cast<Type&>(layout_item().as_widget()), visual, false, applied);
                else if constexpr (std::is_base_of_v<i_layout, Type>)
                    nrc_attributes::apply(static_cast<Type&>(layout_item().as_layout()), visual, false, applied);
                for (auto existing = iVisualDefaults.begin(); existing != iVisualDefaults.end();)
                {
                    if (applied.find(existing->first) == applied.end())
                    {
                        try
                        {
                            existing->second();
                        }
                        catch (...) {}
                        existing = iVisualDefaults.erase(existing);
                    }
                    else
                        ++existing;
                }
                for (auto& restorer : applied)
                    iVisualDefaults.emplace(restorer.first, std::move(restorer.second)); // (the defaults already noted are kept)
            }
            // menu bar menus and toolbar buttons
            if constexpr (std::is_base_of_v<i_widget, Type> && (std::is_base_of_v<i_menu, Type> || std::is_base_of_v<toolbar, Type>))
            {
                // (also rebuilt when resources (and possibly actions) have been added, e.g. by adding an .nrc file to the project)
                std::string itemsSignature = std::to_string(service<i_resource_manager>().resources().size()) + ":";
                nrc_items::signature(*this, itemsSignature);
                if (itemsSignature != iItemsSignature)
                {
                    auto& widget = static_cast<Type&>(layout_item().as_widget());
                    if constexpr (std::is_base_of_v<i_menu, Type>)
                    {
                        iItemsSignature = itemsSignature;
                        i_menu& menu = widget;
                        while (menu.count() != 0u)
                            menu.remove_at(menu.count() - 1u);
                        nrc_items::populate_menu(menu, *this);
                    }
                    else
                    {
                        iItemsSignature = itemsSignature;
                        widget.remove_all_buttons();
                        nrc_items::populate_toolbar(widget, *this);
                    }
                }
            }
            if constexpr (std::is_base_of_v<i_widget, Type>)
            {
                auto& widget = static_cast<Type&>(layout_item().as_widget());
                string const text{ aShowIds ? iId.to_std_string() : design_text() };
                if constexpr (std::is_base_of_v<i_tab_page, Type>)
                    widget.tab().set_text(text);
                else if constexpr (requires(Type& aWidget, string const& aText) { aWidget.set_title_text(aText); })
                {
                    // (as at runtime: a window without a title is captioned with the app's name)
                    string title = text;
                    if (title.empty() && !aShowIds)
                        project().root().visit([&](i_element& aElement)
                        {
                            if (title.empty() && aElement.group() == element_group::App)
                                title = string{ nrc_items::attribute(nrc_items::attributes_of(aElement), "name") };
                        });
                    widget.set_title_text(title);
                }
                else if constexpr (requires(Type& aWidget, string const& aText) { aWidget.set_text(aText); })
                    widget.set_text(text);
            }
        }
        i_layout_item& layout_item() const override
        {
            if (iLayoutItem != nullptr)
                return *iLayoutItem;
            throw no_layout_item();
        }
        bool has_child_layout() const override
        {
            if (!has_layout_item())
                return false;
            if constexpr (std::is_base_of_v<i_layout, Type>)
                return true;
            else if constexpr (std::is_base_of_v<i_widget, Type> && std::is_base_of_v<i_standard_layout_container, Type>)
                return true;
            else if constexpr (std::is_base_of_v<group_box, Type>)
                return true;
            else if constexpr (std::is_base_of_v<i_tab_page, Type>)
                return true;
            else
                return false;
        }
        i_layout& child_layout(neolib::i_string const& aChildType) const override
        {
            if (has_child_layout())
            {
                if constexpr (std::is_base_of_v<i_layout, Type>)
                    return layout_item().as_layout();
                else if constexpr (std::is_base_of_v<i_widget, Type> && std::is_base_of_v<i_standard_layout_container, Type>)
                {
                    i_standard_layout_container& container = static_cast<Type&>(layout_item().as_widget());
                    if (aChildType.to_std_string_view() == "menu_bar" && container.has_layout(standard_layout::Menu))
                        return container.menu_layout();
                    else if (aChildType.to_std_string_view() == "toolbar" && container.has_layout(standard_layout::Toolbar))
                        return container.toolbar_layout();
                    else if (aChildType.to_std_string_view() == "status_bar" && container.has_layout(standard_layout::StatusBar))
                        return container.status_bar_layout();
                    return container.client_layout();
                }
                else if constexpr (std::is_base_of_v<group_box, Type>)
                    return static_cast<Type&>(layout_item().as_widget()).item_layout();
                else if constexpr (std::is_base_of_v<i_tab_page, Type>)
                {
                    auto& page = layout_item().as_widget();
                    if (!page.has_layout())
                        page.set_layout(ref_ptr<i_layout>{ make_ref<vertical_layout>() });
                    return page.layout();
                }
            }
            throw no_child_layout();
        }
        void create_child_layout_item(i_element const& aChild, i_ref_ptr<i_layout_item>& aResult) override
        {
            if constexpr (std::is_base_of_v<i_widget, Type> && std::is_base_of_v<i_tab_page_container, Type>)
            {
                if (has_layout_item())
                {
                    std::string const tabText = aChild.id().to_std_string(); // design view shows ids (see apply_attributes)
                    auto& page = static_cast<Type&>(layout_item().as_widget()).add_tab_page(string{ tabText });
                    // the container owns the page so the element's reference is non-owning
                    aResult = ref_ptr<i_layout_item>{ ref_ptr<i_layout_item>{}, static_cast<i_layout_item*>(&page.as_widget()) };
                    return;
                }
            }
            throw no_child_layout();
        }
        void reveal() override
        {
            // make the element visible on the design surface (e.g. select the tab page it is on)
            if constexpr (std::is_base_of_v<i_tab_page, Type>)
            {
                if (has_layout_item())
                {
                    auto& tab = static_cast<Type&>(layout_item().as_widget()).tab();
                    if (!tab.is_selected())
                    {
                        neolib::scoped_flag sf{ sRevealing };
                        tab.select();
                    }
                }
            }
            if (has_parent())
                parent().reveal();
        }
    public:
        element_mode mode() const override
        {
            return iMode;
        }
        void set_mode(element_mode aMode) override
        {
            if (iMode != aMode)
            {
                iMode = aMode;
                if (mode() == element_mode::Edit)
                {
                    root().visit([&](i_element& aElement)
                    {
                        if (&aElement != this && aElement.mode() == element_mode::Edit)
                            aElement.set_mode(element_mode::None);
                    });
                }
                if (mode() == element_mode::Edit)
                    reveal();
                ModeChanged();
            }
        }
        bool is_selected() const override
        {
            return iSelected;
        }
        void select(bool aSelected = true, bool aDeselectRest = true) override
        {
            if (iSelected != aSelected)
            {
                iSelected = aSelected;
                if (iSelected)
                    reveal();
                SelectionChanged();
            }
            if (aDeselectRest)
            {
                root().visit([&](i_element& aElement)
                {
                    if (&aElement != this && aElement.is_selected())
                        aElement.select(false, false);
                });
            }
        }
    private:
        // value of the element's text attribute (or title/tab text) as plain text
        std::string design_text() const
        {
            for (std::string_view const name : { "text", "title", "tab_text" })
                for (auto const& attribute : iAttributes)
                    if (attribute.first().to_std_string_view() == name)
                    {
                        auto const value = attribute.second().to_std_string_view();
                        if (value.size() < 2u || value.front() != '"' || value.back() != '"')
                            return std::string{ value };
                        std::string result;
                        for (std::size_t i = 1u; i + 1u < value.size(); ++i)
                        {
                            char ch = value[i];
                            if (ch == '\\' && i + 2u < value.size())
                            {
                                ch = value[++i];
                                if (ch == 'n')
                                    ch = '\n';
                                else if (ch == 't')
                                    ch = '\t';
                                else if (ch == 'r')
                                    ch = '\r';
                            }
                            result += ch;
                        }
                        return result;
                    }
            return {};
        }
    private:
        const i_element_library& iLibrary;
        i_project& iProject;
        i_element* iParent;
        element_group iGroup;
        string iType;
        string iId;
        children_t iChildren;
        attributes_t iAttributes;
        std::vector<std::pair<std::string, std::string>> iAppliedVisualAttributes; // the visual attributes last applied to the layout item
        nrc_attributes::restorers iVisualDefaults; // how to put back the layout item's defaults for the visual attributes applied
        i_layout_item const* iVisualDefaultsFor = nullptr;
        std::size_t iAppliedResourceCount = 0u;
        std::string iItemsSignature; // of the element subtree the (menu bar or toolbar) items were last built from
        sink iSink;
        inline static bool sRevealing = false;
        mutable ref_ptr<i_layout_item> iLayoutItem;
        ref_ptr<i_element_caddy> iCaddy;
        element_mode iMode = element_mode::None;
        bool iSelected = false;
    };
}
