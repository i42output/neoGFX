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

#pragma once

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/symbol.hpp>
#include "widget_caddy.hpp"
#include "toolbox_model.hpp"

#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx::DesignStudio
{
    toolbox_presentation_model::toolbox_presentation_model(i_project_manager& aProjectManager)
    {
        iSink = DraggingItem([&](i_drag_drop_item const& aItem)
        {
            auto const& item = item_model().item(to_item_model_index(aItem.index()));
            ref_ptr<i_widget> selectedWidget;
            if (std::holds_alternative<ds::tool_t>(item))
            {
                auto const& tool = std::get<ds::tool_t>(item);
                auto& project = aProjectManager.active_project();
                iSelectedElement = project.create_element(project.root(), tool.second, generate_id(tool.second));
                selectedWidget = iSelectedElement->create_widget();
            }
            if (selectedWidget)
            {
                auto widgetCaddy = make_ref<widget_caddy>(aItem.source().drag_drop_event_monitor().root().as_widget(), point{});
                widgetCaddy->add(selectedWidget);
                auto const idealSize = widgetCaddy->minimum_size();
                widgetCaddy->resize(idealSize);
                widgetCaddy->move(aItem.source().drag_drop_tracking_position() + aItem.source().drag_drop_event_monitor().origin() - widgetCaddy->extents() / 2.0);
                aItem.source().set_drag_drop_widget(widgetCaddy);
            }
        });
        iSink += ItemDropped([&](i_drag_drop_item const& aItem, i_drag_drop_target& aTarget)
        {
            ref_ptr<i_widget> widgetCaddy = aItem.source().drag_drop_widget();
            if (widgetCaddy)
            {
                auto windowPosition = widgetCaddy->to_window_coordinates(widgetCaddy->position());
                aTarget.as_widget().add(widgetCaddy);
                widgetCaddy->move(widgetCaddy->to_client_coordinates(windowPosition));
            }
            iSelectedElement = {};
        });
        iSink += DraggingItemCancelled([&](i_drag_drop_item const& aItem)
        {
            if (iSelectedElement)
            {
                auto& project = aProjectManager.active_project();
                project.remove_element(*iSelectedElement);
                iSelectedElement = {};
            }
        });

    }

    ng::optional_size toolbox_presentation_model::cell_image_size(const ng::item_presentation_model_index& aIndex) const
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

    ng::optional_texture toolbox_presentation_model::cell_image(const ng::item_presentation_model_index& aIndex) const
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

    ng::item_cell_flags toolbox_presentation_model::cell_flags(ng::item_presentation_model_index const& aIndex) const
    {
        auto result = base_type::cell_flags(aIndex);
        auto const& tool = item_model().item(to_item_model_index(aIndex));
        if (std::holds_alternative<ds::tool_t>(tool))
            result |= ng::item_cell_flags::Draggable;
        return result;
    }

    string toolbox_presentation_model::generate_id(const string& aToolName)
    {
        // todo: use configured naming convention
        return to_symbol_name(aToolName + std::to_string(++iIdCounters[aToolName]), naming_convention::LowerCamelCase, named_entity::LocalVariable);
    }

    void populate_toolbox_model(toolbox_model& aModel, toolbox_presentation_model& aPresentationModel)
    {
        auto toolboxProject = aModel.insert_item(aModel.send(), ds::element_group::Project, "Project");
        auto toolboxCode = aModel.insert_item(aModel.send(), ds::element_group::Code, "Code");
        auto toolboxUserInterface = aModel.insert_item(aModel.send(), ds::element_group::UserInterface, "User Interface");
        auto toolboxApp = aModel.append_item(toolboxProject, ds::element_group::App, "Application");
        auto toolboxMenu = aModel.append_item(toolboxUserInterface, ds::element_group::Menu, "Menu");
        auto toolboxAction = aModel.append_item(toolboxUserInterface, ds::element_group::Action, "Action");
        auto toolboxLayout = aModel.append_item(toolboxUserInterface, ds::element_group::Layout, "Layout");
        auto toolboxWidget = aModel.append_item(toolboxUserInterface, ds::element_group::Widget, "Widget");
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
                    default:
                    case ds::element_group::Unknown:
                        break;
                    case ds::element_group::Project:
                        aPresentationModel.projectTexture = elementLibrary->element_icon("project"_s);
                        break;
                    case ds::element_group::Code:
                        aPresentationModel.codeTexture = elementLibrary->element_icon("code"_s);
                        break;
                    case ds::element_group::UserInterface:
                        aPresentationModel.userInterfaceTexture = elementLibrary->element_icon("user_interface"_s);
                        break;
                    case ds::element_group::Script:
                        aModel.append_item(toolboxCode, ds::tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Node:
                        aModel.append_item(toolboxCode, ds::tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::App:
                        aModel.append_item(toolboxApp, ds::tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Menu:
                        aModel.append_item(toolboxMenu, ds::tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Action:
                        aModel.append_item(toolboxAction, ds::tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Widget:
                        aModel.append_item(toolboxWidget, ds::tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    case ds::element_group::Layout:
                        aModel.append_item(toolboxLayout, ds::tool_t{ &*elementLibrary, tool }, stringify_tool(tool));
                        break;
                    }
        }

        aPresentationModel.set_item_model(aModel);
        aPresentationModel.set_column_read_only(0u);
    }
}