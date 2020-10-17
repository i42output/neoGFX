// main.cpp
/*
neoGFX Design Studio
Copyright(C) 2017 Leigh Johnston

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
#include <fstream>
#include <neolib/core/string.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/gui/widget/status_bar.hpp>
#include <neogfx/gui/view/view_container.hpp>
#include <neogfx/gui/widget/dock.hpp>
#include <neogfx/gui/widget/dockable.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/core/css.hpp>
#include <neogfx/gui/dialog/settings_dialog.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/tools/DesignStudio/project_manager.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/settings.hpp>
#include "new_project_dialog.hpp"
#include "toolbox_model.hpp"
#include "workflow_model.hpp"
#include "object_model.hpp"
#include "DesignStudio.ui.hpp"

using namespace neolib::string_literals;

int main(int argc, char* argv[])
{
    neolib::application_info appInfo
    {
        argc, argv,
        "neoGFX Design Studio",
        "i42 Software",
        neolib::version{ 1, 0, 0, 0, "pre-release" },
        "Copyright (c) 2020 Leigh Johnston",
        {}, {}, {}, ".nel"
    };

    std::cout << "------ " << appInfo.name() << " ------" << std::endl;
    std::cout << appInfo.copyright() << std::endl << std::endl;

    ds::main_app app{ appInfo };

    std::cout << "Loading element libraries..." << std::endl;
    app.plugin_manager().load_plugins();
    for (auto const& plugin : app.plugin_manager().plugins())
        std::cout << "Element library '" << plugin->name() << "' loaded." << std::endl;

    try
    {
        ds::main_window mainWindow{ app };
        mainWindow.hide();

        app.actionShowStandardToolbar.checked([&]() { mainWindow.standardToolbar.show(); });
        app.actionShowStandardToolbar.unchecked([&]() { mainWindow.standardToolbar.hide(); });
        app.actionShowStatusBar.checked([&]() { mainWindow.statusBar.show(); });
        app.actionShowStatusBar.unchecked([&]() { mainWindow.statusBar.hide(); });

        app.change_style("Dark");
        app.current_style().set_spacing(ng::size{ 4.0 });

        ds::settings settings;

        auto& autoscaleDocks = settings.setting("environment.tabs_and_windows.autoscale_docks"_s);
        auto& workspaceSize = settings.setting("environment.tabs_and_windows.workspace_size"_s);
        auto& workspacePosition = settings.setting("environment.tabs_and_windows.workspace_position"_s);
        auto& leftDockWidth = settings.setting("environment.tabs_and_windows.left_dock_width"_s);
        auto& rightDockWidth = settings.setting("environment.tabs_and_windows.right_dock_width"_s);
        auto& leftDockWeight = settings.setting("environment.tabs_and_windows.left_dock_weight"_s);
        auto& rightDockWeight = settings.setting("environment.tabs_and_windows.right_dock_weight"_s);

        if (!workspaceSize.is_default())
            mainWindow.set_extents(workspaceSize.value<ng::size>());
        if (!workspacePosition.is_default())
            mainWindow.move(workspacePosition.value<ng::point>());
        else
            mainWindow.center(false);

        ng::dock leftDock{ mainWindow.dock_layout(ng::dock_area::Left), ng::dock_area::Left, ng::size{ leftDockWidth.value<double>() }, ng::size{ leftDockWeight.value<double>() } };
        ng::dock rightDock{ mainWindow.dock_layout(ng::dock_area::Right), ng::dock_area::Right, ng::size{ rightDockWidth.value<double>() }, ng::size{ rightDockWeight.value<double>() } };
        // todo: tidier way of doing this...
        mainWindow.dock_layout(ng::layout_position::Center).set_weight(ng::size{ 1.0 } - leftDock.parent_layout().weight() - rightDock.parent_layout().weight());
        leftDock.hide();
        rightDock.hide();

        auto autoscaleDocksChanged = [&]()
        {
            mainWindow.dock_layout(ng::layout_position::Center).parent_layout().fix_weightings();
            mainWindow.dock_layout(ng::layout_position::Center).parent_layout().set_autoscale(autoscaleDocks.value<bool>(true) ? ng::autoscale::Active : ng::autoscale::Default);
        };
        autoscaleDocks.changing(autoscaleDocksChanged);
        autoscaleDocks.changed(autoscaleDocksChanged);
        mainWindow.dock_layout(ng::layout_position::Center).parent_layout().set_autoscale(autoscaleDocks.value<bool>(true) ? ng::autoscale::Active : ng::autoscale::Default);

        ng::get_property(mainWindow, "Size").property_changed([&](const ng::property_variant& aValue)
        {
            workspaceSize.set_value(std::get<ng::size>(aValue));
        });
        ng::get_property(mainWindow, "Position").property_changed([&](const ng::property_variant& aValue)
        {
            workspacePosition.set_value(std::get<ng::point>(aValue));
        });
        ng::get_property(leftDock.parent_layout(), "Size").property_changed([&](const ng::property_variant& aValue)
        {
            leftDockWidth.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(rightDock.parent_layout(), "Size").property_changed([&](const ng::property_variant& aValue)
        {
            rightDockWidth.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(leftDock.parent_layout(), "Weight").property_changed([&](const ng::property_variant& aValue)
        {
            if (aValue != ng::none)
                leftDockWeight.set_value(std::get<ng::size>(aValue).cx);
        });
        ng::get_property(rightDock.parent_layout(), "Weight").property_changed([&](const ng::property_variant& aValue)
        {
            if (aValue != ng::none)
                rightDockWeight.set_value(std::get<ng::size>(aValue).cx);
        });

        auto toolbox = ng::make_dockable<ng::tree_view>("Toolbox"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame);
        auto workflow = ng::make_dockable<ng::list_view>("Workflow"_t, ng::dock_area::Left, true, ng::frame_style::NoFrame);
        auto objects = ng::make_dockable<ng::table_view>("Object Explorer"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame);
        auto properties = ng::make_dockable<ng::table_view>("Properties"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame);

        toolbox.set_weight(ng::size{ 3.0 });
        workflow.set_weight(ng::size{ 1.0 });

        toolbox.dock(leftDock);
        workflow.dock(leftDock);
        objects.dock(rightDock);
        properties.dock(rightDock);

        ng::i_layout& mainLayout = mainWindow.client_layout();
        mainLayout.set_padding(ng::padding{});
        mainLayout.set_spacing(ng::size{});

        ng::horizontal_layout workspaceLayout{ mainLayout };
        ng::view_container workspace{ workspaceLayout };

        ng::texture backgroundTexture1{ ng::image{ ":/neogfx/DesignStudio/resources/neoGFX.png" } };
        ng::texture backgroundTexture2{ ng::image{ ":/neogfx/DesignStudio/resources/logo_i42.png" } };

        auto& font = settings.setting("environment.fonts_and_colors.font"_s);
        auto& subpixelRendering = settings.setting("environment.fonts_and_colors.subpixel"_s);
        auto& toolbarIconSize = settings.setting("environment.toolbars.icon_size"_s);
        auto& themeColor = settings.setting("environment.fonts_and_colors.theme"_s);
        auto& workspaceGridType = settings.setting("environment.workspace.grid_type"_s);
        auto& workspaceGridSize = settings.setting("environment.workspace.grid_size"_s);
        auto& workspaceGridColor = settings.setting("environment.workspace.grid_color"_s);

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
            switch (toolbarIconSize.value<ds::toolbar_icon_size>(true))
            {
            case ds::toolbar_icon_size::Size16x16:
                mainWindow.standardToolbar.set_button_image_extents(ng::size{ 16.0_dip, 16.0_dip });
                break;
            case ds::toolbar_icon_size::Size24x24:
                mainWindow.standardToolbar.set_button_image_extents(ng::size{ 24.0_dip, 24.0_dip });
                break;
            case ds::toolbar_icon_size::Size32x32:
                mainWindow.standardToolbar.set_button_image_extents(ng::size{ 32.0_dip, 32.0_dip });
                break;
            case ds::toolbar_icon_size::Size48x48:
                mainWindow.standardToolbar.set_button_image_extents(ng::size{ 48.0_dip, 48.0_dip });
                break;
            case ds::toolbar_icon_size::Size64x64:
                mainWindow.standardToolbar.set_button_image_extents(ng::size{ 64.0_dip, 64.0_dip });
                break;
            }
        };
        toolbarIconSize.changing(toolbarIconSizeChanged);
        toolbarIconSize.changed(toolbarIconSizeChanged);
        toolbarIconSizeChanged();

        auto themeColorChanged = [&]()
        {
            ng::service<ng::i_app>().current_style().palette().set_color(ng::color_role::Theme, themeColor.value<ng::color>(true));
            workspace.view_stack().set_background_color(ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Base));
            workspaceGridColor.set_default_value(ng::gradient{ ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Background).with_alpha(0.25) });
        };
        themeColor.changing(themeColorChanged);
        themeColor.changed(themeColorChanged);
        themeColorChanged();

        auto workspaceGridChanged = [&]()
        {
            workspace.view_stack().update();
        };
        workspaceGridType.changing(workspaceGridChanged);
        workspaceGridType.changed(workspaceGridChanged);
        workspaceGridColor.changing(workspaceGridChanged);
        workspaceGridColor.changed(workspaceGridChanged);
        workspaceGridSize.changing(workspaceGridChanged);
        workspaceGridSize.changed(workspaceGridChanged);

        ds::project_manager pm;

        ds::toolbox_model toolboxModel;
        ds::toolbox_presentation_model toolboxPresentationModel;
        ds::populate_toolbox_model(toolboxModel, toolboxPresentationModel);
        auto& toolboxTree = toolbox.docked_widget<ng::tree_view>();
        toolboxTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        toolboxTree.set_presentation_model(toolboxPresentationModel);
        toolboxTree.selection_model().set_mode(ng::item_selection_mode::NoSelection);
        toolboxTree.set_focus_policy(ng::focus_policy::TabFocus);

        ds::workflow_model workflowModel; // todo
        auto workflowCppIde = workflowModel.insert_item(workflowModel.end(), ds::workflow_tool::CppIde, "Build");
        auto workflowNote = workflowModel.insert_item(workflowModel.end(), ds::workflow_tool::StickyNote, "Sticky");
        ds::workflow_presentation_model workflowPresentationModel;

        workflowPresentationModel.set_item_model(workflowModel);
        workflowPresentationModel.set_column_read_only(0u);
        auto& workflowTree = workflow.docked_widget<ng::list_view>();
        workflowTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        workflowTree.set_presentation_model(workflowPresentationModel);
        workflowTree.selection_model().set_mode(ng::item_selection_mode::NoSelection);
        workflowTree.set_focus_policy(ng::focus_policy::TabFocus);

        ds::object_model objectModel;
        objectModel.set_column_name(0u, "Object"_t);
        objectModel.set_column_name(1u, "Type"_t);
        ds::object_presentation_model objectPresentationModel;

        objectPresentationModel.set_item_model(objectModel);
        objectPresentationModel.set_column_read_only(1u);
        objectPresentationModel.set_alternating_row_color(true);
        auto& objectTree = objects.docked_widget<ng::table_view>();
        objectTree.set_minimum_size(ng::size{ 128_dip, 128_dip });
        objectTree.set_presentation_model(objectPresentationModel);
        objectTree.column_header().set_expand_last_column(true);

        workspace.view_stack().Painting([&](ng::i_graphics_context& aGc)
        {
            auto const& cr = workspace.view_stack().client_rect();
            if (pm.projects().empty())
            {
                aGc.draw_texture(
                    ng::point{ (cr.extents() - backgroundTexture1.extents()) / 2.0 },
                    backgroundTexture1,
                    ng::color::White.with_alpha(0.25));
                aGc.draw_texture(
                    ng::rect{ ng::point{ cr.bottom_right() - backgroundTexture2.extents() / 2.0 }, backgroundTexture2.extents() / 2.0 },
                    backgroundTexture2,
                    ng::color::White.with_alpha(0.25));
            }
            else
            {
                aGc.set_gradient(workspaceGridColor.value<ng::gradient>(true), workspace.view_stack().client_rect());
                ng::size const& gridSize = ng::from_dip(ng::basic_size<uint32_t>{ workspaceGridSize.value<uint32_t>(true), workspaceGridSize.value<uint32_t>(true) });
                ng::basic_size<int32_t> const cells = ng::size{ cr.cx / gridSize.cx, cr.cy / gridSize.cy };
                if (workspaceGridType.value<ds::workspace_grid>(true) == ds::workspace_grid::Lines)
                {
                    for (int32_t x = 0; x <= cells.cx; ++x)
                        aGc.draw_line(ng::point{ x * gridSize.cx, 0.0 }, ng::point{ x * gridSize.cx, cr.bottom() }, ng::color::White);
                    for (int32_t y = 0; y <= cells.cy; ++y)
                        aGc.draw_line(ng::point{ 0.0, y * gridSize.cy }, ng::point{ cr.right(), y * gridSize.cy }, ng::color::White);
                }
                else if (workspaceGridType.value<ds::workspace_grid>(true) == ds::workspace_grid::Quads)
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
        });

        auto update_ui = [&]()
        {
            app.actionFileClose.enable(pm.project_active());
            app.actionFileSave.enable(pm.project_active() && pm.active_project().dirty());
            leftDock.show(pm.project_active());
            rightDock.show(pm.project_active());
        };

        update_ui();

        auto project_updated = [&](ds::i_project&) 
        { 
            if (pm.project_active())
            {
                if (objectModel.empty() || objectModel.item(objectModel.sbegin()) != &pm.active_project().root())
                {
                    objectModel.clear();
                    objectModel.updating().trigger();
                    std::function<void(ng::i_item_model::iterator, ds::i_element&)> addNode = [&](ng::i_item_model::iterator aPosition, ds::i_element& aElement)
                    {
                        switch (aElement.group())
                        {
                        case ds::element_group::Unknown:
                            break;
                        case ds::element_group::Project:
                        case ds::element_group::UserInterface:
                        case ds::element_group::App:
                        case ds::element_group::Menu:
                        case ds::element_group::Widget:
                        case ds::element_group::Layout:
                            {
                                auto node = aElement.group() != ds::element_group::Project ?
                                    objectModel.append_item(aPosition, &aElement, aElement.id().to_std_string()) :
                                    objectModel.insert_item(aPosition, &aElement, aElement.id().to_std_string());
                                objectModel.insert_cell_data(node, 1u, aElement.type().to_std_string());
                                for (auto& child : aElement)
                                    addNode(node, *child);
                            }
                            break;
                        case ds::element_group::Action:
                            break;
                        }
                    };
                    addNode(objectModel.send(), pm.active_project().root());
                    objectModel.updated().trigger();
                }
            }
            else
                objectModel.clear();
            update_ui();
        };

        ng::sink sink;
        sink += pm.ProjectAdded(project_updated);
        sink += pm.ProjectRemoved(project_updated);
        sink += pm.ProjectActivated(project_updated);
        sink += app.actionFileClose.triggered([&]() { if (pm.project_active()) pm.close_project(pm.active_project()); });

        app.action_file_open().triggered([&]()
        {
            ng::service<ng::i_window_manager>().save_mouse_cursor();
            ng::service<ng::i_window_manager>().set_mouse_cursor(ng::mouse_system_cursor::Wait);
            auto projectFile = ng::open_file_dialog(mainWindow, ng::file_dialog_spec{ "Open Project", {}, { "*.nrc" }, "Project Files" });
            if (projectFile)
                pm.open_project((*projectFile)[0]);
            ng::service<ng::i_window_manager>().restore_mouse_cursor(mainWindow);
        });

        app.action_file_new().triggered([&]()
        {
            ds::new_project_dialog_ex dialog{ mainWindow };
            if (dialog.exec() == ng::dialog_result::Accepted)
            {
                auto& project = pm.create_project(dialog.projectName.text(), dialog.projectNamespace.text());
                project.set_dirty();
                project.root();
            }
        });

        app.actionSettings.triggered([&]()
        {
            ng::settings_dialog dialog{ mainWindow, settings };
            dialog.exec();
        });

        //        ng::css css{"test.css"};

        mainWindow.activate();

        return app.exec();
    }
    catch (std::exception& e)
    {
        app.halt();
        std::cerr << "neoGFX Design Studio: terminating with exception: " << e.what() << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        app.halt();
        std::cerr << "neoGFX Design Studio: terminating with unknown exception" << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
        std::exit(EXIT_FAILURE);
    }
}

