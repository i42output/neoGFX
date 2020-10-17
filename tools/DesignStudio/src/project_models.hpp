// project_models.hpp
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

#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>

namespace neogfx::DesignStudio
{
    typedef std::pair<ds::i_element_library*, ng::string> tool_t;
    typedef ng::basic_item_tree_model<std::variant<ds::element_group, tool_t>> toolbox_model;

    class toolbox_presentation_model : public ng::basic_item_presentation_model<decltype(toolboxModel)>
    {
    public:
        toolbox_presentation_model()
        {
        }
    public:
        ng::optional_size cell_image_size(const ng::item_presentation_model_index& aIndex) const override
        {
            auto const& tool = item_model().item(to_item_model_index(aIndex));
            if (std::holds_alternative<ds::element_group>(tool))
            {
                switch (std::get<ds::element_group>(tool))
                {
                case ds::element_group::Project:
                case ds::element_group::Code:
                case ds::element_group::UserInterface:
                    return ng::size{ 24.0_dip, 24.0_dip };
                case ds::element_group::Script:
                case ds::element_group::App:
                case ds::element_group::Menu:
                case ds::element_group::Action:
                case ds::element_group::Widget:
                case ds::element_group::Layout:
                    return {};
                default:
                    return {};
                }
            }
            else
            {
                return ng::size{ 24.0_dip, 24.0_dip };
            }
        }
        ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override
        {
            auto const& tool = item_model().item(to_item_model_index(aIndex));
            if (std::holds_alternative<ds::element_group>(tool))
            {
                switch (std::get<ds::element_group>(tool))
                {
                case ds::element_group::Project:
                    return projectTexture;
                case ds::element_group::UserInterface:
                    return userInterfaceTexture;
                case ds::element_group::Code:
                    return codeTexture;
                case ds::element_group::Script:
                case ds::element_group::Node:
                case ds::element_group::App:
                case ds::element_group::Menu:
                case ds::element_group::Action:
                case ds::element_group::Widget:
                case ds::element_group::Layout:
                    return {};
                default:
                    return {};
                }
            }
            else
            {
                auto const& t = std::get<tool_t>(item_model().item(to_item_model_index(aIndex)));
                return t.first->element_icon(t.second);
            }
        }
    public:
        ng::texture projectTexture;
        ng::texture codeTexture;
        ng::texture userInterfaceTexture;
    };

    typedef ng::basic_item_model<ds::workflow_tool> workflow_model; // todo

    class workflow_presentation_model : public ng::basic_item_presentation_model<workflow_model>
    {
        typedef ng::basic_item_presentation_model<workflow_model> base_type;
    public:
        workflow_presentation_model() :
            cppIdeTexture{ ng::colored_icon(ng::image{ ":/neogfx/DesignStudio/resources/cpp.png" }, ng::color::Khaki) },
            stickyNoteTexture{ ng::colored_icon(ng::image{ ":/neogfx/DesignStudio/resources/note.png" }, ng::color::Khaki) }
        {
        }
    public:
        ng::optional_size cell_image_size(const ng::item_presentation_model_index& aIndex) const override
        {
            return ng::size{ 32.0_dip, 32.0_dip };
        }
        ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override
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
        ng::item_cell_flags cell_flags(const ng::item_presentation_model_index& aIndex) const override
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

    typedef ng::basic_item_tree_model<ds::i_element*, 2> object_model;

    class object_presentation_model : public ng::basic_item_presentation_model<object_model>
    {
    public:
        object_presentation_model()
        {
        }
    public:
        ng::optional_size cell_image_size(const ng::item_presentation_model_index& aIndex) const override
        {
            if (aIndex.column() == 0)
                return ng::size{ 16.0_dip, 16.0_dip };
            else
                return {};
        }
        ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override
        {
            if (aIndex.column() == 0)
            {
                auto const& e = *item_model().item(to_item_model_index(aIndex));
                return e.library().element_icon(e.type());
            }
            else
                return {};
        }
    };
}