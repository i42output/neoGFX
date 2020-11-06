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
#include <neogfx/tools/DesignStudio/workflow.hpp>

namespace neogfx::DesignStudio
{
    typedef ng::basic_item_model<ds::workflow_tool> workflow_model; // todo

    class workflow_presentation_model : public ng::basic_item_presentation_model<workflow_model>
    {
        typedef ng::basic_item_presentation_model<workflow_model> base_type;
    public:
        workflow_presentation_model();
    public:
        ng::optional_size cell_image_size(ng::item_presentation_model_index const& aIndex) const override;
        ng::optional_texture cell_image(ng::item_presentation_model_index const& aIndex) const override;
        ng::item_cell_flags cell_flags(ng::item_presentation_model_index const& aIndex) const override;
    public:
        ng::texture cppIdeTexture;
        ng::texture stickyNoteTexture;
    };
}