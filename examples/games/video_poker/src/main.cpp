#include <video_poker/video_poker.hpp>

#include <neolib/neolib.hpp>
#include <neolib/app/i_power.hpp>

#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/game/canvas.hpp>
#include <neogfx/game/game_world.hpp>

#include <video_poker/poker.hpp>
#include <video_poker/table.hpp>

namespace ng = neogfx;
using namespace ng::unit_literals;

int main(int argc, char* argv[])
{
    ng::app app(argc, argv, "neoGFX Example Game - Video Poker");

    try
    {
        app.set_default_window_icon(ng::image(":/video_poker/resources/icon.png"));
        app.change_style("Light").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Dark").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));

        (void)ng::font::load_from_file(":/video_poker/resources/Audiowide-Regular.ttf");
        (void)ng::font::load_from_file(":/video_poker/resources/MeowScript-Regular.ttf");
        (void)ng::font::load_from_file(":/video_poker/resources/GoogleSansFlex_72pt-ExtraBold.ttf");
        (void)ng::font::load_from_file(":/video_poker/resources/SyneMono-Regular.ttf");

        std::optional<ng::window> windowObject;
        if (!app.program_options().full_screen())
            windowObject.emplace(ng::window_style::Default | ng::window_style::SizeToContents);
        else
            windowObject.emplace(ng::video_mode{ *app.program_options().full_screen() });

        auto& window = *windowObject;

        video_poker::table table{ window.client_layout() };

        window.center_on_parent();

        table.set_background_opacity(0.0);
        table.parent().set_background_opacity(0.0);
        window.PaintBackground([&](ng::i_graphics_context& aGc)
            {
                aGc.fill_rect(window.client_rect(), ng::gradient{ ng::color::Blue.darker(0x80), ng::color::Blue.darker(0xE0) });
                window.PaintBackground.accept();
            });

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

