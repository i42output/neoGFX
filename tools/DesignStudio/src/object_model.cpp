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
    object_presentation_model::object_presentation_model(i_project_manager& aProjectManager, item_selection_model& aSelectionModel) :
        iSelectionModel{ aSelectionModel }
    {
        aSelectionModel.set_presentation_model(*this);
        auto update_model = [&]()
        {
            if (aProjectManager.project_active())
            {
                item_model().clear();
                item_model().updating().trigger();
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
                    case element_group::Widget:
                    case element_group::Layout:
                    {
                        auto node = aElement.group() != element_group::Project ?
                            item_model().append_item(aPosition, &aElement, aElement.id().to_std_string()) :
                            item_model().insert_item(aPosition, &aElement, aElement.id().to_std_string());
                        item_model().insert_cell_data(node, 1u, aElement.type().to_std_string());
                        for (auto& child : aElement)
                            addNode(node, *child);
                    }
                    break;
                    case element_group::Action:
                        break;
                    }
                };
                addNode(item_model().send(), aProjectManager.active_project().root());
                item_model().updated().trigger();
            }
            else
                item_model().clear();
        };
        auto project_updated = [&, update_model](i_project& aProject)
        {
            update_model();
            iSink2 = aProject.element_added([&, update_model](i_element& aElement) 
            { 
                // todo: something more granular
                update_model(); 
                iSink2 += aElement.selection_changed([&]()
                {
                    auto const index = from_item_model_index(item_model().find_item(&aElement));
                    if (aElement.is_selected())
                        iSelectionModel.select(index, ng::item_selection_operation::ClearAndSelectRowAsCurrent);
                    else if (iSelectionModel.has_current_index() && iSelectionModel.current_index().row() == index.row())
                        iSelectionModel.select(index, ng::item_selection_operation::ClearAsCurrent);
                });
            }); 
            iSink2 += aProject.element_removed([update_model](i_element&) 
            { 
                // todo: something more granular
                update_model(); 
            }); 
        };

        iSink += aProjectManager.project_added(project_updated);
        iSink += aProjectManager.project_removed(project_updated);
        iSink += aProjectManager.project_activated(project_updated);
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
}