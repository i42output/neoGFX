// i18n.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.

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

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <charconv>
#include <cctype>
#include <algorithm>
#include <optional>

#include <neolib/core/map.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/scoped.hpp>
#include <neolib/file/xml.hpp>
#include <neogfx/app/i18n.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_resource_manager.hpp>

namespace neogfx
{
    namespace
    {
        // the IANA language subtag registry's language and region subtags' (first) descriptions
        struct language_subtag_registry
        {
            std::map<std::string, std::string> languages;
            std::map<std::string, std::string> regions;
            neolib::map<string, string> languageCodes;

            language_subtag_registry()
            {
                auto resource = service<i_resource_manager>().load_resource(":/neogfx/resources/language-subtag-registry.txt"s);
                std::istringstream lines{ std::string{ static_cast<const char*>(resource->data()), resource->size() } };
                std::string line;
                std::string type;
                std::string subtag;
                std::string description;
                auto add = [&]()
                {
                    if (!subtag.empty() && !description.empty())
                    {
                        if (type == "language")
                            languages.emplace(subtag, description);
                        else if (type == "region")
                            regions.emplace(subtag, description);
                    }
                    type.clear();
                    subtag.clear();
                    description.clear();
                };
                while (std::getline(lines, line))
                {
                    if (!line.empty() && line.back() == '\r')
                        line.pop_back();
                    if (line == "%%")
                        add();
                    else if (line.starts_with("Type: "))
                        type = line.substr(6);
                    else if (line.starts_with("Subtag: "))
                        subtag = line.substr(8);
                    else if (line.starts_with("Description: ") && description.empty())
                        description = line.substr(13);
                }
                add();
                for (auto const& language : languages)
                    languageCodes[string{ language.first }] = string{ language.second };
            }
        };

        language_subtag_registry const& registry()
        {
            static language_subtag_registry const sRegistry;
            return sRegistry;
        }
    }

    neolib::i_map<i_string, i_string> const& language_codes()
    {
        return registry().languageCodes;
    }

    string language_name(i_string const& aLanguageTag)
    {
        std::string const tag = aLanguageTag.to_std_string();
        std::vector<std::string> subtags;
        for (std::size_t start = 0; start <= tag.size();)
        {
            auto const end = std::min(tag.find_first_of("-_", start), tag.size());
            subtags.push_back(tag.substr(start, end - start));
            start = end + 1;
        }
        std::string language = subtags[0];
        for (auto& ch : language)
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        auto const existingLanguage = registry().languages.find(language);
        if (existingLanguage == registry().languages.end())
            return string{ tag };
        std::string result = existingLanguage->second;
        for (std::size_t i = 1; i < subtags.size(); ++i)
        {
            std::string region = subtags[i];
            // (a region subtag is two letters or three digits; others (e.g. a script subtag) are ignored)
            if (!((region.size() == 2 && std::isalpha(static_cast<unsigned char>(region[0])) && std::isalpha(static_cast<unsigned char>(region[1]))) ||
                (region.size() == 3 && std::all_of(region.begin(), region.end(), [](char ch) { return std::isdigit(static_cast<unsigned char>(ch)); }))))
                continue;
            for (auto& ch : region)
                ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            auto const existingRegion = registry().regions.find(region);
            if (existingRegion != registry().regions.end())
                result += " (" + existingRegion->second + ")";
            break;
        }
        return string{ result };
    }

    std::pair<std::int64_t, std::int64_t> plurality_range(string const& aPlurality)
    {
        // "n", "from..to", "from.." or "..to" (an invalid bound is no bound)
        auto result = std::make_pair(std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max());
        std::string const n = aPlurality.to_std_string();
        auto to_integer = [](std::string const& aBound) -> std::optional<std::int64_t>
        {
            auto const first = aBound.find_first_not_of(" \t");
            if (first == std::string::npos)
                return {};
            auto const last = aBound.find_last_not_of(" \t");
            std::int64_t value = 0;
            auto const [ptr, ec] = std::from_chars(aBound.data() + first, aBound.data() + last + 1, value);
            if (ec != std::errc{} || ptr != aBound.data() + last + 1)
                return {};
            return value;
        };
        auto const separator = n.find("..");
        if (separator == std::string::npos)
        {
            if (auto const value = to_integer(n))
                result.first = result.second = *value;
        }
        else
        {
            if (auto const from = to_integer(n.substr(0, separator)))
                result.first = *from;
            if (auto const to = to_integer(n.substr(separator + 2)))
                result.second = *to;
        }
        return result;
    }

