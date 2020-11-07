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
    workflow_presentation_model::workflow_presentation_model() :
        cppIdeTexture{ ng::colored_icon(ng::image{ ":/neogfx/DesignStudio/resources/cpp.png" }, ng::color::Khaki) },
        stickyNoteTexture{ ng::colored_icon(ng::image{ ":/neogfx/DesignStudio/resources/note.png" }, ng::color::Khaki) }
    {
        DraggingItemRenderInfo([&](ng::i_drag_drop_item const& aItem, bool& aCanRender, size& aRenderExtents)
        {
            switch (item_model().item(to_item_model_index(aItem.index())))
            {
            case ds::workflow_tool::StickyNote:
                aCanRender = true;
                aRenderExtents = size{ 256.0_dip, 256.0_dip };
                break;
            default:
                aCanRender = false;
                break;
            }
        });
        DraggingItemRender([&](ng::i_drag_drop_item const& aItem, i_graphics_context& aGc, point const& aPosition)
        {
            switch (item_model().item(to_item_model_index(aItem.index())))
            {
            case ds::workflow_tool::StickyNote:
                // todo
                aGc.fill_rect(rect{ point{}, size{256.0_dip, 256.0_dip } }.with_centerd_origin() + aPosition, ng::color::PapayaWhip);
                break;
            default:
                break;
            }
        });
    }

    ng::optional_size workflow_presentation_model::cell_image_size(ng::item_presentation_model_index const& aIndex) const
    {
        return ng::size{ 32.0_dip, 32.0_dip };
    }

    ng::optional_texture workflow_presentation_model::cell_image(ng::item_presentation_model_index const& aIndex) const
    {
        switch (item_model().item(to_item_model_index(aIndex)))
        {
        case ds::workflow_tool::CppIde:
            return cppIdeTexture;
        case ds::workflow_tool::StickyNote:
            return stickyNoteTexture;
        default:
            return {};
        }
    }

    ng::item_cell_flags workflow_presentation_model::cell_flags(ng::item_presentation_model_index const& aIndex) const
    {
        auto result = base_type::cell_flags(aIndex);
        switch (item_model().item(to_item_model_index(aIndex)))
        {
        case ds::workflow_tool::StickyNote:
            result |= ng::item_cell_flags::Draggable;
            break;
        }
        return result;
    }
}