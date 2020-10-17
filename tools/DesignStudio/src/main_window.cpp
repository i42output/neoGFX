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
#include "main_window.hpp"

namespace neogfx::DesignStudio
{
    main_window_ex::main_window_ex(main_app& aApp, settings& aSettings, project_manager& aProjectManager) :
        main_window{ aApp },
        iProjectManager{ aProjectManager },
        autoscaleDocks{ aSettings.setting("environment.tabs_and_windows.autoscale_docks"_s) },
        workspaceSize{ aSettings.setting("environment.tabs_and_windows.workspace_size"_s) },
        workspacePosition{ aSettings.setting("environment.tabs_and_windows.workspace_position"_s) },
        leftDockWidth{ aSettings.setting("environment.tabs_and_windows.left_dock_width"_s) },
        rightDockWidth{ aSettings.setting("environment.tabs_and_windows.right_dock_width"_s) },
        leftDockWeight{ aSettings.setting("environment.tabs_and_windows.left_dock_weight"_s) },
        rightDockWeight{ aSettings.setting("environment.tabs_and_windows.right_dock_weight"_s) },
        font{ aSettings.setting("environment.fonts_and_colors.font"_s) },
        subpixelRendering{ aSettings.setting("environment.fonts_and_colors.subpixel"_s) },
        toolbarIconSize{ aSettings.setting("environment.toolbars.icon_size"_s) },
        themeColor{ aSettings.setting("environment.fonts_and_colors.theme"_s) },
        workspaceGridType{ aSettings.setting("environment.workspace.grid_type"_s) },
        workspaceGridSize{ aSettings.setting("environment.workspace.grid_size"_s) },
        workspaceGridColor{ aSettings.setting("environment.workspace.grid_color"_s) },
        iLeftDock{ dock_layout(ng::dock_area::Left), ng::dock_area::Left, ng::size{ leftDockWidth.value<double>() }, ng::size{ leftDockWeight.value<double>() } },
        iRightDock{ dock_layout(ng::dock_area::Right), ng::dock_area::Right, ng::size{ rightDockWidth.value<double>() }, ng::size{ rightDockWeight.value<double>() } },
        iToolbox{ ng::make_dockable<ng::tree_view>("Toolbox"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iWorkflow{ ng::make_dockable<ng::list_view>("Workflow"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame) },
        iObjects{ ng::make_dockable<ng::table_view>("Object Explorer"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iProperties{ ng::make_dockable<ng::table_view>("Properties"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame) },
        iWorkspaceLayout{ client_layout() },
        iWorkspace{ iWorkspaceLayout },
        iBackgroundTexture1{ ng::image{ ":/neogfx/DesignStudio/resources/neoGFX.png" } },
        iBackgroundTexture2{ ng::image{ ":/neogfx/DesignStudio/resources/logo_i42.png" } }
    {
        // todo: decompose this ctor body into smaller member initialization functions...

        hide();

        aApp.actionShowStandardToolbar.checked([&]() { standardToolbar.show(); });
        aApp.actionShowStandardToolbar.unchecked([&]() { standardToolbar.hide(); });
        aApp.actionShowStatusBar.checked([&]() { statusBar.show(); });
        aApp.actionShowStatusBar.unchecked([&]() { statusBar.hide(); });

        aApp.change_style("Dark");
        aApp.current_style().set_spacing(ng::size{ 4.0 });

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

        iToolbox.set_weight(ng::size{ 3.0 });
        iWorkflow.set_weight(ng::size{ 1.0 });

        iToolbox.dock(iLeftDock);
        iWorkflow.dock(iLeftDock);
        iObjects.dock(iRightDock);
        iProperties.dock(iRightDock);

        ng::i_layout& mainLayout = client_layout();
        mainLayout.set_padding(ng::padding{});
        mainLayout.set_spacing(ng::size{});

        auto fontChanged = [&]()
        {
            ng::service<ng::i_app>().current_style().set_font_info(font.value<ng::font_info>(true));
        };
        font.changing(fontChanged);
        font.changed(fontChanged);
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
            ng::service<ng::i_app>().current_style().palette().set_color(ng::color_role::Theme, themeColor.value<ng::color>(true));
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

        populate_toolbox_model(iToolboxModel, iToolboxPresentationModel);
        auto& toolboxTree = iToolbox.docked_widget<ng::tree_view>();
        toolboxTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        toolboxTree.set_presentation_model(iToolboxPresentationModel);
        toolboxTree.selection_model().set_mode(ng::item_selection_mode::NoSelection);
        toolboxTree.set_focus_policy(ng::focus_policy::TabFocus);

        auto workflowCppIde = iWorkflowModel.insert_item(iWorkflowModel.end(), workflow_tool::CppIde, "Build");
        auto workflowNote = iWorkflowModel.insert_item(iWorkflowModel.end(), workflow_tool::StickyNote, "Sticky");

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
        objectTree.set_presentation_model(iObjectPresentationModel);
        objectTree.column_header().set_expand_last_column(true);

        iWorkspace.view_stack().Painting([&](ng::i_graphics_context& aGc) { paint_workspace(aGc); });

        auto update_ui = [&]()
        {
            aApp.actionFileClose.enable(aProjectManager.project_active());
            aApp.actionFileSave.enable(aProjectManager.project_active() && aProjectManager.active_project().dirty());
            iLeftDock.show(aProjectManager.project_active());
            iRightDock.show(aProjectManager.project_active());
        };

        update_ui();

        auto project_updated = [&, update_ui](i_project&)
        {
            if (aProjectManager.project_active())
            {
                if (iObjectModel.empty() || iObjectModel.item(iObjectModel.sbegin()) != &aProjectManager.active_project().root())
                {
                    iObjectModel.clear();
                    iObjectModel.updating().trigger();
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
                                iObjectModel.append_item(aPosition, &aElement, aElement.id().to_std_string()) :
                                iObjectModel.insert_item(aPosition, &aElement, aElement.id().to_std_string());
                            iObjectModel.insert_cell_data(node, 1u, aElement.type().to_std_string());
                            for (auto& child : aElement)
                                addNode(node, *child);
                        }
                        break;
                        case element_group::Action:
                            break;
                        }
                    };
                    addNode(iObjectModel.send(), aProjectManager.active_project().root());
                    iObjectModel.updated().trigger();
                }
            }
            else
                iObjectModel.clear();
            update_ui();
        };

        iSink += aProjectManager.ProjectAdded(project_updated);
        iSink += aProjectManager.ProjectRemoved(project_updated);
        iSink += aProjectManager.ProjectActivated(project_updated);
        iSink += aApp.actionFileClose.triggered([&]() { if (aProjectManager.project_active()) aProjectManager.close_project(aProjectManager.active_project()); });

        aApp.action_file_open().triggered([&]()
        {
            ng::service<ng::i_window_manager>().save_mouse_cursor();
            ng::service<ng::i_window_manager>().set_mouse_cursor(ng::mouse_system_cursor::Wait);
            auto projectFile = ng::open_file_dialog(mainWindow, ng::file_dialog_spec{ "Open Project", {}, { "*.nrc" }, "Project Files" });
            if (projectFile)
                aProjectManager.open_project((*projectFile)[0]);
            ng::service<ng::i_window_manager>().restore_mouse_cursor(mainWindow);
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

        //        ng::css css{"test.css"};

        activate();
    }

    void main_window_ex::paint_workspace(ng::i_graphics_context& aGc)
    {
        auto const& cr = iWorkspace.view_stack().client_rect();
        if (iProjectManager.projects().empty())
        {
            aGc.draw_texture(
                ng::point{ (cr.extents() - iBackgroundTexture1.extents()) / 2.0 },
                iBackgroundTexture1,
                ng::color::White.with_alpha(0.25));
            aGc.draw_texture(
                ng::rect{ ng::point{ cr.bottom_right() - iBackgroundTexture2.extents() / 2.0 }, iBackgroundTexture2.extents() / 2.0 },
                iBackgroundTexture2,
                ng::color::White.with_alpha(0.25));
        }
        else
        {
            aGc.set_gradient(workspaceGridColor.value<ng::gradient>(true), iWorkspace.view_stack().client_rect());
            ng::size const& gridSize = ng::from_dip(ng::basic_size<uint32_t>{ workspaceGridSize.value<uint32_t>(true), workspaceGridSize.value<uint32_t>(true) });
            ng::basic_size<int32_t> const cells = ng::size{ cr.cx / gridSize.cx, cr.cy / gridSize.cy };
            if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Lines)
            {
                for (int32_t x = 0; x <= cells.cx; ++x)
                    aGc.draw_line(ng::point{ x * gridSize.cx, 0.0 }, ng::point{ x * gridSize.cx, cr.bottom() }, ng::color::White);
                for (int32_t y = 0; y <= cells.cy; ++y)
                    aGc.draw_line(ng::point{ 0.0, y * gridSize.cy }, ng::point{ cr.right(), y * gridSize.cy }, ng::color::White);
            }
            else if (workspaceGridType.value<workspace_grid>(true) == workspace_grid::Quads)
            {
                for (int32_t x = 0; x <= cells.cx; ++x)
                    for (int32_t y = 0; y <= cells.cy; ++y)
                        if ((x + y) % 2 == 0)
                            aGc.fill_rect(ng::rect{ ng::point{ x * gridSize.cx, y * gridSize.cy }, gridSize }, ng::color::White);
            }
            else // Points
            {
                for (int32_t x = 0; x <= cells.cx; ++x)
                    for (int32_t y = 0; y <= cells.cy; ++y)
                        aGc.draw_pixel(ng::point{ x * gridSize.cx, y * gridSize.cy }, ng::color::White);
            }
            aGc.clear_gradient();
        }
    }
}

