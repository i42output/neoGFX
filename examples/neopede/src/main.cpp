#include <neogfx/neogfx.hpp>
#include <neogfx/app.hpp>
#include <neogfx/window.hpp>
#include <neogfx/vertical_layout.hpp>
#include <neogfx/sprite.hpp>
#include <neogfx/sprite_plane.hpp>
#include <neogfx/image.hpp>

namespace ng = neogfx;

void create_game(ng::i_layout& aLayout);

int main()
{
	ng::app app("neopede - neoGFX Sample Game");
	ng::window window(800, 800);
	ng::vertical_layout layout0(window);
	create_game(layout0);
	return app.exec();
}