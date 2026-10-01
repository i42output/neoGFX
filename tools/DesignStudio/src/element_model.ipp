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
                        auto const idealSize = widgetCaddy->transformed_ideal_size();
                        widgetCaddy->resize(idealSize);
                        widgetCaddy->move((iDragDropItem->source().drag_drop_tracking_position() + iDragDropItem->source().drag_drop_event_monitor().origin() - widgetCaddy->extents() / 2.0).ceil());
                        iDragDropItem->source().set_drag_drop_widget(widgetCaddy);
                        // highlight the layout the dragged element would be dropped into
                        i_widget* draggedCaddy = &*widgetCaddy;
                        iDragSink = widgetCaddy->position_changed([this, &aProject, &aElement, draggedCaddy]()
                        {
                            i_element* container = nullptr;
                            auto const dropPosition = design_rect(*draggedCaddy).center();
                            bool const isRootWidget = aElement.has_layout_item() && aElement.layout_item().is_widget() &&
                                aElement.layout_item().as_widget().is_root();
                            if ((aElement.group() == element_group::Widget || aElement.group() == element_group::Layout) && !isRootWidget)
                                container = find_drop_container(aProject.root(), aElement, dropPosition);
                            if (container != nullptr)
                            {
                                try
                                {
                                    show_drop_highlight(*container, aElement.type(), iDropHighlight, dropPosition);
                                }
                                catch (...)
                                {
                                    hide_drop_highlight(iDropHighlight);
                                }
                            }
                            else
                                hide_drop_highlight(iDropHighlight);
                        });
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
                iSelectedElement->attributes().push_back(neolib::pair<string, string>{ string{ "id" }, string{ iSelectedElement->id() } });
                iSelectedElement->set_mode(element_mode::Drag);
                set_design_drag_active(true);
            }
        });
        iSink += base_type::ItemDropped([&](i_drag_drop_item const& aItem, i_drag_drop_target& aTarget)
        {
            struct end_design_drag { ~end_design_drag() { set_design_drag_active(false); } } endDesignDrag; // after the drop (so its insertion point is where the user saw it)
            iDragSink.clear();
            hide_drop_highlight(iDropHighlight);
            ref_ptr<widget_caddy> widgetCaddy = aItem.source().drag_drop_widget();
            if (widgetCaddy && iSelectedElement && 
                (iSelectedElement->group() == element_group::Widget || iSelectedElement->group() == element_group::Layout))
            {
                auto& project = aProjectManager.active_project();
                auto const dropPosition = design_rect(*widgetCaddy).center();
                bool const isRootWidget = iSelectedElement->has_layout_item() && iSelectedElement->layout_item().is_widget() &&
                    iSelectedElement->layout_item().as_widget().is_root();
                auto container = !isRootWidget ? find_drop_container(project.root(), *iSelectedElement, dropPosition) : nullptr;
                if (container != nullptr || iSelectedElement->group() == element_group::Layout)
                {
                    auto dropped = iSelectedElement;
                    iDragDropItem = nullptr;
                    iSelectedElement = {};
                    string const type{ dropped->type() };
                    string const id{ dropped->id() };
                    project.remove_element(*dropped);
                    dropped = {};
                    // layouts can only be dropped on a window/dialog (or other layout container); if no container then discard
                    if (container != nullptr)
                    {
                        auto& newElement = project.create_element(*container, type, id);
                        newElement.attributes().push_back(neolib::pair<string, string>{ string{ "id" }, id });
                        add_to_container(project, newElement, dropPosition);
                        newElement.select();
                        if (newElement.has_text())
                        {
                            // initial text is the allocated id: edit it in place
                            set_text_attribute(newElement, id.to_std_string());
                            newElement.apply_attributes(show_ids());
                            if (newElement.has_caddy())
                                newElement.caddy().begin_text_edit();
                            else if (newElement.has_parent() && newElement.parent().has_caddy())
                                newElement.parent().caddy().begin_text_edit(newElement); // e.g. a tab page
                        }
                    }
                    return;
                }
            }
            if (widgetCaddy && iSelectedElement)
            {
                auto widget = iSelectedElement->needs_caddy() ? ref_ptr<i_widget>{ widgetCaddy } : ref_ptr<i_widget>{ widgetCaddy->element().layout_item().as_widget() };
                auto const position = aTarget.as_widget().to_client_coordinates(widget->to_window_coordinates(point{}));
                aTarget.as_widget().add(widget);
                if (!iSelectedElement->needs_caddy() && iSelectedElement->layout_item().is_widget() && iSelectedElement->layout_item().as_widget().is_root() && 
                    iSelectedElement->layout_item().as_widget().root().is_nested())
                    service<i_surface_manager>().nest_for(aTarget.as_widget(), nest_type::MDI).add(iSelectedElement->layout_item().as_widget().root().native_window());
                widget->move(position);
                iSelectedElement->set_mode(element_mode::None);
                if (iSelectedElement->group() == element_group::Workflow)
                    widget->bring_to_front();
                if (iSelectedElement->has_text())
                {
                    // initial text is the allocated id: edit it in place
                    set_text_attribute(*iSelectedElement, iSelectedElement->id().to_std_string());
                    iSelectedElement->apply_attributes(show_ids());
                    if (iSelectedElement->has_caddy())
                        iSelectedElement->caddy().begin_text_edit();
                }
            }
            iDragDropItem = nullptr;
            iSelectedElement = {};
        });
        iSink += base_type::DraggingItemCancelled([&](i_drag_drop_item const& aItem)
        {
            set_design_drag_active(false);
            iDragSink.clear();
            hide_drop_highlight(iDropHighlight);
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
            case ds::element_group::Perspective:
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