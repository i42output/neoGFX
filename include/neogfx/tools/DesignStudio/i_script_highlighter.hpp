// i_script_highlighter.hpp
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
#include <neolib/core/i_vector.hpp>
#include <neolib/core/i_string.hpp>

namespace neogfx::DesignStudio
{
    // the syntax highlighting of scripts (a service: the neos element library's highlights the languages that neos has schemas for, 
    // from the AST that neos parses a script into)
    class i_script_highlighter : public neolib::i_service
    {
    public:
        enum class category : std::uint32_t
        {
            Comment,
            Keyword,
            String,
            Number,
            Constant,
            Type,
            Function,
            Parameter
        };
        // (a run of a script's text: its code points [first, last))
        struct span
        {
            std::uint32_t first;
            std::uint32_t last;
            category kind;
        };
    public:
        virtual ~i_script_highlighter() = default;
    public:
        // whether scripts whose files have the extension aExtension (e.g. ".py") can be highlighted
        virtual bool can_highlight(neolib::i_string const& aExtension) const = 0;
        // the extensions (e.g. ".py") of the files whose scripts can be highlighted
        virtual void extensions(neolib::i_vector<neolib::i_string>& aExtensions) const = 0;
        // the colours ("#RRGGBB") of the categories (by category; empty: none) in a theme ("dark" or "light") for scripts of files with the 
        // extension aExtension (empty: not configured)
        virtual void colours(neolib::i_string const& aExtension, neolib::i_string const& aTheme, neolib::i_vector<neolib::i_string>& aColours) const = 0;
        // the runs of the script aScript (of a file with the extension aExtension) to highlight; it may take a while so it is called on 
        // a thread other than the GUI's (calls are serialized); false if the script cannot be highlighted (e.g. no schema)
        virtual bool highlight(neolib::i_string const& aExtension, neolib::i_string const& aScript, neolib::i_vector<span>& aSpans) const = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x140bbee0, 0xbdef, 0x4232, 0x95d9, { 0xe0, 0xa7, 0x68, 0xdd, 0x83, 0x44 } }; return sIid; }
    };
}
