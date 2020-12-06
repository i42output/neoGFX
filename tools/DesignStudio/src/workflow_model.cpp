// workflow_model.cpp
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
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gfx/utility.hpp>
#include <neogfx/app/i_drag_drop.hpp>
#include "workflow_model.hpp"

namespace neogfx::DesignStudio
{
    workflow_presentation_model::workflow_presentation_model(i_project_manager& aProjectManager) : 
        base_type{ aProjectManager }
    {
    }

    void populate_workflow_model(workflow_model& aModel, workflow_presentation_model& aPresentationModel)
    {
        auto stringify_tool = [](const ng::i_string& aInput) -> std::string
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
                    case ds::element_group::Workflow:
                        aModel.insert_item(aModel.end(), ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    default:
                        break;
                    }
        }

        aPresentationModel.set_item_model(aModel);
        aPresentationModel.set_column_read_only(0u);
    }
}