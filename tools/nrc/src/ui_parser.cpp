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

        emit(neolib::string{ (boost::format(
            "// This is an automatically generated file, do not edit!\n"
            "\n"
            "namespace%1%\n"
            "{\n"
            " struct ui\n"
            " {\n") % (!aNamespace.empty() ? " " + aNamespace : "")).str() });

        for (auto const& node : aRoot.contents())
            parse(node);

        for (auto const& element : iRootElements)
            element->emit();

        emit(neolib::string{ (boost::format(
            " };\n"
            "}\n")).str() });
    }

    void ui_parser::indent(int32_t aLevel, neolib::i_string& aResult) const
    {
        aResult = std::string(aLevel * 4, ' '); // todo: make indentation configurable
    }

    void ui_parser::current_object_data(const neolib::i_string& aKey, data_t& aData) const
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
        bool newLine = true;
        for (auto const& ch : aText)
        {
            if (ch == ' ' && newLine)
                iOutput << indent(1);
            else
            {
                newLine = false;
                iOutput << ch;
            }
            if (ch == '\n')
                newLine = true;
        }
    }

    neolib::ref_ptr<i_ui_element> ui_parser::create_element(const neolib::i_string& aElementType)
    {
        for (auto const& library : iLibraries)
            if (library->elements().find(aElementType) != library->elements().end())
                return neolib::ref_ptr<i_ui_element>{ library->create_element(*this, aElementType) };
        throw element_type_not_found(aElementType.to_std_string());
    }

    neolib::ref_ptr<i_ui_element> ui_parser::create_element(i_ui_element& aParent, const neolib::i_string& aElementType)
    {
        for (auto const& library : iLibraries)
            if (library->elements().find(aElementType) != library->elements().end())
                return neolib::ref_ptr<i_ui_element>{ library->create_element(*this, aParent, aElementType) };
        throw element_type_not_found(aElementType.to_std_string());
    }

    void ui_parser::parse(const neolib::fjson_value& aNode)
    {
        iCurrentNode = &aNode;
        auto element = create_element(neolib::string{ aNode.name() });
        iRootElements.push_back(element);
        parse(aNode, *element);
    }

    void ui_parser::parse(const neolib::fjson_value& aNode, i_ui_element& aElement)
    {
        iCurrentNode = &aNode;
        switch (aNode.type())
        {
        case neolib::json_type::Object:
            for (auto const& e : aNode.as<neolib::fjson_object>().contents())
                if (e.type() == neolib::json_type::Object)
                    parse(e, *create_element(aElement, neolib::string{ e.name() }));
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
}