// web_view.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/widget/i_web_view.hpp>

namespace neogfx
{
    class web_view : widget<i_web_view>
    {
        meta_object(widget<i_web_view>)
        // events
    public:
        // types
    public:
    private:
        using base_type = widget<i_web_view>;
        // construction
    public:
        web_view(i_widget& aParent);
        web_view(i_layout& aLayout);
        ~web_view();
        // widget
    public:
        void paint(i_graphics_context& aGc) const override;
        // own
    private:
        void init();
    private:
        vertical_layout iLayout;
        ref_ptr<i_web_view> iWebViewInstance;
        std::optional<text_widget> iErrorBox;
        sink iSink;
    };
}
