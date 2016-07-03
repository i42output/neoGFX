#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/game/sprite.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include <neogfx/gfx/image.hpp>

namespace ng = neogfx;

void create_game(ng::i_layout& aLayout);

int main()
{
	ng::app app("neopede - neoGFX Sample Game");
	ng::window window(ng::video_mode{ 800, 800 });
	window.set_margins(ng::margins{});
	ng::vertical_layout layout0(window);
	layout0.set_margins(ng::margins{});
	create_game(layout0);
	return app.exec();
}