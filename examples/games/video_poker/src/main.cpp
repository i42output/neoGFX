#include <neolib/neolib.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include <video_poker/poker.hpp>
#include <video_poker/table.hpp>

namespace ng = neogfx;

int main(int argc, char* argv[])
{
	ng::app app(argc, argv, "neoGFX Example Game: Video Poker");

	try
	{
		app.set_default_window_icon(ng::image(":/video_poker/resources/icon.png"));
		app.change_style("Default").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
		app.change_style("Slate").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));

		ng::window window(ng::size{ 768, 688 });

		ng::sprite_plane spritePlane{ window.client_layout() };
		spritePlane.set_logical_coordinate_system(neogfx::logical_coordinate_system::AutomaticGui);
		ng::vertical_layout spritePlaneLayout{ spritePlane };
		
		video_poker::table table{ spritePlaneLayout, spritePlane };

		// todo

		video_poker::poker_hand test = video_poker::poker_hand::HighCard;
		int go = 0;

		for (;;)
		{
			video_poker::deck deck;
			deck.shuffle();
			video_poker::hand hand;
			deck.deal_hand(hand);
			auto result = video_poker::to_poker_hand(hand);

			if (result != test)
				continue;

			if (++go == 3)
			{
				if (test == video_poker::poker_hand::RoyalFlush)
					break;
				test = static_cast<video_poker::poker_hand>(static_cast<uint32_t>(test) + 1);
				go = 0;
			}

			std::string resultDisplay;
			for (uint32_t i = 0; i < video_poker::hand::game_traits::hand_size; ++i)
				resultDisplay += (hand.card_at(i).to_string() + "\n");

			resultDisplay += ("\n" + video_poker::to_string(result));

			if (ng::message_box::information(window, "Game Result", resultDisplay, ng::standard_button::Ok | ng::standard_button::Close) == ng::standard_button::Close)
				break;
		}

		return app.exec();
	}
	catch (std::exception& e)
	{
		app.halt();
		std::cerr << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
		app.surface_manager().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
		std::exit(EXIT_FAILURE);
	}
	catch (...)
	{
		app.halt();
		std::cerr << "neogfx::app::exec: terminating with unknown exception" << std::endl;
		app.surface_manager().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
		std::exit(EXIT_FAILURE);
	}
}

