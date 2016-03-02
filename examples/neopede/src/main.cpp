#include <neogfx/neogfx.hpp>
#include <neogfx/app.hpp>
#include <neogfx/window.hpp>
#include <neogfx/vertical_layout.hpp>
#include <neogfx/sprite.hpp>
#include <neogfx/sprite_plane.hpp>
#include <neogfx/image.hpp>

namespace ng = neogfx;

const uint8_t sSpaceshipImagePattern[8][8]
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
	ng::app app("neopede - neoGFX Sample Game");
	ng::window window(800, 800);
	ng::vertical_layout layout0(window);
	ng::sprite_plane spritePlane(layout0);
	spritePlane.set_background_colour(ng::colour::Black);
	auto& spaceshipSprite = spritePlane.create_sprite(ng::image(sSpaceshipImagePattern, { {0, ng::colour()}, {1, ng::colour::Goldenrod} }));
	spaceshipSprite.set_size(ng::size(32.0, 32.0));
	spaceshipSprite.set_position(ng::size(rand() % 800, rand() % 800));
	neolib::callback_timer timer(app, [&app, &spaceshipSprite](neolib::callback_timer& aTimer)
	{
		aTimer.again();
		if (app.keyboard().is_key_pressed(ng::ScanCode_UP))
			spaceshipSprite.set_acceleration({0.0, 1.0});
		else if (app.keyboard().is_key_pressed(ng::ScanCode_DOWN))
			spaceshipSprite.set_acceleration({0.0, -1.0});
		else
			spaceshipSprite.set_acceleration({0.0, 0.0});
		if (app.keyboard().is_key_pressed(ng::ScanCode_RIGHT))
			spaceshipSprite.set_spin_degrees(10.0);
		else if (app.keyboard().is_key_pressed(ng::ScanCode_LEFT))
			spaceshipSprite.set_spin_degrees(-10.0);
		else
			spaceshipSprite.set_spin_degrees(0.0);
	}, 250);
	return app.exec();
}