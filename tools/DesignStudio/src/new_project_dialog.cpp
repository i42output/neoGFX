// new_project_dialog.cpp
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

#include <DesignStudio/DesignStudio.hpp>
#include <neolib/file/file.hpp>
#include <neogfx/app/file_dialog.hpp>
#include "new_project_dialog.hpp"

namespace design_studio
{
    new_project_dialog::new_project_dialog(i_widget& aParent) :
        dialog{ aParent, "New Project"_t, ng::window_style::Modal | ng::window_style::TitleBar | ng::window_style::Close },
        iNamingConvention{ naming_convention::NeoGfx },
        iLayout0{ client_layout() }, iLayout01{ iLayout0 }, iLayout1{ iLayout01 }, iLayout2{ iLayout01 }, 
        iType{ iLayout1, "Project Type"_t }, iDefaults{ iLayout2, "Project Details"_t },
        iNewApp{ iType.item_layout(), "Blank app"_t },
        iNewMVCApp{ iType.item_layout(), "MVC (model-view-controller) app"_t },
        iNewDialogApp{ iType.item_layout(), "Dialog app"_t },
        iNew2DGame{ iType.item_layout(), "2D game"_t },
        iNew25DGame{ iType.item_layout(), "2.5D (isometric) game"_t },
        iNew3DGame{ iType.item_layout(), "3D game"_t },
        iNameLayout{ iDefaults.item_layout() },
        iNameLabel{ iNameLayout, "Name:"_t },
        iName{ iNameLayout },
        iLocationLayout{ iDefaults.item_layout() },
        iLocationLabel{ iLocationLayout , "Location:"_t },
        iLocation{ iLocationLayout },
        iLocationBrowse{ iLocationLayout, ng::image{ ":/neogfx/resources/icons.zip#openfolder.png" } },
        iNamespaceLayout{ iDefaults.item_layout() },
        iNamespaceLabel{ iNamespaceLayout, "Namespace:"_t },
        iNamespace{ iNamespaceLayout },
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
        iLocation.set_text(neolib::user_documents_directory());
        iLocationBrowse.image_widget().set_fixed_size( ng::size{ 16.0_dip });
        iLocationBrowse.Clicked([&]()
        {
            auto newLocation = neogfx::select_folder_dialog(*this, "Project Location"_t, iLocation.text());
            if (newLocation)
                iLocation.set_text(*newLocation);
        });
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

    const std::string& new_project_dialog::name() const
    {
        return iName.text();
    }

    const std::string& new_project_dialog::namespace_() const
    {
        return iNamespace.text();
    }
}