#include <neolib/neolib.hpp>
#include <neolib/app/i_power.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/game/canvas.hpp>
#include <neogfx/game/game_world.hpp>
#include <chess/move_validator.hpp>
#include <chess/board.hpp>
#include <chess/human.hpp>
#include <chess/default_player_factory.hpp>

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

        ng::action newGame{ "New Game"_t, ":/chess/resources/power.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action openGame{ "Open Game"_t, ":/chess/resources/folder-open-outline.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action saveGame{ "Save Game"_t, ":/chess/resources/content-save-outline.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action undoMove{ "Undo Move"_t, ":/chess/resources/undo-variant.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action redoMove{ "Redo Move"_t, ":/chess/resources/redo-variant.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action play{ "Resume/Play"_t, ":/chess/resources/play.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action stop{ "Stop"_t, ":/chess/resources/stop.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action resign{ "Resign"_t, ":/chess/resources/handshake-outline.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action suggestMove{ "Suggest Move"_t, ":/chess/resources/lifebuoy.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action connect{ "Connect"_t, ":/chess/resources/lan-connect.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action disconnect{ "Disconnect"_t, ":/chess/resources/lan-disconnect.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action settings{ "Settings"_t, ":/chess/resources/tune-vertical.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };
        ng::action about{ "About"_t, ":/chess/resources/help-circle-outline.png", window.dpi_scale(1.0), ng::texture_sampling::NormalMipmap };

        saveGame.enable(false);
        disconnect.enable(false);

        ng::horizontal_layout ourToolbarLayout{ window.toolbar_layout() };
        ourToolbarLayout.set_size_policy(ng::size_constraint::Expanding, ng::size_constraint::Minimum);
        ourToolbarLayout.add_spacer();
        ng::toolbar toolbar{ ourToolbarLayout };
        toolbar.add_action(newGame);
        toolbar.add_action(openGame);
        toolbar.add_action(saveGame);
        toolbar.add_separator();
        toolbar.add_action(connect);
        toolbar.add_action(disconnect);
        toolbar.add_separator();
        toolbar.add_action(resign);
        toolbar.add_action(suggestMove);
        toolbar.add_separator();
        toolbar.add_action(undoMove);
        toolbar.add_action(redoMove);
        toolbar.add_action(play);
        toolbar.add_action(stop);
        toolbar.add_separator();
        toolbar.add_action(settings);
        toolbar.add_separator();
        toolbar.add_action(about);
        ourToolbarLayout.add_spacer();

        auto update_toolbar = [&]()
        {
            auto const scale = std::min(1.0, window.extents().cx / 800.0_dip);
            for (uint32_t i = 0u; i < toolbar.button_count(); ++i)
                if (!toolbar.button(i).action().is_separator())
                    toolbar.button(i).set_transformation(ng::mat33
                        {{ scale, 0.0, 0.0 },
                         { 0.0, scale, 0.0 },
                         { 0.0, 0.0, 1.0 }});
        };
        update_toolbar();

        window.SizeChanged(update_toolbar);

        chess::move_validator moveValidator;
        chess::gui::board board{ window.client_layout(), moveValidator };

        chess::default_player_factory playerFactory;
        board.new_game(playerFactory, chess::player_type::AI, chess::player_type::AI);

        auto board_changed = [&]()
        {
            undoMove.enable(board.can_undo());
            redoMove.enable(board.can_redo());
            play.enable(board.can_play());
            stop.enable(board.can_stop());
        };

        board.changed(board_changed);
        board_changed();

        undoMove.Triggered([&]() { board.undo(); });
        redoMove.Triggered([&]() { board.redo(); });
        play.Triggered([&]() { board.play(); });
        stop.Triggered([&]() { board.stop(); });

        ng::status_bar statusBar{ window };

        return app.exec();
    }
    catch (std::exception& e)
    {
        app.thread().halt();
        ng::service<ng::debug::logger>() << "neogfx::app::exec: terminating with exception: " << e.what() << ng::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        app.thread().halt();
        ng::service<ng::debug::logger>() << "neogfx::app::exec: terminating with unknown exception" << ng::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
        std::exit(EXIT_FAILURE);
    }
}

