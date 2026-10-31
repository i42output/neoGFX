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
#include <iterator>
#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <neolib/file/json.hpp>
#include <neogfx/app/i_resource_manager.hpp>
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
        iTranslations.clear();
        iRoot = manager().library("project"_s).create_element(*this, "project", aName.to_std_string());
    }

    void project::open(const i_string& aPath)
    {
        // a project is either a single .nrc file or a project file (.dsproj) listing the .nrc files it consists of
        std::filesystem::path const projectFileName{ std::filesystem::absolute(std::filesystem::path{ aPath.to_std_string() }) };
        iFiles.clear();
        iTranslations.clear();
        iPath = aPath;
        iName = projectFileName.stem().string();
        iRoot = manager().library("project"_s).create_element(*this, "project", projectFileName.stem().string());
        manager().library("user_interface"_s).create_element(*iRoot, "user_interface", "User Interface"_t);
        if (projectFileName.extension() == ".dsproj")
        {
            neolib::fjson const input{ projectFileName.string() };
            if (!input.has_root() || !input.root().as<neolib::fjson_object>().has("nrc"))
                throw invalid_project_file("no .nrc files");
            for (auto const& nrcFile : input.root().as<neolib::fjson_object>().at("nrc"))
                load_nrc(projectFileName.parent_path() / std::string{ nrcFile.text().begin(), nrcFile.text().end() });
        }
        else
            load_nrc(projectFileName);
        for (auto& file : iFiles)
            file.saved = generate_nrc(static_cast<std::size_t>(std::distance(&iFiles[0], &file)));
        for (std::size_t i = 0u; i < iFiles.size(); ++i)
            load_translations(i);
        set_clean();
    }

    void project::add_file(const i_string& aPath)
    {
        if (!iRoot)
            throw invalid_project_file("no project");
        std::filesystem::path const fileName{ std::filesystem::absolute(std::filesystem::path{ aPath.to_std_string() }) };
        for (auto const& file : iFiles)
            if (std::filesystem::equivalent(file.path, fileName))
                return; // already in the project
        load_nrc(fileName);
        iFiles.back().saved = generate_nrc(iFiles.size() - 1u);
        load_translations(iFiles.size() - 1u);
        set_dirty(); // (the project file needs to list it)
    }

    std::uint32_t project::file_count() const
    {
        return static_cast<std::uint32_t>(iFiles.size());
    }

    std::uint32_t project::element_file(i_element const& aElement) const
    {
        // (its top level element's)
        if (!iRoot)
            return 0u;
        i_element const* topLevel = &aElement;
        while (topLevel->has_parent() && &topLevel->parent() != &*iRoot && topLevel->parent().group() != element_group::UserInterface)
            topLevel = &topLevel->parent();
        return static_cast<std::uint32_t>(file_of(*topLevel));
    }

    std::uint32_t project::translation_language_count(std::uint32_t aFile) const
    {
        return static_cast<std::uint32_t>(file_translations(aFile).size());
    }

    i_string const& project::translation_language(std::uint32_t aFile, std::uint32_t aLanguageIndex) const
    {
        return file_translations(aFile).at(aLanguageIndex).language;
    }

    void project::add_translation_language(std::uint32_t aFile, i_string const& aLanguage)
    {
        auto& tables = file_translations(aFile);
        for (auto const& table : tables)
            if (table.language == aLanguage)
                return;
        tables.push_back(translation_table{ string{ aLanguage } });
        set_dirty();
    }

    std::uint32_t project::translation_count(std::uint32_t aFile, i_string const& aLanguage) const
    {
        for (auto const& table : file_translations(aFile))
            if (table.language == aLanguage)
                return static_cast<std::uint32_t>(table.translations.size());
        return 0u;
    }

    void project::translation(std::uint32_t aFile, i_string const& aLanguage, std::uint32_t aIndex, i_string& aSource, i_string& aContext, i_string& aPlurality, i_string& aTarget) const
    {
        auto const& translation = language_translations(aFile, aLanguage).translations.at(aIndex);
        aSource = translation.source;
        aContext = translation.context;
        aPlurality = translation.plurality;
        aTarget = translation.target;
    }

    void project::set_translation(std::uint32_t aFile, i_string const& aLanguage, std::uint32_t aIndex, i_string const& aSource, i_string const& aContext, i_string const& aPlurality, i_string const& aTarget)
    {
        auto& translations = language_translations(aFile, aLanguage).translations;
        translation_entry const translation{ string{ aSource }, string{ aContext }, string{ aPlurality }, string{ aTarget } };
        if (aIndex == translations.size())
            translations.push_back(translation);
        else if (translations.at(aIndex) != translation)
            translations[aIndex] = translation;
        else
            return;
        set_dirty();
    }

    i_string const& project::design_language() const
    {
        return iDesignLanguage;
    }

    void project::set_design_language(i_string const& aLanguage)
    {
        iDesignLanguage = aLanguage;
    }

    void project::design_translation(i_element const& aElement, i_string const& aString, i_string& aResult) const
    {
        // as at runtime (see neogfx::app::translate): the language's table or, failing that, its primary language's (e.g. "de" for 
        // "de-AT"); the translatable strings nrc generates have no context and a plurality of 1; an empty target is a string not yet translated
        string const source{ aString };
        aResult = source;
        std::string const language = iDesignLanguage.to_std_string();
        if (language.empty() || source.empty())
            return;
        auto const& tables = file_translations(element_file(aElement));
        std::string const primaryLanguage = language.substr(0, language.find('-'));
        for (auto const& candidate : { language, primaryLanguage })
            for (auto const& table : tables)
                if (boost::iequals(table.language.to_std_string(), candidate))
                {
                    // the target of the narrowest plurality range that includes 1 (otherwise the first)
                    string const* result = nullptr;
                    string const* first = nullptr;
                    std::uint64_t resultWidth = 0u;
                    for (auto const& translation : table.translations)
                    {
                        if (translation.source != source || !translation.context.empty())
                            continue;
                        if (first == nullptr)
                            first = &translation.target;
                        auto const range = plurality_range(translation.plurality);
                        if (range.first > 1 || range.second < 1)
                            continue;
                        auto const width = static_cast<std::uint64_t>(range.second) - static_cast<std::uint64_t>(range.first);
                        if (result == nullptr || width < resultWidth)
                        {
                            result = &translation.target;
                            resultWidth = width;
                        }
                    }
                    if (result == nullptr)
                        result = first;
                    if (result != nullptr && !result->empty())
                        aResult = *result;
                    return;
                }
    }

    void project::load_nrc(std::filesystem::path const& aPath)
    {
        std::filesystem::path const inputFileName{ std::filesystem::absolute(aPath).lexically_normal() };
        neolib::fjson const input{ inputFileName.string() };
        if (!input.has_root())
            throw invalid_project_file("bad root node: " + inputFileName.string());
        iFiles.emplace_back();
        auto& file = iFiles.back();
        file.path = inputFileName;
        if (iFiles.size() == 1u)
            iNamespace = input.root().as<neolib::fjson_object>().has("namespace") ? input.root().as<neolib::fjson_object>().at("namespace").text() : "";
        i_element* userInterface = nullptr;
        for (auto& e : iRoot->children())
            if (e->group() == element_group::UserInterface)
                userInterface = &*e;
        if (userInterface == nullptr)
            userInterface = &*manager().library("user_interface"_s).create_element(*iRoot, "user_interface", "User Interface"_t);
        // resources (e.g. images) are registered as nrc would embed them (":/<namespace>[/<resource namespace>]/<file>", files relative to
        // the .nrc file) so that elements using them can show them
        auto load_resources = [&](neolib::fjson_value const& aResource)
        {
            auto const& resource = aResource.as<neolib::fjson_object>();
            auto text = [](auto const& aText) { return std::string{ aText.begin(), aText.end() }; };
            std::string prefix = input.root().as<neolib::fjson_object>().has("namespace") ? text(input.root().as<neolib::fjson_object>().at("namespace").text()) : std::string{};
            if (resource.has("namespace"))
                prefix += "/" + text(resource.at("namespace").text());
            for (auto pos = prefix.find("::"); pos != std::string::npos; pos = prefix.find("::"))
                prefix.replace(pos, 2u, "/");
            auto load_file = [&](std::string const& aFile)
            {
                auto const filePath = std::filesystem::absolute(inputFileName).parent_path() / aFile;
                auto const uri = ":/" + (!prefix.empty() ? prefix + "/" : std::string{}) + aFile;
                std::ifstream file{ filePath, std::ios::in | std::ios::binary };
                if (!file)
                {
                    std::cerr << "DesignStudio: cannot read resource file '" << filePath.string() << "' (" << uri << ")" << std::endl;
                    return; // (still preserved in the project)
                }
                std::vector<char> data{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
                service<i_resource_manager>().add_resource(uri, data.data(), data.size());
            };
            for (auto const& resourceItem : aResource)
            {
                if (resourceItem.name() == "file")
                    load_file(text(resourceItem.text()));
                else if (resourceItem.name() == "files")
                    for (auto const& fileItem : resourceItem)
                        load_file(text(fileItem.text()));
            }
        };
        std::map<std::string, std::uint32_t> counters;
        // whether an .nrc object node is an element
        auto is_element_node = [](neolib::fjson_value const& aNode)
        {
            std::string const type{ aNode.name().begin(), aNode.name().end() };
            if (!type.empty() && type[0] == '.')
                return false; // (a member of its parent (e.g. a label's .text_widget), not an element: kept as an attribute)
            if (!type.empty() && std::isupper(static_cast<unsigned char>(type[0])))
                return false; // (a composite property's components (e.g. "Palette: { Base: Red }"), not an element: kept as an attribute)
            return true;
        };
        // (progress: the elements created so far of those in the file)
        std::uint32_t elementsLoaded = 0u;
        std::uint32_t elementCount = 0u;
        std::function<void(neolib::fjson_value const&)> count_elements = [&](neolib::fjson_value const& aNode)
        {
            for (auto const& child : aNode)
                if (child.type() == neolib::json_type::Object && is_element_node(child))
                {
                    ++elementCount;
                    count_elements(child);
                }
        };
        for (auto const& item : input.root())
            if (item.name() == "ui" && item.type() == neolib::json_type::Object)
                for (auto const& fragment : item)
                    if (fragment.type() == neolib::json_type::Object)
                        count_elements(fragment);
        // create an element for an .nrc object node; returns nullptr if the node isn't a known element type
        auto create_node_element = [&](i_element& aParent, neolib::fjson_value const& aNode) -> i_element*
        {
            i_element* result = nullptr;
            try
            {
                if (!is_element_node(aNode))
                    return nullptr;
                std::string const type{ aNode.name().begin(), aNode.name().end() };
                std::string const id = aNode.as<neolib::fjson_object>().has("id") ? 
                    std::string{ aNode.as<neolib::fjson_object>().at("id").text().begin(), aNode.as<neolib::fjson_object>().at("id").text().end() } :
                    type + boost::lexical_cast<std::string>(++counters[type]);
                result = &create_element(aParent, string{ type }, string{ id });
            }
            catch (...)
            {
            }
            if (result != nullptr)
                manager().project_loading().trigger(*this, ++elementsLoaded, elementCount); // (outside the try: not a failure to create it)
            return result;
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
                file.attributes.push_back(attribute_t{ string{ "#ui" }, string{} });
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
                            newElement->attributes().push_back(attribute_t{ string{ "#file" }, string{ file.path.string() } });
                            add_node(*newElement, node);
                        }
                    }
                }
            }
            else
            {
                if (item.name() == "resource" && item.type() == neolib::json_type::Object)
                    load_resources(item);
                file.attributes.push_back(attribute_t{ string{ name_to_rjson(item) }, string{ value_to_rjson(item) } });
            }
        }
    }

    std::size_t project::file_of(i_element const& aTopLevelElement) const
    {
        // the .nrc file a top level element is in (elements added in Design Studio are in the first (primary) file)
        for (auto const& attribute : aTopLevelElement.attributes())
            if (attribute.first().to_std_string_view() == "#file")
                for (std::size_t i = 0u; i < iFiles.size(); ++i)
                    if (iFiles[i].path.string() == attribute.second().to_std_string())
                        return i;
        return 0u;
    }

    // a .nrc file's translation tables: "<.nrc file stem>.<language>.xneol" files in the "resources" sub-directory of its directory (any 
    // it already has (a new project's primary file's) are kept)
    void project::load_translations(std::size_t aFile)
    {
        auto& tables = file_translations(static_cast<std::uint32_t>(aFile));
        auto const& nrcPath = iFiles[aFile].path;
        std::string const prefix = nrcPath.stem().string() + ".";
        std::string const extension = ".xneol";
        std::error_code ec;
        for (auto const& entry : std::filesystem::directory_iterator{ nrcPath.parent_path() / "resources", ec })
        {
            auto const fileName = entry.path().filename().string();
            if (entry.path().extension() != extension || !fileName.starts_with(prefix))
                continue;
            auto const fileLanguage = fileName.substr(prefix.size(), fileName.size() - prefix.size() - extension.size());
            if (fileLanguage.empty() || fileLanguage.find('.') != std::string::npos)
                continue; // (another .nrc file's (e.g. "foo.bar.de.xneol" is "foo.bar.nrc"'s))
            if (std::any_of(tables.begin(), tables.end(), [&](translation_table const& aTable) { return aTable.language == string{ fileLanguage }; }))
                continue;
            // (its language is the one in its name (which it is saved as), whatever its trgLang)
            translation_table table;
            string fileTargetLanguage;
            try
            {
                read_translation_file(entry.path(), fileTargetLanguage, table.translations);
            }
            catch (std::exception const& e)
            {
                throw invalid_project_file("bad translation file: " + entry.path().string() + ": " + e.what());
            }
            table.language = string{ fileLanguage };
            table.saved = (fileTargetLanguage == table.language ? std::optional<translation_list>{ table.translations } : std::nullopt);
            tables.push_back(std::move(table));
        }
        std::sort(tables.begin(), tables.end(), [](translation_table const& lhs, translation_table const& rhs) { return lhs.language < rhs.language; });
    }

    void project::save_translations(std::size_t aFile)
    {
        auto const& nrcPath = iFiles[aFile].path;
        for (auto& table : file_translations(static_cast<std::uint32_t>(aFile)))
        {
            // (a translation of an empty string is removed)
            std::erase_if(table.translations, [](translation_entry const& aTranslation) { return aTranslation.source.empty(); });
            auto const path = nrcPath.parent_path() / "resources" / (nrcPath.stem().string() + "." + table.language.to_std_string() + ".xneol");
            if (table.saved != table.translations || !std::filesystem::exists(path))
            {
                std::error_code ec;
                std::filesystem::create_directories(path.parent_path(), ec); // (if it fails writing the file reports the error)
                write_translation_file(path, table.language, table.translations);
                table.saved = table.translations;
            }
        }
    }

    project::translation_tables& project::file_translations(std::uint32_t aFile)
    {
        if (iTranslations.size() < aFile + 1u)
            iTranslations.resize(aFile + 1u);
        return iTranslations[aFile];
    }

    project::translation_tables const& project::file_translations(std::uint32_t aFile) const
    {
        if (aFile < iTranslations.size())
            return iTranslations[aFile];
        static translation_tables const sNone;
        return sNone;
    }

    project::translation_table& project::language_translations(std::uint32_t aFile, i_string const& aLanguage)
    {
        return const_cast<translation_table&>(to_const(*this).language_translations(aFile, aLanguage));
    }

    project::translation_table const& project::language_translations(std::uint32_t aFile, i_string const& aLanguage) const
    {
        for (auto const& table : file_translations(aFile))
            if (table.language == aLanguage)
                return table;
        throw std::logic_error{ "neogfx::DesignStudio::project: no translation table for language: " + aLanguage.to_std_string() };
    }

    std::string project::generate_nrc(std::size_t aFile) const
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
        auto const& file = iFiles[aFile];
        bool hasNamespace = false;
        for (auto const& attribute : file.attributes)
            if (attribute.first().to_std_string_view() == "namespace")
                hasNamespace = true;
        if (!hasNamespace && aFile == 0u && !namespace_().empty())
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
                if (!is_saved_to_nrc(aElement) || file_of(aElement) != aFile)
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
        for (auto const& attribute : file.attributes)
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
        return output.str();
    }

    void project::save(const i_string& aPath)
    {
        auto write = [](std::filesystem::path const& aFilePath, std::string const& aContents)
        {
            std::ofstream file{ aFilePath, std::ios::out | std::ios::trunc | std::ios::binary };
            if (!file)
                throw std::runtime_error{ "neogfx::DesignStudio::project::save: unable to open file for writing: " + aFilePath.string() };
            file << aContents;
            file.close();
            if (!file)
                throw std::runtime_error{ "neogfx::DesignStudio::project::save: error writing file: " + aFilePath.string() };
        };
        std::filesystem::path const projectFileName{ std::filesystem::absolute(std::filesystem::path{ aPath.to_std_string() }) };
        bool const isProjectFile = (projectFileName.extension() == ".dsproj");
        if (iFiles.empty())
        {
            // a new project: its (primary) .nrc file is the one given or one beside the project file
            iFiles.emplace_back();
            iFiles.back().path = isProjectFile ? projectFileName.parent_path() / (projectFileName.stem().string() + ".nrc") : projectFileName;
            iFiles.back().attributes.push_back(attribute_t{ string{ "#ui" }, string{} });
        }
        else if (!isProjectFile)
            iFiles[0].path = projectFileName; // (saving the primary .nrc file as)
        // .nrc files (only those that have changed so their formatting is otherwise untouched)
        for (std::size_t i = 0u; i < iFiles.size(); ++i)
        {
            auto const contents = generate_nrc(i);
            if (contents != iFiles[i].saved || !std::filesystem::exists(iFiles[i].path))
            {
                write(iFiles[i].path, contents);
                iFiles[i].saved = contents;
            }
            save_translations(i);
        }
        // project file listing them (paths relative to it)
        if (isProjectFile)
        {
            std::ostringstream output;
            output << "{\n" << indent(1u) << "nrc: [\n";
            for (auto const& file : iFiles)
            {
                std::error_code ec;
                auto relative = std::filesystem::relative(file.path, projectFileName.parent_path(), ec);
                output << indent(2u) << quoted((ec || relative.empty() ? file.path : relative).generic_string()) << "\n";
            }
            output << indent(1u) << "]\n" << "}\n";
            write(projectFileName, output.str());
        }
        iPath = aPath;
        iName = projectFileName.stem().string();
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
