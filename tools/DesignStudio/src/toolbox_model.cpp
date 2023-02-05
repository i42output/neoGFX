// toolbox_model.cpp
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
#include "toolbox_model.hpp"

#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx::DesignStudio
{
    toolbox_presentation_model::toolbox_presentation_model(i_project_manager& aProjectManager) : 
        base_type{ aProjectManager }
    {
    }

    ng::optional_texture toolbox_presentation_model::cell_image(const ng::item_presentation_model_index& aIndex) const
    {
        auto const& tool = item_model().item(to_item_model_index(aIndex));
        if (std::holds_alternative<ds::element_group>(tool))
        {
            switch (std::get<ds::element_group>(tool))
            {
            case ds::element_group::UserInterface:
                return userInterfaceTexture;
            case ds::element_group::Code:
                return codeTexture;
            default:
                return base_type::cell_image(aIndex);
            }
        }
        else
        {
            return base_type::cell_image(aIndex);
        }
    }

    void populate_toolbox_model(toolbox_model& aModel, toolbox_presentation_model& aPresentationModel)
    {
        auto toolboxCode = aModel.insert_item(aModel.send(), ds::element_group::Code, "Code");
        auto toolboxUserInterface = aModel.insert_item(aModel.send(), ds::element_group::UserInterface, "User Interface");
        auto toolboxMenu = aModel.append_item(toolboxUserInterface, ds::element_group::Menu, "Menu");
        auto toolboxAction = aModel.append_item(toolboxUserInterface, ds::element_group::Action, "Action");
        auto toolboxLayout = aModel.append_item(toolboxUserInterface, ds::element_group::Layout, "Layout");
        auto toolboxWidget = aModel.append_item(toolboxUserInterface, ds::element_group::Widget, "Widget");
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
                    case ds::element_group::Unknown:
                        break;
                    case ds::element_group::Code:
                        aPresentationModel.codeTexture = elementLibrary->element_icon("code"_s);
                        break;
                    case ds::element_group::UserInterface:
                        aPresentationModel.userInterfaceTexture = elementLibrary->element_icon("user_interface"_s);
                        break;
                    case ds::element_group::Script:
                        aModel.append_item(toolboxCode, ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Node:
                        aModel.append_item(toolboxCode, ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Menu:
                        aModel.append_item(toolboxMenu, ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Action:
                        aModel.append_item(toolboxAction, ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Widget:
                        aModel.append_item(toolboxWidget, ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Layout:
                        aModel.append_item(toolboxLayout, ds::element_tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Workflow:
                        break;
                    }
        }

        aPresentationModel.set_item_model(aModel);
        aPresentationModel.set_column_read_only(0u);
    }
}