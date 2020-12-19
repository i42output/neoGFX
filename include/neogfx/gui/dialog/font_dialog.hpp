// font_dialog.hpp
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
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/check_box.hpp>
#include <neogfx/gui/widget/gradient_widget.hpp>

namespace neogfx
{
    class font_dialog : public dialog
    {
    public:
        define_event(SelectionChanged, selection_changed)
    public:
        font_dialog(const neogfx::font& aCurrentFont = neogfx::font{});
        font_dialog(i_widget& aParent, const neogfx::font& aCurrentFont = neogfx::font{});
        ~font_dialog();
    public:
        void enable_text_effects();
        neogfx::font current_font() const;
        neogfx::font selected_font() const;
        void select_font(const neogfx::font& aFont);
    protected:
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    private:
        void init();
        void update_selected_font(const i_widget& aUpdatingWidget);
    private:
        sink iSink;
        bool iUpdating;
        neogfx::font iCurrentFont;
        neogfx::font iSelectedFont;
        horizontal_layout iLayout0;
        vertical_layout iLayout1;
        label iFamilyLabel;
        drop_list iFamilyPicker;
        vertical_layout iLayout2;
        horizontal_layout iLayout3;
        vertical_layout iLayout4;
        label iStyleLabel;
        drop_list iStylePicker;
        vertical_layout iLayout5;
        label iSizeLabel;
        drop_list iSizePicker;
        widget<> iEffects;
        horizontal_layout iLayoutEffects;
        group_box iInkBox;
        radio_button iInkColor;
        radio_button iInkGradient;
        group_box iPaperBox;
        radio_button iPaperColor;
        radio_button iPaperGradient;
        group_box iTextEffectsBox;
        radio_button iTextEffectsOutline;
        radio_button iTextEffectsShadow;
        radio_button iTextEffectsGlow;
        group_box iSampleBox;
        text_widget iSample;
    };
}