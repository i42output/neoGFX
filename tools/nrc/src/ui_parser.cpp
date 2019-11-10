// ui_parser.cpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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

#include <neogfx/neogfx.hpp>
#include <boost/format.hpp>

#include "ui_parser.hpp"

namespace neogfx::nrc
{
    ui_parser::ui_parser(const neolib::i_plugin_manager& aPluginManager, const neolib::fjson_string& aNamespace, const neolib::fjson_object& aRoot, std::ofstream& aOutput) :
        iRoot{ aRoot }, iOutput{ aOutput }, iCurrentNode{ nullptr }
    {
        for (auto const& plugin : aPluginManager.plugins())
        {
            neolib::ref_ptr<i_ui_element_library> library{ *plugin };
            if (library != nullptr)
                iLibraries.push_back(library);
        }

        aOutput << boost::format(
            "// This is an automatically generated file, do not edit!\n"
            "\n"
            "namespace%2%\n"
            "{\n"
            "%1%""struct ui\n"
            "%1%""{\n") % indent(-1) % (!aNamespace.empty() ? " " + aNamespace : "");

        for (auto const& node : aRoot.contents())
            parse(node);

        aOutput <<
            indent(-1) << "};\n"
            "}\n";
    }

    void ui_parser::indent(int32_t aLevel, neolib::i_string& aResult) const
    {
        aResult = std::string((aLevel + 2) * 4, ' '); // todo: make indentation configurable
    }

    void ui_parser::current_object_data(const neolib::i_string& aKey, neolib::i_simple_variant& aData) const
    {
        auto const& value = iCurrentNode->as<neolib::fjson_object>().at(aKey.to_std_string());
        auto& data = aData;
        value.visit([&data](auto&& v)
        {
            typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> vt;
            if constexpr (std::is_integral_v<vt>)
                data = neolib::simple_variant{ static_cast<int64_t>(v) };
            else if constexpr (std::is_floating_point_v<vt>)
                data = neolib::simple_variant{ static_cast<double>(v) };
            else if constexpr (std::is_same_v<vt, bool>)
                data = neolib::simple_variant{ v };
            else if constexpr (std::is_same_v<vt, neolib::fjson_string>)
                data = neolib::simple_variant{ neolib::string{v} };
            else if constexpr (std::is_same_v<vt, neolib::fjson_keyword>)
                data = neolib::simple_variant{ neolib::string{v.text} };
        });
    }

    void ui_parser::emit(const neolib::i_string& aText) const
    {
        iOutput << aText.to_std_string_view();
    }

    void ui_parser::parse(const neolib::fjson_value& aNode)
    {
        iCurrentNode = &aNode;
        neolib::string const elementType{ aNode.name() };
        for (auto const& library : iLibraries)
            if (library->elements().find(elementType) != library->elements().end())
            {
                iElements.push_back(library->create_element(*this, elementType));
                auto& element = *iElements.back();
                parse(aNode, element);
            }
    }

    void ui_parser::parse(const neolib::fjson_value& aNode, i_ui_element& aElement)
    {
        iCurrentNode = &aNode;
        switch (aNode.type())
        {
        case neolib::json_type::Object:
            for (auto const& e : aNode.as<neolib::fjson_object>().contents())
                if (e.type() == neolib::json_type::Object)
                    parse(e);
            break;
        case neolib::json_type::Array:
            break;
        case neolib::json_type::Double:
            break;
        case neolib::json_type::Int64:
            break;
        case neolib::json_type::Uint64:
            break;
        case neolib::json_type::Int:
            break;
        case neolib::json_type::Uint:
            break;
        case neolib::json_type::String:
            break;
        case neolib::json_type::Bool:
            break;
        case neolib::json_type::Null:
            break;
        case neolib::json_type::Keyword:
            break;
        }
    }

    neolib::ref_ptr<i_ui_element> ui_parser::create_object(const neolib::fjson_value& aNode, neolib::ref_ptr<i_ui_element> aParent)
    {
        neolib::string const elementType{ aNode.name() };
        for (auto const& library : iLibraries)
            if (library->elements().find(elementType) != library->elements().end())
            {
                iElements.push_back(library->create_element(*this, elementType));
                return iElements.back();
            }
        throw element_type_not_found(elementType.to_std_string());
    }
}