// i_script_debugger.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2026 Leigh Johnston

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

#include <cstdint>
#include <neolib/core/i_service.hpp>
#include <neolib/core/i_string.hpp>

namespace neogfx::DesignStudio
{
    // the debugging of scripts (a service: the neos element library's debugs the programs that neos runs): the breakpoints of scripts'
    // files (by path, line (1-based) and column (1-based, in bytes; 0: the line's statements)) and where a program being debugged has
    // stopped; a script's editor shows them and asks for what needs it (its cursor, its console: see request)
    class i_script_debugger : public neolib::i_service
    {
    public:
        enum class breakpoint_state : std::uint32_t
        {
            None,
            Enabled,
            Disabled
        };
        // (what the debugger asks of the active script's editor (see set_active_script): Start, StartWithoutDebugging, the script run (in
        // a console: the editor's) with or without debugging; ToggleBreakpoint, a breakpoint toggled where the editor's cursor is (or at
        // the statement of its node graph's node selected))
        enum class request : std::uint32_t
        {
            Start,
            StartWithoutDebugging,
            ToggleBreakpoint
        };
        // (the program being debugged: Continue, if it is stopped; StepInto, StepOver, a step, if it is stopped (else the active script is 
        // run, stopping at its first statement); StepOut, if it is stopped)
        enum class command : std::uint32_t
        {
            Continue,
            StepInto,
            StepOver,
            StepOut
        };
    public:
        declare_event(requested, request)
        declare_event(breakpoints_changed)  ///< (the breakpoints, or their states, have changed)
        declare_event(location_changed)     ///< (the program has stopped (or the frame selected has changed), or it has resumed)
    public:
        virtual ~i_script_debugger() = default;
    public:
        // the script (its file's path) whose editor is active (the requests are its editor's); empty: none
        virtual void set_active_script(neolib::i_string const& aPath) = 0;
        virtual neolib::i_string const& active_script() const = 0;
        // whether a program is being debugged; whether it is stopped
        virtual bool debugging() const = 0;
        virtual bool stopped() const = 0;
        // the breakpoint at a line (aColumn 0: any on the line) of a script's file; one toggled (added, or removed)
        virtual breakpoint_state breakpoint_at(neolib::i_string const& aPath, std::uint32_t aLine, std::uint32_t aColumn = 0u) const = 0;
        virtual void toggle_breakpoint(neolib::i_string const& aPath, std::uint32_t aLine, std::uint32_t aColumn = 0u) = 0;
        // where the stopped program is, in its frame selected (see the call stack): false if it is not stopped (or that frame's source is
        // not known); the statement's text runs from aLine:aColumn to aEndLine:aEndColumn (the column after its last; 0s: not known),
        // aTop is false if the frame is not the innermost
        virtual bool location(neolib::i_string& aPath, std::uint32_t& aLine, std::uint32_t& aColumn, std::uint32_t& aEndLine, std::uint32_t& aEndColumn, bool& aTop) const = 0;
        // a command (see command) carried out (as the Script menu's actions are)
        virtual void execute(command aCommand) = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x5c2e9a41, 0x7f0b, 0x4d36, 0x8a1e, { 0x3b, 0x94, 0xc7, 0x52, 0x0d, 0xe6 } }; return sIid; }
    };
}
