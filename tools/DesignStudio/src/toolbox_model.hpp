// toolbox_model.hpp
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
#include <neogfx/tools/DesignStudio/element.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>

namespace neogfx::DesignStudio
{
    typedef std::pair<ds::i_element_library*, ng::string> tool_t;
    typedef ng::basic_item_tree_model<std::variant<ds::element_group, tool_t>> toolbox_model;

    class toolbox_presentation_model : public ng::basic_item_presentation_model<toolbox_model>
    {
    public:
        toolbox_presentation_model();
    public:
        ng::optional_size cell_image_size(const ng::item_presentation_model_index& aIndex) const override;
        ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override;
    public:
        ng::texture projectTexture;
        ng::texture codeTexture;
        ng::texture userInterfaceTexture;
    };

    void populate_toolbox_model(toolbox_model& aModel, toolbox_presentation_model& aPresentationModel);
}