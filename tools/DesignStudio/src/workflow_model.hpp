// workflow_model.hpp
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
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/tools/DesignStudio/i_project_manager.hpp>
#include "element_model.hpp"

namespace neogfx::DesignStudio
{
    typedef ng::basic_item_model<std::variant<ds::element_group, element_tool_t>> workflow_model;

    extern template class element_presentation_model<workflow_model>;

    class workflow_presentation_model : public element_presentation_model<workflow_model>
    {
        typedef element_presentation_model<workflow_model> base_type;
    public:
        workflow_presentation_model(i_project_manager& aProjectManager);
    };

    void populate_workflow_model(workflow_model& aModel, workflow_presentation_model& aPresentationModel);
}