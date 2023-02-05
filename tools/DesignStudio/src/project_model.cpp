// project_model.cpp
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
#include <neogfx/tools/DesignStudio/symbol.hpp>
#include "widget_caddy.hpp"
#include "project_model.hpp"

#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx::DesignStudio
{
    project_presentation_model::project_presentation_model(i_project_manager& aProjectManager) : 
        base_type{ aProjectManager }
    {
    }

    ng::item_cell_flags project_presentation_model::cell_flags(ng::item_presentation_model_index const& aIndex) const
    {
        auto result = base_type::cell_flags(aIndex);
        if (std::holds_alternative<ds::element_group>(item_model().item(to_item_model_index(aIndex))))
            result = result & ~ng::item_cell_flags::Selectable;
        return result;
    }

    ng::optional_texture project_presentation_model::cell_image(const ng::item_presentation_model_index& aIndex) const
    {
        auto const& tool = item_model().item(to_item_model_index(aIndex));
        if (std::holds_alternative<ds::element_group>(tool))
        {
            switch (std::get<ds::element_group>(tool))
            {
            case ds::element_group::Perspective:
                return perspectiveTexture;
            case ds::element_group::Project:
                return projectTexture;
            default:
                return base_type::cell_image(aIndex);
            }
        }
        else
            return base_type::cell_image(aIndex);
    }

    void populate_project_model(project_model& aModel, project_presentation_model& aPresentationModel)
    {
        auto projectPerspective = aModel.insert_item(aModel.send(), ds::element_group::Perspective, "Perspective");
        auto stringify_tool = [](const ng::i_string& aInput) -> string
        {
            std::string result;
            auto bits = neolib::tokens(aInput.to_std_string(), "_"s);
            for (auto const& bit : bits)
            {
                if (!result.empty())
                    result += ' ';
                result += std::toupper(bit[0]);
                result += bit.substr(1);
            }
            return result;
        };

        for (auto const& plugin : ng::service<ng::i_app>().plugin_manager().plugins())
        {
            ng::ref_ptr<ds::i_element_library> elementLibrary;
            plugin->discover(elementLibrary);
            if (elementLibrary)
                for (auto const& tool : elementLibrary->elements_ordered())
                    switch (elementLibrary->element_group(tool))
                    {
                    default:
                        break;
                    case ds::element_group::Perspective:
                        aPresentationModel.perspectiveTexture = elementLibrary->element_icon("perspective"_s);
                        break;
                    case ds::element_group::Project:
                        aPresentationModel.projectTexture = elementLibrary->element_icon("project"_s);
                        break;
                    case ds::element_group::App:
                        aModel.append_item(projectPerspective, ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    }
        }

        aPresentationModel.set_item_model(aModel);
        aPresentationModel.set_column_read_only(0u);
    }
}