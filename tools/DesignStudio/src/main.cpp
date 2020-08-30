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
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/gui/widget/status_bar.hpp>
#include <neogfx/gui/view/view_container.hpp>
#include <neogfx/gui/widget/dock.hpp>
#include <neogfx/gui/widget/dockable.hpp>
#include <neogfx/gui/widget/tree_view.hpp>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/core/css.hpp>
#include <neogfx/gui/dialog/settings_dialog.hpp>
#include <neogfx/tools/DesignStudio/project_manager.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/settings.hpp>
#include "new_project_dialog.hpp"
#include "DesignStudio.ui.hpp"

using namespace neolib::string_literals;

int main(int argc, char* argv[])
{
    ds::main_app app{ argc, argv, "neoGFX Design Studio" };

    try
    {
        ng::service<ng::i_rendering_engine>().subpixel_rendering_on();

        ds::main_window mainWindow{ app };

        app.actionShowStandardToolbar.checked([&]() { mainWindow.standardToolbar.show(); });
        app.actionShowStandardToolbar.unchecked([&]() { mainWindow.standardToolbar.hide(); });
        app.actionShowStatusBar.checked([&]() { mainWindow.statusBar.show(); });
        app.actionShowStatusBar.unchecked([&]() { mainWindow.statusBar.hide(); });

        app.change_style("Dark");
        app.current_style().set_spacing(ng::size{ 4.0 });

        ng::dock leftDock{ mainWindow.dock_layout(ng::dock_area::Left), ng::dock_area::Left };
        ng::dock rightDock{ mainWindow.dock_layout(ng::dock_area::Right), ng::dock_area::Right };

        auto objects = ng::make_dockable<ng::tree_view>("Objects"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame);
        auto properties = ng::make_dockable<ng::tree_view>("Properties"_t, ng::dock_area::Right, true, ng::frame_style::NoFrame);
        objects.dock(rightDock);
        properties.dock(rightDock);

        leftDock.hide();
        rightDock.hide();

        ng::i_layout& mainLayout = mainWindow.client_layout();
        mainLayout.set_padding(ng::padding{});
        mainLayout.set_spacing(ng::size{});

        ng::horizontal_layout workspaceLayout{ mainLayout };
        ng::view_container workspace{ workspaceLayout };

        ng::texture backgroundTexture1{ ng::image{ ":/neogfx/DesignStudio/resources/neoGFX.png" } };
        ng::texture backgroundTexture2{ ng::image{ ":/neogfx/DesignStudio/resources/logo_i42.png" } };

        ds::settings settings;

        auto& toolbarIconSize = settings.setting("environment.toolbars.icon_size"_s);
        auto& themeColor = settings.setting("environment.general.theme"_s);
        auto& workspaceGridType = settings.setting("environment.workspace.grid_type"_s);
        auto& workspaceGridSize = settings.setting("environment.workspace.grid_size"_s);
        auto& workspaceGridColor = settings.setting("environment.workspace.grid_color"_s);

        auto toolbarIconSizeChanged = [&]()
        {
            switch (toolbarIconSize.value<ds::toolbar_icon_size>(true))
            {
            case ds::toolbar_icon_size::Size16x16:
                mainWindow.standardToolbar.set_button_image_extents(ng::size{ 16.0_dip, 16.0_dip });
                break;
            case ds::toolbar_icon_size::Size32x32:
                mainWindow.standardToolbar.set_button_image_extents(ng::size{ 32.0_dip, 32.0_dip });
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

        auto project_updated = [&](ds::i_project&) { update_ui(); };

        ng::sink sink;
        sink += pm.ProjectAdded(project_updated);
        sink += pm.ProjectRemoved(project_updated);
        sink += pm.ProjectActivated(project_updated);
        sink += app.actionFileClose.triggered([&]() { if (pm.project_active()) pm.close_project(pm.active_project()); });

        app.action_file_new().triggered([&]()
        {
            ds::new_project_dialog_ex dialog{ mainWindow };
            if (dialog.exec() == ng::dialog_result::Accepted)
                pm.create_project(dialog.projectName.text(), dialog.projectNamespace.text()).set_dirty();
        });

        app.actionSettings.triggered([&]()
        {
            ng::settings_dialog dialog{ mainWindow, settings };
            dialog.exec();
        });

        //        ng::css css{"test.css"};

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

