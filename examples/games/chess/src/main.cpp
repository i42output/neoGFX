#include <neolib/neolib.hpp>
#include <neolib/app/i_power.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/game/canvas.hpp>
#include <neogfx/game/game_world.hpp>
#include <chess/move_validator.hpp>
#include <chess/board.hpp>
#include <chess/human.hpp>
#include <chess/human.hpp>

namespace ng = neogfx;
using namespace ng::unit_literals;

int main(int argc, char* argv[])
{
    ng::app app(argc, argv, "neoGFX Sample Application - Chess");

    try
    {
        //app.set_default_window_icon(ng::image(":/chess/resources/icon.png"));
        app.change_style("Light").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Dark").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));

        std::optional<ng::window> windowObject;
        if (!app.program_options().full_screen())
            windowObject.emplace(ng::size{ 720_dip, 720_dip });
        else
            windowObject.emplace(ng::video_mode{ *app.program_options().full_screen() });

        auto& window = *windowObject;
        chess::move_validator moveValidator;
        chess::gui::board board{ window.client_layout(), moveValidator };
        chess::human player1;
        chess::human player2;
        board.new_game(player1, player2);

        return app.exec();
    }
    catch (std::exception& e)
    {
        app.halt();
        ng::service<ng::debug::logger>() << "neogfx::app::exec: terminating with exception: " << e.what() << ng::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        app.halt();
        ng::service<ng::debug::logger>() << "neogfx::app::exec: terminating with unknown exception" << ng::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
        std::exit(EXIT_FAILURE);
    }
}

