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
        define_declared_event(Updated, updated)
        define_declared_event(SinkActivated, sink_activated)
        define_declared_event(SinkDeactivated, sink_deactivated)
    public:
        clipboard(i_native_clipboard& aSystemClipboard);
    public:
        bool sink_active() const override;
        i_clipboard_sink& active_sink() override;
        void activate(i_clipboard_sink& aSink) override;
        void deactivate(i_clipboard_sink& aSink) override;
    public:
        using i_clipboard::text;
        using i_clipboard::set_text;
        bool has_text() const override;
        i_string const& text() const override;
        void set_text(i_string const& aText) override;
        bool has_image() const override;
        neogfx::image image() const override;
        void set_image(const neogfx::image& aImage) override;
    public:
        void cut() override;
        void copy() override;
        void paste() override;
        void delete_selected() override;
        void select_all() override;
    private:
        i_native_clipboard& iSystemClipboard;
        i_clipboard_sink* iActiveSink;
    };
}