// object_model.cpp
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
#include "object_model.hpp"

namespace neogfx::DesignStudio
{
    namespace
    {
        // (the elements Object Explorer shows: one of another group isn't shown, nor are its descendants)
        bool shown(i_element const& aElement)
        {
            switch (aElement.group())
            {
            case element_group::Project:
            case element_group::UserInterface:
            case element_group::App:
            case element_group::Menu:
            case element_group::Node:
            case element_group::Script:
            case element_group::Widget:
            case element_group::Layout:
            case element_group::Workflow:
                return true;
            default:
                return false;
            }
        }

        // (the model's iterators step through the whole tree, into a node's children, rather than through a node's siblings so 
        // siblings are stepped through with the tree's sibling iterator)
        object_model::sibling_iterator as_sibling(i_item_model::iterator const& aNode)
        {
            return aNode.get<object_model::sibling_iterator, object_model::iterator, object_model::sibling_iterator>();
        }

        // the node showing an element: the model's nodes are in the same order as the elements they show so it is found by 
        // following the element's path from the project's root rather than by searching the model (finding an item in the 
        // model is slow: each row's item is found by stepping through the model from its beginning)
        std::optional<i_item_model::iterator> node_of(object_model& aModel, i_element& aElement)
        {
            if (!shown(aElement))
                return {};
            if (!aElement.has_parent())
            {
                if (aModel.sbegin() == aModel.send())
                    return {};
                return aModel.sbegin();
            }
            auto const parentNode = node_of(aModel, aElement.parent());
            if (!parentNode)
                return {};
            auto node = as_sibling(aModel.sbegin(*parentNode));
            auto const end = as_sibling(aModel.send(*parentNode));
            for (auto const& sibling : aElement.parent())
            {
                if (&*sibling == &aElement)
                    break;
                if (shown(*sibling) && node != end)
                    ++node;
            }
            if (node == end || node->value != &aElement)
                return {}; // (it has no node: e.g. the user interface element: it is created without being added to the project)
            return object_model::base_iterator{ node };
        }

        // (an element's descendants' nodes appended to its node)
        void add_child_nodes(object_model& aModel, i_item_model::iterator aNode, i_element& aElement)
        {
            for (auto& child : aElement)
                if (shown(*child))
                {
                    auto const childNode = aModel.append_item(aNode, &*child, child->id());
                    aModel.insert_cell_data(childNode, 1u, child->type());
                    add_child_nodes(aModel, childNode, *child);
                }
        }

        // an element's node (with its descendants' nodes) inserted where it is among its siblings; if its parent is shown but 
        // has no node its parent's node is added instead (which adds this element's node too)
        void add_node(object_model& aModel, i_element& aElement)
        {
            if (!shown(aElement) || !aElement.has_parent())
                return;
            auto const parentNode = node_of(aModel, aElement.parent());
            if (!parentNode)
            {
                add_node(aModel, aElement.parent());
                return;
            }
            auto position = as_sibling(aModel.sbegin(*parentNode));
            auto const end = as_sibling(aModel.send(*parentNode));
            for (auto const& sibling : aElement.parent())
            {
                if (&*sibling == &aElement)
                    break;
                if (shown(*sibling) && position != end)
                    ++position;
            }
            if (position != end && position->value == &aElement)
                return; // (already shown: e.g. added with its parent)
            auto const node = position != end ?
                aModel.insert_item(object_model::base_iterator{ position }, &aElement, aElement.id()) :
                aModel.append_item(*parentNode, &aElement, aElement.id());
            aModel.insert_cell_data(node, 1u, aElement.type());
            add_child_nodes(aModel, node, aElement);
        }
    }

