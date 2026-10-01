// project.cpp
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <charconv>
#include <cstdio>
#include <cctype>
#include <boost/lexical_cast.hpp>

#include <neolib/file/json.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/i_project_manager.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>

namespace neogfx::DesignStudio
{
    namespace
    {
        typedef neolib::pair<string, string> attribute_t;

        std::string indent(std::size_t aLevel)
        {
            return std::string(aLevel * 4u, ' ');
        }

        std::string reindent(std::string_view aText, std::size_t aLevel)
        {
            std::string result;
            for (auto ch : aText)
            {
                result += ch;
                if (ch == '\n')
                    result += indent(aLevel);
            }
            return result;
        }

        std::string quoted(std::string_view aText)
        {
            std::string result = "\"";
            for (auto ch : aText)
                switch (ch)
                {
                case '\"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(ch) >= 32u)
                        result += ch;
                    else
                    {
                        char buffer[8];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(ch)));
                        result += buffer;
                    }
                }
            result += '\"';
            return result;
        }

        std::string name_to_rjson(neolib::fjson_value const& aValue)
        {
            auto const& name = aValue.name();
            bool plain = aValue.name_is_keyword() && !name.empty();
            if (!plain)
            {
                plain = !name.empty();
                for (auto ch : name)
                    if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_' && ch != '.' && ch != '$')
                        plain = false;
            }
            return plain ? std::string{ name.begin(), name.end() } : quoted(std::string_view{ name.data(), name.size() });
        }

        // serialize a parsed .nrc (functional RJSON) value back to .nrc text (relative indentation level 0)
        std::string value_to_rjson(neolib::fjson_value const& aValue, std::size_t aLevel = 0)
        {
            std::ostringstream output;
            switch (aValue.type())
            {
            case neolib::json_type::Object:
                output << "{";
                if (!aValue.empty())
                {
                    output << "\n";
                    for (auto const& child : aValue)
                        output << indent(aLevel + 1u) << name_to_rjson(child) << ": " << value_to_rjson(child, aLevel + 1u) << "\n";
                    output << indent(aLevel);
                }
                output << "}";
                break;
            case neolib::json_type::Array:
                {
                    output << "[";
                    bool first = true;
                    for (auto const& child : aValue)
                    {
                        output << (first ? " " : " ") << value_to_rjson(child, aLevel);
                        first = false;
                    }
                    output << (first ? "]" : " ]");
                }
                break;
            case neolib::json_type::Double:
                {
                    char buffer[64];
                    auto const result = std::to_chars(std::begin(buffer), std::end(buffer), aValue.as<double>());
                    std::string text{ buffer, result.ptr };
                    if (text.find_first_of(".eEni") == std::string::npos)
                        text += ".0";
                    output << text;
                }
                break;
            case neolib::json_type::Int64:
            case neolib::json_type::Uint64:
            case neolib::json_type::Int:
            case neolib::json_type::Uint:
                std::visit([&](auto const& v)
                {
                    using type = std::decay_t<decltype(v)>;
                    if constexpr (std::is_integral_v<type> && !std::is_same_v<type, bool>)
                        output << v;
                }, *aValue);
                break;
            case neolib::json_type::String:
                output << quoted(std::string_view{ aValue.text().data(), aValue.text().size() });
                break;
            case neolib::json_type::Bool:
                output << (std::get<neolib::fjson_bool>(*aValue) ? "true" : "false");
                break;
            case neolib::json_type::Null:
                output << "null";
                break;
            case neolib::json_type::Keyword:
                output << aValue.text();
                break;
            default:
                break;
            }
            return output.str();
        }

        bool is_metadata(neolib::i_string const& aName)
        {
            return !aName.empty() && aName.to_std_string_view()[0] == '#';
        }

        bool is_saved_to_nrc(i_element const& aElement)
        {
            switch (aElement.group())
            {
            case element_group::App:
            case element_group::Menu:
            case element_group::Action:
            case element_group::Widget:
            case element_group::Layout:
                return true;
            default:
                return false;
            }
        }

        void emit_element(std::ostream& aOutput, i_element const& aElement, std::size_t aLevel)
        {
            aOutput << indent(aLevel) << aElement.type().to_std_string_view() << ": {\n";
            auto nextChild = aElement.children().begin();
            auto emit_next_child = [&]()
            {
                while (nextChild != aElement.children().end())
                {
                    auto const& child = **nextChild++;
                    if (is_saved_to_nrc(child))
                    {
                        emit_element(aOutput, child, aLevel + 1u);
                        return;
                    }
                }
            };
            for (auto const& attribute : aElement.attributes())
            {
                if (attribute.first().to_std_string_view() == "#child")
                    emit_next_child();
                else if (!is_metadata(attribute.first()) && !attribute.second().empty())
                    aOutput << indent(aLevel + 1u) << attribute.first().to_std_string_view() << ": " << 
                        reindent(attribute.second().to_std_string_view(), aLevel + 1u) << "\n";
            }
            while (nextChild != aElement.children().end())
                emit_next_child();
            aOutput << indent(aLevel) << "}\n";
        }

        std::string fragment_name(i_element const& aElement)
        {
            for (auto const& attribute : aElement.attributes())
                if (attribute.first().to_std_string_view() == "#fragment")
                    return attribute.second().to_std_string();
            return aElement.id().to_std_string() + "_ui";
        }
    }

    project::project(i_project_manager& aManager) :
        iManager{ aManager }
    {
    }

    i_project_manager& project::manager() const
    {
        return iManager;
    }

    void project::create(const i_string& aName, const i_string& aNamespace)
    {
        iName = aName;
        iNamespace = aNamespace;
        iRoot = manager().library("project"_s).create_element(*this, "project", aName.to_std_string());
    }

    void project::open(const i_string& aPath)
    {
        std::filesystem::path const inputFileName{ aPath.to_std_string() };
        neolib::fjson const input{ inputFileName.string() };
        if (!input.has_root())
            throw invalid_project_file("bad root node");
        iPath = aPath;
        iName = inputFileName.stem().string();
        iNamespace = input.root().as<neolib::fjson_object>().has("namespace") ? input.root().as<neolib::fjson_object>().at("namespace").text() : "";
        iRoot = manager().library("project"_s).create_element(*this, "project", inputFileName.stem().string());
        auto userInterface = manager().library("user_interface"_s).create_element(*iRoot, "user_interface", "User Interface"_t);
        std::map<std::string, std::uint32_t> counters;
        // create an element for an .nrc object node; returns nullptr if the node isn't a known element type
        auto create_node_element = [&](i_element& aParent, neolib::fjson_value const& aNode) -> i_element*
        {
            try
            {
                std::string const type{ aNode.name().begin(), aNode.name().end() };
                std::string const id = aNode.as<neolib::fjson_object>().has("id") ? 
                    std::string{ aNode.as<neolib::fjson_object>().at("id").text().begin(), aNode.as<neolib::fjson_object>().at("id").text().end() } :
                    type + boost::lexical_cast<std::string>(++counters[type]);
                return &create_element(aParent, string{ type }, string{ id });
            }
            catch (...)
            {
            }
            return nullptr;
        };
        // element attributes (and unknown object nodes) are preserved in document order so the file round trips
        std::function<void(i_element&, neolib::fjson_value const&)> add_node = [&](i_element& aElement, neolib::fjson_value const& aNode)
        {
            for (auto const& child : aNode)
            {
                if (child.type() == neolib::json_type::Object)
                {
                    auto newElement = create_node_element(aElement, child);
                    if (newElement != nullptr)
                    {
                        aElement.attributes().push_back(attribute_t{ string{ "#child" }, string{ child.name() } });
                        add_node(*newElement, child);
                        continue;
                    }
                }
                aElement.attributes().push_back(attribute_t{ string{ name_to_rjson(child) }, string{ value_to_rjson(child) } });
            }
        };
        for (auto const& item : input.root())
        {
            if (item.name() == "ui" && item.type() == neolib::json_type::Object)
            {
                iRoot->attributes().push_back(attribute_t{ string{ "#ui" }, string{} });
                for (auto const& fragment : item)
                {
                    if (fragment.type() != neolib::json_type::Object)
                        continue;
                    for (auto const& node : fragment)
                    {
                        if (node.type() != neolib::json_type::Object)
                            continue;
                        auto newElement = create_node_element(*userInterface, node);
                        if (newElement != nullptr)
                        {
                            newElement->attributes().push_back(attribute_t{ string{ "#fragment" }, string{ fragment.name() } });
                            add_node(*newElement, node);
                        }
                    }
                }
            }
            else
                iRoot->attributes().push_back(attribute_t{ string{ name_to_rjson(item) }, string{ value_to_rjson(item) } });
        }
        set_clean();
    }

    void project::save(const i_string& aPath)
    {
        std::ostringstream output;
        output << "{\n";
        bool first = true;
        auto separate = [&]()
        {
            if (!first)
                output << "\n";
            first = false;
        };
        bool hasNamespace = false;
        for (auto const& attribute : root().attributes())
            if (attribute.first().to_std_string_view() == "namespace")
                hasNamespace = true;
        if (!hasNamespace && !namespace_().empty())
        {
            separate();
            output << indent(1u) << "namespace: " << quoted(namespace_().to_std_string_view()) << "\n";
        }
        auto emit_ui = [&]()
        {
            separate();
            output << indent(1u) << "ui: {\n";
            auto emit_fragment = [&](i_element const& aElement)
            {
                if (!is_saved_to_nrc(aElement))
                    return;
                output << indent(2u) << fragment_name(aElement) << ": {\n";
                emit_element(output, aElement, 3u);
                output << indent(2u) << "}\n";
            };
            for (auto const& e : root().children())
            {
                if (e->group() == element_group::UserInterface)
                {
                    for (auto const& uiElement : e->children())
                        emit_fragment(*uiElement);
                }
                else
                    emit_fragment(*e);
            }
            output << indent(1u) << "}\n";
        };
        bool uiEmitted = false;
        for (auto const& attribute : root().attributes())
        {
            if (attribute.first().to_std_string_view() == "#ui")
            {
                emit_ui();
                uiEmitted = true;
            }
            else if (!is_metadata(attribute.first()) && !attribute.second().empty())
            {
                separate();
                output << indent(1u) << attribute.first().to_std_string_view() << ": " << reindent(attribute.second().to_std_string_view(), 1u) << "\n";
            }
        }
        if (!uiEmitted)
            emit_ui();
        output << "}\n";
        std::ofstream file{ std::filesystem::path{ aPath.to_std_string() }, std::ios::out | std::ios::trunc | std::ios::binary };
        if (!file)
            throw std::runtime_error{ "neogfx::DesignStudio::project::save: unable to open file for writing: " + aPath.to_std_string() };
        file << output.str();
        file.close();
        if (!file)
            throw std::runtime_error{ "neogfx::DesignStudio::project::save: error writing file: " + aPath.to_std_string() };
        iPath = aPath;
        iName = std::filesystem::path{ aPath.to_std_string() }.stem().string();
        set_clean();
    }

    bool project::has_path() const
    {
        return !iPath.empty();
    }

    const i_string& project::path() const
    {
        return iPath;
    }

    const i_string& project::name() const
    {
        return iName;
    }

    const i_string& project::namespace_() const
    {
        return iNamespace;
    }

    const i_element& project::root() const
    {
        return *iRoot;
    }

    i_element& project::root()
    {
        return *iRoot;
    }

    i_element& project::create_element(i_element& aParent, const i_string& aType, const i_string& aElementId)
    {
        auto result = manager().library(aType).create_element(aParent, aType.to_std_string(), aElementId.to_std_string());
        ElementAdded(*result);
        result->create_default_children();
        set_dirty();
        return *result;
    }

    void project::remove_element(i_element& aElement)
    {
        ref_ptr<i_element> temp = aElement;
        aElement.parent().remove_child(aElement);
        ElementRemoved(*temp);
        set_dirty();
    }

    void project::move_element(i_element& aElement, i_element& aNewParent, i_element const* aBefore)
    {
        ref_ptr<i_element> temp = aElement;
        aElement.parent().remove_child(aElement);
        aElement.set_parent(aNewParent);
        auto& siblings = aNewParent.children();
        auto before = std::find_if(siblings.begin(), siblings.end(), [&](auto const& e) { return &*e == aBefore; });
        if (aBefore != nullptr && before != siblings.end())
            siblings.insert(before, temp);
        else
            siblings.push_back(temp);
        ElementMoved(aElement);
        set_dirty();
    }
}
