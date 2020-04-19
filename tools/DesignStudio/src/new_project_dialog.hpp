// new_project_dialog.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "DesignStudio.hpp"
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include "symbol.hpp"

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
        new_project_dialog(i_widget& aParent) :
            dialog{ aParent, "New Project"_t, ng::window_style::Modal | ng::window_style::TitleBar | ng::window_style::Close },
            iNamingConvention{ naming_convention::NeoGfx },
            iLayout0{ client_layout() }, iLayout01{ iLayout0 }, iLayout1{ iLayout01 }, iLayout2{ iLayout01 }, 
            iType{ iLayout1, "Project Type"_t }, iDefaults{ iLayout2, "Project Defaults"_t },
            iNewApp{ iType.item_layout(), "Blank app"_t },
            iNewMVCApp{ iType.item_layout(), "MVC (model-view-controller) app"_t },
            iNewDialogApp{ iType.item_layout(), "Dialog app"_t },
            iNew2DGame{ iType.item_layout(), "2D game"_t },
            iNew25DGame{ iType.item_layout(), "2.5D (isometric) game"_t },
            iNew3DGame{ iType.item_layout(), "3D game"_t },
            iLayout3{ iDefaults.item_layout() },
            iNameLabel{ iLayout3, "Name:"_t },
            iName{ iLayout3 },
            iLayout4{ iDefaults.item_layout() },
            iNamespaceLabel{ iLayout4, "Namespace:"_t },
            iNamespace{ iLayout4 },
            iSourceCode{ iDefaults.item_layout(), "Source Code Naming Convention"_t },
            iLowerCaseWithUnderscores{ iSourceCode.item_layout(), "Lower case with underscores"_t },
            iMixedCaseWithUnderscores{ iSourceCode.item_layout(), "Mixed case with underscores"_t },
            iUpperCamelCase{ iSourceCode.item_layout(), "Upper camel case"_t },
            iLowerCamelCase{ iSourceCode.item_layout(), "Lower camel case"_t },
            iNeoGfx{ iSourceCode.item_layout(), "neoGFX (Symbianesque)"_t },
            iSpacer1{ iLayout1 },
            iSpacer2{ iLayout2 }
        {
            button_box().add_button(ng::standard_button::Ok);
            button_box().add_button(ng::standard_button::Cancel);
            iNew2DGame.enable(false);
            iNew25DGame.enable(false);
            iNew3DGame.enable(false);
            iName.set_size_hint(ng::size_hint{ "Medium sized project name" });
            iName.set_focus();
            iNamespace.set_size_hint(ng::size_hint{ "Medium sized namespace name" });
            centre_on_parent();
            auto updateNamespace = [this]()
            {
                iNamespace.set_text(to_symbol_name(iName.text(), iNamingConvention, named_entity::Namespace));
            };
            iName.TextChanged([=]()
            {
                updateNamespace();
            });
            iLowerCaseWithUnderscores.checked([=]() { iNamingConvention = naming_convention::LowerCaseWithUnderscores; updateNamespace(); });
            iMixedCaseWithUnderscores.checked([=]() { iNamingConvention = naming_convention::MixedCaseWithUnderscores; updateNamespace(); });
            iUpperCamelCase.checked([=]() { iNamingConvention = naming_convention::UpperCamelCase; updateNamespace(); });
            iLowerCamelCase.checked([=]() { iNamingConvention = naming_convention::LowerCamelCase; updateNamespace(); });
            iNeoGfx.checked([=]() { iNamingConvention = naming_convention::NeoGfx; updateNamespace(); });
            TryAccept([this](bool& aCanAccept)
            {
                if (to_symbol_name(iName.text(), iNamingConvention, named_entity::Namespace).empty())
                {
                    aCanAccept = false;
                    iName.set_focus();
                }
                else if (to_symbol_name(iNamespace.text(), iNamingConvention, named_entity::Namespace).empty())
                {
                    aCanAccept = false;
                    iNamespace.set_focus();
                }
            });
        }
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
        ng::horizontal_layout iLayout3;
        ng::label iNameLabel;
        ng::line_edit iName;
        ng::horizontal_layout iLayout4;
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