// i_clipboard.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/event.hpp>

namespace neogfx
{
    class i_clipboard;

    class i_clipboard_sink
    {
    public:
        virtual bool can_undo() const = 0;
        virtual bool can_redo() const = 0;
        virtual bool can_cut() const = 0;
        virtual bool can_copy() const = 0;
        virtual bool can_paste() const = 0;
        virtual bool can_delete_selected() const = 0;
        virtual bool can_select_all() const = 0;
        virtual void undo(i_clipboard& aClipboard) = 0;
        virtual void redo(i_clipboard& aClipboard) = 0;
        virtual void cut(i_clipboard& aClipboard) = 0;
        virtual void copy(i_clipboard& aClipboard) = 0;
        virtual void paste(i_clipboard& aClipboard) = 0;
        virtual void delete_selected(i_clipboard& aClipboard) = 0;
        virtual void select_all(i_clipboard& aClipboard) = 0;
    };

    class i_clipboard
    {
    public:
        declare_event(sink_activated)
        declare_event(sink_deactivated)
    public:
        struct no_active_sink : std::logic_error { no_active_sink() : std::logic_error("neogfx::i_clipboard::no_active_sink") {} };
        struct sink_not_active : std::logic_error { sink_not_active() : std::logic_error("neogfx::i_clipboard::sink_not_active") {} };
    public:
        virtual bool sink_active() const = 0;
        virtual i_clipboard_sink& active_sink() = 0;
        virtual void activate(i_clipboard_sink& aSink) = 0;
        virtual void deactivate(i_clipboard_sink& aSink) = 0;
    public:
        virtual bool has_text() const = 0;
        virtual std::string text() const = 0;
        virtual void set_text(const std::string& aText) = 0;
    public:
        virtual void cut() = 0;
        virtual void copy() = 0;
        virtual void paste() = 0;
        virtual void delete_selected() = 0;
        virtual void select_all() = 0;
    };
}