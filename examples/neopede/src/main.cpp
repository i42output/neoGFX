#include <neogfx/neogfx.hpp>
#include <neogfx/app.hpp>
#include <neogfx/window.hpp>
#include <neogfx/vertical_layout.hpp>
#include <neogfx/sprite.hpp>
#include <neogfx/sprite_plane.hpp>
#include <neogfx/image.hpp>

#include <neogfx/numerical.hpp>

namespace ng = neogfx;

const uint8_t sSpaceShip[8][8]
{
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 0, 0 },
	{ 0, 1, 1, 0, 0, 1, 1, 0 },
	{ 0, 1, 0, 1, 1, 0, 1, 0 },
	{ 0, 1, 0, 0, 0, 0, 1, 0 },
};

int main()
{
	ng::app app("neopede - neoGFX Sample Application");
	ng::window window(800, 800);
	ng::vertical_layout layout0(window);
	ng::sprite_plane spritePlane(layout0);
	spritePlane.set_background_colour(ng::colour::Black);
	auto& sprite = spritePlane.create_sprite(ng::image(sSpaceShip, {{0, ng::colour()}, {1, ng::colour::Goldenrod}}));
	sprite.set_size(ng::size(32.0, 32.0));
	return app.exec();
}