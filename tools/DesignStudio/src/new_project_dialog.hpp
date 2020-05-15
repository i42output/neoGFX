// new_project_dialog.hpp
/*
  neogfx C++ App/Game Engine
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

#include <DesignStudio/DesignStudio.hpp>
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <DesignStudio/symbol.hpp>

namespace design_studio
{
    class new_project_dialog : public ng::dialog
    {
    public:
        enum result_code_e
        {
            Accepted,
            Rejected
        };
    public:
        new_project_dialog(i_widget& aParent);
    public:
        const std::string& name() const;
        const std::string& namespace_() const;
    private:
        naming_convention iNamingConvention;
        ng::vertical_layout iLayout0;
        ng::horizontal_layout iLayout01;
        ng::vertical_layout iLayout1;
        ng::vertical_layout iLayout2;
        ng::group_box iType;
        ng::radio_button iNewApp;
        ng::radio_button iNewMVCApp;
        ng::radio_button iNewDialogApp;
        ng::radio_button iNew2DGame;
        ng::radio_button iNew25DGame;
        ng::radio_button iNew3DGame;
        ng::group_box iDefaults;
        ng::horizontal_layout iNameLayout;
        ng::label iNameLabel;
        ng::line_edit iName;
        ng::horizontal_layout iLocationLayout;
        ng::label iLocationLabel;
        ng::line_edit iLocation;
        ng::push_button iLocationBrowse;
        ng::horizontal_layout iNamespaceLayout;
        ng::label iNamespaceLabel;
        ng::line_edit iNamespace;
        ng::group_box iSourceCode;
        ng::radio_button iLowerCaseWithUnderscores;
        ng::radio_button iMixedCaseWithUnderscores;
        ng::radio_button iUpperCamelCase;
        ng::radio_button iLowerCamelCase;
        ng::radio_button iNeoGfx;
        ng::vertical_spacer iSpacer1;
        ng::vertical_spacer iSpacer2;
    };
}