    void read_translation_file(std::filesystem::path const& aTranslationFile, string& aLanguage, translation_list& aTranslations)
    {
        std::ifstream input{ aTranslationFile, std::ios::in | std::ios::binary };
        if (!input)
            throw std::runtime_error{ "neogfx::read_translation_file: unable to open file: " + aTranslationFile.string() };
        read_translation_file(input, aLanguage, aTranslations);
    }

    void read_translation_file(std::istream& aTranslationFile, string& aLanguage, translation_list& aTranslations)
    {
        neolib::xml translationFile;
        translationFile.read(aTranslationFile);
        if (!translationFile.got_root() || translationFile.root().name() != "xneol")
            return;
        aLanguage = string{ translationFile.root().attribute_value("trgLang").to_std_string() };
        for (auto const& item : translationFile.root())
        {
            if (item.name() != "text")
                continue;
            // (line breaks are "\n" whatever the file's are (e.g. a checkout's))
            auto text = [](std::string aText)
            {
                for (auto pos = aText.find("\r\n"); pos != std::string::npos; pos = aText.find("\r\n", pos))
                    aText.erase(pos, 1);
                return string{ aText };
            };
            std::optional<string> source;
            string const context{ item.has_attribute("context") ? text(item.attribute_value("context").to_std_string()) : string{} };
            std::vector<std::pair<string, string>> targets;
            for (auto const& part : item)
            {
                if (part.name() == "source")
                    source = text(part.text().to_std_string());
                else if (part.name() == "target")
                    targets.emplace_back(
                        string{ part.has_attribute("n") ? part.attribute_value("n").to_std_string() : std::string{} },
                        text(part.text().to_std_string()));
            }
            if (!source)
                continue;
            if (targets.empty())
                aTranslations.push_back(translation_entry{ *source, context });
            for (auto const& target : targets)
                aTranslations.push_back(translation_entry{ *source, context, target.first, target.second });
        }
    }

    void write_translation_file(std::filesystem::path const& aTranslationFile, string const& aLanguage, translation_list const& aTranslations)
    {
        neolib::xml translationFile;
        translationFile.set_indent(' ', 4);
        auto& root = translationFile.append(translationFile.document(), "xneol");
        root.set_attribute("xmlns", "https://neogfx.org/xneol");
        root.set_attribute("version", "1.0");
        root.set_attribute("trgLang", aLanguage.to_std_string());
        // a string's translations (in a context) are the targets of one <text> (in the order the strings first appear)
        std::vector<std::vector<translation_entry const*>> texts;
        std::map<std::pair<std::string, std::string>, std::size_t> textIndices;
        for (auto const& translation : aTranslations)
        {
            if (translation.source.empty())
                continue;
            auto const key = std::make_pair(translation.source.to_std_string(), translation.context.to_std_string());
            auto existing = textIndices.find(key);
            if (existing == textIndices.end())
            {
                existing = textIndices.emplace(key, texts.size()).first;
                texts.emplace_back();
            }
            texts[existing->second].push_back(&translation);
        }
        for (auto const& targets : texts)
        {
            auto& text = root.append("text");
            if (!targets[0]->context.empty())
                text.set_attribute("context", targets[0]->context.to_std_string());
            text.append("source").append_text(targets[0]->source.to_std_string());
            for (auto const& translation : targets)
            {
                auto& target = text.append("target");
                if (!translation->plurality.empty())
                    target.set_attribute("n", translation->plurality.to_std_string());
                if (!translation->target.empty())
                    target.append_text(translation->target.to_std_string());
            }
        }
        std::ofstream output{ aTranslationFile, std::ios::out | std::ios::binary | std::ios::trunc };
        if (!output || !translationFile.write(output) || !output.flush())
            throw std::runtime_error{ "neogfx::write_translation_file: error writing file: " + aTranslationFile.string() };
    }

    translation_context::translation_context(string const& aContext)
    {
        context_stack().push_back(aContext);
    }

    translation_context::~translation_context()
    {
        context_stack().pop_back();
    }

    string const& translation_context::context()
    {
        if (!context_stack().empty())
            return context_stack().back();
        static const string sDefaultContext;
        return sDefaultContext;
    }

    std::vector<string>& translation_context::context_stack()
    {
        static std::vector<string> sContextStack;
        return sContextStack;
    }

    translatable_string::translatable_string(i_string const& aTranslatableString, i_string const& aContext) : 
        string{ service<i_app>().translate(aTranslatableString, aContext) },
        iTranslatableString{ aTranslatableString },
        iContext{ aContext }
    {
        register_translatable();
    }

