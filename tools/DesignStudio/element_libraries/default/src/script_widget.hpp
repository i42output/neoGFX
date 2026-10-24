// script_widget.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2021 Leigh Johnston
  
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

#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include <vector>

#include <boost/format.hpp>

#include <neolib/core/service.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/task/timer.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/settings.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include <neogfx/tools/DesignStudio/script.hpp>
#include <neogfx/tools/DesignStudio/i_script_highlighter.hpp>

namespace neogfx::DesignStudio
{
    // a script's editor; a script file (the element's id is its path) is opened in it and, if there is a script highlighter for its 
    // language (see i_script_highlighter), syntax highlighted: on a worker thread once typing has paused, the highlighting being applied 
    // only if the text has not changed since; a script that has changed (its title is marked "*") is saved when the project is (File|Save: 
    // the project is modified when it is)
    class script_widget : public window
    {
        typedef window base_type;
    private:
        // (how long typing must pause before the script is highlighted again; how often the highlighting is checked for)
        static constexpr std::chrono::milliseconds HighlightDelay{ 400 };
        static constexpr std::chrono::milliseconds HighlightPoll{ 50 };
        using span = i_script_highlighter::span;
        // (the highlighting that the worker thread has done: of the text's version)
        struct highlighting
        {
            std::uint64_t version = 0u;
            std::optional<std::vector<span>> spans;  ///< (none: the script could not be highlighted)
        };
    public:
        script_widget(i_element& aElement, i_widget& aParent, window_style aStyle) :
            window{ aParent, window_placement{ rect{ point{}, is_file(aElement.id().to_std_string()) ? size{ 640.0_dip, 480.0_dip } : size{ 256.0_dip, 160.0_dip } } }, aStyle },
            iEditor{ client_layout() }
        {
            set_minimum_size(size{ 256.0_dip, 160.0_dip });
            title_bar().set_icon(aElement.library().element_icon(aElement.type()));
            title_bar().set_title(""_s);
            create_status_bar<neogfx::status_bar>(
                neogfx::status_bar::style::DisplayMessage | neogfx::status_bar::style::DisplaySizeGrip | neogfx::status_bar::style::BackgroundAsWindowBorder );
            iDocInfo = status_bar().add_permanent_widget<text_widget>();
            iCursorInfo = status_bar().add_permanent_widget<text_widget>();
            iDocInfo->set_font_role(font_role::StatusBar);
            iCursorInfo->set_font_role(font_role::StatusBar);
            // (the text is UTF-8; its length and the cursor's position are in characters (code points), as the editor's positions are)
            auto const lead_byte = [](char aByte) { return (static_cast<unsigned char>(aByte) & 0xC0u) != 0x80u; };
            auto text_changed = [&, lead_byte]()
            {
                auto const text = iEditor.plain_text().to_std_string_view();
                iDocInfo->set_text(string{ boost::str(boost::format("Length: %1%  Lines: %2%") %
                    std::count_if(text.begin(), text.end(), lead_byte) %
                    (std::count(text.begin(), text.end(), '\n') + 1)) });
            };
            iEditor.TextChanged(text_changed);
            auto position_changed = [&, lead_byte]()
            {
                auto const text = iEditor.plain_text().to_std_string_view();
                auto const position = iEditor.cursor().position();
                std::size_t line = 1u;
                std::size_t column = 1u;
                std::size_t character = 0u;
                for (std::size_t i = 0u; i < text.size() && character < position; ++i)
                {
                    if (!lead_byte(text[i]))
                        continue;
                    ++character;
                    if (text[i] == '\n')
                    {
                        ++line;
                        column = 1u;
                    }
                    else
                        ++column;
                }
                iCursorInfo->set_text(string{ boost::str(boost::format("Ln: %1%  Col: %2%  Pos: %3%") % line % column % (position + 1)) });
            };
            iEditor.cursor().PositionChanged(position_changed); 
            iEditor.set_word_wrap(false);
            // (the script in the coding font (a setting))
            ref_ptr<i_settings> settings{ aElement.library().application() };
            auto& codingFont = settings->setting("environment.fonts_and_colors.coding_font"_s);
            auto coding_font_changed = [this, &codingFont]()
            {
                iEditor.set_font(neogfx::font{ codingFont.value<font_info>(true) });
            };
            iSink += codingFont.changing(coding_font_changed);
            iSink += codingFont.changed(coding_font_changed);
            coding_font_changed();
            open_file(aElement.id().to_std_string());
            text_changed();
            position_changed();
            if (!iPath.empty())
            {
                iSink += iEditor.TextChanged([this, &aElement]()
                {
                    if (!iModified)
                    {
                        iModified = true;
                        update_title();
                    }
                    if (!aElement.project().dirty())
                        aElement.project().set_dirty();
                });
                iSink += service<i_app>().action_file_save().triggered([this]()
                {
                    save_file();
                });
            }
        }
        ~script_widget()
        {
            if (iHighlighter.joinable())
                iHighlighter.join();
        }
    private:
        // (whether an element's id is a file's path (the window that a file is opened in is larger: its size is placed when it is made 
        // (it cannot be resized until it is nested)))
        static bool is_file(std::string const& aPath)
        {
            std::error_code ec;
            return !aPath.empty() && std::filesystem::is_regular_file(std::filesystem::path{ aPath }, ec);
        }
        // (the script's file: the element's id is its path (an element whose id is not a file's path is a new, empty, script))
        void open_file(std::string const& aPath)
        {
            if (!is_file(aPath))
                return;
            std::filesystem::path const file{ aPath };
            std::ifstream input{ file, std::ios::binary };
            std::ostringstream contents;
            contents << input.rdbuf();
            iEditor.set_plain_text(string{ contents.str() });
            iEditor.cursor().set_position(0);
            iPath = aPath;
            update_title();
            iExtension = file.extension().string();
            if (!neolib::services::get_service_provider().service_registered(i_script_highlighter::iid()))
                return;
            auto& highlighter = neolib::services::get_service_provider().service<i_script_highlighter>();
            if (!highlighter.can_highlight(string{ iExtension }))
                return;
            iScriptHighlighter = &highlighter;
            iLastEdit = std::chrono::steady_clock::now() - HighlightDelay; // (the file as opened: highlighted straight away)
            iSink += iEditor.TextChanged([this]()
            {
                ++iVersion;
                iLastEdit = std::chrono::steady_clock::now();
            });
            iHighlightPoller.emplace(neolib::service<neolib::i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                poll_highlighting();
                aTimer.again();
            }, HighlightPoll);
        }
        void update_title()
        {
            title_bar().set_title(string{ std::filesystem::path{ iPath }.filename().string() + (iModified ? " *" : "") });
        }
        // (the script, if it has changed, written to its file)
        void save_file()
        {
            if (!iModified)
                return;
            std::ofstream output{ std::filesystem::path{ iPath }, std::ios::binary | std::ios::trunc };
            auto const& text = iEditor.plain_text().to_std_string_view();
            output.write(text.data(), static_cast<std::streamsize>(text.size()));
            output.close();
            if (!output)
            {
                service<i_surface_manager>().display_error_message("Save Script"_t, string{ "Unable to write file: " + iPath });
                return;
            }
            iModified = false;
            update_title();
        }
        // GUI thread: the worker thread's highlighting applied (if the text has not changed since), or the text highlighted again (once 
        // typing has paused, if it has changed since it was last highlighted)
        void poll_highlighting()
        {
            std::optional<highlighting> done;
            {
                std::lock_guard<std::mutex> lock{ iHighlightingMutex };
                done = std::exchange(iHighlighted, std::nullopt);
            }
            if (done)
            {
                iHighlighting = false;
                if (iHighlighter.joinable())
                    iHighlighter.join();
                if (done->version == iVersion)
                {
                    iHighlightedVersion = iVersion;
                    if (done->spans)
                        apply_highlighting(*done->spans);
                }
            }
            if (iHighlighting || iHighlightedVersion == iVersion || std::chrono::steady_clock::now() - iLastEdit < HighlightDelay)
                return;
            iHighlighting = true;
            iHighlighter = std::thread{ [this, highlighter = iScriptHighlighter, extension = iExtension, script = iEditor.plain_text().to_std_string(), version = iVersion]()
            {
                highlighting result{ version };
                try
                {
                    neolib::vector<span> spans;
                    if (highlighter->highlight(string{ extension }, string{ script }, spans))
                    {
                        result.spans.emplace();
                        for (auto const& s : spans)
                            result.spans->push_back(s);
                    }
                }
                catch (...)
                {
                }
                std::lock_guard<std::mutex> lock{ iHighlightingMutex };
                iHighlighted = std::move(result);
            } };
        }
        // GUI thread: each run of the text (code points) in the style of its category (else the editor's default style); the styles of 
        // characters are changed in place (see text_edit::apply_style): the text, the cursor and the undo history are unchanged
        void apply_highlighting(std::vector<span> const& aSpans)
        {
            auto const& text = iEditor.plain_text().to_std_string_view();
            std::size_t length = 0u;
            for (auto ch : text)
                if ((static_cast<unsigned char>(ch) & 0xC0u) != 0x80u)
                    ++length;
            // (later spans (e.g. the AST's) replace earlier ones (e.g. the tokens'))
            std::vector<std::optional<i_script_highlighter::category>> categories(length);
            for (auto const& s : aSpans)
                for (auto i = std::min<std::size_t>(s.first, length); i < std::min<std::size_t>(s.last, length); ++i)
                    categories[i] = s.kind;
            if (!iCategoryStyles)
            {
                // (the syntax highlighting configuration's colours (see i_script_highlighter::colours): its dark theme's if the text is light; 
                // a category without a colour is in the editor's default style)
                neolib::vector<neolib::string> colours;
                iScriptHighlighter->colours(string{ iExtension }, string{ iEditor.default_text_color().light() ? "dark" : "light" }, colours);
                iCategoryStyles.emplace();
                for (std::uint32_t c = 0u; c <= static_cast<std::uint32_t>(i_script_highlighter::category::Parameter); ++c)
                    iCategoryStyles->push_back(c < colours.size() && !colours[c].empty() ? 
                        text_edit::style{ text_edit::character_style{}.set_text_color(color{ colours[c].to_std_string() }) } : iEditor.default_style());
            }
            for (std::size_t first = 0u; first < length;)
            {
                auto last = first + 1u;
                while (last < length && categories[last] == categories[first])
                    ++last;
                auto const& style = categories[first] ? (*iCategoryStyles)[static_cast<std::size_t>(*categories[first])] : iEditor.default_style();
                iEditor.apply_style(static_cast<text_edit::position_type>(first), static_cast<text_edit::position_type>(last), style);
                first = last;
            }
        }
    private:
        text_edit iEditor;
        ref_ptr<text_widget> iDocInfo;
        ref_ptr<text_widget> iCursorInfo;
        std::string iPath;
        bool iModified = false;
        std::string iExtension;
        i_script_highlighter const* iScriptHighlighter = nullptr;
        std::uint64_t iVersion = 0u;
        std::uint64_t iHighlightedVersion = ~std::uint64_t{ 0u };
        std::chrono::steady_clock::time_point iLastEdit;
        bool iHighlighting = false;
        std::mutex iHighlightingMutex;
        std::optional<highlighting> iHighlighted;
        std::thread iHighlighter;
        std::optional<std::vector<text_edit::style>> iCategoryStyles;
        std::optional<neolib::callback_timer> iHighlightPoller;
        sink iSink;
    };

    template <>
    struct element_traits<script_widget> : element_traits<i_element>
    {
        typedef script base;
        static constexpr bool needsCaddy = false;
    };
}
