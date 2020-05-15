// clipboard.hpp
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
#include "i_clipboard.hpp"

namespace neogfx
{
    class i_native_clipboard;

    class clipboard : public i_clipboard
    {
    public:
        define_declared_event(SinkActivated, sink_activated)
        define_declared_event(SinkDeactivated, sink_deactivated)
    public:
        clipboard(i_native_clipboard& aSystemClipboard);
    public:
        virtual bool sink_active() const;
        virtual i_clipboard_sink& active_sink();
        virtual void activate(i_clipboard_sink& aSink);
        virtual void deactivate(i_clipboard_sink& aSink);
    public:
        virtual bool has_text() const;
        virtual std::string text() const;
        virtual void set_text(const std::string& aText);
    public:
        virtual void cut();
        virtual void copy();
        virtual void paste();
        virtual void delete_selected();
        virtual void select_all();
    private:
        i_native_clipboard& iSystemClipboard;
        i_clipboard_sink* iActiveSink;
    };
}