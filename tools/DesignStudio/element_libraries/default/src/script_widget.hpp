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
#include <functional>
#include <sstream>
#include <thread>
#include <vector>

#include <boost/format.hpp>

#include <neolib/core/service.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/task/timer.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_resource_manager.hpp>
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
#include <neogfx/tools/DesignStudio/i_script_debugger.hpp>
#include <neogfx/tools/DesignStudio/settings.hpp>
#include "script_graph.hpp"
#include "console_widget.hpp"

namespace neogfx::DesignStudio
{
    // a script's code (see script_widget): its editor, with a gutter (beside the text: the editor's left padding) that shows the 
    // script's breakpoints (clicked: a breakpoint toggled on the line) and, while a program being debugged is stopped in the script, 
    // the statement that it is at (highlighted, and an arrow in the gutter)
    class script_editor : public text_edit
    {
        typedef text_edit base_type;
    public:
        using breakpoint_state = i_script_debugger::breakpoint_state;
        // (a statement: its text [first, last) (code points); top: the program's innermost frame is at it (else a frame that called it))
        struct statement
        {
            position_type first;
            position_type last;
            bool top;
        };
    public:
        define_event(GutterClicked, gutter_clicked, std::uint32_t) ///< (a line (1-based))
    private:
        static constexpr dimension GutterWidth = 16.0;  ///< (device-independent pixels)
    public:
        script_editor(i_layout& aLayout) :
            base_type{ aLayout }
        {
            auto gutter = padding();
            gutter.left += dpi_scale(GutterWidth);
            set_padding(gutter);
            iSink += TextChanged([this]() { iLineStarts.reset(); });
        }
    public:
        // (the state of the breakpoints on a line (1-based), as the gutter shows them)
        void set_breakpoint_query(std::function<breakpoint_state(std::uint32_t)> aQuery)
        {
            iBreakpointAt = std::move(aQuery);
            update();
        }
        void set_statement(std::optional<statement> const& aStatement)
        {
            iStatement = aStatement;
            update();
        }
        // (a position's line (1-based); the position of a line's start; positions and (1-based) columns in bytes of the text (UTF-8))
        std::uint32_t line_of(position_type aPosition) const
        {
            auto const& starts = line_starts();
            return static_cast<std::uint32_t>(std::upper_bound(starts.begin(), starts.end(), aPosition) - starts.begin());
        }
        position_type line_start(std::uint32_t aLine) const
        {
            auto const& starts = line_starts();
            return starts[std::clamp<std::size_t>(aLine, 1u, starts.size()) - 1u];
        }
        position_type to_position(std::uint32_t aLine, std::uint32_t aColumn) const
        {
            auto const text = plain_text().to_std_string_view();
            auto const start = line_start(aLine);
            auto result = start;
            // (the column's byte, in the line (not past its end): its code point)
            for (auto byte = byte_of(start); aColumn > 1u && byte < text.size() && text[byte] != '\n'; --aColumn)
                if (++byte < text.size() && lead_byte(text[byte]))
                    ++result;
            return result;
        }
        std::uint32_t column_of(position_type aPosition) const
        {
            return static_cast<std::uint32_t>(byte_of(aPosition) - byte_of(line_start(line_of(aPosition))) + 1u);
        }
    protected:
        void paint(i_graphics_context& aGc) const override
        {
            base_type::paint(aGc);
            auto const inner = client_rect(false);
            auto const gutterWidth = dpi_scale(GutterWidth);
            rect const gutter{ point{ inner.left() - gutterWidth, client_rect().top() }, size{ gutterWidth, client_rect().cy } };
            // (the gutter in the text's background (the text, scrolled horizontally, is not drawn there))
            auto const& paper = default_style().character().paper_color();
            aGc.fill_rect(gutter, std::holds_alternative<color>(paper) ? std::get<color>(paper) : background_color());
            auto const& starts = line_starts();
            auto const firstVisible = line_of(document_hit_test(point{ inner.left(), inner.top() }));
            auto const statementFirst = iStatement ? line_of(iStatement->first) : 0u;
            auto const statementLast = iStatement ? std::max(statementFirst, line_of(std::max(iStatement->first, iStatement->last - 1))) : 0u;
            color const statementColour = iStatement && iStatement->top ? color{ 0xFF, 0xE0, 0x66 } : color{ 0x8C, 0xD8, 0x8C };
            for (auto line = std::max(firstVisible, 1u); line <= starts.size(); ++line)
            {
                auto const r = line_rect(starts[line - 1u]);
                if (!r || r->top() > client_rect().bottom())
                    break;
                if (r->bottom() < client_rect().top())
                    continue;
                point const centre{ gutter.left() + gutterWidth / 2.0, r->top() + r->cy / 2.0 };
                auto const radius = std::max(2.0, std::min(gutterWidth, r->cy) * 0.33);
                if (line >= statementFirst && line <= statementLast)
                    aGc.fill_rect(rect{ point{ inner.left(), r->top() }, size{ inner.cx, r->cy } }, statementColour.with_alpha(0.3));
                if (iBreakpointAt)
                    switch (iBreakpointAt(line))
                    {
                    case breakpoint_state::Enabled:
                        aGc.fill_circle(centre, radius, color{ 0xE5, 0x14, 0x00 });
                        break;
                    case breakpoint_state::Disabled:
                        aGc.draw_circle(centre, radius, pen{ color{ 0xE5, 0x14, 0x00 }, std::max(1.0, dpi_scale(1.0)) });
                        break;
                    default:
                        break;
                    }
                if (line == statementFirst)
                {
                    // (the arrow: on the breakpoint, if there is one)
                    auto const half = radius * 1.1;
                    aGc.fill_triangle(
                        point{ centre.x - half, centre.y - half }, point{ centre.x + half, centre.y }, point{ centre.x - half, centre.y + half },
                        statementColour);
                    aGc.draw_triangle(
                        point{ centre.x - half, centre.y - half }, point{ centre.x + half, centre.y }, point{ centre.x - half, centre.y + half },
                        pen{ statementColour.darker(0x60), 1.0 });
                }
            }
        }
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override
        {
            auto const inner = client_rect(false);
            if (aButton == mouse_button::Left && aPosition.x < inner.left() && aPosition.x >= inner.left() - dpi_scale(GutterWidth))
            {
                // (the line beside the click)
                GutterClicked(line_of(document_hit_test(point{ inner.left(), aPosition.y })));
                return;
            }
            base_type::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        }
    private:
        static bool lead_byte(char aByte)
        {
            return (static_cast<unsigned char>(aByte) & 0xC0u) != 0x80u;
        }
        // (the byte (of the text: UTF-8) that a position's code point begins at)
        std::size_t byte_of(position_type aPosition) const
        {
            auto const text = plain_text().to_std_string_view();
            position_type character = -1;
            for (std::size_t byte = 0u; byte < text.size(); ++byte)
                if (lead_byte(text[byte]) && ++character == aPosition)
                    return byte;
            return text.size();
        }
        // (the position (code point) of each line's start)
        std::vector<position_type> const& line_starts() const
        {
            if (!iLineStarts)
            {
                iLineStarts.emplace(1u, position_type{ 0 });
                auto const text = plain_text().to_std_string_view();
                position_type character = 0;
                for (auto ch : text)
                {
                    if (!lead_byte(ch))
                        continue;
                    ++character;
                    if (ch == '\n')
                        iLineStarts->push_back(character);
                }
            }
            return *iLineStarts;
        }
    private:
        std::function<breakpoint_state(std::uint32_t)> iBreakpointAt;
        std::optional<statement> iStatement;
        mutable std::optional<std::vector<position_type>> iLineStarts;
        sink iSink;
    };

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
            iElement{ aElement },
            iToolbar{ client_layout() },
            iSplitter{ client_layout(), splitter_style::Horizontal | splitter_style::DrawGrip },
            iEditor{ iSplitter.layout() },
            iGraph{ iSplitter.layout() },
            iDebug{ "Debug"_t, debug_image() },
            iRun{ "Run"_t, run_image() },
            iShowGraph{ "Node Graph"_t, toolbar_image(sNodeGraphImagePattern) },
            iFitGraph{ "Zoom to Fit"_t, toolbar_image(sZoomToFitImagePattern) },
            iResetGraph{ "Reset Layout"_t, toolbar_image(sResetLayoutImagePattern) }
        {
            set_minimum_size(size{ 256.0_dip, 160.0_dip });
            // (a script's file: its window sized to 75% of the workspace once nested (MDI) in it, unless it is maximized there)
            if (is_file(aElement.id().to_std_string()) && has_native_window())
                iSink += native_window().added_to_nest([this]()
                {
                    auto& nest = service<i_surface_manager>().find_nest(native_window());
                    if (iSizedToWorkspace || nest.type() != nest_type::MDI || native_window().is_maximized())
                        return;
                    iSizedToWorkspace = true;
                    resize(nest.widget().client_rect(false).extents() * 0.75);
                });
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
            iToolbar.add_action(iDebug);
            iToolbar.add_action(iRun);
            iToolbar.add_separator();
            iToolbar.add_action(iShowGraph);
            iToolbar.add_action(iFitGraph);
            iToolbar.add_action(iResetGraph);
            iShowGraph.set_checkable(true);
            iShowGraph.set_checked(true);
            iShowGraph.set_tool_tip_text("Show or hide the script's node graph"_t);
            iRun.set_tool_tip_text("Run the script in a neos console, without debugging it (Ctrl+F5)"_t);
            iRun.set_enabled(false); // (unless its language is one that neos runs: see open_file)
            iSink += iRun.triggered([this]() { run(false); });
            iDebug.set_tool_tip_text("Debug the script in a neos console (F5)"_t);
            iDebug.set_enabled(false);
            iSink += iDebug.triggered([this]() { run(true); });
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
                case toolbar_icon_size::Size8x8:
                    iToolbar.set_button_image_extents(size{ 8.0_dip, 8.0_dip });
                    break;
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
                connect_debugger();
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
            std::erase(editors(), this);
            if (iDebugger && iDebugger->active_script().to_std_string() == iPath)
                iDebugger->set_active_script(string{});
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
            // (it can be run (its syntax errors, if any, are neos's to report when it is compiled: the highlighter's grammar, which the 
            // node graph's outline comes from, need not be all of the language's))
            iRun.set_enabled(true);
            iLastEdit = std::chrono::steady_clock::now() - HighlightDelay; // (the file as opened: highlighted straight away)
            iSink += iEditor.TextChanged([this]()
            {
                ++iVersion;
                iLastEdit = std::chrono::steady_clock::now();
                iGraph.script_changed();
            });
            iHighlightPoller.emplace(neolib::service<neolib::i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                poll_highlighting();
                aTimer.again();
            }, HighlightPoll);
        }
        // (the script (saved, if it has changed) run (debugged, if aDebug) in a neos console: the one that it was last run in, if it is 
        // still open (its neos session reused if it has one), else a new one (opened in the workspace as a console dropped on it is); a 
        // beep if it cannot be (e.g. the script is still running))
        void run(bool aDebug)
        {
            if (iPath.empty() || !has_parent())
            {
                service<i_basic_services>().system_beep();
                return;
            }
            save_file();
            if (iDebugger)
                iDebugger->set_active_script(string{ iPath }); // (if it stops in another script, we open it (see show_location))
            if (!iConsole)
                open_console();
            // (a neos session that only runs programs (only the script's input and output is shown in the console))
            if (!iConsole || !(iConsole->has_session() ? 
                iConsole->execute((aDebug ? "debug " : "run ") + iPath) : iConsole->start_session((aDebug ? "neos -g " : "neos ") + iPath)))
            {
                service<i_basic_services>().system_beep();
                return;
            }
            iConsole->activate_console();
        }
        // (a console opened in the workspace (its view stack: our parent) beside us, as a window opened in it is (see the main window's 
        // Open Script))
        void open_console()
        {
            auto& workspace = parent();
            auto& project = iElement.project();
            auto& element = project.create_element(project.root(), "console"_s);
            auto scratch = make_ref<widget<>>(workspace.root().as_widget());
            element.create_layout_item(*scratch);
            if (!element.has_layout_item() || !element.layout_item().is_widget())
                return;
            auto consoleWindow = ref_ptr<i_widget>{ element.layout_item().as_widget() };
            workspace.add(consoleWindow);
            if (consoleWindow->is_root() && consoleWindow->root().is_nested())
                service<i_surface_manager>().nest_for(workspace, nest_type::MDI).add(consoleWindow->root().native_window());
            consoleWindow->move(position() + point{ 48.0_dip, 48.0_dip });
            iConsole = dynamic_cast<console_widget*>(&*consoleWindow);
            iConsoleSink.clear();
            if (iConsole)
                iConsoleSink += iConsole->destroying([this]() { iConsole = nullptr; });
        }
        // (the script's debugging (see i_script_debugger), if there is a debugger: its breakpoints in the gutter and the node graph, the 
        // statement that the program being debugged is at, and the debugger's requests of the active script's editor (ours, once we are 
        // activated (or our code or graph has the focus)))
        void connect_debugger()
        {
            editors().push_back(this);
            if (!neolib::services::get_service_provider().service_registered(i_script_debugger::iid()))
                return;
            iDebugger = &neolib::services::get_service_provider().service<i_script_debugger>();
            iDebug.set_enabled(iScriptHighlighter != nullptr);
            iEditor.set_breakpoint_query([this](std::uint32_t aLine) { return iDebugger->breakpoint_at(string{ iPath }, aLine); });
            iGraph.set_breakpoint_query([this](script_graph::position_type aFirst)
            {
                // (a statement's (its column's), or (its line's first statement) its line's)
                auto const position = static_cast<text_edit::position_type>(aFirst);
                auto const line = iEditor.line_of(position);
                auto const column = iEditor.column_of(position);
                auto const statement = iDebugger->breakpoint_at(string{ iPath }, line, column);
                if (statement != i_script_debugger::breakpoint_state::None)
                    return statement;
                auto const text = iEditor.plain_text().to_std_string_view();
                auto const lineByte = column_byte(iEditor.line_start(line));
                auto const firstByte = text.find_first_not_of(" \t", lineByte);
                if (firstByte == std::string_view::npos || column != firstByte - lineByte + 1u)
                    return i_script_debugger::breakpoint_state::None;
                return iDebugger->breakpoint_at(string{ iPath }, line);
            });
            iSink += iEditor.GutterClicked([this](std::uint32_t aLine) { iDebugger->toggle_breakpoint(string{ iPath }, aLine); });
            auto activated = [this]() { iDebugger->set_active_script(string{ iPath }); };
            iSink += Activated(activated);
            iSink += iEditor.Focus([activated](neogfx::focus_event aEvent, focus_reason) { if (aEvent == neogfx::focus_event::FocusGained) activated(); });
            iSink += iGraph.Focus([activated](neogfx::focus_event aEvent, focus_reason) { if (aEvent == neogfx::focus_event::FocusGained) activated(); });
            iSink += iDebugger->requested([this](i_script_debugger::request aRequest)
            {
                if (iDebugger->active_script().to_std_string() != iPath)
                    return;
                switch (aRequest)
                {
                case i_script_debugger::request::Start:
                case i_script_debugger::request::StartWithoutDebugging:
                    if (iRun.is_enabled())
                        run(aRequest == i_script_debugger::request::Start);
                    else
                        service<i_basic_services>().system_beep(); // (it is not in a language that neos runs)
                    break;
                case i_script_debugger::request::ToggleBreakpoint:
                    // (the graph's node selected (its statement), if the graph has the focus; else the cursor's line)
                    if (iGraph.has_focus())
                    {
                        if (auto const first = iGraph.selected_statement())
                        {
                            auto const position = static_cast<text_edit::position_type>(*first);
                            iDebugger->toggle_breakpoint(string{ iPath }, iEditor.line_of(position), iEditor.column_of(position));
                        }
                    }
                    else
                        iDebugger->toggle_breakpoint(string{ iPath }, iEditor.line_of(static_cast<text_edit::position_type>(iEditor.cursor().position())));
                    break;
                }
            });
            iSink += iDebugger->breakpoints_changed([this]()
            {
                iEditor.update();
                iGraph.update();
            });
            iSink += iDebugger->location_changed([this]() { show_location(true, true); });
            show_location(true, false);
        }
        // (the statement that the program being debugged is at (in its frame selected), shown if it is in our script (aFollow: the cursor 
        // moved to it; aActivate: and we are activated); if no editor has its script open, the active script's editor opens it)
        void show_location(bool aFollow, bool aActivate)
        {
            neolib::string path;
            std::uint32_t line = 0u, column = 0u, endLine = 0u, endColumn = 0u;
            bool top = true;
            if (!iDebugger->location(path, line, column, endLine, endColumn, top) || !same_file(path.to_std_string(), iPath))
            {
                iEditor.set_statement({});
                iGraph.set_statement({}, true);
                if (aActivate && !path.empty() && iDebugger->active_script().to_std_string() == iPath &&
                    std::none_of(editors().begin(), editors().end(), [&](script_widget* aEditor) { return same_file(path.to_std_string(), aEditor->iPath); }))
                    open_script(path.to_std_string());
                return;
            }
            auto const first = column != 0u ? iEditor.to_position(line, column) : iEditor.line_start(line);
            auto const last = endLine != 0u && endColumn != 0u ? iEditor.to_position(endLine, endColumn) : 
                (line < iEditor.line_of(static_cast<text_edit::position_type>(iEditor.plain_text().size())) ? iEditor.line_start(line + 1u) - 1 : 
                    static_cast<text_edit::position_type>(iEditor.plain_text().size()));
            iEditor.set_statement(script_editor::statement{ first, std::max(first + 1, last), top });
            iGraph.set_statement(std::make_pair(static_cast<script_graph::position_type>(first), static_cast<script_graph::position_type>(std::max(first + 1, last))), top);
            if (aFollow)
                iEditor.cursor().set_position(static_cast<neogfx::cursor::position_type>(first));
            // (activated for the innermost frame (where it has stopped), not as callers' frames are selected in the call stack)
            if (aActivate && top)
            {
                root().activate();
                iEditor.set_focus();
            }
        }
        // (the byte that a position's code point begins at, in the text)
        std::size_t column_byte(text_edit::position_type aPosition) const
        {
            auto const text = iEditor.plain_text().to_std_string_view();
            text_edit::position_type character = -1;
            for (std::size_t byte = 0u; byte < text.size(); ++byte)
                if ((static_cast<unsigned char>(text[byte]) & 0xC0u) != 0x80u && ++character == aPosition)
                    return byte;
            return text.size();
        }
        static bool same_file(std::string const& aLhs, std::string const& aRhs)
        {
            if (aLhs == aRhs)
                return true;
            std::error_code ec;
            return std::filesystem::equivalent(std::filesystem::path{ aLhs }, std::filesystem::path{ aRhs }, ec) && !ec;
        }
        // (the script editors (of this library) open)
        static std::vector<script_widget*>& editors()
        {
            static std::vector<script_widget*> sEditors;
            return sEditors;
        }
        // (a script opened in the workspace (our parent) beside us, as the main window's Open Script opens one)
        void open_script(std::string const& aPath)
        {
            if (!has_parent())
                return;
            auto& workspace = parent();
            auto& project = iElement.project();
            auto& element = project.create_element(project.root(), "script"_s, string{ aPath });
            auto scratch = make_ref<widget<>>(workspace.root().as_widget());
            element.create_layout_item(*scratch);
            if (!element.has_layout_item() || !element.layout_item().is_widget())
                return;
            auto scriptWindow = ref_ptr<i_widget>{ element.layout_item().as_widget() };
            workspace.add(scriptWindow);
            if (scriptWindow->is_root() && scriptWindow->root().is_nested())
                service<i_surface_manager>().nest_for(workspace, nest_type::MDI).add(scriptWindow->root().native_window());
            scriptWindow->move(position() + point{ 32.0_dip, 32.0_dip });
            if (scriptWindow->is_root())
                scriptWindow->root().activate();
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
                for (std::uint32_t c = 0u; c <= static_cast<std::uint32_t>(i_script_highlighter::category::Variable); ++c)
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
        // (the Debug button's image: the Run button's triangle, filled)
        static image debug_image()
        {
            static string const sUri{ ":/neogfx/DesignStudio/default_nel/resources/script_debug.svg" };
            static bool const sAdded = (service<i_resource_manager>().add_resource(sUri, sDebugImage, std::char_traits<char>::length(sDebugImage)), true);
            (void)sAdded;
            return image{ sUri };
        }
        static constexpr char const* sDebugImage
        {
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\" viewBox=\"0 0 24 24\">"
            "<path d=\"M6.5 6 Q6.5 4.5 7.82 5.21 L19.18 11.29 Q20.5 12 19.18 12.71 L7.82 18.79 Q6.5 19.5 6.5 18 Z\" fill=\"#94F591\" stroke=\"#94F591\" stroke-width=\"2\" stroke-linejoin=\"round\"/>"
            "</svg>"
        };
        // (the Run button's image: a light green outlined play triangle, its corners slightly rounded (an SVG, a resource of its own: 
        // rasterised larger than the largest toolbar icon size, so that it is sharp at any))
        static image run_image()
        {
            static string const sUri{ ":/neogfx/DesignStudio/default_nel/resources/script_run.svg" };
            static bool const sAdded = (service<i_resource_manager>().add_resource(sUri, sRunImage, std::char_traits<char>::length(sRunImage)), true);
            (void)sAdded;
            return image{ sUri };
        }
        static constexpr char const* sRunImage
        {
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\" viewBox=\"0 0 24 24\">"
            "<path d=\"M6.5 6 Q6.5 4.5 7.82 5.21 L19.18 11.29 Q20.5 12 19.18 12.71 L7.82 18.79 Q6.5 19.5 6.5 18 Z\" fill=\"none\" stroke=\"#94F591\" stroke-width=\"2\" stroke-linejoin=\"round\"/>"
            "</svg>"
        };
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
        i_element& iElement;
        neogfx::toolbar iToolbar;
        splitter iSplitter;
        script_editor iEditor;
        script_graph iGraph;
        neogfx::action iDebug;
        neogfx::action iRun;
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
        console_widget* iConsole = nullptr;             ///< (the console that the script was last run in (while it is open))
        i_script_debugger* iDebugger = nullptr;
        bool iSizedToWorkspace = false;
        sink iConsoleSink;
        sink iSink;
    };

    template <>
    struct element_traits<script_widget> : element_traits<i_element>
    {
        typedef script base;
        static constexpr bool needsCaddy = false;
    };
}
