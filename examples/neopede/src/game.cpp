#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/game/sprite.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include <neogfx/game/text.hpp>

namespace ng = neogfx;

const uint8_t sSpaceshipImagePattern[9][9]
{
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 2, 1, 0, 0, 0 },
	{ 0, 0, 0, 1, 2, 1, 0, 0, 0 },
	{ 0, 0, 1, 2, 2, 2, 1, 0, 0 },
	{ 0, 0, 1, 2, 2, 2, 1, 0, 0 },
	{ 0, 1, 1, 2, 2, 2, 1, 1, 0 },
	{ 0, 1, 0, 1, 1, 1, 0, 1, 0 },
	{ 0, 1, 0, 0, 0, 0, 0, 1, 0 },
	{ 0, 1, 0, 0, 0, 0, 0, 1, 0 },
};

class bullet : public ng::sprite
{
public:
	bullet(const ng::i_sprite& aParent, ng::angle aAngle) : 
		ng::sprite{ aParent.container(), ng::colour{ rand() % 160 + 96, rand() % 160 + 96, rand() % 160 + 96 } }
	{
		shape::set_size(ng::size{ 3.0, 3.0 });
		ng::vec3 relativePos = aParent.physics().origin();
		relativePos[1] += 18.0;
		auto tm = aParent.transformation_matrix();
		physics().set_position(aParent.physics().position() + tm * relativePos);
		physics().set_mass(0.016);
		physics().set_angle_radians(aParent.physics().angle_radians() + ng::vec3{0.0, 0.0, ng::to_rad(aAngle)});
		physics().set_velocity(transformation_matrix() * ng::vec3{0.0, 360.0, 0.0} + aParent.physics().velocity());
	}
};

void create_game(ng::i_layout& aLayout)
{
	auto spritePlane = std::make_shared<ng::sprite_plane>();
	aLayout.add_item(spritePlane);
	spritePlane->set_font(ng::font(spritePlane->font(), ng::font::Bold, 28));
	spritePlane->set_background_colour(ng::colour::Black);
	spritePlane->enable_z_sorting(true);
	for (uint32_t i = 0; i < 1000; ++i)
		spritePlane->add_shape(std::make_shared<ng::rectangle>(
			*spritePlane,
			ng::vec3{static_cast<ng::scalar>(std::rand() % 800), static_cast<ng::scalar>(std::rand() % 800), -(static_cast<ng::scalar>(std::rand() % 32))},
			ng::size{static_cast<ng::scalar>(std::rand() % 64), static_cast<ng::scalar>(std::rand() % 64)},
			ng::colour(std::rand() % 64, std::rand() % 64, std::rand() % 64).lighter(0x40)));
	//spritePlane->set_uniform_gravity();
	//spritePlane->set_gravitational_constant(0.0);
	//spritePlane->create_earth();
	spritePlane->reserve(10000);
	auto& spaceshipSprite = spritePlane->create_sprite(ng::image(sSpaceshipImagePattern, { {0, ng::colour()}, {1, ng::colour::LightGoldenrod}, {2, ng::colour::DarkGoldenrod4} }));
	spaceshipSprite.physics().set_mass(1.0);
	spaceshipSprite.set_size(ng::size(36.0, 36.0));
	spaceshipSprite.set_position_3D(ng::vec3(400.0, 18.0, 1.0));
	auto shipInfo = std::make_shared<ng::text>(*spritePlane, ng::vec3{}, "", ng::font("SnareDrum One NBP", "Regular", 24.0), ng::colour::White);
	shipInfo->set_border(1.0);
	shipInfo->set_margins(ng::margins(2.0));
	shipInfo->set_buddy(spaceshipSprite, ng::vec3{18.0, 18.0, 0.0});
	spritePlane->add_shape(shipInfo);
	spritePlane->sprites_painted([spritePlane](ng::graphics_context& aGraphicsContext)
	{
		aGraphicsContext.draw_shape(ng::rectangle{ *spritePlane, ng::vec3{ 250.0, 250.0, 0.0 }, ng::size{ 25.0, 25.0 } }, 
			ng::pen{ ng::colour::Goldenrod, 3.0 }, ng::colour::DarkGoldenrod4);
		aGraphicsContext.draw_text(ng::point(0.0, 0.0), "Hello, World!", spritePlane->font(), ng::colour::White);
	});
	auto bullets = std::make_shared<std::vector<bullet>>();
	spritePlane->applying_physics([spritePlane, &spaceshipSprite, shipInfo, bullets](ng::sprite_plane::step_time_interval aPhysicsStepTime)
	{
		const auto& keyboard = ng::app::instance().keyboard();
		spaceshipSprite.physics().set_acceleration({  
			keyboard.is_key_pressed(ng::ScanCode_RIGHT) ? 16.0 : keyboard.is_key_pressed(ng::ScanCode_LEFT) ? -16.0 : 0.0,
			keyboard.is_key_pressed(ng::ScanCode_UP) ? 16.0 : keyboard.is_key_pressed(ng::ScanCode_DOWN) ? -16.0 : 0.0});
		if (keyboard.is_key_pressed(ng::ScanCode_X))
			spaceshipSprite.physics().set_spin_degrees(30.0);
		else if (keyboard.is_key_pressed(ng::ScanCode_Z))
			spaceshipSprite.physics().set_spin_degrees(-30.0);
		else
			spaceshipSprite.physics().set_spin_degrees(0.0);
		bullets->reserve(100000);
		if (keyboard.is_key_pressed(ng::ScanCode_SPACE) && bullets->size() < bullets->capacity() - 7)
		{
			if ((aPhysicsStepTime / 10) % 2 == 0 && (aPhysicsStepTime / 100) % 2 == 0)
			{
				for (double a = -30.0; a <= 30.0; a += 10.0)
				{
					bullets->emplace_back(spaceshipSprite, a);
					spritePlane->add_sprite(bullets->back());
				}
			}
		}
		
		std::ostringstream oss;
		oss << "VELOCITY:  " << spaceshipSprite.physics().velocity().magnitude() << " m/s" << "\n";
		oss << "ACCELERATION:  " << spaceshipSprite.physics().acceleration().magnitude() << " m/s/s";
		shipInfo->set_value(oss.str());
	});
}