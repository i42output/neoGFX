// physics_system.hpp
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
#include <neogfx/game/system.hpp>

namespace neogfx
{
	class physics_system : public system
	{
	public:
		physics_system(const ecs::context& aContext) :
			system{ aContext }
		{
		}
	public:
		const system_id& id() const override
		{
			static const neolib::uuid sId = { 0x49443e26, 0x762e, 0x4517, 0xbbb8, { 0xc3, 0xd6, 0x95, 0x7b, 0xe9, 0xd4 } };
			return sId;
		}
		const neolib::i_string& name() const override
		{
			static const neolib::string sName = "Physics";
			return sName;
		}
	public:
		void apply() override
		{
			/* -- TODO - Refactor this
			auto ax = angle_radians()[0];
			auto ay = angle_radians()[1];
			auto az = angle_radians()[2];
			auto rotation = [ax, ay, az]() -> mat33
			{
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
			next_physics().iVelocity = current_physics().iVelocity + ((current_physics().iMass == 0 ? vec3{} : aForce / current_physics().iMass) + (rotation * current_physics().iAcceleration)) * vec3 { aElapsedTime, aElapsedTime, aElapsedTime };
			next_physics().iPosition = current_physics().iPosition + vec3{ 1.0, 1.0, 1.0 } *(current_physics().iVelocity * aElapsedTime + ((next_physics().iVelocity - current_physics().iVelocity) * aElapsedTime / 2.0));
			next_physics().iAngle = (current_physics().iAngle + current_physics().iSpin * aElapsedTime) % (2.0 * boost::math::constants::pi<scalar>());
			return next_physics().iPosition != current_physics().iPosition || next_physics().iAngle != current_physics().iAngle;
			*/
		}
	};
}