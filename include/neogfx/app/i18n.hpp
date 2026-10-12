// i18n.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <vector>
#include <functional>
#include <type_traits>
#include <iosfwd>
#include <filesystem>

#include <neolib/task/i_event.hpp>

namespace neogfx
{
    using neolib::i_string;
    using neolib::string;

    class translation_context
    {
    public:
        translation_context(string const& aContext);
        ~translation_context();
    public:
        static string const& context();
    private:
        static std::vector<string>& context_stack();
    };

    // A translatable string (e.g. "Foo"_t) is its translation in the app's current language; while it exists the app knows it is 
    // translatable (see i_app::is_translatable) so text holders given it (e.g. a text_widget) translate it again if the language changes 
    // (see translatable<>).
    class translatable_string : public string
    {
    public:
        translatable_string(i_string const& aTranslatableString, i_string const& aContext);
        translatable_string(translatable_string const& aOther);
        translatable_string(translatable_string&& aOther);
        ~translatable_string();
    public:
        translatable_string& operator=(translatable_string const& aOther);
        translatable_string& operator=(translatable_string&& aOther);
    public:
        /// @todo add support for multiple plurals in a string
        translatable_string& operator()(std::int64_t aPlurality);
    public:
        operator std::string() const { return to_std_string(); }
    private:
        void register_translatable();
        void unregister_translatable();
    private:
        string iTranslatableString;
        string iContext;
        std::int64_t iPlurality = 1;
        bool iRegistered = false;
    };

    translatable_string translate(string const& aTranslatableString);
    translatable_string translate(string const& aTranslatableString, string const& aContext);
    translatable_string operator""_t(const char* aTranslatableString, std::size_t aStringLength);
    translatable_string operator""_t(const char8_t* aTranslatableString, std::size_t aStringLength);

    // Text kept translated: if text given to a text holder (track) is a translatable string (e.g. "Foo"_t) the holder is given 
    // (aUpdate) its translation again whenever the app's language changes; other text (or none) ends that (see translatable<>).
    class translatable_base
    {
    public:
        using update_function = std::function<void(i_string const&)>;
    public:
        translatable_base(update_function aUpdate = {});
        translatable_base(translatable_base const&) = delete;
        virtual ~translatable_base();
    public:
        translatable_base& operator=(translatable_base const&) = delete;
    public:
        bool active() const noexcept;
        void track(i_string const& aText);
        void track(neolib::i_optional<i_string> const& aText);
        void untrack();
    protected:
        virtual void apply(i_string const& aTranslation); // (no update function: the translation is the text (see translatable<>))
    private:
        void update();
    private:
        update_function iUpdate;
        bool iActive = false;
        bool iUpdating = false;
        string iSource;
        string iContext;
        std::int64_t iPlurality = 1;
        neolib::sink iSink;
    };

    // A text holder's text (Text: string or optional<string>) kept translated (see translatable_base): when the language changes 
    // the translation is given to aUpdate (e.g. the holder's setter) or, if there isn't one, assigned to the text itself.
    template <typename Text>
    class translatable : public Text, public translatable_base
    {
    public:
        using text_type = Text;
    public:
        translatable(update_function aUpdate = {}) :
            Text{}, translatable_base{ std::move(aUpdate) }
        {
        }
        template <typename... Args>
            requires (sizeof...(Args) > 0 && std::is_constructible_v<Text, Args&&...>)
        translatable(update_function aUpdate, Args&&... aArgs) :
            Text{ std::forward<Args>(aArgs)... }, translatable_base{ std::move(aUpdate) }
        {
        }
        template <typename... Args>
            requires (sizeof...(Args) > 0 && std::is_constructible_v<Text, Args&&...>)
        translatable(Args&&... aArgs) :
            Text{ std::forward<Args>(aArgs)... }
        {
        }
    public:
        using Text::operator=;
    protected:
        void apply(i_string const& aTranslation) override
        {
            static_cast<Text&>(*this) = string{ aTranslation };
        }
    };

    // An .xneol translation file's translations (one per <target> of each <text>):
    // <xneol xmlns="https://neogfx.org/xneol" version="1.0" trgLang="de">
    //     <text context="optional context">
    //         <source>Foo</source>
    //         <target n="optional plurality: n, from..to, from.. or ..to">Translation</target>
    //     </text>
    // </xneol>
    struct translation_entry
    {
        string source;
        string context;
        string plurality;
        string target;

        bool operator==(translation_entry const&) const = default;
    };
    using translation_list = std::vector<translation_entry>;

    void read_translation_file(std::filesystem::path const& aTranslationFile, string& aLanguage, translation_list& aTranslations);
    void read_translation_file(std::istream& aTranslationFile, string& aLanguage, translation_list& aTranslations);
    void write_translation_file(std::filesystem::path const& aTranslationFile, string const& aLanguage, translation_list const& aTranslations);
    std::pair<std::int64_t, std::int64_t> plurality_range(string const& aPlurality);

    // IANA language subtag registry: language subtags (e.g. "de") and their (English) descriptions
    neolib::i_map<i_string, i_string> const& language_codes();
    // e.g. "German (Germany)" for "de-DE"; the tag itself if its language is unknown
    string language_name(i_string const& aLanguageTag);
}

namespace neolib
{
    // (a live translation of an optional text is an optional (e.g. for neolib::optional's comparison operators))
    template <typename Text>
    struct is_optional<neogfx::translatable<Text>> : is_optional<Text> {};
}

using neogfx::operator""_t;