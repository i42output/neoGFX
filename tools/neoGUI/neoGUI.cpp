// neoGUI.cpp : Defines the entry point for the console application.
/*
neogfx C++ GUI Library
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

#include <neogfx/app/app.hpp>
#include <fstream>
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

int main(int argc, char* argv[])
{
	ng::app app(argc, argv, "neoGFX GUI Designer (neoGUI)");
	try
	{
		app.rendering_engine().subpixel_rendering_on();
		app.current_style().palette().set_colour(ng::colour{ 64, 64, 64 });
		app.current_style().set_spacing(ng::size{ 4.0 });

		ng::window mainWindow{ app.basic_services().display().desktop_rect() * ng::size{ 0.5, 0.5 } };
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
		toolbar.set_button_image_extents(mainWindow.dpi_scale(ng::size{ 16.0, 16.0 }));
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

		workspace.view_stack().painting([&workspace](ng::graphics_context& aGc)
		{
			static ng::texture sBackgroundTexture1{ ng::image{ ":/neoGUI/resource/neoGFX.png" } };
			static ng::texture sBackgroundTexture2{ ng::image{ ":/neoGUI/resource/logo_i42.png" } };
			auto rc = workspace.view_stack().client_rect();
			aGc.draw_texture(
				ng::point{ (rc.extents() - sBackgroundTexture1.extents()) / 2.0 },
				sBackgroundTexture1,
				ng::colour::White.with_alpha(32));
			aGc.draw_texture(
				ng::point{ rc.bottom_right() - sBackgroundTexture2.extents() },
				sBackgroundTexture2,
				ng::colour::White.with_alpha(32));
		});

//		ng::css css{"test.css"};

		return app.exec();
	}
	catch (std::exception& e)
	{
		app.halt();
		std::cerr << "neoGUI: terminating with exception: " << e.what() << std::endl;
		app.surface_manager().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
		std::exit(EXIT_FAILURE);
	}
	catch (...)
	{
		app.halt();
		std::cerr << "neoGUI: terminating with unknown exception" << std::endl;
		app.surface_manager().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
		std::exit(EXIT_FAILURE);
	}
}

