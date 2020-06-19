// new_project_dialog.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/tools/DesignStudio/symbol.hpp>
#include "DesignStudio.ui.hpp"

namespace neogfx::DesignStudio
{
    class new_project_dialog_ex : public new_project_dialog
    {
    public:
        enum result_code_e
        {
            Accepted,
            Rejected
        };
    public:
        new_project_dialog_ex(i_widget& aParent);
    public:
        naming_convention namingConvention;
    };
}