// element_model.tpp
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
#include "element_model.hpp"

#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx::DesignStudio
{
    template <typename Model>
    element_presentation_model<Model>::element_presentation_model(i_project_manager& aProjectManager)
    {
        iSink = aProjectManager.project_added([&](i_project& aProject)
        {
            iSink += aProject.element_added([&](i_element& aElement)
            {
                if (aElement.parent().is_root() && iDragDropItem)
                {
                    auto const& item = base_type::item_model().item(base_type::to_item_model_index(iDragDropItem->index()));
                    if (std::holds_alternative<ds::element_tool_t>(item))
                    {
                        auto widgetCaddy = make_ref<widget_caddy>(aProject, aElement, iDragDropItem->source().drag_drop_event_monitor().root().as_widget(), point{});
                        auto const idealSize = widgetCaddy->minimum_size();
                        widgetCaddy->resize(idealSize);
                        widgetCaddy->move((iDragDropItem->source().drag_drop_tracking_position() + iDragDropItem->source().drag_drop_event_monitor().origin() - widgetCaddy->extents() / 2.0).ceil());
                        iDragDropItem->source().set_drag_drop_widget(widgetCaddy);
                    }
                }
            });
        });
        iSink += base_type::DraggingItem([&](i_drag_drop_item const& aItem)
        {
            iDragDropItem = &aItem;
            auto const& item = base_type::item_model().item(base_type::to_item_model_index(aItem.index()));
            if (std::holds_alternative<ds::element_tool_t>(item))
            {
                auto const& tool = std::get<ds::element_tool_t>(item);
                auto& project = aProjectManager.active_project();
                iSelectedElement = project.create_element(project.root(), tool.second, generate_id(tool.second));
                iSelectedElement->set_mode(element_mode::Drag);
            }
        });
        iSink += base_type::ItemDropped([&](i_drag_drop_item const& aItem, i_drag_drop_target& aTarget)
        {
            ref_ptr<widget_caddy> widgetCaddy = aItem.source().drag_drop_widget();
            if (widgetCaddy)
            {
                auto widget = iSelectedElement->needs_caddy() ? ref_ptr<i_widget>{ widgetCaddy } : ref_ptr<i_widget>{ widgetCaddy->element().layout_item().as_widget() };
                auto const position = aTarget.as_widget().to_client_coordinates(widget->to_window_coordinates(point{}));
                aTarget.as_widget().add(widget);
                widget->move(position);
                iSelectedElement->set_mode(element_mode::None);
                if (iSelectedElement->group() == element_group::Workflow)
                    widget->bring_to_front();
            }
            iDragDropItem = nullptr;
            iSelectedElement = {};
        });
        iSink += base_type::DraggingItemCancelled([&](i_drag_drop_item const& aItem)
        {
            if (iSelectedElement)
            {
                auto& project = aProjectManager.active_project();
                project.remove_element(*iSelectedElement);
                iDragDropItem = nullptr;
                iSelectedElement = {};
            }
        });
    }

    template <typename Model>
    ng::optional_size element_presentation_model<Model>::cell_image_size(const ng::item_presentation_model_index& aIndex) const
    {
        auto const& tool = base_type::item_model().item(base_type::to_item_model_index(aIndex));
        if (std::holds_alternative<ds::element_group>(tool))
        {
            switch (std::get<ds::element_group>(tool))
            {
            case ds::element_group::Project:
            case ds::element_group::Code:
            case ds::element_group::UserInterface:
                return ng::size{ 24.0_dip, 24.0_dip };
            case ds::element_group::Node:
            case ds::element_group::Script:
            case ds::element_group::App:
            case ds::element_group::Menu:
            case ds::element_group::Action:
            case ds::element_group::Widget:
            case ds::element_group::Layout:
                return {};
            case ds::element_group::Workflow:
                return ng::size{ 32.0_dip, 32.0_dip };
            default:
                return {};
            }
        }
        else
        {
            return ng::size{ 24.0_dip, 24.0_dip };
        }
    }

    template <typename Model>
    ng::optional_texture element_presentation_model<Model>::cell_image(const ng::item_presentation_model_index& aIndex) const
    {
        auto const& tool = base_type::item_model().item(base_type::to_item_model_index(aIndex));
        if (std::holds_alternative<ds::element_group>(tool))
        {
            return {};
        }
        else
        {
            auto const& t = std::get<element_tool_t>(base_type::item_model().item(base_type::to_item_model_index(aIndex)));
            return t.first->element_icon(t.second);
        }
    }

    template <typename Model>
    ng::item_cell_flags element_presentation_model<Model>::cell_flags(ng::item_presentation_model_index const& aIndex) const
    {
        auto result = base_type::cell_flags(aIndex);
        auto const& tool = base_type::item_model().item(base_type::to_item_model_index(aIndex));
        if (std::holds_alternative<ds::element_tool_t>(tool))
            result |= ng::item_cell_flags::Draggable;
        return result;
    }

    template <typename Model>
    string element_presentation_model<Model>::generate_id(const string& aToolName)
    {
        // todo: use configured naming convention
        return to_symbol_name(aToolName + std::to_string(++iIdCounters[aToolName]), naming_convention::LowerCamelCase, named_entity::LocalVariable);
    }
}