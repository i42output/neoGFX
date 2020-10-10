// font_widget.hpp
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
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gui/widget/framed_widget.hpp>
#include <neogfx/gui/widget/drop_list.hpp>

namespace neogfx
{
    class font_widget : public framed_widget<>
    {
    public:
        define_event(SelectionChanged, selection_changed)
    public:
        font_widget(const neogfx::font& aCurrentFont = neogfx::font{});
        font_widget(i_widget& aParent, const neogfx::font& aCurrentFont = neogfx::font{});
        font_widget(i_layout& aLayout, const neogfx::font& aCurrentFont = neogfx::font{});
        ~font_widget();
    public:
        neogfx::font current_font() const;
        neogfx::font selected_font() const;
        void select_font(const neogfx::font& aFont);
    private:
        void init();
        void update_selected_font(const i_widget& aUpdatingWidget);
    private:
        sink iSink;
        bool iUpdating;
        neogfx::font iCurrentFont;
        neogfx::font iSelectedFont;
        horizontal_layout iLayout0;
        drop_list iFamilyPicker;
        drop_list iStylePicker;
        drop_list iSizePicker;
    };
}