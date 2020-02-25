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

#include <neogfx/neogfx.hpp>
#include <fstream>
#include <neogfx/app/app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/gui/widget/status_bar.hpp>
#include <neogfx/gui/view/view_container.hpp>
#include <neogfx/core/css.hpp>
#include "new_project_dialog.hpp"

namespace ng = neogfx;
using namespace ng::unit_literals;

int main(int argc, char* argv[])
{
    ng::app app(argc, argv, "neoGFX Design Studio");
    try
    {
        ng::service<ng::i_rendering_engine>().enable_frame_rate_limiter(false);
        
        app.current_style().palette().set_color(ng::color_role::Theme, ng::color{ 64, 64, 64 });
        app.current_style().set_spacing(ng::size{ 4.0 });

        ng::window mainWindow{ ng::service<ng::i_basic_services>().display().desktop_rect().extents() * ng::size{ 0.5, 0.5 } };
        ng::i_layout& mainLayout = mainWindow.client_layout();
        mainLayout.set_margins(ng::margins{});
        mainLayout.set_spacing(ng::size{});

        ng::menu_bar mainMenu{ mainWindow.menu_layout() };

        auto& fileMenu = app.add_standard_menu(mainMenu, ng::standard_menu::File);
        auto& editMenu = app.add_standard_menu(mainMenu, ng::standard_menu::Edit);

        app.action_file_new().triggered([&]()
        {
            neogui::new_project_dialog dialog{ mainWindow };
            if (dialog.exec() == ng::dialog_result::Accepted)
            {
                // todo
            }
        });

        ng::toolbar toolbar{ mainWindow.toolbar_layout() };
        toolbar.set_button_image_extents(ng::size{ 16.0_dip, 16.0_dip });
        toolbar.add_action(app.action_file_new());
        toolbar.add_action(app.action_file_open());
        toolbar.add_action(app.action_file_save());
        toolbar.add_separator();
        toolbar.add_action(app.action_undo());
        toolbar.add_action(app.action_redo());
        toolbar.add_separator();
        toolbar.add_action(app.action_cut());
        toolbar.add_action(app.action_copy());
        toolbar.add_action(app.action_paste());

        ng::status_bar statusBar{ mainWindow.status_bar_layout() };

        ng::horizontal_layout workspaceLayout{ mainLayout };
        ng::view_container workspace{ workspaceLayout };

        ng::texture backgroundTexture1{ ng::image{ ":/DesignStudio/resources/neoGFX.png" } };
        ng::texture backgroundTexture2{ ng::image{ ":/DesignStudio/resources/logo_i42.png" } };

        workspace.view_stack().Painting([&](ng::i_graphics_context& aGc)
        {
            auto const& cr = workspace.view_stack().client_rect();
            aGc.draw_texture(
                ng::point{ (cr.extents() - backgroundTexture1.extents()) / 2.0 },
                backgroundTexture1,
                ng::color::White.with_alpha(32));
            aGc.draw_texture(
                ng::rect{ ng::point{ cr.bottom_right() - backgroundTexture2.extents() / 2.0 }, backgroundTexture2.extents() / 2.0 },
                backgroundTexture2,
                ng::color::White.with_alpha(32));
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

