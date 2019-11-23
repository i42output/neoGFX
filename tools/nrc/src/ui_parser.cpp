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
        iRoot{ aRoot }, iOutput{ aOutput }, iNamespace{ aNamespace }, iCurrentNode{ nullptr }, iAnonymousIdCounter{ 0u }
    {
        for (auto const& plugin : aPluginManager.plugins())
        {
            neolib::ref_ptr<i_ui_element_library> library{ *plugin };
            if (library != nullptr)
                iLibraries.push_back(library);
        }

        for (auto const& node : aRoot.contents())
            parse(node);

        emit(
            "// This is an automatically generated file, do not edit!\n"
            "\n"
            "%1%\n"
            "\n"
            "namespace%2%\n"
            "{\n"
            " using namespace neogfx;\n"
            " using namespace neogfx::unit_literals;\n"
            "\n"
            " struct ui\n"
            " {\n", headers(), (!aNamespace.empty() ? " " + aNamespace : ""));

        for (auto const& element : iRootElements)
            element->emit();

        emit(" };\n"
            "}\n");
    }

    const neolib::i_string& ui_parser::element_namespace() const
    {
        return iNamespace;
    }

    void ui_parser::generate_anonymous_id(neolib::i_string& aNewAnonymousId) const
    {
        aNewAnonymousId = neolib::string { element_namespace() + "_" + boost::lexical_cast<std::string>(++iAnonymousIdCounter) };
    }

    void ui_parser::indent(int32_t aLevel, neolib::i_string& aResult) const
    {
        aResult = std::string(aLevel * 4, ' '); // todo: make indentation configurable
    }

    bool ui_parser::do_data_exists(const neolib::i_string& aKey) const
    {
        return iCurrentNode->as<neolib::fjson_object>().has(aKey.to_std_string());
    }

    const ui_parser::data_t& ui_parser::do_get_data(const neolib::i_string& aKey) const
    {
        if (!data_exists(aKey))
            throw element_data_not_found(aKey.to_std_string());
        auto const& value = iCurrentNode->as<neolib::fjson_object>().at(aKey.to_std_string());
        auto& data = iDataCache[std::make_pair(iCurrentNode, aKey.to_std_string())];
        value.visit([&data](auto&& v)
        {
            typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> vt;
            if constexpr (std::is_same_v<vt, bool>)
                data = neolib::simple_variant{ v };
            else if constexpr (std::is_integral_v<vt>)
                data = neolib::simple_variant{ static_cast<int64_t>(v) };
            else if constexpr (std::is_floating_point_v<vt>)
                data = neolib::simple_variant{ static_cast<double>(v) };
            else if constexpr (std::is_same_v<vt, neolib::fjson_string>)
                data = neolib::simple_variant{ neolib::string{v} };
            else if constexpr (std::is_same_v<vt, neolib::fjson_keyword>)
                data = neolib::simple_variant{ neolib::string{v.text} };
        });
        return data;
    }

    ui_parser::data_t& ui_parser::do_get_data(const neolib::i_string& aKey)
    {
        return const_cast<ui_parser::data_t&>(to_const(*this).do_get_data(aKey));
    }
        
    const ui_parser::array_data_t& ui_parser::do_get_array_data(const neolib::i_string& aKey) const
    {
        if (!data_exists(aKey))
            throw element_data_not_found(aKey.to_std_string());
        auto const& value = iCurrentNode->as<neolib::fjson_object>().at(aKey.to_std_string()).as<neolib::fjson_array>();
        auto& arrayData = iArrayDataCache[std::make_pair(iCurrentNode, aKey.to_std_string())];
        for(auto const& e : value.contents())
            e.visit([&arrayData](auto&& v)
            {
                typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> vt;
                data_t data;
                if constexpr (std::is_same_v<vt, bool>)
                    data = neolib::simple_variant{ v };
                else if constexpr (std::is_integral_v<vt>)
                    data = neolib::simple_variant{ static_cast<int64_t>(v) };
                else if constexpr (std::is_floating_point_v<vt>)
                    data = neolib::simple_variant{ static_cast<double>(v) };
                else if constexpr (std::is_same_v<vt, neolib::fjson_string>)
                    data = neolib::simple_variant{ neolib::string{v} };
                else if constexpr (std::is_same_v<vt, neolib::fjson_keyword>)
                    data = neolib::simple_variant{ neolib::string{v.text} };
                arrayData.push_back(data);
            });
        return arrayData;
    }

    ui_parser::array_data_t& ui_parser::do_get_array_data(const neolib::i_string& aKey)
    {
        return const_cast<ui_parser::array_data_t&>(to_const(*this).do_get_array_data(aKey));
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

    std::string ui_parser::headers() const
    {
        std::set<std::string> result;
        for (auto const& e : iRootElements)
            next_header(*e, result);
        std::string resultString = "#include <neogfx/neogfx.hpp>";
        for (auto const& h : result)
            if (!h.empty())
                resultString += ("\n#include <" + h + ">");
        return resultString;
    }

    void ui_parser::next_header(const i_ui_element& aElement, std::set<std::string>& aHeaders) const
    {
        aHeaders.insert(aElement.header().to_std_string());
        for (auto const& e : aElement.children())
            next_header(*e, aHeaders);
    }

    neolib::ref_ptr<i_ui_element> ui_parser::create_element(const neolib::i_string& aElementType)
    {
        for (auto const& library : iLibraries)
            if (library->handles_element(aElementType))
                return neolib::ref_ptr<i_ui_element>{ library->create_element(*this, aElementType) };
        throw element_type_not_found(aElementType.to_std_string());
    }

    neolib::ref_ptr<i_ui_element> ui_parser::create_element(i_ui_element& aParent, const neolib::i_string& aElementType)
    {
        for (auto const& library : iLibraries)
            if (library->handles_element(aParent, aElementType))
                return neolib::ref_ptr<i_ui_element>{ library->create_element(*this, aParent, aElementType) };
        throw element_type_not_found(aElementType.to_std_string(), aParent.id().to_std_string());
    }

    void ui_parser::parse(const neolib::fjson_value& aNode)
    {
        iCurrentNode = &aNode;
        try
        {
            auto element = create_element(neolib::string{ aNode.name() });
            iRootElements.push_back(element);
            parse(aNode, *element);
        }
        catch (element_type_not_found& e)
        {
            #ifndef IGNORE_UNKNOWN_ELEMENTS
            (void)e;
            throw;
            #else
            std::cerr << "error: nrc: " << e.what() << std::endl;
            #endif
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
                {
                    iCurrentNode = &e;
                    try
                    {
                        neolib::string elementName{ e.name() };
                        if (!aElement.consume_element(elementName))
                            parse(e, *create_element(aElement, elementName));
                    }
                    catch (element_type_not_found& e)
                    {
                        #ifndef IGNORE_UNKNOWN_ELEMENTS
                        (void)e;
                        throw;
                        #else
                        std::cerr << "error: nrc: " << e.what() << std::endl;
                        #endif
                    }
                }
                else if (e.name() != "id")
                    parse(e, aElement);
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
            aElement.parse(neolib::string{ aNode.name() }, data_t{ neolib::string{ aNode.as<neolib::fjson_string>() } });
            break;
        case neolib::json_type::Bool:
            break;
        case neolib::json_type::Null:
            break;
        case neolib::json_type::Keyword:
            aElement.parse(neolib::string{ aNode.name() }, data_t{ neolib::string{ aNode.as<neolib::fjson_keyword>().text } });
            break;
        }
    }
}