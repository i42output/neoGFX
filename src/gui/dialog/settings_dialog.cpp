// settings_dialog.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <neolib/core/scoped.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/settings_dialog.hpp>

namespace neogfx
{
    settings_dialog::settings_dialog() :
        dialog{ "Settings", window_style::Modal | window_style::TitleBar | window_style::Close }
    {
        init();
    }

    settings_dialog::settings_dialog(i_widget& aParent) :
        dialog{ aParent, "Settings", window_style::Modal | window_style::TitleBar | window_style::Close }
    {
        init();
    }

    settings_dialog::~settings_dialog()
    {
    }

    size settings_dialog::minimum_size(const optional_size& aAvailableSpace) const
    {
        auto result = dialog::minimum_size(aAvailableSpace);
        if (dialog::has_minimum_size())
            return result;
        return result;
    }

    void settings_dialog::init()
    {
        button_box().add_button(standard_button::Ok);
        button_box().add_button(standard_button::Cancel);

        center_on_parent();
        set_ready_to_render(true);
    }
}