    object_presentation_model::object_presentation_model(i_project_manager& aProjectManager)
    {
        selection_model().set_presentation_model(*this);
        auto update_model = [&]()
        {
            if (aProjectManager.project_active())
            {
                scoped_item_update siu{ *this };
                item_model().clear();
                std::function<void(ng::i_item_model::iterator, i_element&)> addNode = [&](ng::i_item_model::iterator aPosition, i_element& aElement)
                {
                    switch (aElement.group())
                    {
                    case element_group::Unknown:
                        break;
                    case element_group::Project:
                    case element_group::UserInterface:
                    case element_group::App:
                    case element_group::Menu:
                    case element_group::Node:
                    case element_group::Script:
                    case element_group::Widget:
                    case element_group::Layout:
                    case element_group::Workflow:
                    {
                        auto node = aElement.group() != element_group::Project ?
                            item_model().append_item(aPosition, &aElement, aElement.id()) :
                            item_model().insert_item(aPosition, &aElement, aElement.id());
                        item_model().insert_cell_data(node, 1u, aElement.type());
                        for (auto& child : aElement)
                            addNode(node, *child);
                    }
                    break;
                    case element_group::Action:
                        break;
                    }
                };
                addNode(item_model().send(), aProjectManager.active_project().root());
            }
            else
                item_model().clear();
        };
        // keep Object Explorer in step with the design surface: an element being edited (e.g. clicked) becomes the current 
        // item (which scrolls it into view) and selected elements are selected rows
        auto subscribe = [this](i_element& aElement)
        {
            auto find = [this](i_element& aElement) -> std::optional<item_model_index>
            {
                try
                {
                    auto const modelIndex = item_model().find_item(&aElement);
                    expand_to(modelIndex);
                    return modelIndex;
                }
                catch (...)
                {
                    return {}; // element not shown in Object Explorer
                }
            };
            iSink2 += aElement.mode_changed([this, &aElement, find]()
            {
                if (aElement.mode() == element_mode::Edit)
                {
                    auto const modelIndex = find(aElement);
                    if (modelIndex)
                        selection_model().set_current_index(from_item_model_index(*modelIndex));
                }
            });
            iSink2 += aElement.selection_changed([this, &aElement, find]()
            {
                auto const modelIndex = find(aElement);
                if (!modelIndex)
                    return;
                auto const index = from_item_model_index(*modelIndex);
                if (aElement.is_selected())
                    selection_model().select(index, ng::item_selection_operation::SelectRow);
                else
                    selection_model().select(index, ng::item_selection_operation::DeselectRow);
            });
        };
        // an element added (e.g. dropped from Toolbox or loaded from an .nrc file) is inserted where it is among its siblings 
        // (rather than the model being rebuilt: that made loading an .nrc file quadratic (or worse) in its number of elements)
        auto add_element = [this](i_element& aElement)
        {
            add_node(item_model(), aElement);
        };
        auto project_updated = [&, update_model, add_element, subscribe](i_project& aProject)
        {
            update_model();
            iSink2 = aProject.element_added([&, add_element, subscribe](i_element& aElement) 
            { 
                add_element(aElement);
                subscribe(aElement);
            }); 
            iSink2 += aProject.element_removed([update_model](i_element&) 
            { 
                // todo: something more granular
                update_model(); 
            }); 
            iSink2 += aProject.element_moved([update_model](i_element&) 
            { 
                // todo: something more granular
                update_model(); 
            }); 
            // elements already in the project (e.g. loaded from an .nrc file) 
            aProject.root().visit([&](i_element& aElement) { subscribe(aElement); });
        };

        iSink += aProjectManager.project_added(project_updated);
        iSink += aProjectManager.project_removed(project_updated);
        iSink += aProjectManager.project_activated(project_updated);
    }

    item_selection_model& object_presentation_model::selection_model()
    {
        return iSelectionModel;
    }

    ng::optional_size object_presentation_model::cell_image_size(const ng::item_presentation_model_index& aIndex) const
    {
        if (aIndex.column() == 0)
            return ng::size{ 16.0_dip, 16.0_dip };
        else
            return {};
    }

    ng::optional_texture object_presentation_model::cell_image(const ng::item_presentation_model_index& aIndex) const
    {
        if (aIndex.column() == 0)
        {
            auto const& e = *item_model().item(to_item_model_index(aIndex));
            return e.library().element_icon(e.type());
        }
        else
            return {};
    }

    ng::item_cell_flags object_presentation_model::cell_flags(ng::item_presentation_model_index const& aIndex) const
    {
        auto result = basic_item_presentation_model::cell_flags(aIndex);
        auto const& e = *item_model().item(to_item_model_index(aIndex));
        // widgets and layouts can be dragged to move them
        bool const movable = (e.group() == element_group::Widget || e.group() == element_group::Layout) && e.has_caddy() &&
            !(e.has_layout_item() && e.layout_item().is_widget() && e.layout_item().as_widget().is_root());
        if (movable)
            result |= ng::item_cell_flags::Draggable;
        return result;
    }
}
