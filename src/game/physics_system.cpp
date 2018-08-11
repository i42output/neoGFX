// physics_system.cpp
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
#include <neogfx/game/physics_system.hpp>
#include <neogfx/game/rigid_body.hpp>

namespace neogfx::game
{
	physics_system::physics_system(const ecs::context& aContext) :
		system{ aContext }
	{
	}

	const system_id& physics_system::id() const
	{
		return meta::id();
	}

	const neolib::i_string& physics_system::name() const
	{
		return meta::name();
	}

	void physics_system::apply()
	{
		if (!ecs::instance().component_instantiated<rigid_body>(context()))
			return;
		auto& time = ecs::instance().component<clock>(context());
		auto& rigidBodies = ecs::instance().component<rigid_body>(context());
		for (auto& rigidBody : rigidBodies.contents())
		{
			while (*iPhysicsTime <= now)
			{
				++frames;
				applying_physics.trigger(*iPhysicsTime);
				sort_objects();
				if (iG != 0.0)
				{
					for (auto& i1 : iObjects)
					{
						vec3 totalForce;
						if (i1->category() == object_category::Shape)
							break;
						if (i1->killed())
							continue;
						auto& o1 = (*i1).as_physical_object();
						if (o1.mass() == 0.0)
							break;
						if (iUniformGravity != std::nullopt)
							totalForce = *iUniformGravity * o1.mass();
						for (auto& i2 : iObjects)
						{
							if (i2->category() == object_category::Shape)
								break;
							if (i2->killed())
								continue;
							auto& o2 = (*i2).as_physical_object();
							if (&o2 == &o1)
								continue;
							if (o2.mass() == 0.0)
								break;
							vec3 force;
							vec3 r12 = o1.position() - o2.position();
							if (r12.magnitude() > 0.0)
								force = -iG * o2.mass() * o1.mass() * r12 / std::pow(r12.magnitude(), 3.0);
							if (force.magnitude() >= 1.0e-6)
								totalForce += force;
							else
								break;
						}
						bool o1updated = o1.update(from_step_time(*iPhysicsTime), totalForce);
						updated = (o1updated || updated);
					}
				}
			}
			auto rotation = [&rigidBody]() -> mat33
			{
				scalar ax = rigidBody.angle.x;
				scalar ay = rigidBody.angle.y;
				scalar az = rigidBody.angle.z;
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
			auto v0 = rigidBody.velocity;
			auto p0 = rigidBody.position;
			auto a0 = rigidBody.angle;
			rigidBody.velocity = v0 + ((rigidBody.mass == 0 ? vec3{} : aForce / rigidBody.mass) + (rotation * rigidBody.acceleration)) * vec3 { aElapsedTime, aElapsedTime, aElapsedTime };
			rigidBody.position = rigidBody.position + vec3{ 1.0, 1.0, 1.0 } * (v0 * aElapsedTime + ((rigidBody.velocity - v0) * aElapsedTime / 2.0));
			rigidBody.angle = (rigidBody.angle + rigidBody.spin * aElapsedTime) % (2.0 * boost::math::constants::pi<scalar>());
			// if position or angle has changed then needs rendering
		}
	}
}