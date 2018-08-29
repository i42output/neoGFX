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
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/simple_physics_system.hpp>
#include <neogfx/game/time_system.hpp>
#include <neogfx/game/physics.hpp>
#include <neogfx/game/rigid_body.hpp>

namespace neogfx::game
{
	simple_physics_system::simple_physics_system(const ecs::context& aContext) :
		system{ aContext }
	{
		if (!ecs().system_registered<time_system>(aContext))
			ecs().register_system<time_system>(aContext);
		if (!ecs().component_registered<physics>(aContext))
			ecs().register_component<physics>(aContext);
		if (ecs().shared_component<physics>(context()).component_data().empty())
			ecs().populate_shared<physics>(aContext, physics{ 6.67408e-11 });
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
		if (!ecs().component_instantiated<rigid_body>(context()))
			return;
		auto& worldClock = ecs().shared_component<clock>(context()).component_data()[0];
		auto& physicalConstants = ecs().shared_component<physics>(context()).component_data()[0];
		auto now = to_step_time(
			chrono::to_seconds(std::chrono::duration_cast<chrono::flicks>(std::chrono::high_resolution_clock::now().time_since_epoch())), 
			worldClock.timeStep);
		auto& rigidBodies = ecs().component<rigid_body>(context());
		while (worldClock.time <= now)
		{
			applying_physics.trigger(worldClock.time);
			if (physicalConstants.gravitationalConstant != 0.0)
			{
				rigidBodies.sort([](const rigid_body& lhs, const rigid_body& rhs) { return lhs.mass > rhs.mass; });
				for (auto& rigidBody1 : rigidBodies.component_data())
				{
					vec3 totalForce;
					if (rigidBody1.mass == 0.0)
						break;
					if (physicalConstants.uniformGravity != std::nullopt)
						totalForce = *physicalConstants.uniformGravity * rigidBody1.mass;
					for (auto& rigidBody2 : rigidBodies.component_data())
					{
						if (&rigidBody2 == &rigidBody1)
							continue;
						if (rigidBody2.mass == 0.0)
							break;
						vec3 force;
						vec3 r12 = rigidBody1.position - rigidBody2.position;
						if (r12.magnitude() > 0.0)
							force = -physicalConstants.gravitationalConstant * rigidBody2.mass * rigidBody1.mass * r12 / std::pow(r12.magnitude(), 3.0);
						if (force.magnitude() >= 1.0e-6)
							totalForce += force;
						else
							break;
					}
					auto rotation = [&rigidBody1]() -> mat33
					{
						scalar ax = rigidBody1.angle.x;
						scalar ay = rigidBody1.angle.y;
						scalar az = rigidBody1.angle.z;
						if (ax != 0.0 || ay != 0.0)
						{
							mat33 rx = { { 1.0, 0.0, 0.0 },{ 0.0, std::cos(ax), -std::sin(ax) },{ 0.0, std::sin(ax), std::cos(ax) } };
							mat33 ry = { { std::cos(ay), 0.0, std::sin(ay) },{ 0.0, 1.0, 0.0 },{ -std::sin(ay), 0.0, std::cos(ay) } };
							mat33 rz = { { std::cos(az), -std::sin(az), 0.0 },{ std::sin(az), std::cos(az), 0.0 },{ 0.0, 0.0, 1.0 } };
							return rz * ry * rx;
						}
						else
						{
							return mat33{ { std::cos(az), -std::sin(az), 0.0 },{ std::sin(az), std::cos(az), 0.0 },{ 0.0, 0.0, 1.0 } };
						}
					}();
					// GCSE-level physics (Newtonian) going on here... :)
					// v = u + at
					// F = ma; a = F/m
					auto v0 = rigidBody1.velocity;
					auto p0 = rigidBody1.position;
					auto a0 = rigidBody1.angle;
					auto elapsedTime = from_step_time(worldClock.timeStep);
					rigidBody1.velocity = v0 + ((rigidBody1.mass == 0 ? vec3{} : totalForce / rigidBody1.mass) + (rotation * rigidBody1.acceleration)) * vec3 { elapsedTime, elapsedTime, elapsedTime };
					rigidBody1.position = rigidBody1.position + vec3{ 1.0, 1.0, 1.0 } *(v0 * elapsedTime + ((rigidBody1.velocity - v0) * elapsedTime / 2.0));
					rigidBody1.angle = (rigidBody1.angle + rigidBody1.spin * elapsedTime) % (2.0 * boost::math::constants::pi<scalar>());
				}
			}
			physics_applied.trigger(worldClock.time);
			worldClock.time += worldClock.timeStep;
		}
	}
}