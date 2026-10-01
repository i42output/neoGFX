// main_window.cpp
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
#include <neogfx/tools/DesignStudio/context_menu.hpp>
#include <neogfx/tools/DesignStudio/i_node.hpp>
#include "main_window.hpp"
#include "widget_caddy.hpp"
#include <neogfx/gui/dialog/color_dialog.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>

namespace neogfx::DesignStudio
{
    class compact_tree_view : public ng::tree_view
    {
    public:
        using ng::tree_view::tree_view;
    public:
        size minimum_size(optional_size const& aAvailableSpace = {}) const override
        {
            return ng::tree_view::minimum_size(aAvailableSpace) + total_item_area(*this);
        }
    public:
        void model_changed() override
        {
            ng::tree_view::model_changed();
            update_layout(true, true);
        }
        void presentation_model_changed() override
        {
            ng::tree_view::presentation_model_changed();
            update_layout(true, true);
        }
        void tree_changed() override
        {
            ng::tree_view::tree_changed();
            update_layout(true, true);
        }
    };

    main_window_ex::main_window_ex(main_app& aApp, settings& aSettings, project_manager& aProjectManager) :
        main_window{ aApp },
        iProjectManager{ aProjectManager },
        autoscaleDocks{ aSettings.setting("environment.windows_and_tabs.autoscale_docks"_s) },
        workspaceSize{ aSettings.setting("environment.windows_and_tabs.workspace_size"_s) },
        workspacePosition{ aSettings.setting("environment.windows_and_tabs.workspace_position"_s) },
        leftDockWidth{ aSettings.setting("environment.windows_and_tabs.left_dock_width"_s) },
        rightDockWidth{ aSettings.setting("environment.windows_and_tabs.right_dock_width"_s) },
        leftDockWeight{ aSettings.setting("environment.windows_and_tabs.left_dock_weight"_s) },
        rightDockWeight{ aSettings.setting("environment.windows_and_tabs.right_dock_weight"_s) },
        workspaceFont{ aSettings.setting("environment.fonts_and_colors.workspace_font"_s) },
        subpixelRendering{ aSettings.setting("environment.fonts_and_colors.subpixel"_s) },
        toolbarIconSize{ aSettings.setting("environment.toolbars.icon_size"_s) },
        themeColor{ aSettings.setting("environment.fonts_and_colors.theme"_s) },
        workspaceGridType{ aSettings.setting("environment.workspace.grid_type"_s) },
        workspaceGridSize{ aSettings.setting("environment.workspace.grid_size"_s) },
        workspaceGridSubdivisions{ aSettings.setting("environment.workspace.grid_subdivisions"_s) },
        workspaceGridColor{ aSettings.setting("environment.workspace.grid_color"_s) },
        iLeftDock{ dock_layout(ng::dock_area::Left), ng::dock_area::Left, ng::size{ leftDockWidth.value<double>() }, ng::size{ leftDockWeight.value<double>() } },
        iRightDock{ dock_layout(ng::dock_area::Right), ng::dock_area::Right, ng::size{ rightDockWidth.value<double>() }, ng::size{ rightDockWeight.value<double>() } },
        iProject{ ng::make_dockable<compact_tree_view>("Project"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iToolbox{ ng::make_dockable<ng::tree_view>("Toolbox"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iWorkflow{ ng::make_dockable<ng::list_view>("Workflow"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iObjects{ ng::make_dockable<ng::table_view>("Object Explorer"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iProperties{ ng::make_dockable<ng::table_view>("Properties"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iWorkspaceLayout{ client_layout() },
        iWorkspace{ iWorkspaceLayout },
        iBackgroundTexture1{ ng::image{ ":/neogfx/DesignStudio/resources/neoGFX.png" } },
        iBackgroundTexture2{ ng::image{ ":/neogfx/DesignStudio/resources/logo_i42.png" } },
        iProjectPresentationModel{ aProjectManager },
        iToolboxPresentationModel{ aProjectManager },
        iWorkflowPresentationModel{ aProjectManager },
        iObjectPresentationModel{ aProjectManager }
    {
        // todo: decompose this ctor body into smaller member initialization functions...

        hide();

        aApp.actionShowStandardToolbar.checked([&]() { standardToolbar.show(); });
        aApp.actionShowStandardToolbar.unchecked([&]() { standardToolbar.hide(); });
        aApp.actionShowStatusBar.checked([&]() { statusBar.show(); });
        aApp.actionShowStatusBar.unchecked([&]() { statusBar.hide(); });

        auto& showLayoutIcons = aSettings.setting("environment.workspace.show_layout_icons"_s);
        aApp.actionShowLayoutIcons.set_checked(showLayoutIcons.value<bool>(true));
        aApp.actionShowLayoutIcons.checked([&showLayoutIcons]() { if (!showLayoutIcons.value<bool>(true)) showLayoutIcons.set_value(true); });
        aApp.actionShowLayoutIcons.unchecked([&showLayoutIcons]() { if (showLayoutIcons.value<bool>(true)) showLayoutIcons.set_value(false); });
        auto showLayoutIconsChanged = [&aApp, &showLayoutIcons]()
        {
            aApp.actionShowLayoutIcons.set_checked(showLayoutIcons.value<bool>(true));
        };
        showLayoutIcons.changing(showLayoutIconsChanged);
        showLayoutIcons.changed(showLayoutIconsChanged);

        if (!workspaceSize.is_default())
            set_extents(workspaceSize.value<ng::size>());
        if (!workspacePosition.is_default())
            move(workspacePosition.value<ng::point>());
        else
            center(false);

        // todo: tidier way of doing this...
        dock_layout(ng::layout_position::Center).set_weight(ng::size{ 1.0 } - iLeftDock.parent_layout().weight() - iRightDock.parent_layout().weight());
        iLeftDock.hide();
        iRightDock.hide();

        auto autoscaleDocksChanged = [&]()
        {
            dock_layout(ng::layout_position::Center).parent_layout().fix_weightings();
            dock_layout(ng::layout_position::Center).parent_layout().set_autoscale(autoscaleDocks.value<bool>(true) ? ng::autoscale::Active : ng::autoscale::Default);
        };
        autoscaleDocks.changing(autoscaleDocksChanged);
        autoscaleDocks.changed(autoscaleDocksChanged);
        dock_layout(ng::layout_position::Center).parent_layout().set_autoscale(autoscaleDocks.value<bool>(true) ? ng::autoscale::Active : ng::autoscale::Default);

        ng::get_property(mainWindow, "Size").property_changed([&](const ng::property_variant& aValue)
        {
            workspaceSize.set_value(std::get<ng::size>(aValue));
        });
        ng::get_property(mainWindow, "Position").property_changed([&](const ng::property_variant& aValue)
        {
            workspacePosition.set_value(std::get<ng::point>(aValue));
        });
        ng::get_property(iLeftDock.parent_layout(), "Size").property_changed([&](const ng::property_variant& aValue)
        {
            leftDockWidth.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(iRightDock.parent_layout(), "Size").property_changed([&](const ng::property_variant& aValue)
        {
            rightDockWidth.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(iLeftDock.parent_layout(), "Weight").property_changed([&](const ng::property_variant& aValue)
        {
            if (aValue != ng::none)
                leftDockWeight.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(iRightDock.parent_layout(), "Weight").property_changed([&](const ng::property_variant& aValue)
        {
            if (aValue != ng::none)
                rightDockWeight.set_value(std::get<ng::size>(aValue).cx);
        });

        iProject.set_weight(ng::size{ 0.0 });
        iToolbox.set_weight(ng::size{ 3.0 });
        iWorkflow.set_weight(ng::size{ 1.0 });

        iProject.dock(iLeftDock);
        iToolbox.dock(iLeftDock);
        iWorkflow.dock(iLeftDock);
        iObjects.dock(iRightDock);
        iProperties.dock(iRightDock);

        iToolbox.docked_widget<ng::tree_view>().enable_drag_drop_source();
        iWorkflow.docked_widget<ng::list_view>().enable_drag_drop_source();

        ng::i_layout& mainLayout = client_layout();
        mainLayout.set_padding(ng::padding{});
        mainLayout.set_spacing(ng::size{});

        auto fontChanged = [&]()
        {
            ng::service<ng::i_app>().current_style().set_font_info(workspaceFont.value<ng::font_info>(true));
        };
        workspaceFont.changing(fontChanged);
        workspaceFont.changed(fontChanged);
        fontChanged();

        auto subpixelRenderingChanged = [&]()
        {
            if (subpixelRendering.value<bool>(true))
                ng::service<ng::i_rendering_engine>().subpixel_rendering_on();
            else
                ng::service<ng::i_rendering_engine>().subpixel_rendering_off();
        };
        subpixelRendering.changing(subpixelRenderingChanged);
        subpixelRendering.changed(subpixelRenderingChanged);
        subpixelRenderingChanged();

        auto toolbarIconSizeChanged = [&]()
        {
            switch (toolbarIconSize.value<toolbar_icon_size>(true))
            {
            case toolbar_icon_size::Size16x16:
                standardToolbar.set_button_image_extents(ng::size{ 16.0_dip, 16.0_dip });
                break;
            case toolbar_icon_size::Size24x24:
                standardToolbar.set_button_image_extents(ng::size{ 24.0_dip, 24.0_dip });
                break;
            case toolbar_icon_size::Size32x32:
                standardToolbar.set_button_image_extents(ng::size{ 32.0_dip, 32.0_dip });
                break;
            case toolbar_icon_size::Size48x48:
                standardToolbar.set_button_image_extents(ng::size{ 48.0_dip, 48.0_dip });
                break;
            case toolbar_icon_size::Size64x64:
                standardToolbar.set_button_image_extents(ng::size{ 64.0_dip, 64.0_dip });
                break;
            }
        };
        toolbarIconSize.changing(toolbarIconSizeChanged);
        toolbarIconSize.changed(toolbarIconSizeChanged);
        toolbarIconSizeChanged();

        auto themeColorChanged = [&]()
        {
            ng::service<ng::i_app>().current_style().set_palette_color(ng::color_role::Theme, themeColor.value<ng::color>(true));
            iWorkspace.view_stack().set_background_color(ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Base));
            workspaceGridColor.set_default_value(ng::gradient{ ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Background).with_alpha(0.25) });
        };
        themeColor.changing(themeColorChanged);
        themeColor.changed(themeColorChanged);
        themeColorChanged();

        auto workspaceGridChanged = [&]()
        {
            iWorkspace.view_stack().update();
        };
        workspaceGridType.changing(workspaceGridChanged);
        workspaceGridType.changed(workspaceGridChanged);
        workspaceGridColor.changing(workspaceGridChanged);
        workspaceGridColor.changed(workspaceGridChanged);
        workspaceGridSize.changing(workspaceGridChanged);
        workspaceGridSize.changed(workspaceGridChanged);
        workspaceGridSubdivisions.changing(workspaceGridChanged);
        workspaceGridSubdivisions.changed(workspaceGridChanged);

        iWorkspace.view_stack().enable_drag_drop_target();
        iWorkspace.view_stack().object_acceptable([&](const ng::i_drag_drop_object& aObject, ng::optional_point const& aDropPosition, ng::drop_operation& aOperation)
        {
            aOperation = preview_mode() ? ng::drop_operation::None : ng::drop_operation::Move;
        });
        iWorkspace.view_stack().set_focus_policy(ng::focus_policy::ClickFocus);

        populate_project_model(iProjectModel, iProjectPresentationModel);
        auto& projectTree = iProject.docked_widget<compact_tree_view>();
        projectTree.set_presentation_model(iProjectPresentationModel);
        projectTree.selection_model().set_mode(ng::item_selection_mode::SingleSelection);
        projectTree.set_focus_policy(ng::focus_policy::TabFocus);

        populate_toolbox_model(iToolboxModel, iToolboxPresentationModel);
        auto& toolboxTree = iToolbox.docked_widget<ng::tree_view>();
        toolboxTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        toolboxTree.set_presentation_model(iToolboxPresentationModel);
        toolboxTree.selection_model().set_mode(ng::item_selection_mode::SingleSelection);
        toolboxTree.set_focus_policy(ng::focus_policy::TabFocus);

        populate_workflow_model(iWorkflowModel, iWorkflowPresentationModel);
        iWorkflowPresentationModel.set_item_model(iWorkflowModel);
        iWorkflowPresentationModel.set_column_read_only(0u);
        auto& workflowTree = iWorkflow.docked_widget<ng::list_view>();
        workflowTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        workflowTree.set_presentation_model(iWorkflowPresentationModel);
        workflowTree.selection_model().set_mode(ng::item_selection_mode::NoSelection);
        workflowTree.set_focus_policy(ng::focus_policy::TabFocus);

        iObjectModel.set_column_name(0u, "Object"_t);
        iObjectModel.set_column_name(1u, "Type"_t);

        iObjectPresentationModel.set_item_model(iObjectModel);
        iObjectPresentationModel.set_column_read_only(1u);
        iObjectPresentationModel.set_alternating_row_color(true);
        auto& objectTree = iObjects.docked_widget<ng::table_view>();
        objectTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        objectTree.set_selection_model(iObjectPresentationModel.selection_model());
        iObjectPresentationModel.selection_model().set_mode(ng::item_selection_mode::ExtendedSelection);
        objectTree.set_presentation_model(iObjectPresentationModel);
        objectTree.column_header().set_expand_last_column(true);
        iObjectPresentationModel.selection_model().current_index_changed([&](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
        {
            if (aCurrentIndex)
            {
                auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*aCurrentIndex));
                element.set_mode(element_mode::Edit);
                iPropertyElement = weak_ref_ptr<i_element>{ element };
            }
            else 
            {
                if (aPreviousIndex)
                {
                    auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*aPreviousIndex));
                    element.set_mode(element_mode::None);
                }
                iPropertyElement = weak_ref_ptr<i_element>{};
            }
            iPropertiesNeedUpdate = true;
        });
        iObjectPresentationModel.selection_model().selection_changed([&](const ng::item_selection& aCurrentSelection, const ng::item_selection& aPreviousSelection)
        {
            static bool inHere;
            if (inHere)
                return;
            neolib::scoped_flag sf{ inHere };
            for (auto row = row_begin(aCurrentSelection); row != row_end(aCurrentSelection); ++row)
            {
                auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*row));
                element.select(true, false);
            }
            for (auto row = row_begin(aPreviousSelection); row != row_end(aPreviousSelection); ++row)
            {
                if (!contains(aCurrentSelection, *row))
                {
                    auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*row));
                    element.select(false, false);
                }
            }
        });

        iPropertyModel.set_column_name(0u, "Property"_t);
        iPropertyModel.set_column_name(1u, "Value"_t);
        iPropertyPresentationModel.set_item_model(iPropertyModel);
        iPropertyPresentationModel.set_alternating_row_color(true);
        auto& propertyTable = iProperties.docked_widget<ng::table_view>();
        propertyTable.set_minimum_size(ng::size{ 128_dip, 128_dip });
        propertyTable.set_presentation_model(iPropertyPresentationModel);
        propertyTable.column_header().set_expand_last_column(true);
        iSink += iPropertyModel.item_changed([&](item_model_index const& aIndex)
        {
            if (iUpdatingProperties || !iPropertyElement.valid() || !iProjectManager.project_active())
                return;
            auto& element = *iPropertyElement;
            auto cell_text = [&](item_model_index const& aCellIndex)
            {
                auto const& cellData = iPropertyModel.cell_data(aCellIndex);
                return std::holds_alternative<string>(cellData) ? std::get<string>(cellData) : string{};
            };
            auto const text = cell_text(aIndex);
            auto const& item = iPropertyModel.item(aIndex);
            if (std::holds_alternative<std::monostate>(item))
                return; // class node
            if (std::holds_alternative<i_property*>(item))
            {
                // object property
                if (aIndex.column() != 1u)
                    return;
                auto& property = *std::get<i_property*>(item);
                auto const value = property_value_from_string(property, text.to_std_string());
                if (value)
                    property.set_from_variant(*value);
                iPropertiesNeedUpdate = true; // show the resulting value
                return;
            }
            auto const attributeIndex = std::get<std::uint32_t>(item);
            if (attributeIndex == property_presentation_model::new_property_row)
            {
                // new property: name entered in the trailing row
                if (aIndex.column() != 0u || text.empty())
                    return;
                auto value = cell_text(aIndex.with_column(1u));
                if (value.empty())
                    value = "\"\"";
                element.attributes().push_back(neolib::pair<string, string>{ text, value });
                iPropertiesNeedUpdate = true;
            }
            else if (aIndex.column() == 1u && attributeIndex < element.attributes().size())
                std::next(element.attributes().begin(), attributeIndex)->second() = text; // an empty value removes the property on save
            else
                return;
            element.apply_attributes(show_ids());
            iProjectManager.active_project().set_dirty();
        });
        // the property being edited in place (if any)
        auto edited_property = [this, &propertyTable]() -> i_property*
        {
            if (!propertyTable.editing() || propertyTable.editing()->column() != 1u)
                return nullptr;
            auto const& item = iPropertyModel.item(iPropertyPresentationModel.to_item_model_index(*propertyTable.editing()));
            return std::holds_alternative<i_property*>(item) ? std::get<i_property*>(item) : nullptr;
        };
        // "..." button: opens the color or font dialog for the property being edited; the result replaces the editor text and is committed
        auto open_property_dialog = [this, &propertyTable, edited_property]()
        {
            auto property = edited_property();
            if (property == nullptr || iPropertyDialogOpen)
                return;
            neolib::scoped_flag sf{ iPropertyDialogOpen }; // the button mustn't be destroyed while its click handler is running
            std::string const text = propertyTable.editor_has_text_edit() ? propertyTable.editor_text_edit().text().to_std_string() : std::string{};
            std::optional<std::string> newText;
            switch (property_dialog_for(*property))
            {
            case property_dialog::Color:
                {
                    auto const current = property_parse(text, static_cast<ng::color const*>(nullptr));
                    ng::color_dialog dialog{ *this, current && text != "(none)" && !text.empty() ? *current : ng::color::Black };
                    if (dialog.exec() == ng::dialog_result::Accepted)
                        newText = property_text(dialog.selected_color());
                }
                break;
            case property_dialog::Font:
                {
                    auto const current = property_parse(text, static_cast<ng::font const*>(nullptr));
                    ng::font_dialog dialog{ *this, current ? *current : ng::font{} };
                    if (dialog.exec() == ng::dialog_result::Accepted)
                        newText = property_text(dialog.selected_font());
                }
                break;
            default:
                break;
            }
            if (!newText)
                return;
            if (edited_property() == property && propertyTable.editor_has_text_edit())
            {
                propertyTable.editor_text_edit().set_text(*newText);
                propertyTable.end_edit(true);
            }
            else if (auto const value = property_value_from_string(*property, *newText))
            {
                // the in-place edit ended while the dialog was open
                property->set_from_variant(*value);
                iPropertiesNeedUpdate = true;
            }
        };
        iPropertiesUpdater.emplace(*this, [this, &propertyTable, edited_property, open_property_dialog](ng::widget_timer& aTimer)
        {
            aTimer.again();
            if (iPropertiesNeedUpdate || (!iPropertyElement.valid() && iPropertyModel.rows() != 0u))
            {
                iPropertiesNeedUpdate = false;
                update_properties();
            }
            // show the "..." button at the right of the in-place editor of a color or font property
            if (iPropertyDialogOpen)
                return;
            auto property = edited_property();
            if (property != nullptr && property_dialog_for(*property) != property_dialog::None)
            {
                if (!iPropertyDialogButton)
                {
                    iPropertyDialogButton = std::make_unique<ng::push_button>(propertyTable, ng::string{ "..." });
                    iPropertyDialogButton->set_focus_policy(ng::focus_policy::NoFocus); // keep focus (and the edit) in the editor
                    iPropertyDialogButton->Clicked(open_property_dialog);
                }
                auto const& editor = propertyTable.editor();
                auto const buttonSize = editor.extents().cy;
                iPropertyDialogButton->move(ng::point{ editor.position().x + editor.extents().cx - buttonSize, editor.position().y });
                iPropertyDialogButton->resize(ng::size{ buttonSize, buttonSize });
                iPropertyDialogButton->bring_to_front();
            }
            else
                iPropertyDialogButton = nullptr;
        }, std::chrono::milliseconds{ 20 });

        // dragging within Object Explorer moves elements: onto a container (layout, window, group box, tab page) appends to it, 
        // onto any other element inserts before it, onto the project/user interface makes a widget top level
        objectTree.enable_drag_drop_source();
        objectTree.enable_drag_drop_target();
        struct object_drop
        {
            i_element* element = nullptr;
            i_element* container = nullptr;
            i_element const* before = nullptr;
            bool toCanvas = false;
        };
        auto resolve_object_drop = [this, &objectTree](i_drag_drop_object const& aObject, optional_point const& aDropPosition) -> object_drop
        {
            object_drop result;
            if (preview_mode() || !iProjectManager.project_active() || !aDropPosition || aObject.ddo_type() != i_drag_drop_item::otid())
                return result;
            auto const& item = static_cast<i_drag_drop_item const&>(aObject);
            if (&item.presentation_model() != static_cast<i_item_presentation_model const*>(&iObjectPresentationModel))
                return result;
            auto& dragged = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(item.index()));
            if (!can_be_moved(dragged))
                return result;
            auto const targetIndex = objectTree.item_at(*aDropPosition);
            if (!targetIndex)
                return result;
            auto& target = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(*targetIndex));
            if (&target == &dragged)
                return result;
            for (i_element const* e = &target; e->has_parent(); e = &e->parent())
                if (&e->parent() == &dragged)
                    return result; // can't move into itself
            if (target.group() == element_group::Project || target.group() == element_group::UserInterface)
            {
                if (dragged.group() == element_group::Widget && dragged.is_nested())
                {
                    result.element = &dragged;
                    result.container = &target;
                    result.toCanvas = true;
                }
                return result;
            }
            if (target.has_child_layout())
            {
                result.element = &dragged;
                result.container = &target;
            }
            else if (target.has_parent() && target.parent().has_child_layout())
            {
                result.element = &dragged;
                result.container = &target.parent();
                result.before = &target;
            }
            return result;
        };
        objectTree.object_acceptable([resolve_object_drop](i_drag_drop_object const& aObject, optional_point const& aDropPosition, drop_operation& aOperation)
        {
            if (resolve_object_drop(aObject, aDropPosition).element != nullptr)
                aOperation = drop_operation::Move;
        });
        objectTree.object_dropped([this, resolve_object_drop](i_drag_drop_object const& aObject, optional_point const& aDropPosition)
        {
            auto const drop = resolve_object_drop(aObject, aDropPosition);
            if (drop.element == nullptr)
                return;
            auto& project = iProjectManager.active_project();
            try
            {
                if (drop.toCanvas)
                {
                    auto& workspace = iWorkspace.view_stack();
                    move_element_to_canvas(project, *drop.element, *drop.container, workspace, design_rect(workspace).top_left() + point{ 128.0_dip, 128.0_dip });
                }
                else
                    move_element_to_container(project, *drop.element, *drop.container, drop.before);
            }
            catch (...)
            {
                // not droppable there
            }
        });

        objectTree.cell_context_menu([&](item_presentation_model_index const& aIndex)
        {
            auto& element = *iObjectModel.item(iObjectPresentationModel.to_item_model_index(aIndex));
            display_element_context_menu(objectTree, element);
        });

        objectTree.Focus([&](neogfx::focus_event aEvent, focus_reason aReason)
        {
            if (aEvent == neogfx::focus_event::FocusGained)
            {
                service<i_clipboard>().activate(*this);
            }
            else if (aEvent == neogfx::focus_event::FocusLost &&
                service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            {
                service<i_clipboard>().deactivate(*this);
            }
        });

        thread_local optional_point tMouseSelectorAnchor;
        thread_local optional_point tMouseSelectorMousePos;

        iWorkspace.view_stack().Painting([&](ng::i_graphics_context& aGc) 
        { 
            paint_workspace(aGc); 
        });

        iWorkspace.view_stack().Painted([&](ng::i_graphics_context& aGc)
        {
            if (tMouseSelectorAnchor)
            {
                aGc.draw_rect(rect{ tMouseSelectorAnchor->min(*tMouseSelectorMousePos), tMouseSelectorAnchor->max(*tMouseSelectorMousePos) },
                    service<i_app>().current_style().palette().color(color_role::Selection), service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(0.25));
            }
        });

        iWorkspace.view_stack().Focus([&](neogfx::focus_event aEvent, focus_reason aReason)
        {
            if (aEvent == neogfx::focus_event::FocusGained)
            {
                service<i_clipboard>().activate(*this);
            }
            else if (aEvent == neogfx::focus_event::FocusLost &&
                service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            {
                service<i_clipboard>().deactivate(*this);
            }
        });

        iWorkspace.view_stack().set_focus();

        iWorkspace.view_stack().Mouse([&](ng::mouse_event const& aEvent)
        {
            if (!iProjectManager.project_active() || preview_mode())
                return;
            auto const eventPos = aEvent.position() - iWorkspace.view_stack().origin();
            switch (aEvent.type())
            {
            case mouse_event_type::ButtonClicked:
                if (aEvent.is_left_button())
                {
                    iProjectManager.active_project().root().select(false, true);
                    iProjectManager.active_project().root().visit([&](i_element& aElement) { aElement.set_mode(element_mode::None); });
                    tMouseSelectorAnchor = eventPos;
                    tMouseSelectorMousePos = eventPos;
                    iWorkspace.view_stack().update();
                }
                break;
            case mouse_event_type::Moved:
                if (tMouseSelectorAnchor)
                {
                    tMouseSelectorMousePos = eventPos;
                    iProjectManager.active_project().root().visit([&](i_element& aElement)
                    {
                        if (aElement.has_layout_item() && (aElement.layout_item().is_widget() || aElement.layout_item().has_parent_widget()))
                        {
                            auto& elementWidget = aElement.layout_item().is_widget() ? aElement.layout_item().as_widget() : aElement.layout_item().parent_widget();
                            if (rect{ tMouseSelectorAnchor->min(*tMouseSelectorMousePos), tMouseSelectorAnchor->max(*tMouseSelectorMousePos) }.contains(
                                iWorkspace.view_stack().to_client_coordinates(elementWidget.to_window_coordinates(elementWidget.client_rect())).center()))
                                aElement.select(true, false);
                            else
                                aElement.select(false, false);
                        }
                    });
                    iWorkspace.view_stack().update();
                }
                break;
            case mouse_event_type::ButtonReleased:
                if (aEvent.is_left_button() && tMouseSelectorAnchor)
                {
                    tMouseSelectorAnchor = std::nullopt;
                    tMouseSelectorMousePos = std::nullopt;
                    iWorkspace.view_stack().update();
                }
                else if (aEvent.is_right_button())
                {
                    context_menu menu{ *this, root().mouse_position() + root().window_position() };
                    auto& actionCut = service<i_app>().action_cut();
                    auto& actionCopy = service<i_app>().action_copy();
                    auto& actionPaste = service<i_app>().action_paste();
                    auto& actionDelete = service<i_app>().action_delete();
                    auto& actionSelectAll = service<i_app>().action_select_all();
                    menu.menu().add_action(actionCut);
                    menu.menu().add_action(actionCopy);
                    menu.menu().add_action(actionPaste);
                    menu.menu().add_action(actionDelete);
                    menu.menu().add_separator();
                    menu.menu().add_action(actionSelectAll);
                    menu.exec();
                }
                break;
            }
        });

        iWorkspace.view_stack().QueryMouseCursor([&](ng::mouse_cursor& cursor)
        {
            if (cursor.is_system_cursor() && cursor.system_cursor() == ng::mouse_system_cursor::Arrow)
            {
                if (!toolboxTree.selection_model().selection().empty())
                    cursor = ng::mouse_system_cursor::Crosshair;
            }
        });

        toolboxTree.presentation_model().dragging_item([&](i_drag_drop_item const&)
        {
            toolboxTree.selection_model().clear_selection();
        });

        // preview (off by default): widgets behave as in a running application; no editing
        aApp.actionPreview.set_checked(preview_mode());
        aApp.actionPreview.checked([&]() { set_preview_mode(true); });
        aApp.actionPreview.unchecked([&]() { set_preview_mode(false); });
        // display ids (off by default): the editor shows element ids rather than their text (never in preview)
        aApp.actionDisplayIds.set_checked(display_ids());
        aApp.actionDisplayIds.checked([&]() { set_display_ids(true); });
        aApp.actionDisplayIds.unchecked([&]() { set_display_ids(false); });
        iSink += preview_mode_changed()([&]()
        {
            toolboxTree.enable_drag_drop_source(!preview_mode());
            objectTree.enable_drag_drop_source(!preview_mode());
            iWorkspace.view_stack().update();
        });

        iWorkspace.view_stack().Keyboard([&](const ng::keyboard_event& aEvent)
        {
            if (aEvent.type() == ng::keyboard_event_type::KeyPressed && aEvent.scan_code() == ScanCode_ESCAPE && iProjectManager.project_active())
                iProjectManager.active_project().root().select(false, true);
        });

        auto update_ui = [&]()
        {
            aApp.actionFileClose.enable(aProjectManager.project_active());
            aApp.actionFileSave.enable(aProjectManager.project_active() && aProjectManager.active_project().dirty());
            iLeftDock.show(aProjectManager.project_active());
            iRightDock.show(aProjectManager.project_active());
        };

        update_ui();

        iSink += aProjectManager.ProjectAdded([&, update_ui](i_project& aProject) 
        { 
            iSink += aProject.modified([update_ui]() { update_ui(); });
            update_ui(); 
        });
        iSink += aProjectManager.ProjectRemoved([update_ui](i_project&) { update_ui(); });
        iSink += aProjectManager.ProjectActivated([update_ui](i_project&) { update_ui(); });

        iSink += aApp.actionFileClose.triggered([&]() 
        { 
            if (aProjectManager.project_active())
            {
                remove_caddies(aProjectManager.active_project());
                aProjectManager.close_project(aProjectManager.active_project());
            }
        });

        iSink += aApp.actionFileSave.triggered([&, update_ui]()
        {
            if (!aProjectManager.project_active())
                return;
            auto& project = aProjectManager.active_project();
            std::string path;
            if (project.has_path())
                path = project.path().to_std_string();
            else
            {
                auto file = ng::save_file_dialog(mainWindow, ng::file_dialog_spec{ "Save Project", project.name().to_std_string() + ".nrc", { "*.nrc" }, "Project Files" });
                if (!file)
                    return;
                path = *file;
                if (std::filesystem::path{ path }.extension().empty())
                    path += ".nrc";
            }
            try
            {
                project.save(ng::string{ path });
            }
            catch (std::exception const& e)
            {
                ng::service<ng::i_surface_manager>().display_error_message("Save Project"_t, ng::string{ e.what() });
            }
            update_ui();
        });

        aApp.action_file_open().triggered([&]()
        {
            auto files = ng::open_file_dialog(mainWindow, ng::file_dialog_spec{ "Open Project", {}, { "*.nrc" }, "Project Files" });
            if (files)
            {
                for (auto const& file : files.value())
                {
                    std::filesystem::path const filePath{ file };
                    if (filePath.extension() == ".nrc")
                    {
                        auto& project = aProjectManager.open_project(file);
                        create_caddies(project, iWorkspace.view_stack());
                    }
                    else if (aProjectManager.project_active())
                        aProjectManager.active_project().create_element(aProjectManager.active_project().root(), "file"_s, ng::string{ file });
                    else
                    {
                        ng::sink sink = aProjectManager.project_added([&](i_project& aProject)
                        {
                            aProject.create_element(aProject.root(), "file"_s, ng::string{ file });
                        });
                        aApp.action_file_new().triggered()();
                    }
                }
            }
        });

        aApp.action_file_new().triggered([&]()
        {
            new_project_dialog_ex dialog{ mainWindow };
            if (dialog.exec() == ng::dialog_result::Accepted)
            {
                auto& project = aProjectManager.create_project(dialog.projectName.text(), dialog.projectNamespace.text());
                project.set_dirty();
                project.root();
            }
        });

        aApp.actionSettings.triggered([&]()
        {
            ng::settings_dialog dialog{ mainWindow, aSettings };
            dialog.exec();
        });

        activate();
    }

    void main_window_ex::close()
    {
        if (iProjectManager.project_active())
        {
            remove_caddies(iProjectManager.active_project());
            iProjectManager.close_project(iProjectManager.active_project());
        }
        main_window::close();
    }

    void main_window_ex::add_action(uuid const& aMenuId, i_action& aAction)
    {
        if (aMenuId == id::MenuBar)
        {
            menuBar.insert_action_at(menuBar.find(menuTools), aAction);
        }
        else if (aMenuId == id::ToolsMenu)
        {
            menuTools.add_action(aAction);
        }
        else
        {
            menuBar.insert_action_at(menuBar.find_sub_menu(aMenuId), aAction);
        }
    }

    void main_window_ex::add_sub_menu(uuid const& aMenuId, i_menu& aSubMenu)
    {
        if (aMenuId == id::MenuBar)
        {
            menuBar.insert_sub_menu_at(menuBar.find(menuTools), aSubMenu);
        }
        else if (aMenuId == id::ToolsMenu)
        {
            menuTools.add_sub_menu(aSubMenu);
        }
        else
        {
            menuBar.insert_sub_menu_at(menuBar.find_sub_menu(aMenuId), aSubMenu);
        }
    }

    bool main_window_ex::can_undo() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_redo() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_cut() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_copy() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_paste() const
    {
        // todo
        return false;
    }

    bool main_window_ex::can_delete_selected() const
    {
        if (!iProjectManager.project_active() || preview_mode())
            return false;
        bool someSelected = false;
        iProjectManager.active_project().root().visit([&](i_element& aElement)
        {
            if (aElement.is_selected())
                someSelected = true;
        });
        return someSelected;
    }

    bool main_window_ex::can_select_all() const
    {
        if (!iProjectManager.project_active() || preview_mode())
            return false;
        return !iProjectManager.active_project().root().children().empty();
    }

    void main_window_ex::undo(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::redo(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::cut(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::copy(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::paste(i_clipboard& aClipboard)
    {
        // todo
    }

    void main_window_ex::delete_selected()
    {
        if (!iProjectManager.project_active())
            return;
        thread_local std::vector<weak_ref_ptr<i_element>> tToDelete;
        iProjectManager.active_project().root().visit([&](i_element& aElement)
        {
            if (aElement.is_selected())
                tToDelete.push_back(aElement);
        });
        for (auto& e : tToDelete)
        {
            if (e.valid())
                iProjectManager.active_project().remove_element(*e);
        }
        tToDelete.clear();
    }

    void main_window_ex::select_all()
    {
        if (!iProjectManager.project_active())
            return;
        iProjectManager.active_project().root().visit([&](i_element& aElement)
        {
            if (aElement.has_layout_item())
                aElement.select(true, false);
        });
    }

    void main_window_ex::update_properties()
    {
        neolib::scoped_flag sf{ iUpdatingProperties };
        iPropertyModel.clear();
        if (!iPropertyElement.valid())
            return;
        auto& element = *iPropertyElement;
        // .nrc attributes (saved to the project file)
        auto attributesNode = iPropertyModel.insert_item(iPropertyModel.send(), property_model_item{}, string{ element.type().to_std_string() + " (.nrc)" });
        std::uint32_t attributeIndex = 0u;
        for (auto const& attribute : element.attributes())
        {
            // skip metadata ('#' prefix) and removed (empty) properties
            if (!attribute.first().empty() && attribute.first().to_std_string_view()[0] != '#' && !attribute.second().empty())
            {
                auto row = iPropertyModel.append_item(attributesNode, property_model_item{ attributeIndex }, string{ attribute.first() });
                iPropertyModel.insert_cell_data(row, 1u, string{ attribute.second() });
            }
            ++attributeIndex;
        }
        // trailing row for adding a new attribute (name then value)
        iPropertyModel.append_item(attributesNode, property_model_item{ property_presentation_model::new_property_row }, string{});
        // object properties grouped by class: derived classes first, base classes last
        if (!element.has_layout_item())
            return;
        auto& owner = element.layout_item();
        std::map<std::string, std::vector<i_property*>> classes;
        for (auto const& entry : std::as_const(owner.properties()).property_map())
            classes[property_class_name(*entry.second())].push_back(entry.second());
        auto unqualified = [](std::string const& aName, std::string const& aSeparator)
        {
            auto const pos = aName.rfind(aSeparator);
            return pos == std::string::npos ? aName : aName.substr(pos + aSeparator.size());
        };
        std::vector<std::string> hierarchy; // class_names(): most derived first, e.g. "push_button:button:widget:layout_item:..."
        {
            std::istringstream names{ class_names(owner) };
            std::string name;
            while (std::getline(names, name, ':'))
                if (!name.empty())
                    hierarchy.push_back(unqualified(name, "--"));
        }
        auto rank = [&](std::string const& aClass)
        {
            auto const name = unqualified(aClass, "::");
            auto const existing = std::find(hierarchy.begin(), hierarchy.end(), name);
            return static_cast<std::size_t>(std::distance(hierarchy.begin(), existing));
        };
        std::vector<std::pair<std::size_t, std::string>> orderedClasses;
        for (auto const& c : classes)
            orderedClasses.emplace_back(rank(c.first), c.first);
        std::sort(orderedClasses.begin(), orderedClasses.end());
        // e.g. "neogfx::layout_item" -> "Layout Item"
        auto display_name = [&](std::string const& aName)
        {
            std::string displayName;
            for (auto const& word : neolib::tokens(unqualified(aName, "::"), "_"s))
            {
                if (word.empty())
                    continue;
                if (!displayName.empty())
                    displayName += ' ';
                displayName += static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
                displayName += word.substr(1);
            }
            return displayName;
        };
        // property categories in declaration order (see property_category); any others follow
        static std::vector<std::type_index> const categoryOrder =
        {
            typeid(property_category::soft_geometry),
            typeid(property_category::hard_geometry),
            typeid(property_category::font),
            typeid(property_category::color),
            typeid(property_category::other_appearance),
            typeid(property_category::interaction),
            typeid(property_category::other)
        };
        auto category_rank = [&](std::type_index const& aCategory)
        {
            return static_cast<std::size_t>(std::distance(categoryOrder.begin(), std::find(categoryOrder.begin(), categoryOrder.end(), aCategory)));
        };
        for (auto const& c : orderedClasses)
        {
            auto classNode = iPropertyModel.insert_item(iPropertyModel.send(), property_model_item{}, string{ display_name(c.second) });
            std::map<std::pair<std::size_t, std::string>, std::vector<i_property*>> categories;
            for (auto property : classes[c.second])
            {
                std::type_index const category{ property->category() };
                categories[{ category_rank(category), property_class_name(property->category()) }].push_back(property);
            }
            for (auto& category : categories)
            {
                auto categoryNode = iPropertyModel.append_item(classNode, property_model_item{}, string{ display_name(category.first.second) });
                auto& properties = category.second;
                std::sort(properties.begin(), properties.end(), [](i_property const* lhs, i_property const* rhs) 
                { 
                    return lhs->name().to_std_string_view() < rhs->name().to_std_string_view(); 
                });
                for (auto property : properties)
                {
                    auto row = iPropertyModel.append_item(categoryNode, property_model_item{ property }, string{ property->name() });
                    iPropertyModel.insert_cell_data(row, 1u, string{ property_value_to_string(*property) });
                }
            }
        }
    }

    void main_window_ex::paint_workspace(ng::i_graphics_context& aGc)
    {
        auto const& scrollArea = iWorkspace.view_stack().scroll_area();
        if (iProjectManager.projects().empty())
        {
            aGc.draw_texture(
                ng::point{ (scrollArea.extents() - iBackgroundTexture1.extents()) / 2.0 },
                iBackgroundTexture1,
                ng::color::White.with_alpha(0.25));
            aGc.draw_texture(
                ng::rect{ ng::point{ scrollArea.bottom_right() - iBackgroundTexture2.extents() / 2.0 }, iBackgroundTexture2.extents() / 2.0 },
                iBackgroundTexture2,
                ng::color::White.with_alpha(0.25));
        }
        else
        {
            if (workspaceGridType.value<workspace_grid>(true) != workspace_grid::None)
            {
                auto const& gridSize = ng::from_dip(ng::basic_size<std::uint32_t>{
                    workspaceGridSize.value<std::uint32_t>(true) / workspaceGridSubdivisions.value<std::uint32_t>(true),
                        workspaceGridSize.value<std::uint32_t>(true) / workspaceGridSubdivisions.value<std::uint32_t>(true)});
                ng::basic_size<std::int32_t> const cells = ng::size{ scrollArea.cx / gridSize.cx, scrollArea.cy / gridSize.cy };
                aGc.set_gradient(workspaceGridColor.value<ng::gradient>(true), scrollArea);
                if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Lines)
                {
                    for (std::int32_t x = 0; x <= cells.cx; ++x)
                        aGc.draw_line(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() }, ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.bottom() }, ng::color::White);
                    for (std::int32_t y = 0; y <= cells.cy; ++y)
                        aGc.draw_line(ng::point{ scrollArea.left(), scrollArea.top() + y * gridSize.cy }, ng::point{ scrollArea.right(), scrollArea.top() + y * gridSize.cy }, ng::color::White);
                }
                else if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Quads)
                {
                    for (std::int32_t x = 0; x <= cells.cx; ++x)
                        for (std::int32_t y = 0; y <= cells.cy; ++y)
                            if ((x + y) % 2 == 0)
                                aGc.fill_rect(ng::rect{ ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() + y * gridSize.cy }, gridSize }, ng::color::White);
                }
                else // Points
                {
                    for (std::int32_t x = 0; x <= cells.cx; ++x)
                        for (std::int32_t y = 0; y <= cells.cy; ++y)
                            aGc.draw_pixel(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() + y * gridSize.cy }, ng::color::White);
                }
                aGc.clear_gradient();
                for (std::int32_t x = 0; x <= cells.cx; x += workspaceGridSubdivisions.value<std::uint32_t>(true))
                    aGc.draw_line(ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.top() }, ng::point{ scrollArea.left() + x * gridSize.cx, scrollArea.bottom() }, service<i_app>().current_style().palette().color(color_role::Void));
                for (std::int32_t y = 0; y <= cells.cy; y += workspaceGridSubdivisions.value<std::uint32_t>(true))
                    aGc.draw_line(ng::point{ scrollArea.left(), scrollArea.top() + y * gridSize.cy }, ng::point{ scrollArea.right(), scrollArea.top() + y * gridSize.cy }, service<i_app>().current_style().palette().color(color_role::Void));
            }            
        }
        if (iProjectManager.project_active())
        {
            for (auto& e : iProjectManager.active_project().root())
            {
                if (e->group() == element_group::Node)
                {
                    auto& node = static_cast<i_node&>(*e);
                    for (auto const& connection : node.connections())
                    {
                        if (&connection->source().get() == &node)
                        {
                            auto const& node0 = connection->source().get();
                            auto const& node1 = connection->destination().get();
                            auto const& pinWidget0 = connection->source().as_widget();
                            auto const& pinWidget1 = connection->destination().as_widget();
                            auto const nodeRect0 = iWorkspace.view_stack().to_client_coordinates(node0.widget().to_window_coordinates(node0.widget().client_rect()));
                            auto const nodeRect1 = iWorkspace.view_stack().to_client_coordinates(node1.widget().to_window_coordinates(node1.widget().client_rect()));
                            auto const pinRect0 = iWorkspace.view_stack().to_client_coordinates(pinWidget0.icon().to_window_coordinates(pinWidget0.icon().client_rect()));
                            auto const pinRect1 = iWorkspace.view_stack().to_client_coordinates(pinWidget1.icon().to_window_coordinates(pinWidget1.icon().client_rect()));
                            auto const placementRect = pinRect0.combined(pinRect1);
                            auto p0 = pinRect0.center();
                            auto p3 = pinRect1.center();
                            auto const bendRadius = 128.0_dip;
                            auto const negativeControlPoint = bendRadius * 2.0 + std::sqrt(placementRect.width());
                            bool const xPositive = (p3.x - p0.x > bendRadius / 4.0);
                            bool const yPositive = (p3.y >= p0.y);
                            auto p1 = point{ xPositive ? p0.mid(p3).x : nodeRect1.left() - negativeControlPoint, placementRect.top() };
                            auto p2 = point{ xPositive ? p0.mid(p3).x : nodeRect0.right() + negativeControlPoint, placementRect.bottom() };
                            if (xPositive)
                            {
                                if (!yPositive)
                                    std::swap(p1.y, p2.y);
                            }
                            else
                            {
                                std::swap(p0, p3);
                                if (!yPositive)
                                    std::swap(p1.y, p2.y);
                                if (std::abs(yPositive ? nodeRect1.top() - nodeRect0.bottom() : nodeRect0.top() - nodeRect1.bottom()) > bendRadius)
                                {
                                    p1.y = p0.mid(p3).y;
                                    p2.y = p0.mid(p3).y;
                                }
                            }
                            aGc.draw_cubic_bezier(p0, p1, p2, p3, pen{ connection->source().color(), 2.0_dip });
                            aGc.draw_cubic_bezier(p0, p1, p2, p3, pen{ color::Black, 4.0_dip });
                            aGc.draw_cubic_bezier(p0, p1, p2, p3, pen{ connection->source().color(), 2.0_dip });
                        }
                    }
                }
            }
        }
    }
}

