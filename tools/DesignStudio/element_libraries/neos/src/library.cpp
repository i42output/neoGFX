// library.cpp
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

#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <streambuf>
#include <istream>
#include <ostream>
#include <optional>
#include <utility>
#include <map>
#include <set>
#include <vector>
#include <functional>
#include <chrono>
#include <string_view>
#include <filesystem>
#include <cctype>
#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <neolib/core/string_ci.hpp>
#include <neolib/core/service.hpp>
#include <neolib/task/timer.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/utility.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/element.hpp>
#include <neogfx/tools/DesignStudio/console_client.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/dockable.hpp>
#include <neogfx/gui/widget/tab_page_container.hpp>
#include <neogfx/gui/widget/tab_page.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/tools/DesignStudio/i_ide.hpp>
#include <neogfx/tools/DesignStudio/i_script_highlighter.hpp>
#include <neogfx/tools/DesignStudio/i_script_debugger.hpp>
#include <neos/context.hpp>
#include <neos/debug.hpp>
#include <neos/console.hpp>
#include <neos/language/schema.hpp>
#include "library.hpp"

namespace neogfx::DesignStudio
{
    // the debugger's panes (docked in the main window while a program is being debugged): its call stack (and why it stopped), the
    // variables of the frame selected (its locals, then the globals), its watches (expressions evaluated in that frame), its threads
    // and its breakpoints (and catchpoints)
    class neos_debug_pane : public tab_page_container<>
    {
    public:
        // (a value shown (a variable, a watch): its parts (shown as its children) as many as were fetched)
        struct value
        {
            std::string name;
            std::string type;
            std::string text;
            std::vector<value> parts;
        };
        struct breakpoint_row
        {
            std::uint32_t id;
            std::string text;
            std::string condition;
            std::uint32_t hits;
            bool enabled;
        };
    public:
        define_event(FrameSelected, frame_selected, std::size_t)
        define_event(ThreadSelected, thread_selected, std::uint32_t)
        define_event(WatchAdded, watch_added, std::string const&)
        define_event(WatchRemoved, watch_removed, std::size_t)
        define_event(BreakpointAdded, breakpoint_added, std::string const&)
        define_event(BreakpointRemoved, breakpoint_removed, std::uint32_t)
        define_event(BreakpointToggled, breakpoint_toggled, std::uint32_t)
    private:
        // (a tree of values (Locals, Watch): the values expanded stay expanded (by their names' paths) as they are shown again)
        class value_table
        {
        public:
            value_table(i_layout& aLayout) :
                iView{ aLayout, true, frame_style::NoFrame }
            {
                iModel.set_column_name(0u, "Name"_t);
                iModel.set_column_name(1u, "Value"_t);
                iModel.set_column_name(2u, "Type"_t);
                iPresentation.set_item_model(iModel);
                for (item_model_index::column_type c = 0u; c < 3u; ++c)
                    iPresentation.set_column_read_only(c);
                iPresentation.set_alternating_row_color(true);
                iView.set_minimum_size(size{ 128.0_dip, 96.0_dip });
                iView.set_presentation_model(iPresentation);
                iView.column_header().set_expand_last_column(last_column_expansion::ExpandToFitView);
                iSink += iPresentation.ItemExpanded([this](item_presentation_model_index const& aIndex)
                {
                    if (auto const path = path_of(aIndex))
                        iExpanded.insert(*path);
                });
                iSink += iPresentation.ItemCollapsed([this](item_presentation_model_index const& aIndex)
                {
                    if (auto const path = path_of(aIndex))
                        iExpanded.erase(*path);
                });
            }
        public:
            table_view& view() { return iView; }
            std::optional<std::size_t> selected_row() const
            {
                if (!iView.selection_model().has_current_index())
                    return {};
                // (the top-level row that it is in: its place among the top-level rows)
                auto index = iPresentation.to_item_model_index(iView.selection_model().current_index());
                while (iModel.has_parent(index))
                    index = iModel.parent(index);
                std::size_t result = 0u;
                for (item_model_index::row_type row = 0u; row < index.row(); ++row)
                    if (!iModel.has_parent(item_model_index{ row, 0u }))
                        ++result;
                return result;
            }
            void set(std::vector<value> const& aValues)
            {
                iUpdating = true;
                iModel.clear();
                iPaths.clear();
                std::vector<i_item_model::iterator> expand;
                for (auto const& v : aValues)
                    add(std::nullopt, v, v.name, expand);
                // (parents first (pre-order): a child is expanded once its parent has been)
                for (auto const& node : expand)
                    iPresentation.expand(iPresentation.from_item_model_index(iModel.iterator_to_index(node)));
                iUpdating = false;
            }
        private:
            void add(std::optional<i_item_model::iterator> const& aParent, value const& aValue, std::string const& aPath, std::vector<i_item_model::iterator>& aExpand)
            {
                iPaths.push_back(aPath);
                auto* const tag = reinterpret_cast<void*>(iPaths.size()); // (its path: iPaths[tag - 1])
                auto const node = aParent ?
                    iModel.append_item(*aParent, tag, item_cell_data{ string{ aValue.name } }) :
                    iModel.append_item(tag, item_cell_data{ string{ aValue.name } });
                iModel.insert_cell_data(node, 1u, item_cell_data{ string{ aValue.text } });
                iModel.insert_cell_data(node, 2u, item_cell_data{ string{ aValue.type } });
                if (!aValue.parts.empty() && iExpanded.contains(aPath))
                    aExpand.push_back(node);
                for (auto const& part : aValue.parts)
                    add(node, part, aPath + '\x1F' + part.name, aExpand);
            }
            std::optional<std::string> path_of(item_presentation_model_index const& aIndex) const
            {
                if (iUpdating)
                    return {};
                auto const tag = reinterpret_cast<std::size_t>(iModel.item(iPresentation.to_item_model_index(aIndex)));
                if (tag == 0u || tag > iPaths.size())
                    return {};
                return iPaths[tag - 1u];
            }
        private:
            table_view iView;
            item_tree_model iModel;
            basic_item_presentation_model<item_tree_model> iPresentation;
            std::vector<std::string> iPaths;
            std::set<std::string> iExpanded;
            bool iUpdating = false;
            sink iSink;
        };
        // (a table of rows (Call Stack, Threads, Breakpoints): the row selected is reported (not as it is filled))
        class row_table
        {
        public:
            row_table(i_layout& aLayout, std::initializer_list<string> aColumns) :
                iView{ aLayout, true, frame_style::NoFrame }
            {
                item_model_index::column_type column = 0u;
                for (auto const& name : aColumns)
                    iModel.set_column_name(column++, name);
                iPresentation.set_item_model(iModel);
                for (item_model_index::column_type c = 0u; c < column; ++c)
                    iPresentation.set_column_read_only(c);
                iPresentation.set_alternating_row_color(true);
                iView.set_minimum_size(size{ 128.0_dip, 96.0_dip });
                iView.set_presentation_model(iPresentation);
                iView.column_header().set_expand_last_column(last_column_expansion::ExpandToFitView);
                iSink += iView.selection_model().current_index_changed([this](optional_item_presentation_model_index const& aCurrent, optional_item_presentation_model_index const&)
                {
                    if (!iUpdating && aCurrent)
                        RowSelected(iPresentation.to_item_model_index(*aCurrent).row());
                });
            }
        public:
            define_event(RowSelected, row_selected, std::size_t)
        public:
            table_view& view() { return iView; }
            std::optional<std::size_t> selected_row() const
            {
                if (!iView.selection_model().has_current_index())
                    return {};
                return iPresentation.to_item_model_index(iView.selection_model().current_index()).row();
            }
            void set(std::vector<std::vector<std::string>> const& aRows, std::optional<std::size_t> aSelected = {})
            {
                iUpdating = true;
                iModel.clear();
                for (auto const& row : aRows)
                {
                    auto const item = iModel.append_item(static_cast<void*>(nullptr), item_cell_data{ string{ row.empty() ? std::string{} : row[0] } });
                    for (std::size_t c = 1u; c < row.size(); ++c)
                        iModel.insert_cell_data(item, static_cast<item_model_index::column_type>(c), item_cell_data{ string{ row[c] } });
                }
                if (aSelected && *aSelected < aRows.size())
                    iView.selection_model().set_current_index(iPresentation.from_item_model_index(item_model_index{ static_cast<item_model_index::row_type>(*aSelected), 0u }));
                iUpdating = false;
            }
        private:
            table_view iView;
            item_model iModel;
            item_presentation_model iPresentation;
            bool iUpdating = false;
            sink iSink;
        };
    public:
        neos_debug_pane() :
            tab_page_container<>{ false, tab_container_style::TabAlignmentBottom },
            iCallStackTab{ *this, "Call Stack"_t },
            iCallStackLayout{ iCallStackTab },
            iStatus{ iCallStackLayout },
            iCallStack{ iCallStackLayout, { "Function"_t, "Location"_t } },
            iLocalsTab{ *this, "Locals"_t },
            iLocalsLayout{ iLocalsTab },
            iLocals{ iLocalsLayout },
            iWatchTab{ *this, "Watch"_t },
            iWatchLayout{ iWatchTab },
            iWatchEntryLayout{ iWatchLayout },
            iWatchEntry{ iWatchEntryLayout },
            iAddWatch{ iWatchEntryLayout, "Add"_t },
            iRemoveWatch{ iWatchEntryLayout, "Remove"_t },
            iWatches{ iWatchLayout },
            iThreadsTab{ *this, "Threads"_t },
            iThreadsLayout{ iThreadsTab },
            iThreads{ iThreadsLayout, { "Thread"_t, "State"_t, "Location"_t } },
            iBreakpointsTab{ *this, "Breakpoints"_t },
            iBreakpointsLayout{ iBreakpointsTab },
            iBreakpointHelp{ iBreakpointsLayout, "A breakpoint: [file:]line [if condition]; a catchpoint: throw [exception] [if condition]"_t },
            iBreakpointEntryLayout{ iBreakpointsLayout },
            iBreakpointEntry{ iBreakpointEntryLayout },
            iAddBreakpoint{ iBreakpointEntryLayout, "Add"_t },
            iRemoveBreakpoint{ iBreakpointEntryLayout, "Remove"_t },
            iToggleBreakpoint{ iBreakpointEntryLayout, "Enable/Disable"_t },
            iBreakpoints{ iBreakpointsLayout, { "Breakpoint"_t, "Condition"_t, "Hits"_t, "State"_t } }
        {
            for (i_layout* layout : std::initializer_list<i_layout*>{ &iCallStackLayout, &iLocalsLayout, &iWatchLayout, &iThreadsLayout, &iBreakpointsLayout })
            {
                layout->set_padding(neogfx::padding{});
                layout->set_spacing(size{});
            }
            iWatchEntry.set_size_policy(size_constraint::Expanding, size_constraint::Minimum);
            iBreakpointEntry.set_size_policy(size_constraint::Expanding, size_constraint::Minimum);
            iSink += iCallStack.RowSelected([this](std::size_t aRow) { FrameSelected(aRow); });
            iSink += iThreads.RowSelected([this](std::size_t aRow)
            {
                if (aRow < iThreadIds.size())
                    ThreadSelected(iThreadIds[aRow]);
            });
            iSink += iAddWatch.Clicked([this]()
            {
                auto const text = trimmed(iWatchEntry.text().to_std_string());
                if (text.empty())
                    return;
                iWatchEntry.set_text(string{});
                WatchAdded(text);
            });
            iSink += iRemoveWatch.Clicked([this]()
            {
                if (auto const row = iWatches.selected_row())
                    WatchRemoved(*row);
            });
            iSink += iAddBreakpoint.Clicked([this]()
            {
                auto const text = trimmed(iBreakpointEntry.text().to_std_string());
                if (text.empty())
                    return;
                iBreakpointEntry.set_text(string{});
                BreakpointAdded(text);
            });
            iSink += iRemoveBreakpoint.Clicked([this]()
            {
                if (auto const row = iBreakpoints.selected_row(); row && *row < iBreakpointIds.size())
                    BreakpointRemoved(iBreakpointIds[*row]);
            });
            iSink += iToggleBreakpoint.Clicked([this]()
            {
                if (auto const row = iBreakpoints.selected_row(); row && *row < iBreakpointIds.size())
                    BreakpointToggled(iBreakpointIds[*row]);
            });
            set_status({});
        }
    public:
        void set_status(std::string const& aStatus)
        {
            iStatus.set_text(string{ aStatus });
            iStatus.show(!aStatus.empty());
        }
        void set_call_stack(std::vector<std::vector<std::string>> const& aRows, std::size_t aSelected)
        {
            iCallStack.set(aRows, aSelected);
        }
        void set_locals(std::vector<value> const& aValues)
        {
            iLocals.set(aValues);
        }
        void set_watches(std::vector<value> const& aValues)
        {
            iWatches.set(aValues);
        }
        void set_threads(std::vector<std::uint32_t> const& aIds, std::vector<std::vector<std::string>> const& aRows, std::optional<std::size_t> aSelected)
        {
            iThreadIds = aIds;
            iThreads.set(aRows, aSelected);
        }
        void set_breakpoints(std::vector<breakpoint_row> const& aBreakpoints)
        {
            iBreakpointIds.clear();
            std::vector<std::vector<std::string>> rows;
            for (auto const& b : aBreakpoints)
            {
                iBreakpointIds.push_back(b.id);
                rows.push_back({ b.text, b.condition, std::to_string(b.hits), b.enabled ? "Enabled"_t.to_std_string() : "Disabled"_t.to_std_string() });
            }
            iBreakpoints.set(rows);
        }
        // (the program is running (or has ended): what it was stopped at is no longer shown)
        void clear_stop()
        {
            iCallStack.set({});
            iLocals.set({});
            iThreads.set({});
            iThreadIds.clear();
        }
        void show_breakpoints()
        {
            iBreakpointsTab.tab().select();
            iBreakpointEntry.set_focus();
        }
    private:
        static std::string trimmed(std::string const& aText)
        {
            auto const first = aText.find_first_not_of(" \t");
            if (first == std::string::npos)
                return {};
            return aText.substr(first, aText.find_last_not_of(" \t") - first + 1u);
        }
    private:
        neogfx::tab_page iCallStackTab;
        vertical_layout iCallStackLayout;
        text_widget iStatus;
        row_table iCallStack;
        neogfx::tab_page iLocalsTab;
        vertical_layout iLocalsLayout;
        value_table iLocals;
        neogfx::tab_page iWatchTab;
        vertical_layout iWatchLayout;
        horizontal_layout iWatchEntryLayout;
        line_edit iWatchEntry;
        push_button iAddWatch;
        push_button iRemoveWatch;
        value_table iWatches;
        neogfx::tab_page iThreadsTab;
        vertical_layout iThreadsLayout;
        row_table iThreads;
        std::vector<std::uint32_t> iThreadIds;
        neogfx::tab_page iBreakpointsTab;
        vertical_layout iBreakpointsLayout;
        text_widget iBreakpointHelp;
        horizontal_layout iBreakpointEntryLayout;
        line_edit iBreakpointEntry;
        push_button iAddBreakpoint;
        push_button iRemoveBreakpoint;
        push_button iToggleBreakpoint;
        row_table iBreakpoints;
        std::vector<std::uint32_t> iBreakpointIds;
        sink iSink;
    };

