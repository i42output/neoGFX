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
#include <neogfx/gui/widget/splitter.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include <neogfx/tools/DesignStudio/script.hpp>
#include <neogfx/tools/DesignStudio/i_script_highlighter.hpp>
#include <neogfx/tools/DesignStudio/settings.hpp>
#include "script_graph.hpp"

namespace neogfx::DesignStudio
{
    // a script's editor; a script file (the element's id is its path) is opened in it and, if there is a script highlighter for its 
    // language (see i_script_highlighter), syntax highlighted and outlined (its node graph, beside the code (see script_graph), shown or 
    // hidden by the toolbar's Node Graph button): on a worker thread once typing has paused, the highlighting being applied only if the 
    // text has not changed since; a script that has changed (its title is marked "*") is saved when the project is (File|Save: the project 
    // is modified when it is)
    class script_widget : public window
    {
        typedef window base_type;
    private:
        // (how long typing must pause before the script is highlighted again; how often the highlighting is checked for)
        static constexpr std::chrono::milliseconds HighlightDelay{ 400 };
        static constexpr std::chrono::milliseconds HighlightPoll{ 50 };
        using span = i_script_highlighter::span;
        using outline_node = i_script_highlighter::outline_node;
        // (the highlighting (and outlining) that the worker thread has done: of the text's version)
        struct highlighting
        {
            std::uint64_t version = 0u;
            std::optional<std::vector<span>> spans;             ///< (none: the script could not be highlighted)
            std::optional<std::vector<outline_node>> outline;   ///< (none: the script could not be outlined (e.g. it does not parse))
        };
    public:
        script_widget(i_element& aElement, i_widget& aParent, window_style aStyle) :
            window{ aParent, window_placement{ rect{ point{}, is_file(aElement.id().to_std_string()) ? size{ 1024.0_dip, 600.0_dip } : size{ 256.0_dip, 160.0_dip } } }, aStyle },
            iToolbar{ client_layout() },
            iSplitter{ client_layout(), splitter_style::Horizontal | splitter_style::DrawGrip },
            iEditor{ iSplitter.layout() },
            iGraph{ iSplitter.layout() },
            iShowGraph{ "Node Graph"_t, toolbar_image(sNodeGraphImagePattern) },
            iFitGraph{ "Zoom to Fit"_t, toolbar_image(sZoomToFitImagePattern) },
            iResetGraph{ "Reset Layout"_t, toolbar_image(sResetLayoutImagePattern) }
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
            // (the code and its node graph, side by side (the code narrower); the graph's node selected as the code's cursor moves, and the 
            // code of the graph's node selected selected; the graph's edits of the script are the editor's)
            iSplitter.set_size_policy(size_constraint::Expanding); // (a horizontal splitter's height is otherwise its minimum)
            iEditor.set_weight(size{ 2.0, 1.0 });
            iGraph.set_weight(size{ 3.0, 1.0 });
            iToolbar.add_action(iShowGraph);
            iToolbar.add_action(iFitGraph);
            iToolbar.add_action(iResetGraph);
            iShowGraph.set_checkable(true);
            iShowGraph.set_checked(true);
            iShowGraph.set_tool_tip_text("Show or hide the script's node graph"_t);
            iFitGraph.set_tool_tip_text("Zoom the node graph to fit"_t);
            iResetGraph.set_tool_tip_text("Reset the node graph to its automatic layout"_t);
            iSink += iShowGraph.checked([this]() { iGraph.show(); iFitGraph.enable(); iResetGraph.enable(); });
            iSink += iShowGraph.unchecked([this]() { iGraph.hide(); iFitGraph.disable(); iResetGraph.disable(); });
            iSink += iFitGraph.triggered([this]() { iGraph.zoom_to_fit(); });
            iSink += iResetGraph.triggered([this]() { iGraph.reset_layout(); });
            iSink += iEditor.cursor().PositionChanged([this]()
            {
                if (!iSelectingFromGraph)
                    iGraph.select_at(static_cast<script_graph::position_type>(iEditor.cursor().position()));
            });
            iSink += iGraph.NodeSelected([this](script_graph::position_type aFirst, script_graph::position_type aLast)
            {
                iSelectingFromGraph = true;
                iEditor.cursor().set_position(aLast);
                iEditor.cursor().set_position(aFirst, false);
                iSelectingFromGraph = false;
            });
            iSink += iGraph.ScriptEdit([this](script_graph::position_type aFirst, script_graph::position_type aLast, i_string const& aText)
            {
                iEditor.delete_text(aFirst, aLast);
                if (!aText.empty())
                    iEditor.insert_text(static_cast<text_edit::position_type>(aFirst), aText);
            });
            // (the script in the coding font (a setting))
            ref_ptr<i_settings> settings{ aElement.library().application() };
            auto& codingFont = settings->setting("environment.fonts_and_colors.coding_font"_s);
            auto coding_font_changed = [this, &codingFont]()
            {
                iEditor.set_font(neogfx::font{ codingFont.value<font_info>(true) });
                iGraph.set_font(neogfx::font{ codingFont.value<font_info>(true) });
                iGraph.refresh_layout();
            };
            iSink += codingFont.changing(coding_font_changed);
            iSink += codingFont.changed(coding_font_changed);
            coding_font_changed();
            // (the graph's grid in the workspace's grid colour (a setting))
            auto& gridColour = settings->setting("environment.workspace.grid_color"_s);
            auto grid_colour_changed = [this, &gridColour]()
            {
                iGraph.set_grid_colour(gridColour.value<neogfx::gradient>(true));
            };
            iSink += gridColour.changing(grid_colour_changed);
            iSink += gridColour.changed(grid_colour_changed);
            grid_colour_changed();
            // (the toolbar's icons in the script editors' icon size (a setting))
            auto& iconSize = settings->setting("environment.toolbars.script_editor_icon_size"_s);
            auto icon_size_changed = [this, &iconSize]()
            {
                switch (iconSize.value<toolbar_icon_size>(true))
                {
                case toolbar_icon_size::Size16x16:
                default:
                    iToolbar.set_button_image_extents(size{ 16.0_dip, 16.0_dip });
                    break;
                case toolbar_icon_size::Size24x24:
                    iToolbar.set_button_image_extents(size{ 24.0_dip, 24.0_dip });
                    break;
                case toolbar_icon_size::Size32x32:
                    iToolbar.set_button_image_extents(size{ 32.0_dip, 32.0_dip });
                    break;
                case toolbar_icon_size::Size48x48:
                    iToolbar.set_button_image_extents(size{ 48.0_dip, 48.0_dip });
                    break;
                case toolbar_icon_size::Size64x64:
                    iToolbar.set_button_image_extents(size{ 64.0_dip, 64.0_dip });
                    break;
                }
            };
            iSink += iconSize.changing(icon_size_changed);
            iSink += iconSize.changed(icon_size_changed);
            icon_size_changed();
            open_file(aElement.id().to_std_string());
            if (!iScriptHighlighter)
            {
                // (a script that is not highlighted (nor outlined) has no node graph)
                iToolbar.hide();
                iGraph.hide();
            }
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
                    // (the graph shows the last outline (the script may not parse as it is being edited))
                    if (done->outline)
                        iGraph.set_script(neolib::utf8_to_utf32(iEditor.plain_text().to_std_string()), done->spans ? *done->spans : std::vector<span>{}, *done->outline);
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
                    neolib::vector<outline_node> outline;
                    if (highlighter->outline(string{ extension }, string{ script }, outline))
                        result.outline.emplace(outline.begin(), outline.end());
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
                std::vector<std::optional<color>> graphColours;
                for (std::uint32_t c = 0u; c <= static_cast<std::uint32_t>(i_script_highlighter::category::Parameter); ++c)
                {
                    iCategoryStyles->push_back(c < colours.size() && !colours[c].empty() ? 
                        text_edit::style{ text_edit::character_style{}.set_text_color(color{ colours[c].to_std_string() }) } : iEditor.default_style());
                    graphColours.push_back(c < colours.size() && !colours[c].empty() ? std::optional<color>{ color{ colours[c].to_std_string() } } : std::nullopt);
                }
                // (the graph's background the editor's: its paper colour, else its background colour)
                auto const& paper = iEditor.default_style().character().paper_color();
                iGraph.set_colours(graphColours, iEditor.default_text_color(), 
                    std::holds_alternative<color>(paper) ? std::get<color>(paper) : iEditor.background_color());
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
        // (the toolbar's buttons' images: an image of a pattern in the text's colour)
        static image toolbar_image(char const* aPattern)
        {
            return image{ string{ aPattern }, { { "paper", color{} }, { "ink1", service<i_app>().current_style().palette().color(color_role::Text) } } };
        }
        static constexpr char const* sNodeGraphImagePattern
        {
            "[16,16]"
            "{0,paper}"
            "{1,ink1}"

            "0000000000000000"
            "0111110000000000"
            "0100010000000000"
            "0100011111100000"
            "0100010000100000"
            "0111110000100000"
            "0000000000100000"
            "0000000000100000"
            "0000000000100000"
            "0000000001111100"
            "0000000001000100"
            "0000000001000100"
            "0000000001000100"
            "0000000001111100"
            "0000000000000000"
            "0000000000000000"
        };
        static constexpr char const* sZoomToFitImagePattern
        {
            "[16,16]"
            "{0,paper}"
            "{1,ink1}"

            "0000000000000000"
            "0111100000011110"
            "0100000000000010"
            "0100000000000010"
            "0100000000000010"
            "0000000000000000"
            "0000000000000000"
            "0000000000000000"
            "0000000000000000"
            "0000000000000000"
            "0000000000000000"
            "0100000000000010"
            "0100000000000010"
            "0100000000000010"
            "0111100000011110"
            "0000000000000000"
        };
        static constexpr char const* sResetLayoutImagePattern
        {
            "[16,16]"
            "{0,paper}"
            "{1,ink1}"

            "0000000000000000"
            "0111110001111100"
            "0100010001000100"
            "0100010001000100"
            "0100010001000100"
            "0111110001111100"
            "0000000000000000"
            "0000000000000000"
            "0000000000000000"
            "0000000000000000"
            "0111110001111100"
            "0100010001000100"
            "0100010001000100"
            "0100010001000100"
            "0111110001111100"
            "0000000000000000"
        };
    private:
        neogfx::toolbar iToolbar;
        splitter iSplitter;
        text_edit iEditor;
        script_graph iGraph;
        neogfx::action iShowGraph;
        neogfx::action iFitGraph;
        neogfx::action iResetGraph;
        bool iSelectingFromGraph = false;
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
