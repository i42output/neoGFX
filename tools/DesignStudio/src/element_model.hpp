// element_model.hpp
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
#include <neogfx/app/i_drag_drop.hpp>
#include <neogfx/tools/DesignStudio/element.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include <neogfx/tools/DesignStudio/i_project_manager.hpp>

namespace neogfx::DesignStudio
{
    typedef std::pair<ds::i_element_library*, ng::string> element_tool_t;

    template <typename Model>
    class element_presentation_model : public ng::basic_item_presentation_model<Model>
    {
        typedef ng::basic_item_presentation_model<Model> base_type;
    public:
        element_presentation_model(i_project_manager& aProjectManager);
    public:
        ng::optional_size cell_image_size(const ng::item_presentation_model_index& aIndex) const override;
        ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override;
        ng::item_cell_flags cell_flags(ng::item_presentation_model_index const& aIndex) const override;
    private:
        string generate_id(const string& aToolName);
    private:
        sink iSink;
        i_drag_drop_item const* iDragDropItem = nullptr;
        ref_ptr<i_element> iSelectedElement;
        std::map<std::string, uint32_t> iIdCounters;
    };
}