// main.cpp
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

#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/project_manager.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/settings.hpp>
#include "main_window.hpp"

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

    app.change_style("Dark");
    app.current_style().set_spacing(ng::size{ 4.0 });

    std::cout << "Loading element libraries..." << std::endl;
    app.plugin_manager().load_plugins();
    for (auto const& plugin : app.plugin_manager().plugins())
        std::cout << "Element library '" << plugin->name() << "' loaded." << std::endl;

    try
    {
        ds::settings settings;
        ds::project_manager pm;
        ds::main_window_ex mainWindow{ app, settings, pm };

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