    translatable_string::translatable_string(translatable_string const& aOther) :
        string{ static_cast<string const&>(aOther) },
        iTranslatableString{ aOther.iTranslatableString },
        iContext{ aOther.iContext },
        iPlurality{ aOther.iPlurality }
    {
        register_translatable();
    }

    translatable_string::translatable_string(translatable_string&& aOther) :
        string{ std::move(static_cast<string&>(aOther)) },
        iTranslatableString{ std::move(aOther.iTranslatableString) },
        iContext{ std::move(aOther.iContext) },
        iPlurality{ aOther.iPlurality }
    {
        aOther.unregister_translatable();
        register_translatable();
    }

    translatable_string::~translatable_string()
    {
        unregister_translatable();
    }

    translatable_string& translatable_string::operator=(translatable_string const& aOther)
    {
        if (&aOther != this)
        {
            string::operator=(static_cast<string const&>(aOther));
            iTranslatableString = aOther.iTranslatableString;
            iContext = aOther.iContext;
            iPlurality = aOther.iPlurality;
            register_translatable();
        }
        return *this;
    }

    translatable_string& translatable_string::operator=(translatable_string&& aOther)
    {
        if (&aOther != this)
        {
            string::operator=(std::move(static_cast<string&>(aOther)));
            iTranslatableString = std::move(aOther.iTranslatableString);
            iContext = std::move(aOther.iContext);
            iPlurality = aOther.iPlurality;
            aOther.unregister_translatable();
            register_translatable();
        }
        return *this;
    }

    translatable_string& translatable_string::operator()(std::int64_t aPlurality)
    {
        iPlurality = aPlurality;
        assign(service<i_app>().translate(iTranslatableString, iContext, aPlurality));
        register_translatable();
        return *this;
    }

    void translatable_string::register_translatable()
    {
        service<i_app>().register_translatable(static_cast<i_string const&>(*this), iTranslatableString, iContext, iPlurality);
        iRegistered = true;
    }

    void translatable_string::unregister_translatable()
    {
        if (!iRegistered)
            return;
        iRegistered = false;
        try
        {
            service<i_app>().unregister_translatable(static_cast<i_string const&>(*this));
        }
        catch (...)
        {
            // (no app: nothing to unregister from)
        }
    }

    translatable_string translate(string const& aTranslatableString)
    {
        return translatable_string{ aTranslatableString, translation_context::context() };
    }

    translatable_string translate(string const& aTranslatableString, string const& aContext)
    {
        return translatable_string{ aTranslatableString, aContext };
    }

    translatable_string operator""_t(const char* aTranslatableString, std::size_t aStringLength)
    {
        return translatable_string{ string{ aTranslatableString, aStringLength }, translation_context::context() };
    }

    translatable_string operator""_t(const char8_t* aTranslatableString, std::size_t aStringLength)
    {
        return translatable_string{ string{ reinterpret_cast<const char*>(aTranslatableString), aStringLength }, translation_context::context() };
    }

    translatable_base::translatable_base(update_function aUpdate) :
        iUpdate{ std::move(aUpdate) }
    {
    }

    translatable_base::~translatable_base()
    {
    }

    bool translatable_base::active() const noexcept
    {
        return iActive;
    }

    void translatable_base::track(i_string const& aText)
    {
        if (iUpdating)
            return; // (the holder being given the new translation)
        string translatableString;
        string context;
        std::int64_t plurality = 1;
        if (service<i_app>().is_translatable(aText, translatableString, context, plurality))
        {
            iSource = translatableString;
            iContext = context;
            iPlurality = plurality;
            if (!iActive)
            {
                iActive = true;
                iSink = service<i_app>().language_changed([this]() { update(); });
            }
        }
        else
            untrack();
    }

    void translatable_base::track(neolib::i_optional<i_string> const& aText)
    {
        if (aText.has_value())
            track(aText.value());
        else if (!iUpdating)
            untrack();
    }

    void translatable_base::untrack()
    {
        if (iActive)
        {
            iActive = false;
            iSink.clear();
        }
    }

    void translatable_base::apply(i_string const&)
    {
    }

    void translatable_base::update()
    {
        neolib::scoped_flag sf{ iUpdating };
        auto const& translation = service<i_app>().translate(iSource, iContext, iPlurality);
        if (iUpdate)
            iUpdate(translation);
        else
            apply(translation);
    }
}