    // the debugging of the programs that neos runs (see i_script_debugger): one debugging session (and so one set of breakpoints) for
    // the program that a console's neos session runs with "debug <program>" (see neos_session); it stops, and is examined, on its own
    // thread (see neos::debug::remote_frontend) while the GUI thread polls for what it has done (as neos_session polls for its output)
    // and asks it (a request run on its thread, waited for) what it is doing when it has stopped
    class neos_script_debugger : public i_script_debugger
    {
    public:
        define_declared_event(Requested, requested, request)
        define_declared_event(BreakpointsChanged, breakpoints_changed)
        define_declared_event(LocationChanged, location_changed)
    private:
        using value = neos_debug_pane::value;
        static constexpr std::chrono::milliseconds Poll{ 20 };
        static constexpr std::size_t MaxDepth = 4u;         ///< (the depth to which a value's parts are fetched)
        static constexpr std::size_t MaxParts = 256u;       ///< (of a value, at most)
        static constexpr std::size_t MaxValues = 4096u;     ///< (fetched for a stop's locals (and watches), at most)
        struct stop_state
        {
            neos::debug::stop_reason reason;
            std::uint32_t thread;
            std::optional<std::uint32_t> breakpoint;
            std::string message;
        };
    public:
        neos_script_debugger() :
            iFrontend{
                [this](neos::debug::stop_reason aReason, std::uint32_t aThread, std::optional<std::uint32_t> aBreakpoint, std::string const& aMessage)
                {
                    std::lock_guard<std::mutex> lock{ iMutex };
                    iPendingStop = stop_state{ aReason, aThread, aBreakpoint, aMessage };
                },
                [this](neos::debug::breakpoint const&)
                {
                    std::lock_guard<std::mutex> lock{ iMutex };
                    iBreakpointsDirty = true;
                } },
            iSession{ std::make_shared<neos::debug::session>(iFrontend) }
        {
        }
    public:
        // (the GUI is ready: the panes docked (hidden until a program is debugged), and polling begins)
        void ide_ready(i_ide& aIde)
        {
            iPane.emplace(std::make_shared<neos_debug_pane>(), "Debug"_t, dock_area::Right);
            aIde.add_dockable(*iPane);
            iPane->hide();
            auto& pane = this->pane();
            iSink += pane.FrameSelected([this](std::size_t aFrame)
            {
                if (aFrame == iFrame || aFrame >= iFrames.size())
                    return;
                iFrame = aFrame;
                refresh(false);
                LocationChanged();
            });
            iSink += pane.ThreadSelected([this](std::uint32_t aThread)
            {
                if (iThread == aThread || (!iThread && iStop && iStop->thread == aThread))
                    return;
                iThread = aThread;
                iFrame = 0u;
                refresh(true);
                LocationChanged();
            });
            iSink += pane.WatchAdded([this](std::string const& aExpression)
            {
                iWatches.push_back(aExpression);
                refresh_watches();
            });
            iSink += pane.WatchRemoved([this](std::size_t aIndex)
            {
                if (aIndex < iWatches.size())
                    iWatches.erase(std::next(iWatches.begin(), aIndex));
                refresh_watches();
            });
            iSink += pane.BreakpointAdded([this](std::string const& aText) { add_breakpoint(aText); });
            iSink += pane.BreakpointRemoved([this](std::uint32_t aId)
            {
                iSession->remove_breakpoint(aId);
                breakpoints_changed_here();
            });
            iSink += pane.BreakpointToggled([this](std::uint32_t aId)
            {
                if (auto const b = iSession->find_breakpoint(aId))
                    iSession->enable_breakpoint(aId, !b->enabled);
                breakpoints_changed_here();
            });
            refresh_watches();
            refresh_breakpoints();
            iPoller.emplace(neolib::service<neolib::i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                poll();
                aTimer.again();
            }, Poll);
        }
    public:
        void set_active_script(neolib::i_string const& aPath) final
        {
            iActiveScript = aPath;
        }
        neolib::i_string const& active_script() const final
        {
            return iActiveScript;
        }
        bool debugging() const final
        {
            return iDebugging;
        }
        bool stopped() const final
        {
            return iStop.has_value();
        }
        breakpoint_state breakpoint_at(neolib::i_string const& aPath, std::uint32_t aLine, std::uint32_t aColumn) const final
        {
            auto const& path = canonical(aPath.to_std_string());
            auto result = breakpoint_state::None;
            for (auto const& b : iBreakpoints)
                if (!b.first.exception && b.second == path && b.first.line.value_or(b.first.requested.line) == aLine &&
                    (aColumn == 0u || b.first.requested.column == aColumn))
                {
                    if (b.first.enabled)
                        return breakpoint_state::Enabled;
                    result = breakpoint_state::Disabled;
                }
            return result;
        }
        void toggle_breakpoint(neolib::i_string const& aPath, std::uint32_t aLine, std::uint32_t aColumn) final
        {
            // (a line's: all of the breakpoints on the line removed (if it has any); a statement's (a column): that one)
            auto const path = canonical(aPath.to_std_string());
            bool removed = false;
            for (auto const& b : iBreakpoints)
                if (!b.first.exception && b.second == path && b.first.line.value_or(b.first.requested.line) == aLine &&
                    (aColumn == 0u || b.first.requested.column == aColumn))
                    removed = iSession->remove_breakpoint(b.first.id) || removed;
            if (!removed)
                iSession->add_breakpoint(neos::debug::location{ aPath.to_std_string(), aLine, aColumn });
            breakpoints_changed_here();
        }
        bool location(neolib::i_string& aPath, std::uint32_t& aLine, std::uint32_t& aColumn, std::uint32_t& aEndLine, std::uint32_t& aEndColumn, bool& aTop) const final
        {
            if (!iStop || iFrame >= iFrames.size() || iFrames[iFrame].source.empty() || iFrames[iFrame].line == 0u)
                return false;
            auto const& frame = iFrames[iFrame];
            aPath = neolib::string{ frame.source };
            aLine = frame.line;
            aColumn = frame.column;
            aEndLine = frame.endLine;
            aEndColumn = frame.endColumn;
            aTop = (iFrame == 0u);
            return true;
        }
        void execute(command aCommand) final
        {
            switch (aCommand)
            {
            case command::Continue:
                if (iStop)
                    resume(neos::debug::action::Continue);
                break;
            case command::StepInto:
                step(neos::debug::action::StepInto);
                break;
            case command::StepOver:
                step(neos::debug::action::StepOver);
                break;
            case command::StepOut:
                step(neos::debug::action::StepOut);
                break;
            }
        }
    public:
        // (the library's actions)
        // F5: the program continues (if it is stopped), else the active script is run under the debugger (F10, F11: stopping at its
        // first statement); Ctrl+F5: it is run (not debugged)
        void start(bool aStopOnEntry)
        {
            if (iStop)
                resume(neos::debug::action::Continue);
            else if (!iDebugging)
            {
                {
                    std::lock_guard<std::mutex> lock{ iMutex };
                    iStopOnEntry = aStopOnEntry;
                }
                Requested(request::Start);
            }
        }
        void start_without_debugging()
        {
            if (!iDebugging)
                Requested(request::StartWithoutDebugging);
        }
        void step(neos::debug::action aAction)
        {
            if (iStop)
                resume(aAction);
            else if (!iDebugging && aAction != neos::debug::action::StepOut)
                start(true);
        }
        void stop_debugging()
        {
            if (!iDebugging)
                return;
            iSession->terminate();
            if (iStop)
                resume(neos::debug::action::Terminate);
            else
                interrupt_input();
        }
        void break_all()
        {
            if (!iDebugging || iStop)
                return;
            iSession->pause();
            interrupt_input();
        }
        void toggle_breakpoint()
        {
            Requested(request::ToggleBreakpoint);
        }
        void new_breakpoint()
        {
            iPane->show();
            pane().show_breakpoints();
        }
        void delete_all_breakpoints()
        {
            iSession->clear_breakpoints();
            breakpoints_changed_here();
        }
        void disable_all_breakpoints()
        {
            for (auto const& b : iSession->breakpoints())
                iSession->enable_breakpoint(b.id, false);
            breakpoints_changed_here();
        }
    public:
        // (a console's neos session's worker thread: a program is to be debugged (the debugging session that it runs under, set up to
        // run it; none: another program is being debugged); aInterruptInput, if a read of its input is waiting, it fails (see
        // neos::debug::blocking_read: that read is then read again, once the program has stopped if it is to stop))
        std::shared_ptr<neos::debug::session> begin_run(std::function<void()> aInterruptInput)
        {
            std::lock_guard<std::mutex> lock{ iMutex };
            if (iRunning)
                return {};
            iRunning = true;
            ++iRunSerial;
            iPendingStop.reset();
            iInterruptInput = std::move(aInterruptInput);
            iSession->set_stop_on_entry(std::exchange(iStopOnEntry, false));
            iSession->begin_run();
            return iSession;
        }
        // (the worker thread: the program has ended)
        void end_run()
        {
            std::lock_guard<std::mutex> lock{ iMutex };
            iRunning = false;
            iPendingStop.reset();
            iInterruptInput = {};
            iSession->set_stop_on_entry(false);
        }
        // (a console's neos session: Ctrl+C, while the program that it runs is debugged)
        void interrupt()
        {
            break_all();
        }
    private:
        neos_debug_pane& pane()
        {
            return iPane->docked_widget<neos_debug_pane>();
        }
        void interrupt_input()
        {
            std::function<void()> interrupt;
            {
                std::lock_guard<std::mutex> lock{ iMutex };
                interrupt = iInterruptInput;
            }
            if (interrupt)
                interrupt();
        }
        // (GUI thread: what the program has done since the last poll)
        void poll()
        {
            bool running = false;
            std::uint64_t runSerial = 0u;
            std::optional<stop_state> stop;
            bool breakpointsDirty = false;
            {
                std::lock_guard<std::mutex> lock{ iMutex };
                running = iRunning;
                runSerial = iRunSerial;
                stop = std::exchange(iPendingStop, std::nullopt);
                breakpointsDirty = std::exchange(iBreakpointsDirty, false);
            }
            bool changed = false;
            if (runSerial != iRunSerialSeen)
            {
                // (a program is being debugged (and its breakpoints' hits reset))
                iRunSerialSeen = runSerial;
                iDebugging = true;
                iStop.reset();
                iFrames.clear();
                pane().clear_stop();
                pane().set_status("Running"_t.to_std_string());
                iPane->show();
                breakpointsDirty = true;
                changed = true;
            }
            if (stop && running)
            {
                iStop = stop;
                iThread.reset();
                iFrame = 0u;
                refresh(true);
                breakpointsDirty = true; // (hits)
                changed = true;
            }
            if (!running && iDebugging)
            {
                iDebugging = false;
                iStop.reset();
                iFrames.clear();
                pane().clear_stop();
                pane().set_status({});
                iPane->hide();
                changed = true;
            }
            if (breakpointsDirty)
                refresh_breakpoints();
            if (changed)
                LocationChanged();
        }
        // (the program resumed: what it stopped at is no longer shown)
        void resume(neos::debug::action aAction)
        {
            if (!iStop)
                return;
            iStop.reset();
            iFrames.clear();
            pane().clear_stop();
            pane().set_status("Running"_t.to_std_string());
            iFrontend.resume(aAction);
            LocationChanged();
        }
        // (what the stopped program is doing, asked of it: its threads, the frames of the thread selected (aFrames), the variables of the
        // frame selected and its watches)
        void refresh(bool aFrames)
        {
            if (!iStop)
                return;
            auto const thread = iThread;
            auto frame = iFrame;
            std::vector<neos::debug::thread_info> threads;
            std::vector<neos::debug::frame> frames;
            std::vector<value> locals;
            std::vector<value> watches;
            auto const watchExpressions = iWatches;
            bool const posted = iFrontend.post([&](neos::debug::i_stop& aStop)
            {
                threads = aStop.threads();
                frames = aStop.frames(thread);
                if (frame >= frames.size())
                    frame = 0u;
                if (frames.empty())
                    return;
                std::size_t budget = MaxValues;
                for (auto const& v : aStop.locals(frame, thread))
                    fetch(aStop, v, locals.emplace_back(), MaxDepth, budget);
                auto const globals = aStop.globals(frame, thread);
                if (!globals.empty())
                {
                    auto& node = locals.emplace_back();
                    node.name = "Globals"_t.to_std_string();
                    for (auto const& v : globals)
                        fetch(aStop, v, node.parts.emplace_back(), MaxDepth, budget);
                }
                watches = evaluate(aStop, watchExpressions, frame, thread, budget);
            });
            if (!posted)
            {
                // (it has resumed (e.g. it was ended))
                iStop.reset();
                return;
            }
            iFrame = frame;
            iFrames = std::move(frames);
            if (aFrames)
            {
                std::vector<std::vector<std::string>> rows;
                for (auto const& f : iFrames)
                    rows.push_back({ f.function.empty() ? std::string{ "?" } : f.function,
                        f.source.empty() ? std::string{} : std::filesystem::path{ f.source }.filename().string() + (f.line != 0u ? ":" + std::to_string(f.line) : std::string{}) });
                pane().set_call_stack(rows, iFrame);
                std::vector<std::uint32_t> ids;
                std::vector<std::vector<std::string>> threadRows;
                std::optional<std::size_t> selected;
                auto const selectedThread = iThread.value_or(iStop->thread);
                for (auto const& t : threads)
                {
                    if (t.id == selectedThread)
                        selected = ids.size();
                    ids.push_back(t.id);
                    threadRows.push_back({ std::to_string(t.id), t.state,
                        t.top ? t.top->function + (t.top->source.empty() ? std::string{} : " (" + std::filesystem::path{ t.top->source }.filename().string() + ":" + std::to_string(t.top->line) + ")") : std::string{} });
                }
                pane().set_threads(ids, threadRows, selected);
                pane().set_status(status());
            }
            pane().set_locals(locals);
            pane().set_watches(watches);
        }
        void refresh_watches()
        {
            if (!iStop)
            {
                std::vector<value> watches;
                for (auto const& w : iWatches)
                    watches.push_back(value{ w, {}, {}, {} });
                pane().set_watches(watches);
                return;
            }
            refresh(false);
        }
        std::string status() const
        {
            if (!iStop)
                return {};
            using neos::debug::stop_reason;
            switch (iStop->reason)
            {
            case stop_reason::Entry:
                return "Stopped at the program's entry"_t.to_std_string();
            case stop_reason::Breakpoint:
                return "Stopped at breakpoint"_t.to_std_string() + (iStop->breakpoint ? " " + std::to_string(*iStop->breakpoint) : std::string{});
            case stop_reason::Step:
                return "Stopped (step)"_t.to_std_string();
            case stop_reason::Pause:
                return "Paused"_t.to_std_string();
            case stop_reason::Throw:
                return "Exception thrown"_t.to_std_string() + ": " + iStop->message;
            case stop_reason::Error:
            default:
                return "Error"_t.to_std_string() + ": " + iStop->message;
            }
        }
        // (the breakpoints as they now are: shown in the panes and the editors (they may have been changed by the program's thread: e.g.
        // found on the line of a statement))
        void breakpoints_changed_here()
        {
            refresh_breakpoints();
        }
        void refresh_breakpoints()
        {
            iBreakpoints.clear();
            std::vector<neos_debug_pane::breakpoint_row> rows;
            for (auto const& b : iSession->breakpoints())
            {
                iBreakpoints.emplace_back(b, b.exception ? std::string{} : canonical(b.requested.source));
                std::string text;
                if (b.exception)
                    text = "throw" + (b.exception->empty() ? std::string{} : " " + *b.exception);
                else
                {
                    text = std::filesystem::path{ b.requested.source }.filename().string() + ":" + std::to_string(b.line.value_or(b.requested.line));
                    if (b.requested.column != 0u)
                        text += ":" + std::to_string(b.requested.column);
                }
                if (b.temporary)
                    text += " (" + "temporary"_t.to_std_string() + ")";
                rows.push_back(neos_debug_pane::breakpoint_row{ b.id, text, b.condition, b.hits, b.enabled });
            }
            if (iPane)
                pane().set_breakpoints(rows);
            BreakpointsChanged();
        }
        // ("[file:]line [if condition]" or "throw [exception] [if condition]" (see the Breakpoints pane))
        void add_breakpoint(std::string const& aText)
        {
            auto text = aText;
            std::string condition;
            if (auto const pos = text.find(" if "); pos != std::string::npos)
            {
                condition = text.substr(pos + 4u);
                text.erase(pos);
            }
            if (text == "throw" || text.rfind("throw ", 0u) == 0u)
            {
                auto exception = text.substr(5u);
                exception.erase(0u, exception.find_first_not_of(' '));
                iSession->add_catchpoint(exception, condition);
            }
            else
            {
                auto const colon = text.rfind(':');
                auto const lineText = colon == std::string::npos ? text : text.substr(colon + 1u);
                auto const file = colon == std::string::npos ? iActiveScript.to_std_string() : text.substr(0u, colon);
                if (lineText.empty() || file.empty() || lineText.find_first_not_of("0123456789") != std::string::npos)
                {
                    service<i_basic_services>().system_beep();
                    return;
                }
                iSession->add_breakpoint(neos::debug::location{ file, static_cast<std::uint32_t>(std::stoul(lineText)) }, condition);
            }
            breakpoints_changed_here();
        }
        // (the program's thread: a variable's value, and its parts (as many as the budget allows))
        static void fetch(neos::debug::i_stop& aStop, neos::debug::variable const& aVariable, value& aValue, std::size_t aDepth, std::size_t& aBudget)
        {
            aValue.name = aVariable.name;
            aValue.type = aVariable.type;
            aValue.text = aVariable.value;
            if (aBudget > 0u)
                --aBudget;
            if (aVariable.reference == 0u || aDepth == 0u || aBudget == 0u)
                return;
            for (auto const& part : aStop.parts(aVariable.reference, 0u, std::min(MaxParts, aBudget)))
            {
                if (aBudget == 0u)
                    break;
                fetch(aStop, part, aValue.parts.emplace_back(), aDepth - 1u, aBudget);
            }
        }
        static std::vector<value> evaluate(neos::debug::i_stop& aStop, std::vector<std::string> const& aExpressions, std::size_t aFrame,
            std::optional<std::uint32_t> aThread, std::size_t& aBudget)
        {
            std::vector<value> result;
            for (auto const& expression : aExpressions)
            {
                std::string error;
                if (auto const v = aStop.evaluate(aFrame, expression, error, aThread))
                {
                    fetch(aStop, *v, result.emplace_back(), MaxDepth, aBudget);
                    result.back().name = expression;
                }
                else
                    result.push_back(value{ expression, {}, error, {} });
            }
            return result;
        }
        // (a file's path as compared: the same file, however it is named (the last asked for remembered: the editors ask for theirs as 
        // they paint))
        std::string const& canonical(std::string const& aPath) const
        {
            if (aPath != iCanonicalFor)
            {
                std::error_code ec;
                auto const result = std::filesystem::weakly_canonical(std::filesystem::path{ aPath }, ec);
                iCanonicalFor = aPath;
                iCanonical = ec ? aPath : result.string();
            }
            return iCanonical;
        }
    private:
        neos::debug::remote_frontend iFrontend;
        std::shared_ptr<neos::debug::session> iSession;
        // (worker and program threads -> GUI thread)
        std::mutex iMutex;
        bool iRunning = false;
        std::uint64_t iRunSerial = 0u;
        std::optional<stop_state> iPendingStop;
        bool iBreakpointsDirty = false;
        std::function<void()> iInterruptInput;
        bool iStopOnEntry = false;      ///< (the next program debugged stops at its first statement (read by begin_run: under the mutex))
        // (GUI thread)
        neolib::string iActiveScript;
        std::optional<dockable> iPane;
        std::optional<neolib::callback_timer> iPoller;
        std::uint64_t iRunSerialSeen = 0u;
        bool iDebugging = false;
        std::optional<stop_state> iStop;
        std::optional<std::uint32_t> iThread;   ///< (the thread selected; none: the one that stopped)
        std::size_t iFrame = 0u;                ///< (the frame selected)
        std::vector<neos::debug::frame> iFrames;
        std::vector<std::string> iWatches;
        std::vector<std::pair<neos::debug::breakpoint, std::string>> iBreakpoints;  ///< (and their sources' canonical paths)
        mutable std::string iCanonicalFor;
        mutable std::string iCanonical;
        sink iSink;
    };

    // a neos session in a console window (the "neos" command): the neos console's commands (see neos::process_command) on a neos
    // context whose standard streams are the window's (the output of the context and of the programs it runs goes to the window; 
    // what is typed into the window while a command runs is its programs' input). Commands run on a worker thread (each session 
    // has its own, and its own context), so the GUI thread never blocks: it only exchanges data with the worker through 
    // shared_state (under a mutex held only for brief copies) and polls for the worker's output with a timer (as the ssh client).
    class neos_session : public console_client
    {
    private:
        struct shared_state
        {
            // (the context's output and error streams: to the window (output, below), in the order written, from any thread)
            class output_buffer : public std::streambuf
            {
            public:
                output_buffer(shared_state& aShared) : iShared{ aShared } {}
            protected:
                int_type overflow(int_type aCharacter) override
                {
                    if (!traits_type::eq_int_type(aCharacter, traits_type::eof()))
                    {
                        std::lock_guard<std::mutex> lock{ iShared.mutex };
                        iShared.output.push_back(traits_type::to_char_type(aCharacter));
                    }
                    return traits_type::not_eof(aCharacter);
                }
                std::streamsize xsputn(char const* aText, std::streamsize aCount) override
                {
                    std::lock_guard<std::mutex> lock{ iShared.mutex };
                    iShared.output.append(aText, static_cast<std::size_t>(aCount));
                    return aCount;
                }
            private:
                shared_state& iShared;
            };
            // (the context's input stream: the lines typed into the window while a command runs (input, below); a read waits for 
            // one; the end of input (Ctrl+Z or Ctrl+D) is read once)
            class input_buffer : public std::streambuf
            {
            public:
                input_buffer(shared_state& aShared) : iShared{ aShared } {}
            public:
                void reset()
                {
                    iBuffer.clear();
                    setg(nullptr, nullptr, nullptr);
                }
            protected:
                int_type underflow() override
                {
                    if (gptr() < egptr())
                        return traits_type::to_int_type(*gptr());
                    std::unique_lock<std::mutex> lock{ iShared.mutex };
                    iShared.reading = true;
                    iShared.wake.wait(lock, [&]() { return !iShared.input.empty() || iShared.endOfInput || iShared.cancelled || iShared.interrupted; });
                    iShared.reading = false;
                    // (the debugger interrupted the read (see neos_script_debugger::begin_run): it fails, and is then read again)
                    bool const interrupted = std::exchange(iShared.interrupted, false);
                    if (iShared.input.empty())
                    {
                        if (!interrupted)
                            iShared.endOfInput = false; // (the end of input: read once)
                        return traits_type::eof();
                    }
                    // (the last character read is kept, so that it can be put back)
                    bool const kept = eback() != nullptr && gptr() > eback();
                    char const last = kept ? gptr()[-1] : '\0';
                    iBuffer.assign(kept ? 1u : 0u, last);
                    iBuffer += iShared.input;
                    iShared.input.clear();
                    setg(iBuffer.data(), iBuffer.data() + (kept ? 1 : 0), iBuffer.data() + iBuffer.size());
                    return traits_type::to_int_type(*gptr());
                }
            private:
                shared_state& iShared;
                std::string iBuffer;
            };

            std::mutex mutex;
            std::condition_variable wake;
            bool cancelled = false;
            // GUI thread -> worker thread
            std::optional<std::string> command;
            std::string input;
            bool endOfInput = false;
            // worker thread -> GUI thread
            std::string output;
            std::string prompt;
            bool busy = false;      ///< (a command is running: lines typed are its programs' input)
            bool closed = false;
            bool runOnly = false;   ///< (programs are run (each command is a program's path): only their input and output is shown)
            bool debug = false;     ///< (runOnly: the command's program is debugged (see neos_script_debugger))
            bool debugging = false; ///< (a program that it runs is being debugged)
            bool reading = false;   ///< (a read of the input is waiting)
            bool interrupted = false;
            neos_script_debugger* debugger = nullptr;
            // (the context's standard streams (destroyed after it))
            output_buffer outputBuffer{ *this };
            input_buffer inputBuffer{ *this };
            std::ostream out{ &outputBuffer };
            std::istream in{ &inputBuffer };
            std::optional<neos::context> context;
        };
    public:
        // (a program, if any: the session only runs programs (it and those that it is asked to run (see execute)), showing only 
        // their input and output: it has no prompt, and nothing typed while none is running is a command (the "neos <program>" command))
        // ("-g <program>": the program is debugged)
        neos_session(neolib::i_service_provider& aServiceProvider, neos_script_debugger& aDebugger, std::string const& aProgram = {}) :
            iShared{ std::make_shared<shared_state>() }
        {
            iShared->debugger = &aDebugger;
            if (!aProgram.empty())
            {
                iShared->runOnly = true;
                iShared->debug = aProgram.rfind("-g ", 0u) == 0u;
                iShared->command = iShared->debug ? aProgram.substr(aProgram.find_first_not_of(' ', 3u)) : aProgram;
                iShared->busy = true;
            }
            try
            {
                iShared->in.tie(&iShared->out);
                iShared->context.emplace(iShared->in, iShared->out, iShared->out, aServiceProvider);
                // The worker thread is detached and never joined (a program it is running may not end); it owns everything it 
                // touches (via shared_state) and exits once its command has finished if the session has been closed.
                std::thread{ [shared = iShared]() { run(shared); } }.detach();
            }
            catch (std::exception const& e)
            {
                iShared->output = std::string{ "neos: " } + e.what() + "\n";
                iShared->closed = true;
            }
        }
        ~neos_session()
        {
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                iShared->cancelled = true;
            }
            iShared->wake.notify_all();
        }
    public:
        void start() final
        {
            iPoller.emplace(neolib::service<neolib::i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                destroyed_flag destroyed{ *this };
                poll();
                if (!destroyed && !iClosed)
                    aTimer.again();
            }, std::chrono::milliseconds{ 10 });
        }
        void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) final
        {
        }
        bool execute(std::string const& aCommand) final
        {
            if (iClosed)
                return false;
            std::string prompt;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->busy)
                    return false;
                if (iShared->runOnly)
                {
                    // ("run <program>": the program run (as the session's first was); "debug <program>": debugged)
                    if (aCommand.rfind("run ", 0u) == 0u)
                    {
                        iShared->command = aCommand.substr(4u);
                        iShared->debug = false;
                    }
                    else if (aCommand.rfind("debug ", 0u) == 0u)
                    {
                        iShared->command = aCommand.substr(6u);
                        iShared->debug = true;
                    }
                    else
                        return false;
                    iShared->busy = true;
                }
                prompt = iShared->prompt;
            }
            if (iShared->runOnly)
            {
                iLine.clear();
                iShared->wake.notify_all();
                return true;
            }
            // (as typed (a line partly typed is discarded))
            Output((iLine.empty() ? std::string{} : "^C\r\n" + prompt) + aCommand + "\r\n");
            iLine.clear();
            enter(aCommand);
            return true;
        }
        void input(std::string const& aText) final
        {
            if (iClosed || aText.empty() || aText[0] == '\x1B') // (escape sequences (e.g. the cursor keys) are not edited)
                return;
            {
                // (a session that only runs programs: what is typed while none is running is ignored)
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->runOnly && !iShared->busy)
                    return;
            }
            destroyed_flag destroyed{ *this };
            std::string echo;
            for (char ch : aText)
            {
                // (the terminal sends "\n" after "\r" for Enter)
                if (ch == '\n' && iLastInputWasCR)
                {
                    iLastInputWasCR = false;
                    continue;
                }
                iLastInputWasCR = ch == '\r';
                bool const repeatedTab = ch == '\t' && iLastInputWasTab;
                iLastInputWasTab = ch == '\t';
                if (ch == '\r' || ch == '\n')
                {
                    Output(echo + "\r\n");
                    echo.clear();
                    enter(std::exchange(iLine, {}));
                }
                else if (ch == '\x7F' || ch == '\b')
                {
                    if (!iLine.empty())
                    {
                        // (a character: a UTF-8 sequence)
                        while (iLine.size() > 1u && (static_cast<unsigned char>(iLine.back()) & 0xC0u) == 0x80u)
                            iLine.pop_back();
                        iLine.pop_back();
                        echo += "\b\x1B[K";
                    }
                }
                else if (ch == '\t')
                    echo += complete(repeatedTab);
                else if (ch == '\x03') // (Ctrl+C: the line is discarded (a running program is not interrupted, unless it is debugged: it pauses))
                {
                    iLine.clear();
                    bool debugging = false;
                    {
                        std::lock_guard<std::mutex> lock{ iShared->mutex };
                        echo += "^C\r\n" + (iShared->busy ? std::string{} : iShared->prompt);
                        debugging = iShared->debugging;
                    }
                    if (debugging)
                        iShared->debugger->interrupt();
                }
                else if ((ch == '\x04' || ch == '\x1A') && iLine.empty()) // (Ctrl+D or Ctrl+Z: the end of input (no command running: the end of the session))
                {
                    Output(echo + "\r\n");
                    echo.clear();
                    bool busy = false;
                    {
                        std::lock_guard<std::mutex> lock{ iShared->mutex };
                        busy = iShared->busy;
                        if (busy)
                            iShared->endOfInput = true;
                        else
                            iShared->cancelled = true;
                    }
                    iShared->wake.notify_all();
                    if (!busy)
                    {
                        iClosed = true;
                        Closed();
                        return; // *this may no longer exist
                    }
                }
                else if (static_cast<unsigned char>(ch) >= 0x20u)
                {
                    iLine.push_back(ch);
                    echo.push_back(ch);
                }
            }
            if (!destroyed && !echo.empty())
                Output(echo);
        }
    private:
        // GUI thread: a line typed: a command, or the input of the command running
        void enter(std::string const& aLine)
        {
            std::string prompt;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->busy)
                    iShared->input += aLine + "\n";
                else if (!aLine.empty())
                {
                    iShared->command = aLine;
                    iShared->busy = true;
                }
                else
                    prompt = iShared->prompt;
            }
            iShared->wake.notify_all();
            if (!prompt.empty())
                Output(prompt);
        }
        // GUI thread: Tab (no command running): the path being typed (the line's last word, or what follows an unclosed quote) 
        // completed as far as the names of the files and directories that it may be the start of allow (one name: a directory's is 
        // followed by a separator, a quoted file's by its closing quote); a beep if there are none (or several and it is already as far as they allow: 
        // a second Tab lists them); the text to echo
        std::string complete(bool aList)
        {
            std::string prompt;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                if (iShared->busy)
                    return {};
                prompt = iShared->prompt;
            }
