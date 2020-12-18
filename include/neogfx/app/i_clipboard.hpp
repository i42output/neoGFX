// i_clipboard.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/gfx/image.hpp>

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
        virtual void delete_selected() = 0;
        virtual void select_all() = 0;
    };

    class default_clipboard_sink : public i_clipboard_sink
    {
    public:
        bool can_undo() const override { return false; }
        bool can_redo() const override { return false; }
        bool can_cut() const override { return false; }
        bool can_copy() const override { return false; }
        bool can_paste() const override { return false; }
        bool can_delete_selected() const override { return false; }
        bool can_select_all() const override { return false; }
        void undo(i_clipboard& aClipboard) override {}
        void redo(i_clipboard& aClipboard) override {}
        void cut(i_clipboard& aClipboard) override {}
        void copy(i_clipboard& aClipboard) override {}
        void paste(i_clipboard& aClipboard) override {}
        void delete_selected() override {}
        void select_all() override {}
    };

    class i_clipboard : public i_service
    {
    public:
        declare_event(updated)
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
        virtual void set_text(std::string const& aText) = 0;
        virtual bool has_image() const = 0;
        virtual neogfx::image image() const = 0;
        virtual void set_image(const neogfx::image& aImage) = 0;
    public:
        virtual void cut() = 0;
        virtual void copy() = 0;
        virtual void paste() = 0;
        virtual void delete_selected() = 0;
        virtual void select_all() = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x441eee78, 0x6c80, 0x464b, 0xb733, { 0x18, 0x91, 0x90, 0xa8, 0x39, 0xb9 } }; return sIid; }
    };
}