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

    namespace
    {
        std::vector<std::string> language_subtags(std::string const& aLanguageTag)
        {
            std::vector<std::string> subtags;
            for (std::size_t start = 0; start <= aLanguageTag.size();)
            {
                auto const end = std::min(aLanguageTag.find_first_of("-_", start), aLanguageTag.size());
                subtags.push_back(aLanguageTag.substr(start, end - start));
                start = end + 1;
            }
            return subtags;
        }

        // e.g. " (Germany)" for "de-DE"; empty if there is no (known) region
        std::string region_suffix(std::vector<std::string> const& aSubtags)
        {
            for (std::size_t i = 1; i < aSubtags.size(); ++i)
            {
                std::string region = aSubtags[i];
                // (a region subtag is two letters or three digits; others (e.g. a script subtag) are ignored)
                if (!((region.size() == 2 && std::isalpha(static_cast<unsigned char>(region[0])) && std::isalpha(static_cast<unsigned char>(region[1]))) ||
                    (region.size() == 3 && std::all_of(region.begin(), region.end(), [](char ch) { return std::isdigit(static_cast<unsigned char>(ch)); }))))
                    continue;
                for (auto& ch : region)
                    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                auto const existingRegion = registry().regions.find(region);
                if (existingRegion != registry().regions.end())
                    return " (" + existingRegion->second + ")";
                break;
            }
            return {};
        }
    }

    string language_name(i_string const& aLanguageTag)
    {
        std::string const tag = aLanguageTag.to_std_string();
        auto const subtags = language_subtags(tag);
        std::string language = subtags[0];
        for (auto& ch : language)
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        auto const existingLanguage = registry().languages.find(language);
        if (existingLanguage == registry().languages.end())
            return string{ tag };
        return string{ existingLanguage->second + region_suffix(subtags) };
    }

    string native_language_name(i_string const& aLanguageTag)
    {
        // (whole tags (a language in a region or script) and then languages, in lower case)
        static std::map<std::string, std::string> const sNativeNames = []()
            {
                struct native_name { char const* tag; char8_t const* name; };
                native_name const names[] =
                {
                    { "zh-cn", u8"\u7B80\u4F53\u4E2D\u6587" },
                    { "zh-sg", u8"\u7B80\u4F53\u4E2D\u6587" },
                    { "zh-hans", u8"\u7B80\u4F53\u4E2D\u6587" },
                    { "zh-tw", u8"\u7E41\u9AD4\u4E2D\u6587" },
                    { "zh-hk", u8"\u7E41\u9AD4\u4E2D\u6587" },
                    { "zh-mo", u8"\u7E41\u9AD4\u4E2D\u6587" },
                    { "zh-hant", u8"\u7E41\u9AD4\u4E2D\u6587" },
                    { "pt-br", u8"Portugu\u00EAs (Brasil)" },
                    { "pt-pt", u8"Portugu\u00EAs (Portugal)" },
                    { "en-gb", u8"English (United Kingdom)" },
                    { "en-us", u8"English (United States)" },
                    { "es-es", u8"Espa\u00F1ol (Espa\u00F1a)" },
                    { "es-mx", u8"Espa\u00F1ol (M\u00E9xico)" },
                    { "es-419", u8"Espa\u00F1ol (Latinoam\u00E9rica)" },
                    { "fr-fr", u8"Fran\u00E7ais (France)" },
                    { "fr-ca", u8"Fran\u00E7ais (Canada)" },
                    { "de-de", u8"Deutsch (Deutschland)" },
                    { "de-at", u8"Deutsch (\u00D6sterreich)" },
                    { "de-ch", u8"Deutsch (Schweiz)" },
                    { "af", u8"Afrikaans" },
                    { "am", u8"\u12A0\u121B\u122D\u129B" },
                    { "ar", u8"\u0627\u0644\u0639\u0631\u0628\u064A\u0629" },
                    { "az", u8"Az\u0259rbaycan" },
                    { "be", u8"\u0411\u0435\u043B\u0430\u0440\u0443\u0441\u043A\u0430\u044F" },
                    { "bg", u8"\u0411\u044A\u043B\u0433\u0430\u0440\u0441\u043A\u0438" },
                    { "bn", u8"\u09AC\u09BE\u0982\u09B2\u09BE" },
                    { "bs", u8"Bosanski" },
                    { "ca", u8"Catal\u00E0" },
                    { "cs", u8"\u010Ce\u0161tina" },
                    { "cy", u8"Cymraeg" },
                    { "da", u8"Dansk" },
                    { "de", u8"Deutsch" },
                    { "el", u8"\u0395\u03BB\u03BB\u03B7\u03BD\u03B9\u03BA\u03AC" },
                    { "en", u8"English" },
                    { "eo", u8"Esperanto" },
                    { "es", u8"Espa\u00F1ol" },
                    { "et", u8"Eesti" },
                    { "eu", u8"Euskara" },
                    { "fa", u8"\u0641\u0627\u0631\u0633\u06CC" },
                    { "fi", u8"Suomi" },
                    { "fil", u8"Filipino" },
                    { "fr", u8"Fran\u00E7ais" },
                    { "ga", u8"Gaeilge" },
                    { "gd", u8"G\u00E0idhlig" },
                    { "gl", u8"Galego" },
                    { "gu", u8"\u0A97\u0AC1\u0A9C\u0AB0\u0ABE\u0AA4\u0AC0" },
                    { "he", u8"\u05E2\u05D1\u05E8\u05D9\u05EA" },
                    { "hi", u8"\u0939\u093F\u0928\u094D\u0926\u0940" },
                    { "hr", u8"Hrvatski" },
                    { "hu", u8"Magyar" },
                    { "hy", u8"\u0540\u0561\u0575\u0565\u0580\u0565\u0576" },
                    { "id", u8"Bahasa Indonesia" },
                    { "is", u8"\u00CDslenska" },
                    { "it", u8"Italiano" },
                    { "ja", u8"\u65E5\u672C\u8A9E" },
                    { "ka", u8"\u10E5\u10D0\u10E0\u10D7\u10E3\u10DA\u10D8" },
                    { "kk", u8"\u049A\u0430\u0437\u0430\u049B \u0442\u0456\u043B\u0456" },
                    { "km", u8"\u1781\u17D2\u1798\u17C2\u179A" },
                    { "kn", u8"\u0C95\u0CA8\u0CCD\u0CA8\u0CA1" },
                    { "ko", u8"\uD55C\uAD6D\uC5B4" },
                    { "ky", u8"\u041A\u044B\u0440\u0433\u044B\u0437\u0447\u0430" },
                    { "lo", u8"\u0EA5\u0EB2\u0EA7" },
                    { "lt", u8"Lietuvi\u0173" },
                    { "lv", u8"Latvie\u0161u" },
                    { "mk", u8"\u041C\u0430\u043A\u0435\u0434\u043E\u043D\u0441\u043A\u0438" },
                    { "ml", u8"\u0D2E\u0D32\u0D2F\u0D3E\u0D33\u0D02" },
                    { "mn", u8"\u041C\u043E\u043D\u0433\u043E\u043B" },
                    { "mr", u8"\u092E\u0930\u093E\u0920\u0940" },
                    { "ms", u8"Bahasa Melayu" },
                    { "mt", u8"Malti" },
                    { "my", u8"\u1019\u103C\u1014\u103A\u1019\u102C" },
                    { "nb", u8"Norsk bokm\u00E5l" },
                    { "ne", u8"\u0928\u0947\u092A\u093E\u0932\u0940" },
                    { "nl", u8"Nederlands" },
                    { "nn", u8"Norsk nynorsk" },
                    { "no", u8"Norsk" },
                    { "pa", u8"\u0A2A\u0A70\u0A1C\u0A3E\u0A2C\u0A40" },
                    { "pl", u8"Polski" },
                    { "pt", u8"Portugu\u00EAs" },
                    { "ro", u8"Rom\u00E2n\u0103" },
                    { "ru", u8"\u0420\u0443\u0441\u0441\u043A\u0438\u0439" },
                    { "si", u8"\u0DC3\u0DD2\u0D82\u0DC4\u0DBD" },
                    { "sk", u8"Sloven\u010Dina" },
                    { "sl", u8"Sloven\u0161\u010Dina" },
                    { "sq", u8"Shqip" },
                    { "sr", u8"\u0421\u0440\u043F\u0441\u043A\u0438" },
                    { "sv", u8"Svenska" },
                    { "sw", u8"Kiswahili" },
                    { "ta", u8"\u0BA4\u0BAE\u0BBF\u0BB4\u0BCD" },
                    { "te", u8"\u0C24\u0C46\u0C32\u0C41\u0C17\u0C41" },
                    { "th", u8"\u0E44\u0E17\u0E22" },
                    { "tl", u8"Tagalog" },
                    { "tr", u8"T\u00FCrk\u00E7e" },
                    { "uk", u8"\u0423\u043A\u0440\u0430\u0457\u043D\u0441\u044C\u043A\u0430" },
                    { "ur", u8"\u0627\u0631\u062F\u0648" },
                    { "uz", u8"O\u02BBzbek" },
                    { "vi", u8"Ti\u1EBFng Vi\u1EC7t" },
                    { "zh", u8"\u4E2D\u6587" },
                    { "zu", u8"isiZulu" },
                };
                std::map<std::string, std::string> result;
                for (auto const& name : names)
                    result.emplace(name.tag, reinterpret_cast<char const*>(name.name));
                return result;
            }();
        std::string tag = aLanguageTag.to_std_string();
        for (auto& ch : tag)
            ch = (ch == '_' ? '-' : static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        if (auto const existing = sNativeNames.find(tag); existing != sNativeNames.end())
            return string{ existing->second };
        auto const subtags = language_subtags(tag);
        if (auto const existing = sNativeNames.find(subtags[0]); existing != sNativeNames.end())
            return string{ existing->second + region_suffix(subtags) };
        return language_name(aLanguageTag);
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

    string const& translatable_base::source() const noexcept
    {
        return iSource;
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