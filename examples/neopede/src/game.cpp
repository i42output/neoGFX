#include <neogfx/neogfx.hpp>
#include <neolib/random.hpp>
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

class missile : public ng::sprite
{
public:
	missile(ng::sprite_plane& aWorld, const ng::i_sprite& aParent, std::pair<uint32_t, ng::text>& aScore, std::shared_ptr<ng::texture> aExplosion, ng::angle aAngle) :
		ng::sprite{ ng::colour{ rand() % 160 + 96, rand() % 160 + 96, rand() % 160 + 96 } }, iWorld{ aWorld }, iScore(aScore), iExplosion(aExplosion)
	{
		set_collision_mask(1ull);
		shape::set_extents(ng::vec2{ 3.0, 3.0 });
		ng::vec3 relativePos = aParent.physics().origin();
		relativePos[1] += 18.0;
		auto tm = ng::without_translation(aParent.transformation_matrix());
		physics().set_position(aParent.physics().position() + *(tm * ng::vec4{ relativePos.x, relativePos.y, relativePos.z, 1.0 }).xyz);
		physics().set_mass(0.016);
		physics().set_angle_radians(aParent.physics().angle_radians() + ng::vec3{0.0, 0.0, ng::to_rad(aAngle)});
		physics().set_velocity(*(transformation_matrix() * ng::vec4{0.0, 360.0, 0.0, 0.0}).xyz + aParent.physics().velocity());
	}
public:
	const ng::object_type& type() const override
	{
		static ng::object_type sTypeId = neolib::make_uuid("F5B70B06-6B72-465B-9499-44EB994D2923");
		return sTypeId;
	}
	bool update(const optional_time_interval& aNow, const ng::vec3& aForce) override
	{
		bool updated = physical_object::update(aNow, aForce);
		if (updated && bounding_box_2d().intersection(iWorld.client_rect()).empty())
			destroy();
		return updated;
	}
	void collided(const i_physical_object& aOther) override
	{
		iScore.first += 250;
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(6) << iScore.first;
		iScore.second.set_value(oss.str());
		static boost::fast_pool_allocator<ng::sprite> alloc;
		auto explosion = std::allocate_shared<ng::sprite, boost::fast_pool_allocator<sprite>>(
			alloc, *iExplosion, ng::sprite::animation_info{ ng::point{}, ng::size{ 60.0, 60.0 }, 12, 0.040, false });
		explosion->set_collision_mask(1ull);
		static neolib::basic_random<double> r;
		explosion->set_position(position() + ng::vec3{ r.get(-10.0, 10.0), r.get(-10.0, 10.0), 1.0 });
		explosion->set_angle_degrees(ng::vec3{ 0.0, 0.0, r.get(360.0) });
		explosion->set_extents(ng::vec2{r.get(40.0, 80.0), r.get(40.0, 80.0)});
		iWorld.add_sprite(explosion);
		destroy();
	}
private:
	ng::sprite_plane& iWorld;
	std::pair<uint32_t, ng::text>& iScore;
	std::shared_ptr<ng::texture> iExplosion;
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
			ng::vec3{ static_cast<ng::scalar>(std::rand() % 800), static_cast<ng::scalar>(std::rand() % 800), -(static_cast<ng::scalar>(std::rand() % 32)) / 32.0 },
			ng::vec2{ static_cast<ng::scalar>(std::rand() % 64), static_cast<ng::scalar>(std::rand() % 64) },
			ng::colour{ std::rand() % 64, std::rand() % 64, std::rand() % 64 }.lighter(0x40)));
	//spritePlane->set_uniform_gravity();
	//spritePlane->set_gravitational_constant(0.0);
	//spritePlane->create_earth();
	spritePlane->reserve(10000);
	auto& spaceshipSprite = spritePlane->create_sprite(ng::image(sSpaceshipImagePattern, { {0, ng::colour()}, {1, ng::colour::LightGoldenrod}, {2, ng::colour::DarkGoldenrod4} }));
	spaceshipSprite.physics().set_collision_mask(1ull);
	spaceshipSprite.physics().set_mass(1.0);
	spaceshipSprite.set_extents(ng::size{ 36.0, 36.0 });
	spaceshipSprite.set_position(ng::vec3{ 400.0, 18.0, 0.0 });
	auto score = std::make_shared<std::pair<uint32_t, ng::text>>(0, ng::text{ *spritePlane, ng::vec3{}, "", ng::font("SnareDrum Two NBP", "Regular", 60.0), ng::colour::White });
	score->second.set_value("000000");
	score->second.set_position(ng::vec2{ 0.0, 0.0 });
	auto positionScore = [spritePlane, score]()
	{
		score->second.set_position(ng::point{ spritePlane->extents().cx - 256.0, spritePlane->extents().cy - 40 });
	};
	spritePlane->size_changed(positionScore);
	positionScore();
	spritePlane->add_shape(score->second);
	auto shipInfo = std::make_shared<ng::text>(*spritePlane, ng::vec3{}, "", ng::font("SnareDrum One NBP", "Regular", 24.0), ng::colour::White);
	shipInfo->set_border(1.0);
	shipInfo->set_margins(ng::margins(2.0));
	shipInfo->set_tag_of(spaceshipSprite, ng::vec3{18.0, 18.0, 0.0});
	spritePlane->add_shape(shipInfo);
	auto target = std::make_shared<ng::sprite>(ng::colour::Goldenrod);
	spritePlane->add_sprite(target);
	target->set_position(ng::vec3{ 250.0, 250.0, 0.0 });
	target->set_extents(ng::vec2{ 25.0, 25.0 });
	target->set_mass(1.0);
	target->set_spin_degrees(360.0);
	spritePlane->sprites_painted([spritePlane, target](ng::graphics_context& aGraphicsContext)
	{
		aGraphicsContext.draw_text(ng::point(0.0, 0.0), "Hello, World!", spritePlane->font(), ng::colour::White);
	});
	auto explosion = std::make_shared<ng::texture>(ng::image{ ":/test/resources/explosion.png" });
	spritePlane->applying_physics([spritePlane, &spaceshipSprite, score, shipInfo, explosion](ng::sprite_plane::step_time_interval aPhysicsStepTime)
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
		if (keyboard.is_key_pressed(ng::ScanCode_SPACE))
		{
			if ((aPhysicsStepTime / 10) % 2 == 0 && (aPhysicsStepTime / 100) % 2 == 0)
			{
				for (double a = -30.0; a <= 30.0; a += 10.0)
				{
					static boost::fast_pool_allocator<missile> alloc;
					spritePlane->add_sprite(std::allocate_shared<missile, boost::fast_pool_allocator<missile>>(alloc, *spritePlane, spaceshipSprite, *score, explosion, a));
				}
			}
		}
		
		std::ostringstream oss;
		oss << "VELOCITY:  " << spaceshipSprite.physics().velocity().magnitude() << " m/s" << "\n";
		oss << "ACCELERATION:  " << spaceshipSprite.physics().acceleration().magnitude() << " m/s/s";
		shipInfo->set_value(oss.str());
	});
}