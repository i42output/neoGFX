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
#include <neogfx/gfx/utility.hpp>
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
        workflow_presentation_model() :
            cppIdeTexture{ ng::colored_icon(ng::image{ ":/neogfx/DesignStudio/resources/cpp.png" }, ng::color::Khaki) },
            stickyNoteTexture{ ng::colored_icon(ng::image{ ":/neogfx/DesignStudio/resources/note.png" }, ng::color::Khaki) }
        {
            DraggingItemRenderInfo([&](ng::item_presentation_model_index const& aIndex, bool& aCanRender, size& aRenderExtents)
            {
                switch (item_model().item(to_item_model_index(aIndex)))
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
            DraggingItemRender([&](ng::item_presentation_model_index const& aIndex, i_graphics_context& aGc, point const& aPosition)
            {
                switch (item_model().item(to_item_model_index(aIndex)))
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
    public:
        ng::optional_size cell_image_size(ng::item_presentation_model_index const& aIndex) const override
        {
            return ng::size{ 32.0_dip, 32.0_dip };
        }
        ng::optional_texture cell_image(ng::item_presentation_model_index const& aIndex) const override
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
        ng::item_cell_flags cell_flags(ng::item_presentation_model_index const& aIndex) const override
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
    public:
        ng::texture cppIdeTexture;
        ng::texture stickyNoteTexture;
    };
}