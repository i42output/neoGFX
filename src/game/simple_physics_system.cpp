// simple_physics_system.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/thread.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/simple_physics_system.hpp>
#include <neogfx/game/time_system.hpp>
#include <neogfx/game/physics.hpp>
#include <neogfx/game/rigid_body.hpp>

namespace neogfx::game
{
	class simple_physics_system::thread : public neolib::thread
	{
	public:
		thread(simple_physics_system& aOwner) : neolib::thread{ "neogfx::game::simple_physics_system::thread" }, iOwner{ aOwner }
		{
			start();
		}
	public:
		void exec() override
		{
			while (!finished())
			{
				iOwner.apply();
			}
		}
	private:
		simple_physics_system& iOwner;
	};


	simple_physics_system::simple_physics_system(game::i_ecs& aEcs) :
		system{ aEcs }, iUniversalGravitationEnabled{ false }
	{
		if (!ecs().system_registered<time_system>())
			ecs().register_system<time_system>();
		if (!ecs().shared_component_registered<physics>())
			ecs().register_shared_component<physics>();
		if (ecs().shared_component<physics>().component_data().empty())
			ecs().populate_shared<physics>("Standard Universe", physics{ 6.67408e-11 });
		iThread = std::make_unique<thread>(*this);
	}

	simple_physics_system::~simple_physics_system()
	{
	}

	const system_id& simple_physics_system::id() const
	{
		return meta::id();
	}

	const neolib::i_string& simple_physics_system::name() const
	{
		return meta::name();
	}

	void simple_physics_system::apply()
	{
		if (!ecs().component_instantiated<rigid_body>())
			return;
		if (paused())
			return;
		if (!iThread->in()) // ignore ECS apply request (we have our own thread that does this)
			return;
		auto& worldClock = ecs().shared_component<clock>().component_data().begin()->second;
		auto& physicalConstants = ecs().shared_component<physics>().component_data().begin()->second;
		auto uniformGravity = physicalConstants.uniformGravity != std::nullopt ?
			*physicalConstants.uniformGravity : vec3{};
		auto now = ecs().system<time_system>().system_time();
		auto& rigidBodies = ecs().component<rigid_body>();
		while (worldClock.time <= now)
		{
			neolib::thread::sleep(1);
			component_lock_guard<rigid_body> lgRigidBodies{ ecs() };
			ecs().applying_physics.trigger(worldClock.time);
			bool useUniversalGravitation = (universal_gravitation_enabled() && physicalConstants.gravitationalConstant != 0.0);
			if (useUniversalGravitation)
				rigidBodies.sort([](const rigid_body& lhs, const rigid_body& rhs) { return lhs.mass > rhs.mass; });
			auto firstMassless = useUniversalGravitation ?
				std::find_if(rigidBodies.component_data().begin(), rigidBodies.component_data().end(), [](const rigid_body& body) { return body.mass == 0.0; }) :
				rigidBodies.component_data().begin();
			for (auto& rigidBody1 : rigidBodies.component_data())
			{
				vec3 totalForce = rigidBody1.mass * uniformGravity;
				if (useUniversalGravitation)
				{
					for (auto iterRigidBody2 = rigidBodies.component_data().begin(); iterRigidBody2 != firstMassless; ++iterRigidBody2)
					{
						auto& rigidBody2 = *iterRigidBody2;
						vec3 distance = rigidBody1.position - rigidBody2.position;
						if (distance.magnitude() > 0.0) // avoid division by zero or rigidBody1 == rigidBody2
							totalForce += -physicalConstants.gravitationalConstant * rigidBody2.mass * rigidBody1.mass * distance / std::pow(distance.magnitude(), 3.0);
					}
				}
				// GCSE-level physics (Newtonian) going on here... :)
				// v = u + at
				// F = ma; a = F/m
				auto v0 = rigidBody1.velocity;
				auto p0 = rigidBody1.position;
				auto a0 = rigidBody1.angle;
				auto elapsedTime = from_step_time(worldClock.timeStep);
				rigidBody1.velocity = v0 + ((rigidBody1.mass == 0 ? vec3{} : totalForce / rigidBody1.mass) + (rotation_matrix(rigidBody1.angle) * rigidBody1.acceleration)) * vec3 { elapsedTime, elapsedTime, elapsedTime };
				rigidBody1.position = rigidBody1.position + vec3{ 1.0, 1.0, 1.0 } * (elapsedTime * (v0 + (rigidBody1.velocity - v0) / 2.0));
				rigidBody1.angle = (rigidBody1.angle + rigidBody1.spin * elapsedTime) % (2.0 * boost::math::constants::pi<scalar>());
			}
			ecs().physics_applied.trigger(worldClock.time);
			shared_component_lock_guard<clock> lgClock{ ecs() };
			worldClock.time += worldClock.timeStep;
		}
	}

	bool simple_physics_system::universal_gravitation_enabled() const
	{
		return iUniversalGravitationEnabled;
	}

	void simple_physics_system::enable_universal_gravitation()
	{
		iUniversalGravitationEnabled = true;
	}

	void simple_physics_system::disable_universal_gravitation()
	{
		iUniversalGravitationEnabled = false;
	}
}