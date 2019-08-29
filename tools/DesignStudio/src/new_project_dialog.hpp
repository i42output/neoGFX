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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include "symbol.hpp"

namespace neogui
{
    class new_project_dialog : public neogfx::dialog
    {
    public:
        enum result_code_e
        {
            Accepted,
            Rejected
        };
    public:
        new_project_dialog(i_widget& aParent) :
            dialog{ aParent, "New Project"_t, neogfx::window_style::Modal | neogfx::window_style::TitleBar | neogfx::window_style::Close },
            iNamingConvention{ naming_convention::NeoGfx },
            iLayout0{ client_layout() }, iLayout01{ iLayout0 }, iLayout1{ iLayout01 }, iLayout2{ iLayout01 }, 
            iType{ iLayout1, "Project Type"_t }, iDefaults{ iLayout2, "Project Defaults"_t },
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
            button_box().add_button(neogfx::standard_button::Ok);
            button_box().add_button(neogfx::standard_button::Cancel);
            iNew2DGame.enable(false);
            iNew25DGame.enable(false);
            iNew3DGame.enable(false);
            iName.set_size_hint(neogfx::size_hint{ "Medium sized project name" });
            iName.set_focus();
            iNamespace.set_size_hint(neogfx::size_hint{ "Medium sized namespace name" });
            centre_on_parent();
            auto updateNamespace = [this]()
            {
                iNamespace.set_text(to_symbol_name(iName.text(), iNamingConvention, named_entity::Namespace));
            };
            iName.evTextChanged([=]()
            {
                updateNamespace();
            });
            iLowerCaseWithUnderscores.evChecked([=]() { iNamingConvention = naming_convention::LowerCaseWithUnderscores; updateNamespace(); });
            iMixedCaseWithUnderscores.evChecked([=]() { iNamingConvention = naming_convention::MixedCaseWithUnderscores; updateNamespace(); });
            iUpperCamelCase.evChecked([=]() { iNamingConvention = naming_convention::UpperCamelCase; updateNamespace(); });
            iLowerCamelCase.evChecked([=]() { iNamingConvention = naming_convention::LowerCamelCase; updateNamespace(); });
            iNeoGfx.evChecked([=]() { iNamingConvention = naming_convention::NeoGfx; updateNamespace(); });
            evTryAccept([this](bool& aCanAccept)
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
        neogfx::vertical_layout iLayout0;
        neogfx::horizontal_layout iLayout01;
        neogfx::vertical_layout iLayout1;
        neogfx::vertical_layout iLayout2;
        neogfx::group_box iType;
        neogfx::radio_button iNewMVCApp;
        neogfx::radio_button iNewDialogApp;
        neogfx::radio_button iNew2DGame;
        neogfx::radio_button iNew25DGame;
        neogfx::radio_button iNew3DGame;
        neogfx::group_box iDefaults;
        neogfx::horizontal_layout iLayout3;
        neogfx::label iNameLabel;
        neogfx::line_edit iName;
        neogfx::horizontal_layout iLayout4;
        neogfx::label iNamespaceLabel;
        neogfx::line_edit iNamespace;
        neogfx::group_box iSourceCode;
        neogfx::radio_button iLowerCaseWithUnderscores;
        neogfx::radio_button iMixedCaseWithUnderscores;
        neogfx::radio_button iUpperCamelCase;
        neogfx::radio_button iLowerCamelCase;
        neogfx::radio_button iNeoGfx;
        neogfx::vertical_spacer iSpacer1;
        neogfx::vertical_spacer iSpacer2;
    };
}