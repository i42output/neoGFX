// new_project_dialog.cpp
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neolib/file/file.hpp>
#include <neogfx/app/file_dialog.hpp>
#include "new_project_dialog.hpp"

namespace neogfx::DesignStudio
{
    new_project_dialog_ex::new_project_dialog_ex(i_widget& aParent) :
        new_project_dialog{ aParent },
        namingConvention{ naming_convention::Neogfx }
    {
        projectLocation.set_text(neolib::user_documents_directory());
        browseProjectLocation.Clicked([&]()
        {
            auto newLocation = neogfx::select_folder_dialog(*this, "Project Location"_t, projectLocation.text());
            if (newLocation)
                projectLocation.set_text(*newLocation);
        });
        auto updateNamespace = [this]()
        {
            projectNamespace.set_text(to_symbol_name(projectName.text(), namingConvention, named_entity::Namespace));
        };
        projectName.TextChanged([=]()
        {
            updateNamespace();
        });
        namingLowerCaseSnake.checked([=]() { namingConvention = naming_convention::LowerCaseSnake; updateNamespace(); });
        namingMixedCaseSnake.checked([=]() { namingConvention = naming_convention::MixedCaseSnake; updateNamespace(); });
        namingUpperCamelCase.checked([=]() { namingConvention = naming_convention::UpperCamelCase; updateNamespace(); });
        namingLowerCamelCase.checked([=]() { namingConvention = naming_convention::LowerCamelCase; updateNamespace(); });
        namingNeogfx.checked([=]() { namingConvention = naming_convention::Neogfx; updateNamespace(); });
        TryAccept([this](bool& aCanAccept, bool aQueryOnly)
        {
            if (to_symbol_name(projectName.text(), namingConvention, named_entity::Namespace).empty())
            {
                aCanAccept = false;
                if (!aQueryOnly)
                    projectName.set_focus();
            }
            else if (to_symbol_name(projectNamespace.text(), namingConvention, named_entity::Namespace).empty())
            {
                aCanAccept = false;
                if (!aQueryOnly)
                    projectNamespace.set_focus();
            }
        });
        projectName.set_text("Project1");
        projectName.select_all();
    }
}