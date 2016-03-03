#include <neogfx/neogfx.hpp>
#include <neogfx/app.hpp>
#include <neogfx/i_layout.hpp>
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

void create_game(ng::i_layout& aLayout)
{
	auto spritePlane = std::make_shared<ng::sprite_plane>();
	aLayout.add_widget(spritePlane);
	spritePlane->set_background_colour(ng::colour::Black);
	auto& spaceshipSprite = spritePlane->create_sprite(ng::image(sSpaceshipImagePattern, { {0, ng::colour()}, {1, ng::colour::Goldenrod} }));
	spaceshipSprite.set_size(ng::size(32.0, 32.0));
	spaceshipSprite.set_position(ng::size(rand() % 800, rand() % 800));
	spritePlane->sprites_updating([&spaceshipSprite]()
	{
		const auto& keyboard = ng::app::instance().keyboard();
		if (keyboard.is_key_pressed(ng::ScanCode_UP))
			spaceshipSprite.set_acceleration({0.0, 1.0});
		else if (keyboard.is_key_pressed(ng::ScanCode_DOWN))
			spaceshipSprite.set_acceleration({0.0, -1.0});
		else
			spaceshipSprite.set_acceleration({0.0, 0.0});
		if (keyboard.is_key_pressed(ng::ScanCode_RIGHT))
			spaceshipSprite.set_spin_degrees(10.0);
		else if (keyboard.is_key_pressed(ng::ScanCode_LEFT))
			spaceshipSprite.set_spin_degrees(-10.0);
		else
			spaceshipSprite.set_spin_degrees(0.0);
	});
}