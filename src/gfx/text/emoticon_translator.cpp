// emoticon_translator.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2025 Leigh Johnston.  All Rights Reserved.
  
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

#include <filesystem>

#include <neolib/core/uuid.hpp>
#include <neolib/file/file.hpp>
#include <neolib/file/json.hpp>

#include <neogfx/app/i_resource_manager.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/gfx/text/i_emoticon_translator.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/gui/widget/i_menu_item_widget.hpp>


namespace neogfx
{
    namespace
    {
        std::string emoticon_database_uri()
        {
            if (std::filesystem::exists(neolib::program_directory() + "/emoticon.json"))
                return "file:///" + neolib::program_directory() + "/emoticon.json";
            return ":/neogfx/resources/emoticon.json";
        }

        std::chrono::seconds const DEFAULT_UNDO_TIMEOUT{ 5 };
    }

    class emoticon_translator : public i_emoticon_translator, private i_keyboard_handler
    {
    private:
        using emoji_map = std::unordered_map<std::string, std::vector<std::string>>;
    public:
        emoticon_translator()
        {
            auto resource = service<i_resource_manager>().load_resource(emoticon_database_uri());
            std::istringstream lines{ std::string{ static_cast<const char*>(resource->data()), resource->size() } };
            neolib::json const database{ lines };
            for (auto const& entry : database.root().as<neolib::json_object>().at("data"))
            {
                std::vector<std::string> emoji;
                for (auto const& emojiVariant : entry.as<neolib::json_object>().at("emoji").as<neolib::json_array>())
                    emoji.push_back(emojiVariant->as<neolib::json_string>().to_std_string());
                for (auto const& emoticonVariant : entry.as<neolib::json_object>().at("emoticons").as<neolib::json_array>())
                    iEmojiMap[emoticonVariant->as<neolib::json_string>().to_std_string()] = emoji;
            }
        }
    public:
        bool active() const final
        {
            return iActiveWidget != nullptr;
        }
        bool active(i_widget const& aWidget) const final
        {
            return iActiveWidget == &aWidget;
        }
        void activate(i_widget& aWidget) final
        {
            if (active())
                throw std::logic_error("neogfx::emoticon_translator::activate: already active!");
            iActiveWidget = &aWidget;
            iSink = aWidget.destroying([&]() { if (active(aWidget)) deactivate(); });
            service<i_keyboard>().grab_keyboard(*this);
        }
        void deactivate() final
        {
            if (!active())
                throw std::logic_error("neogfx::emoticon_translator::deactivate: not active!");
            service<i_keyboard>().ungrab_keyboard(*this);
            iActiveWidget = nullptr;
            iCursorPosition = std::nullopt;
            iBuffer.clear();
            iEmoticonMatches.clear();
            iLastTranslationForUndo = std::nullopt;
        }
    public:
        void update_cursor_position(point const& aPosition) final
        {
            if (!active())
                throw std::logic_error("neogfx::emoticon_translator::update_cursor_position: not active!");
            iCursorPosition = aPosition + iActiveWidget->non_client_rect().top_left() + iActiveWidget->root().window_position();
        }
    private:
        void handle_emoticon_matches(std::string_view const& aText, bool aHaveExtra, bool& aSuppressBufferClear)
        {
            iActiveWidget->text_input(neolib::string{ aText });

            std::set<std::string> availableEmoji;
            std::optional<std::string> selectedEmoji;

            for (auto const& emojiAlternatives : iEmoticonMatches)
                for (auto const& emoji : emojiAlternatives->second)
                    availableEmoji.insert(emoji);

            if (availableEmoji.empty())
                return;
            else if (availableEmoji.size() == 1)
                selectedEmoji = *availableEmoji.begin();
            else
            {
                context_menu contextMenu{ *iActiveWidget, iCursorPosition.value(), popup_menu::DEFAULT_STYLE | window_style::DismissOnParentClick | window_style::HorizontalMenuLayout };

                for (auto const& emoji : availableEmoji)
                {
                    auto const u32emoji = neolib::utf8_to_utf32(emoji);
                    if (service<i_font_manager>().emoji_atlas().is_emoji(u32emoji))
                    {
                        auto const emojiId = service<i_font_manager>().emoji_atlas().emoji(u32emoji);
                        auto a = make_ref<action>("", service<i_font_manager>().emoji_atlas().emoji_texture(emojiId));
                        contextMenu.menu().add_action(a);
                        a->Triggered.set_trigger_type(trigger_type::Synchronous);
                        a->Triggered([&, emoji]() 
                            { selectedEmoji = emoji; });
                    }
                }

                if (contextMenu.menu().count() > 0)
                {
                    contextMenu.menu().opened([&]()
                        {
                            auto const iconSize = size{ std::max(iActiveWidget->font().height(), 16.0) };
                            for (i_menu::item_index i = 0; i < contextMenu.menu().count(); ++i)
                                contextMenu.menu().item_at(i).as_widget().set_icon_size(iconSize);
                        });

                    contextMenu.menu().dismiss_on_text_input([&](i_string const& aText, bool& aDismiss)
                        {
                            aDismiss = true;
                            iBuffer.clear();
                            iEmoticonMatches.clear();
                            aSuppressBufferClear = true;
                        });

                    contextMenu.menu().select_item_at(0);
                    contextMenu.exec();
                }
            }

            if (selectedEmoji.has_value())
            {
                std::u32string const codePoints = neolib::utf8_to_utf32(iBuffer);
                for (std::size_t i = 0; i < codePoints.size(); ++i)
                    iActiveWidget->key_pressed(ScanCode_BACKSPACE, KeyCode_BACKSPACE, key_modifier::None);
                auto const& toInsert = neolib::string{ selectedEmoji.value() } + (aHaveExtra ? aText : std::string_view{});
                iActiveWidget->text_input(toInsert);
                if (availableEmoji.size() == 1) // only need undo if we didn't popup a menu
                    iLastTranslationForUndo.emplace(std::make_tuple(std::chrono::steady_clock::now(), iBuffer, toInsert));
            }
        }
    private:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) final
        {
            if (active())
            {
                auto const lastTranslationForUndo = iLastTranslationForUndo;
                iLastTranslationForUndo = std::nullopt;
                if (aScanCode == ScanCode_BACKSPACE && !iBuffer.empty())
                {
                    std::u32string codePoints = neolib::utf8_to_utf32(iBuffer);
                    codePoints.pop_back();
                    iBuffer = neolib::utf32_to_utf8(codePoints);
                }
                else if (aScanCode == ScanCode_ESCAPE && lastTranslationForUndo.has_value() &&
                    std::chrono::steady_clock::now() - std::get<0>(lastTranslationForUndo.value()) < DEFAULT_UNDO_TIMEOUT)
                {
                    std::u32string const codePoints = neolib::utf8_to_utf32(std::get<2>(lastTranslationForUndo.value()));
                    for (std::size_t i = 0; i < codePoints.size(); ++i)
                        iActiveWidget->key_pressed(ScanCode_BACKSPACE, KeyCode_BACKSPACE, key_modifier::None);
                    iActiveWidget->text_input(neolib::string{ std::get<1>(lastTranslationForUndo.value()) });
                    return true;
                }
                else if ((aKeyCode < static_cast<key_code_e>(' ') || aKeyCode > static_cast<key_code_e>(0xFF)) &&
                    aScanCode != ScanCode_LSHIFT && aScanCode != ScanCode_RSHIFT)
                    iBuffer.clear();
            }
            return false;
        }
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) final
        {
            return false;
        }
        bool text_input(i_string const& aText) final
        {
            if (!active())
                return false;

            struct cleanup
            {
                emoticon_translator& et;
                bool suppress = false;
                ~cleanup()
                {
                    if (!suppress)
                    {
                        et.iBuffer.clear();
                        et.iEmoticonMatches.clear();
                    }
                }
            } c{ *this };

            thread_local std::string potentialMatch;
            potentialMatch = iBuffer;
            potentialMatch += aText.to_std_string_view();

            thread_local std::vector<emoji_map::const_iterator> emoticonMatches;

            bool const previousMatches = !iEmoticonMatches.empty();
            bool anyMatches;
            bool partialMatches;

            auto find = [&](std::string_view const& aPotentialMatch, bool aIgnorePartials = false)
                {
                    emoticonMatches.clear();
                    anyMatches = false;
                    partialMatches = false;
                    for (auto e = iEmojiMap.begin(); e != iEmojiMap.end(); ++e)
                    {
                        if (e->first.starts_with(aPotentialMatch))
                        {
                            bool const isPartial = (e->first != aPotentialMatch);
                            if (isPartial)
                            {
                                if (!aIgnorePartials)
                                    partialMatches = true;
                            }
                            if (!isPartial || !aIgnorePartials)
                            {
                                anyMatches = true;
                                emoticonMatches.push_back(e);
                            }
                        }
                    }
                };

            find(potentialMatch);

            bool haveExtra = false;

            if (!anyMatches && previousMatches)
            {
                find(static_cast<std::string_view const&>(potentialMatch).substr(0, potentialMatch.size() - 1), true);
                if (anyMatches)
                    haveExtra = true;
            }

            if (anyMatches)
            {
                std::swap(iEmoticonMatches, emoticonMatches);
                iBuffer = potentialMatch;
                if (!partialMatches)
                {
                    handle_emoticon_matches(aText.to_std_string_view(), haveExtra, c.suppress);
                    return true;
                }
                else
                    c.suppress = true;
            }

            return false;
        }
        bool sys_text_input(i_string const& aText) final
        {
            if (active())
            {
                iBuffer.clear();
                iEmoticonMatches.clear();
            }
            return false;
        }
    private:
        emoji_map iEmojiMap;
        i_widget* iActiveWidget = nullptr;
        std::optional<point> iCursorPosition;
        std::string iBuffer;
        std::vector<emoji_map::const_iterator> iEmoticonMatches;
        std::optional<std::tuple<std::chrono::steady_clock::time_point, std::string, std::string>> iLastTranslationForUndo;
        sink iSink;
    };
}

template<> neogfx::i_emoticon_translator& services::start_service<neogfx::i_emoticon_translator>()
{
    static neogfx::emoticon_translator sEmoticonTranslator;
    return sEmoticonTranslator;
}
