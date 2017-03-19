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
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/view/view_container.hpp>

namespace ng = neogfx;

int main()
{
	ng::app app("neoGFX GUI Designer (neoGUI)");
	try
	{
		app.current_style().set_colour(ng::colour{ 64, 64, 64 });

		ng::window mainWindow(app.basic_services().desktop_rect() * ng::size{ 0.5, 0.5 });
		mainWindow.set_margins(ng::margins{});
		ng::vertical_layout mainLayout(mainWindow);
		mainLayout.set_margins(ng::margins{});
		ng::menu_bar mainMenu(mainLayout);

		auto& fileMenu = mainMenu.add_sub_menu("&File");
		fileMenu.add_action(app.action_file_new());
		fileMenu.add_action(app.action_file_open());
		fileMenu.add_separator();
		fileMenu.add_action(app.action_file_close());
		fileMenu.add_separator();
		fileMenu.add_action(app.action_file_save());
		fileMenu.add_separator();
		fileMenu.add_action(app.action_file_exit());

		auto& editMenu = mainMenu.add_sub_menu("&Edit");
		editMenu.add_action(app.action_undo());
		editMenu.add_action(app.action_redo());
		editMenu.add_separator();
		editMenu.add_action(app.action_cut());
		editMenu.add_action(app.action_copy());
		editMenu.add_action(app.action_paste());
		editMenu.add_action(app.action_delete());
		editMenu.add_separator();
		editMenu.add_action(app.action_select_all());

		ng::horizontal_layout workspaceLayout(mainLayout);
		ng::view_container workspace(workspaceLayout);

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

