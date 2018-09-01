#include <neogfx/neogfx.hpp>
#include <boost/format.hpp>
#include <neolib/random.hpp>
#include <neolib/singleton.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/game/chrono.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/ecs_canvas.hpp>
#include <neogfx/game/rigid_body.hpp>
#include <neogfx/game/sprite.hpp>
#include <neogfx/game/text.hpp>
#include <neogfx/game/collider.hpp>

namespace ng = neogfx;
using namespace neolib::stdint_suffix;
/*
void create_target(ng::ecs_canvas& aWorld)
{
	auto target = std::make_shared<ng::sprite>(ng::colour::from_hsl(static_cast<ng::scalar>(std::rand() % 360), 1.0, 0.75));
	aWorld.add_sprite(target);
	target->set_collision_mask(0x2ull);
	target->set_position(ng::vec3{ static_cast<ng::scalar>(std::rand() % 800), static_cast<ng::scalar>(std::rand() % 800), 0.0 });
	auto w = static_cast<ng::scalar>(std::rand() % 40) + 10.0;
	target->set_extents(ng::vec3{ w, w });
	target->set_mass(1.0);
	target->set_spin_degrees((std::rand() % 180 + 180) * (std::rand() % 2 == 0 ? 1.0 : -1.0));
}

class missile : public ng::sprite
{
public:
	missile(ng::ecs_canvas& aWorld, const ng::i_sprite& aParent, std::pair<uint32_t, ng::text>& aScore, std::shared_ptr<ng::texture> aExplosion, ng::angle aAngle) :
		ng::sprite{ ng::colour{ rand() % 160 + 96, rand() % 160 + 96, rand() % 160 + 96 } }, iWorld{ aWorld }, iScore(aScore), iExplosion(aExplosion)
	{
		set_collision_mask(0x1ull);
		shape::set_extents(ng::vec3{ 3.0, 3.0 });
		ng::vec3 relativePos = aParent.origin();
		relativePos[1] += 18.0;
		auto tm = ng::without_translation(aParent.transformation_matrix());
		set_position(aParent.position() + ~(tm * ng::vec4{ relativePos.x, relativePos.y, relativePos.z, 1.0 }).xyz);
		set_mass(0.016);
		set_angle_radians(aParent.angle_radians() + ng::vec3{ 0.0, 0.0, ng::to_rad(aAngle) });
		set_velocity(~(transformation_matrix() * ng::vec4{ 0.0, 360.0, 0.0, 0.0 }).xyz + aParent.velocity());
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
			kill();
		return updated;
	}
	void collided(i_collidable_object& aOther) override
	{
		auto& other = aOther.as_physical_object();
		iScore.first += 250;
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(6) << iScore.first;
		iScore.second.set_value(oss.str());
		static boost::fast_pool_allocator<ng::sprite> alloc;
		auto explosion = std::allocate_shared<ng::sprite, boost::fast_pool_allocator<sprite>>(
			alloc, *iExplosion, ng::sprite::animation_info{ { { ng::point{}, ng::size{ 60.0, 60.0 } } }, 12, 0.040, false });
		explosion->set_collision_mask(0x1ull);
		static neolib::basic_random<double> r;
		explosion->set_position(position() + ng::vec3{ r.get(-10.0, 10.0), r.get(-10.0, 10.0), -0.1 });
		explosion->set_angle_degrees(ng::vec3{ 0.0, 0.0, r.get(360.0) });
		explosion->set_extents(ng::vec3{ r.get(40.0, 80.0), r.get(40.0, 80.0) });
		iWorld.add_sprite(explosion);
		kill();
		other.kill();
		if (other.collision_mask() != 0x4ull)
			create_target(iWorld);
	}
private:
	ng::ecs_canvas& iWorld;
	std::pair<uint32_t, ng::text>& iScore;
	std::shared_ptr<ng::texture> iExplosion;
};

void create_game(ng::i_layout& aLayout)
{
	auto& spritePlane = aLayout.add(std::make_shared<ng::ecs_canvas>());
	spritePlane.set_font(ng::font(spritePlane.font(), ng::font::Bold, 28));
	spritePlane.set_background_colour(ng::colour::Black);
	spritePlane.enable_z_sorting(true);
#ifdef NDEBUG
	for (uint32_t i = 0; i < 1000; ++i)
#else
	for (uint32_t i = 0; i < 100; ++i)
#endif
		spritePlane.add_shape(std::make_shared<ng::rectangle>(
			ng::vec3{ static_cast<ng::scalar>(std::rand() % 800), static_cast<ng::scalar>(std::rand() % 800), -1.0 + 0.5 * (static_cast<ng::scalar>(std::rand() % 32)) / 32.0 },
			ng::vec2{ static_cast<ng::scalar>(std::rand() % 64), static_cast<ng::scalar>(std::rand() % 64) },
			ng::colour{ std::rand() % 64, std::rand() % 64, std::rand() % 64 }.lighter(0x40)));
	//spritePlane.set_uniform_gravity();
	//spritePlane.set_gravitational_constant(0.0);
	//spritePlane.create_earth();
	spritePlane.reserve(10000);

	const char* spaceshipSpriteImagePattern
	{
		"[9,9]"
		"{0,paper}"
		"{1,ink1}"
		"{2,ink2}"

		"000010000"
		"000121000"
		"000121000"
		"001222100"
		"001222100"
		"011222110"
		"010111010"
		"010000010"
		"010000010"
	};
	
	auto& spaceshipSprite = spritePlane.create_sprite(
		ng::image{ 
			spaceshipSpriteImagePattern,
			{ { "paper", ng::colour{} }, { "ink1", ng::colour::LightGoldenrod }, { "ink2", ng::colour::DarkGoldenrod4 } } });

	spaceshipSprite.set_collision_mask(0x1ull);
	spaceshipSprite.set_mass(1.0);
	spaceshipSprite.set_extents(ng::size{ 36.0, 36.0 });
	spaceshipSprite.set_position(ng::vec3{ 400.0, 18.0, 0.0 });

	auto score = std::make_shared<std::pair<uint32_t, ng::text>>(0, ng::text{ spritePlane, ng::vec3{}, "", ng::font("SnareDrum Two NBP", "Regular", 60.0), ng::text_appearance{ ng::colour::White, ng::text_effect{ ng::text_effect::Outline, ng::colour::Black } } });
	score->second.set_value("000000");
	score->second.set_position(ng::vec3{ 0.0, 0.0, 1.0 });
	auto positionScore = [&spritePlane, score]()
	{
		score->second.set_position(ng::vec3{ spritePlane.extents().cx - score->second.extents().x, spritePlane.extents().cy - score->second.extents().y, 1.0 });
	};
	spritePlane.size_changed(positionScore);
	positionScore();
	spritePlane.add_shape(score->second);
	auto shipInfo = std::make_shared<ng::text>(spritePlane, ng::vec3{}, "", ng::font("SnareDrum One NBP", "Regular", 24.0), ng::colour::White);
	shipInfo->set_border(1.0);
	shipInfo->set_margins(ng::margins(2.0));
	shipInfo->set_tag_of(spaceshipSprite, ng::vec3{ 18.0, 18.0, 1.0 });
	spritePlane.add_shape(shipInfo);
#ifdef NDEBUG
	for (int i = 0; i < 50; ++i)
#else
	for (int i = 0; i < 5; ++i)
#endif
	{
		create_target(spritePlane);
	}
	auto debugInfo = std::make_shared<ng::text>(spritePlane, ng::vec3{ 0.0, 132.0, 1.0 }, "", spritePlane.font().with_size(spritePlane.font().size() * 0.5), ng::text_appearance{ ng::colour::Orange.with_lightness(0.9), ng::text_effect{ ng::text_effect::Outline, ng::colour::Black } });
	spritePlane.add_shape(debugInfo);
	spritePlane.sprites_painted([&spritePlane](ng::graphics_context& aGraphicsContext)
	{
		aGraphicsContext.draw_text(ng::point{ 0.0, 0.0 }, "Hello, World!", spritePlane.font().with_style(ng::font_info::Underline), ng::colour::White);
		if (ng::app::instance().keyboard().is_key_pressed(ng::ScanCode_C))
			spritePlane.collision_tree_2d().visit_aabbs([&aGraphicsContext](const neogfx::aabb_2d& aAabb)
			{
				ng::rect aabb{ ng::point{ aAabb.min }, ng::point{ aAabb.max } };
				aGraphicsContext.draw_rect(aabb, ng::pen{ ng::colour::Blue });
			});
	});

	auto explosion = std::make_shared<ng::texture>(ng::image{ ":/test/resources/explosion.png" });

	~~~~spritePlane.applying_physics([&spritePlane, &spaceshipSprite, score, shipInfo, explosion](ng::ecs_canvas::step_time_interval aPhysicsStepTime)
	{
		const auto& keyboard = ng::app::instance().keyboard();
		spaceshipSprite.set_acceleration(ng::vec3{
			keyboard.is_key_pressed(ng::ScanCode_RIGHT) ? 16.0 : keyboard.is_key_pressed(ng::ScanCode_LEFT) ? -16.0 : 0.0,
			keyboard.is_key_pressed(ng::ScanCode_UP) ? 16.0 : keyboard.is_key_pressed(ng::ScanCode_DOWN) ? -16.0 : 0.0 });
		if (keyboard.is_key_pressed(ng::ScanCode_X))
			spaceshipSprite.set_spin_degrees(30.0);
		else if (keyboard.is_key_pressed(ng::ScanCode_Z))
			spaceshipSprite.set_spin_degrees(-30.0);
		else
			spaceshipSprite.set_spin_degrees(0.0);
		static bool sExtraFire = false;
		if (keyboard.is_key_pressed(ng::ScanCode_SPACE) || sExtraFire)
		{
			auto stepTime_ms = static_cast<decltype(aPhysicsStepTime)>(ng::chrono::to_milliseconds(ng::chrono::flicks{ aPhysicsStepTime }));
			static auto sLastTime_ms = stepTime_ms;
			auto sinceLastTime_ms = stepTime_ms - sLastTime_ms;
			if (sinceLastTime_ms < 100)
			{
				if (sinceLastTime_ms / 10 % 2 == 0)
				{
					sExtraFire = false;
					for (double a = -30.0; a <= 30.0; a += 10.0)
					{
						static boost::fast_pool_allocator<missile> alloc;
						spritePlane.add_sprite(std::allocate_shared<missile, boost::fast_pool_allocator<missile>>(alloc, spritePlane, spaceshipSprite, *score, explosion, a));
					}
				}
			}
			else
				sExtraFire = true;
			if (sinceLastTime_ms > 200)
				sLastTime_ms = stepTime_ms;
		}
		if (keyboard.is_key_pressed(ng::ScanCode_D))
			spritePlane.enable_dynamic_update(true);
		else if (keyboard.is_key_pressed(ng::ScanCode_F))
			spritePlane.enable_dynamic_update(false);

		std::ostringstream oss;
		oss << "VELOCITY:  " << spaceshipSprite.velocity().magnitude() << " m/s" << "\n";
		oss << "ACCELERATION:  " << spaceshipSprite.acceleration().magnitude() << " m/s/s";
		shipInfo->set_value(oss.str());
	});

	~~~~spritePlane.physics_applied([debugInfo, &spritePlane](ng::ecs_canvas::step_time_interval)
	{
		debugInfo->set_value(
			"Objects: " + boost::lexical_cast<std::string>(spritePlane.objects().size()) + "\n" +
			"Collision tree (quadtree) size: " + boost::lexical_cast<std::string>(spritePlane.collision_tree_2d().count()) + "\n" +
			"Collision tree (quadtree) depth: " + boost::lexical_cast<std::string>(spritePlane.collision_tree_2d().depth()) + "\n" +
			"Collision tree (quadtree) update type: " + (spritePlane.dynamic_update_enabled() ? "dynamic" : "full") + "\n" +
			"Physics update time: " + boost::str(boost::format("%.6f") % spritePlane.update_time()) + " s");
	});

	spritePlane.mouse_event([&spritePlane, &spaceshipSprite](const neogfx::mouse_event& e)
	{
		if ((e.type() == neogfx::mouse_event_type::ButtonPressed || 
			e.type() == neogfx::mouse_event_type::Moved) && (e.mouse_button() & neogfx::mouse_button::Left) == neogfx::mouse_button::Left)
		{
			auto newPos = ng::point{ e.position() - spritePlane.origin() };
			newPos.y = spritePlane.extents().cy - newPos.y;
			spaceshipSprite.set_position(newPos.to_vec3());
		}
	});
}
*/

