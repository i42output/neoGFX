/*
neogfx C++ App/Game Engine - Examples - MOD Tracker
Copyright(C) 2026 Leigh Johnston

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

#include <mod_tracker/mod_tracker.hpp>

#include <neolib/neolib.hpp>

#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/hid/i_surface_manager.hpp>

#include <mod_tracker/tracker.hpp>

using namespace ng::unit_literals;

int main(int argc, char* argv[])
{
    ng::app app(argc, argv, "neoGFX Example - MOD Tracker");

    try
    {
        app.change_style("Light").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Dark").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));

        ng::window window{ ng::size{ 1120_dip, 720_dip } };

        mod_tracker::tracker tracker{ window.client_layout() };

        // n.b. false: the default also shrinks the window to its minimum size
        window.center_on_parent(false);

        return app.exec();
    }
    catch (std::exception& e)
    {
        app.thread().halt();
        ng::service<ng::debug::logger>() << "ng::app::exec: terminating with exception: " << e.what() << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination"_t : "Abnormal Program Termination - "_t + app.name(), "main: terminating with exception: "_t + e.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        app.thread().halt();
        ng::service<ng::debug::logger>() << "ng::app::exec: terminating with unknown exception" << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination"_t : "Abnormal Program Termination - "_t + app.name(), "main: terminating with unknown exception"_t);
        std::exit(EXIT_FAILURE);
    }
}
