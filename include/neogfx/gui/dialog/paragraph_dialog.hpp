// paragraph_dialog.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/check_box.hpp>
#include <neogfx/gui/widget/unit_spin_box.hpp>
#include <neogfx/gui/widget/text_edit.hpp>

namespace neogfx
{
    class paragraph_dialog : public dialog
    {
        meta_object(dialog)
    public:
        define_event(SelectionChanged, selection_changed)
    public:
        paragraph_dialog(paragraph_format const& aCurrentParagraphFormat = {});
        paragraph_dialog(i_widget& aParent, paragraph_format const& aCurrentParagraphFormat = {});
        ~paragraph_dialog();
    public:
        paragraph_format const& current_format() const;
        paragraph_format const& selected_format() const;
        void select_paragraph_format(neogfx::paragraph_format const& aFormat);
    protected:
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    private:
        void init();
        void update_selected_format(i_widget const& aUpdatingWidget);
        void update_widgets();
    private:
        sink iSink;
        bool iUpdating;
        paragraph_format iCurrentParagraphFormat;
        paragraph_format iSelectedParagraphFormat;
        horizontal_layout iLayout0;
        group_box iSampleBox;
        text_edit iSample;
    };
}