namespace archetypes
{
	const ng::game::entity_archetype spaceship
	{
		neolib::uuid{ 0x26730030, 0xa999, 0x4d99, 0xa7ad,{ 0x80, 0x89, 0x69, 0x23, 0x95, 0xf9 } },
		"Spaceship",
		{ ng::game::rigid_body::meta::id(), ng::game::box_collider::meta::id() }
	};

	const ng::game::entity_archetype asteroid
	{
		neolib::uuid{ 0x8283650f, 0x6e59, 0x4f0f, 0x8ae0, { 0x1d, 0x99, 0xeb, 0xf2, 0x49, 0xe0 } },
		"Asteroid",
		{ ng::game::rigid_body::meta::id(), ng::game::box_collider::meta::id() }
	};
}

void create_game(ng::i_layout& aLayout)
{
	auto& canvas = aLayout.add(std::make_shared<ng::game::ecs_canvas>());
	auto& ecs = canvas.ecs();
	canvas.set_font(ng::font(canvas.font(), ng::font::Bold, 28));
	canvas.set_background_colour(ng::colour::Black);
	ecs.register_archetype(archetypes::spaceship);
	ecs.register_archetype(archetypes::asteroid);
	auto spaceship = ecs.create_entity(archetypes::spaceship.id(), ng::game::rigid_body{ ng::vec3{ 400.0, 18.0, 0.0 }, 1.0 });
	neolib::basic_random<double> prng;
	for (int i = 0; i < 1000; ++i)
		auto asteroid = ecs.create_entity(archetypes::asteroid.id(), ng::game::rigid_body{ ng::vec3{ prng(1000.0), prng(1000.0), 0.0 }, 1.0 });
}