#ifdef _WIN32
            char const* const separators = "/\\";
            bool const hidingDotNames = false;
            auto const same = [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); };
#else
            char const* const separators = "/";
            bool const hidingDotNames = true;
            auto const same = [](char a, char b) { return a == b; };
#endif
            bool const quoted = std::count(iLine.begin(), iLine.end(), '"') % 2 == 1;
            auto const wordStart = quoted ? iLine.rfind('"') + 1u : iLine.find_last_of(' ') == std::string::npos ? 0u : iLine.find_last_of(' ') + 1u;
            std::string const word = iLine.substr(wordStart);
            auto const lastSeparator = word.find_last_of(separators);
            std::string const directory = lastSeparator == std::string::npos ? std::string{} : word.substr(0u, lastSeparator + 1u);
            std::string const prefix = word.substr(directory.size());
            char const separator = lastSeparator != std::string::npos ? word[lastSeparator] : static_cast<char>(std::filesystem::path::preferred_separator);
            auto const utf8 = [](std::filesystem::path const& aPath) { auto const name = aPath.u8string(); return std::string{ name.begin(), name.end() }; };
            // (the names that the prefix may be of: those that start with it (not those that start with "." unless it does, except on 
            // Windows))
            std::vector<std::pair<std::string, bool>> matches;
            std::error_code ec;
            std::filesystem::path const searched = directory.empty() ? std::filesystem::path{ "." } : 
                std::filesystem::path{ std::u8string{ directory.begin(), directory.end() } };
            for (std::filesystem::directory_iterator entry{ searched, ec }, end; !ec && entry != end; entry.increment(ec))
            {
                auto const name = utf8(entry->path().filename());
                if (name.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), name.begin(), same) && (!hidingDotNames || name[0] != '.' || (!prefix.empty() && prefix[0] == '.')))
                {
                    std::error_code isDirectoryError;
                    matches.emplace_back(name, entry->is_directory(isDirectoryError));
                }
            }
            if (matches.empty())
            {
                service<i_basic_services>().system_beep();
                return {};
            }
            // (as far as the names allow: their common prefix)
            std::string completion = matches[0].first;
            for (auto const& match : matches)
                completion.erase(static_cast<std::size_t>(std::mismatch(completion.begin(), completion.end(), match.first.begin(), match.first.end(), same).first - completion.begin()));
            if (matches.size() > 1u && completion.size() == prefix.size())
            {
                service<i_basic_services>().system_beep();
                if (!aList)
                    return {};
                std::sort(matches.begin(), matches.end());
                std::string list = "\r\n";
                for (auto const& match : matches)
                    list += match.first + (match.second ? std::string(1u, separator) : std::string{}) + "  ";
                return list + "\r\n" + prompt + iLine;
            }
            // (a name with a space in it is quoted)
            bool const quote = !quoted && completion.find(' ') != std::string::npos;
            if (matches.size() == 1u)
                completion += matches[0].second ? std::string(1u, separator) : quoted || quote ? "\"" : "";
            std::string const completed = (quote ? "\"" : "") + directory + completion;
            // (the word typed replaced (its case may differ from the name's) unless it is the start of it)
            std::string echo;
            if (completed.compare(0u, word.size(), word) == 0)
                echo = completed.substr(word.size());
            else
            {
                for (char ch : word)
                    if ((static_cast<unsigned char>(ch) & 0xC0u) != 0x80u)
                        echo += "\b";
                echo += "\x1B[K" + completed;
            }
            iLine = iLine.substr(0u, wordStart) + completed;
            return echo;
        }
        // GUI thread: collect the worker's output; never waits on the worker
        void poll()
        {
            std::string output;
            bool closed = false;
            {
                std::lock_guard<std::mutex> lock{ iShared->mutex };
                output.swap(iShared->output);
                closed = iShared->closed;
            }
            if (!output.empty())
            {
                // (the terminal's line ending: "\r\n")
                std::string text;
                text.reserve(output.size() + output.size() / 8u);
                for (char ch : output)
                {
                    if (ch == '\n' && !iLastOutputWasCR)
                        text.push_back('\r');
                    text.push_back(ch);
                    iLastOutputWasCR = ch == '\r';
                }
                destroyed_flag destroyed{ *this };
                Output(text);
                if (destroyed)
                    return;
            }
            if (closed)
            {
                iClosed = true;
                Closed();
                return; // *this may no longer exist
            }
        }
        // the worker thread: the session's commands, until it is closed (the quit command, or the session is cancelled)
        static void run(std::shared_ptr<shared_state> aShared)
        {
            auto& shared = *aShared;
            auto& context = *shared.context;
            bool interactive = false;
            auto end_command = [&](bool aClosed)
            {
                shared.out.flush();
                shared.inputBuffer.reset();
                shared.in.clear();
                auto const prompt = aClosed || shared.runOnly ? std::string{} : neos::console_prompt(context, interactive);
                std::lock_guard<std::mutex> lock{ shared.mutex };
                // (input that the command's programs did not read is discarded)
                shared.input.clear();
                shared.endOfInput = false;
                shared.output += prompt;
                shared.prompt = prompt;
                shared.busy = false;
                shared.closed = shared.closed || aClosed;
            };
            try
            {
                if (!shared.runOnly)
                {
                    shared.out << "neos " << neos::NEOS_VERSION << std::endl;
                    end_command(false);
                }
                for (;;)
                {
                    std::string command;
                    {
                        std::unique_lock<std::mutex> lock{ shared.mutex };
                        shared.wake.wait(lock, [&]() { return shared.command || shared.cancelled; });
                        if (shared.cancelled)
                            return;
                        command = std::move(*shared.command);
                        shared.command.reset();
                    }
                    bool more = true;
                    if (shared.runOnly)
                    {
                        bool debug = false;
                        {
                            std::lock_guard<std::mutex> lock{ shared.mutex };
                            debug = shared.debug;
                        }
                        // (the program (cleared: only its input and output shown), and its errors)
                        shared.out << "\x1B[3J" << std::flush;
                        try
                        {
                            // (a program debugged: compiled to be (see neos::language::i_code_generator::debug_info), and run under 
                            // the debugger's session)
                            auto& compiler = context.compiler();
                            auto const debugInfo = compiler.debug_info();
                            compiler.set_debug_info(debug);
                            struct restore { neos::language::compiler& c; bool d; ~restore() { c.set_debug_info(d); } } const restoreDebugInfo{ compiler, debugInfo };
                            context.load_program(command);
                            context.compile_program();
                            if (!debug)
                                context.run();
                            else if (auto const session = shared.debugger->begin_run([weakShared = std::weak_ptr<shared_state>{ aShared }]()
                                {
                                    // (a read of the program's input that is waiting fails (and is then read again))
                                    if (auto const shared = weakShared.lock())
                                    {
                                        {
                                            std::lock_guard<std::mutex> lock{ shared->mutex };
                                            if (!shared->reading)
                                                return;
                                            shared->interrupted = true;
                                        }
                                        shared->wake.notify_all();
                                    }
                                }))
                            {
                                {
                                    std::lock_guard<std::mutex> lock{ shared.mutex };
                                    shared.debugging = true;
                                }
                                struct end_run 
                                { 
                                    shared_state& s; 
                                    neos::context& c;
                                    ~end_run() 
                                    { 
                                        c.set_debug_session(nullptr);
                                        s.debugger->end_run();
                                        std::lock_guard<std::mutex> lock{ s.mutex };
                                        s.debugging = false;
                                    } 
                                } const endRun{ shared, context };
                                context.set_debug_session(session);
                                context.run();
                                shared.out << std::flush;
                                if (session->terminate_requested())
                                    shared.out << "[Program terminated]" << std::endl;
                            }
                            else
                                shared.out << "Error: another program is being debugged" << std::endl;
                        }
                        catch (std::exception const& e)
                        {
                            shared.out << (std::string_view{ e.what() }.find("error:") == std::string_view::npos ? "Error: " : "") << e.what() << std::endl;
                        }
                    }
                    else if (!interactive && (command == "cls" || command == "clear"))
                        shared.out << "\x1B[3J";
                    else
                        more = neos::process_command(context, interactive, command, true);
                    end_command(!more);
                    if (!more)
                        return;
                }
            }
            catch (std::exception const& e)
            {
                shared.out << "neos: " << e.what() << std::endl;
                end_command(true);
            }
            catch (...)
            {
                shared.out << "neos: unknown error" << std::endl;
                end_command(true);
            }
        }
    private:
        std::shared_ptr<shared_state> iShared;
        std::optional<neolib::callback_timer> iPoller;
        std::string iLine;
        bool iLastInputWasCR = false;
        bool iLastInputWasTab = false;
        bool iLastOutputWasCR = false;
        bool iClosed = false;
    };

    // the syntax highlighting of scripts (see i_script_highlighter) in the languages that neos has (installed) schemas for (see 
    // neos::context::highlight). Calls are serialized (the neos context, for its concept libraries, compiler and the schemas' parsers, is 
    // used by one thread at a time).
    class neos_script_highlighter : public i_script_highlighter
    {
    public:
        bool can_highlight(neolib::i_string const& aExtension) const final
        {
            return schema_path(aExtension.to_std_string()).has_value();
        }
        void extensions(neolib::i_vector<neolib::i_string>& aExtensions) const final
        {
            aExtensions.clear();
            for (auto const& schema : neos::language::schema::installed())
                if (std::find_if(aExtensions.begin(), aExtensions.end(), [&](neolib::i_string const& e) { return e.to_std_string_view() == schema.first; }) == aExtensions.end())
                    aExtensions.push_back(neolib::string{ schema.first });
        }
        void colours(neolib::i_string const& aExtension, neolib::i_string const& aTheme, neolib::i_vector<neolib::i_string>& aColours) const final
        {
            aColours.clear();
            if (auto const path = schema_path(aExtension.to_std_string()))
                for (auto const& colour : neos::context::highlight_colours(*path, aTheme.to_std_string()))
                    aColours.push_back(neolib::string{ colour });
        }
        bool highlight(neolib::i_string const& aExtension, neolib::i_string const& aScript, neolib::i_vector<span>& aSpans) const final
        {
            aSpans.clear();
            auto const path = schema_path(aExtension.to_std_string());
            if (!path)
                return false;
            std::lock_guard<std::mutex> lock{ iMutex };
            try
            {
                if (!iContext)
                    iContext.emplace(iInput, iOutput, iOutput, neolib::get_service_provider());
                std::string_view const source = aScript.to_std_string_view();
                auto const codePoints = code_points(source);
                // (neos's categories are i_script_highlighter's)
                static_assert(static_cast<std::uint32_t>(neos::context::highlight_category::Variable) == static_cast<std::uint32_t>(category::Variable));
                for (auto const& s : iContext->highlight(*path, source))
                    aSpans.push_back(span{ codePoints[s.first], codePoints[s.last], static_cast<category>(s.category) });
                return true;
            }
            catch (...)
            {
                aSpans.clear();
                return false;
            }
        }
        bool outline(neolib::i_string const& aExtension, neolib::i_string const& aScript, neolib::i_vector<outline_node>& aOutline) const final
        {
            aOutline.clear();
            auto const path = schema_path(aExtension.to_std_string());
            if (!path)
                return false;
            std::lock_guard<std::mutex> lock{ iMutex };
            try
            {
                if (!iContext)
                    iContext.emplace(iInput, iOutput, iOutput, neolib::get_service_provider());
                std::string_view const source = aScript.to_std_string_view();
                auto const codePoints = code_points(source);
                // (neos's kinds are i_script_highlighter's)
                static_assert(static_cast<std::uint32_t>(neos::context::outline_kind::Statement) == static_cast<std::uint32_t>(outline_kind::Statement));
                static_assert(neos::context::outline_node::NoParent == outline_node::NoParent);
                auto const at = [&](std::size_t aByte) { return codePoints[std::min(aByte, source.size())]; };
                for (auto const& n : iContext->outline(*path, source))
                    aOutline.push_back(outline_node{ n.parent, static_cast<outline_kind>(n.kind), at(n.first), at(n.last), at(n.headerLast) });
                return !aOutline.empty();
            }
            catch (...)
            {
                aOutline.clear();
                return false;
            }
        }
    private:
        // (the code point of each byte of a source (and of its end): spans and outlines are of code points, as the text_edit's positions are)
        static std::vector<std::uint32_t> code_points(std::string_view aSource)
        {
            std::vector<std::uint32_t> result(aSource.size() + 1u);
            std::uint32_t codePoint = 0u;
            for (std::size_t i = 0u; i < aSource.size(); ++i)
            {
                if ((static_cast<unsigned char>(aSource[i]) & 0xC0u) != 0x80u)
                    result[i] = codePoint++;
                else
                    result[i] = codePoint - 1u;
            }
            result[aSource.size()] = codePoint;
            return result;
        }
        // (the installed schema for a file extension: neos's (see neos::home))
        std::optional<std::string> const& schema_path(std::string const& aExtension) const
        {
            std::lock_guard<std::mutex> lock{ iPathMutex };
            auto existing = iPaths.find(aExtension);
            if (existing == iPaths.end())
                existing = iPaths.emplace(aExtension, aExtension.empty() ? std::optional<std::string>{} : neos::language::schema::find(aExtension)).first;
            return existing->second;
        }
    private:
        mutable std::mutex iMutex;
        mutable std::mutex iPathMutex;
        mutable std::map<std::string, std::optional<std::string>> iPaths;
        // (the context and its standard streams (unused))
        mutable std::istream iInput{ nullptr };
        mutable std::ostream iOutput{ nullptr };
        mutable std::optional<neos::context> iContext;
    };

    neos_element_library::neos_element_library(neolib::i_application& aApplication, std::string const& aPluginPath) :
        iApplication{ aApplication },
        iPluginPath{ aPluginPath },
        iClientManager{ aApplication },
        iElementsOrdered
        {
        },
        iElements{ elements_ordered().begin(), elements_ordered().end() },
        iSeparator{},
        iNewScript{ "New Script..."_t },
        iOpenScript{ "Open Script..."_t },
        iStartDebugging{ "Start Debugging"_t },
        iStartWithoutDebugging{ "Start Without Debugging"_t },
        iStepInto{ "Step Into"_t },
        iStepOver{ "Step Over"_t },
        iStepOut{ "Step Out"_t },
        iBreakAll{ "Break All"_t },
        iStopDebugging{ "Stop Debugging"_t },
        iToggleBreakpoint{ "Toggle Breakpoint"_t },
        iNewBreakpoint{ "New Breakpoint"_t },
        iDeleteAllBreakpoints{ "Delete All Breakpoints"_t },
        iDisableAllBreakpoints{ "Disable All Breakpoints"_t },
        iImportScriptingLanguage{ "Import Scripting Language..."_t },
        iEditScriptingLanguage{ "Edit Scripting Language..."_t },
        iScriptMenu{ "Script"_t }
    {
        iSeparator.set_group(library_id());
        iNewScript.set_group(library_id());
        iOpenScript.set_group(library_id());
        iStartDebugging.set_group(library_id()).set_shortcut("F5");
        iStartWithoutDebugging.set_group(library_id()).set_shortcut("Ctrl+F5");
        iStepInto.set_group(library_id()).set_shortcut("F11");
        iStepOver.set_group(library_id()).set_shortcut("F10");
        iStepOut.set_group(library_id()).set_shortcut("Shift+F11");
        iBreakAll.set_group(library_id()).set_shortcut("Ctrl+Alt+Break");
        iStopDebugging.set_group(library_id()).set_shortcut("Shift+F5");
        iToggleBreakpoint.set_group(library_id()).set_shortcut("F9");
        iNewBreakpoint.set_group(library_id());
        iDeleteAllBreakpoints.set_group(library_id());
        iDisableAllBreakpoints.set_group(library_id());
        iImportScriptingLanguage.set_group(library_id());
        iEditScriptingLanguage.set_group(library_id());

        iScriptMenu.set_group(library_id());

        // (neos's concept libraries, compiler plugins, runtime plugins and languages: the installed neos's, not this program's folder's)
        neos::set_home(neos::installed_home());

        // (the syntax highlighting of scripts: a service, for the script editors of other libraries)
        iScriptHighlighter = std::make_unique<neos_script_highlighter>();
        neolib::services::register_service<i_script_highlighter>(*iScriptHighlighter);
        // (and the debugging of scripts: a service, for their editors (see i_script_debugger))
        iScriptDebugger = std::make_unique<neos_script_debugger>();
        neolib::services::register_service<i_script_debugger>(*iScriptDebugger);
        iScriptMenu.add_action(iNewScript);
        iScriptMenu.add_action(iOpenScript);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iStartDebugging);
        iScriptMenu.add_action(iStartWithoutDebugging);
        iScriptMenu.add_action(iBreakAll);
        iScriptMenu.add_action(iStopDebugging);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iStepInto);
        iScriptMenu.add_action(iStepOver);
        iScriptMenu.add_action(iStepOut);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iToggleBreakpoint);
        iScriptMenu.add_action(iNewBreakpoint);
        iScriptMenu.add_action(iDeleteAllBreakpoints);
        iScriptMenu.add_action(iDisableAllBreakpoints);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iImportScriptingLanguage);
        iScriptMenu.add_action(iEditScriptingLanguage);

        iClientManager->start_console_client_session(
            [&](i_terminal& aTerminal, i_string const& aCommand, i_ref_ptr<i_console_client>& aClient)
            {
                auto bits = neolib::tokens(aCommand.to_std_string(), " "s);
                if (bits.size() >= 1)
                {
                    if (bits[0] == "neos")
                    {
                        // ("neos <program>": a session that only runs programs (see neos_session); "neos -g <program>": debugged)
                        auto const command = aCommand.to_std_string();
                        auto const program = command.find_first_not_of(' ', command.find("neos") + 4u);
                        aClient = make_ref<neos_session>(neolib::get_service_provider(), *iScriptDebugger,
                            program == std::string::npos ? std::string{} : command.substr(program));
                    }
                }
            });
    }

    neos_element_library::~neos_element_library()
    {
        neolib::services::unregister_service<i_script_debugger>(*iScriptDebugger);
        neolib::services::unregister_service<i_script_highlighter>(*iScriptHighlighter);
    }

    neolib::i_application& neos_element_library::application() const
    {
        return iApplication;
    }

    void neos_element_library::ide_ready(i_ide& aIde)
    {
        aIde.add_sub_menu(id::MenuBar, iScriptMenu);
        // (opening script files into the active project (enabled while there is one): each in a script editor (a nested window on the 
        // IDE's workspace), as a window dropped on the workspace from the toolbox is)
        auto& projectManager = aIde.projects();
        auto& workspace = aIde.workspace();
        auto update_open_script = [this, &projectManager]() { iOpenScript.enable(projectManager.project_active()); };
        iSink += projectManager.project_activated([update_open_script](i_project&) { update_open_script(); });
        iSink += projectManager.project_deactivated([update_open_script](i_project&) { update_open_script(); });
        update_open_script();
        iSink += iOpenScript.triggered([this, &projectManager, &workspace]()
        {
            if (!projectManager.project_active())
                return;
            // (the files of the languages that can be highlighted)
            filter_patterns patterns;
            neolib::vector<neolib::string> extensions;
            iScriptHighlighter->extensions(extensions);
            for (auto const& extension : extensions)
                patterns.push_back("*" + extension.to_std_string());
            if (patterns.empty())
                patterns.push_back("*.*");
            auto files = open_file_dialog(workspace.root().as_widget(), file_dialog_spec{ "Open Script", {}, patterns, "Script Files" }, true);
            if (!files)
                return;
            auto& project = projectManager.active_project();
            point position{ 32.0_dip, 32.0_dip };
            for (auto const& file : files.value())
            {
                try
                {
                    auto& element = project.create_element(project.root(), "script"_s, string{ file });
                    // (its window is made for a scratch parent (whose caddy nest it is added to, which would make it follow the scratch's 
                    // geometry: gone with it) and then nested (MDI) in the workspace)
                    auto scratch = make_ref<widget<>>(workspace.root().as_widget());
                    element.create_layout_item(*scratch);
                    if (element.has_layout_item() && element.layout_item().is_widget())
                    {
                        auto script = ref_ptr<i_widget>{ element.layout_item().as_widget() };
                        workspace.add(script);
                        if (script->is_root() && script->root().is_nested())
                            service<i_surface_manager>().nest_for(workspace, nest_type::MDI).add(script->root().native_window());
                        script->move(position);
                        position += point{ 32.0_dip, 32.0_dip };
                    }
                }
                catch (std::exception const& e)
                {
                    service<i_surface_manager>().display_error_message("Open Script"_t, string{ e.what() });
                }
            }
        });
        // (the debugger: its panes, and its actions (enabled as it is debugging, or not))
        auto& debugger = *iScriptDebugger;
        debugger.ide_ready(aIde);
        iSink += iStartDebugging.triggered([&debugger]() { debugger.start(false); });
        iSink += iStartWithoutDebugging.triggered([&debugger]() { debugger.start_without_debugging(); });
        iSink += iStepInto.triggered([&debugger]() { debugger.step(neos::debug::action::StepInto); });
        iSink += iStepOver.triggered([&debugger]() { debugger.step(neos::debug::action::StepOver); });
        iSink += iStepOut.triggered([&debugger]() { debugger.step(neos::debug::action::StepOut); });
        iSink += iBreakAll.triggered([&debugger]() { debugger.break_all(); });
        iSink += iStopDebugging.triggered([&debugger]() { debugger.stop_debugging(); });
        iSink += iToggleBreakpoint.triggered([&debugger]() { debugger.toggle_breakpoint(); });
        iSink += iNewBreakpoint.triggered([&debugger]() { debugger.new_breakpoint(); });
        iSink += iDeleteAllBreakpoints.triggered([&debugger]() { debugger.delete_all_breakpoints(); });
        iSink += iDisableAllBreakpoints.triggered([&debugger]() { debugger.disable_all_breakpoints(); });
        auto update_actions = [this, &debugger]()
        {
            bool const debugging = debugger.debugging();
            bool const stopped = debugger.stopped();
            iStartDebugging.set_text(debugging ? "Continue"_t : "Start Debugging"_t);
            iStartDebugging.enable(!debugging || stopped);
            iStartWithoutDebugging.enable(!debugging);
            iBreakAll.enable(debugging && !stopped);
            iStopDebugging.enable(debugging);
            iStepInto.enable(!debugging || stopped);
            iStepOver.enable(!debugging || stopped);
            iStepOut.enable(stopped);
        };
        iSink += debugger.LocationChanged(update_actions);
        update_actions();
    }

    const neos_element_library::elements_t& neos_element_library::elements() const
    {
        return iElements;
    }

    const neos_element_library::elements_ordered_t& neos_element_library::elements_ordered() const
    {
        return iElementsOrdered;
    }

    void neos_element_library::create_element(i_project& aProject, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<string, std::function<i_element* (i_project& aProject, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ROOT_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, string{ #Type }, aElementId }; } },
            #define MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, string{ #Name }, aElementId }; } },
            MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(project, i_project)
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aProject, aElementId));
            return;
        }
        throw unknown_element_type();
    }

    void neos_element_library::create_element(i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<string, std::function<i_element*(i_element&, const neolib::i_string&, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, string{ #Type }, aElementId, element_group(aElementType) }; } },
            #define MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, string{ #Name }, aElementId, element_group(aElementType) }; } },
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aParent, aElementType, aElementId));
            return;
        }
        throw unknown_element_type();
    }

    element_group neos_element_library::element_group(const neolib::i_string& aElementType) const
    {
        static const std::map<string, DesignStudio::element_group> sElementGroups =
        {
        };
        auto group = sElementGroups.find(aElementType);
        if (group != sElementGroups.end())
        {
            return group->second;
        }
        throw unknown_element_type();
    }

    i_texture const& neos_element_library::element_icon(const neolib::i_string& aElementType) const
    {
        auto& icons = iIcons[service<i_app>().current_style().palette().color(color_role::Text)];
        static std::map<string, std::function<void(texture&)>> sIconResources =
        {
        };
        auto existing = sIconResources.find(aElementType);
        if (existing != sIconResources.end())
        {
            if (icons.find(aElementType) == icons.end())
                existing->second(icons[aElementType]);
            return icons[aElementType];
        }
        if (icons.find("default") == icons.end())
            icons["default"] = texture{ size{32.0, 32.0} };
        return icons["default"];
    }
}
