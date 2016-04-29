// physical_object.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include <boost/math/constants/constants.hpp>
#include "physical_object.hpp"

namespace neogfx
{
	physical_object::physical_object() :
		iOrigin{}
	{
	}

	physical_object::physical_object(const physical_object& aOther) :
		iOrigin(aOther.iOrigin),
		iTimeOfLastUpdate(aOther.iTimeOfLastUpdate),
		iCurrentPhysics(aOther.iCurrentPhysics),
		iNextPhysics(aOther.iNextPhysics)
	{
	}

	const vec3& physical_object::origin() const
	{
		return iOrigin;
	}

	const vec3& physical_object::position() const
	{
		return current_physics().iPosition;
	}

	const vec3& physical_object::angle_radians() const
	{
		return current_physics().iAngle;
	}

	vec3 physical_object::angle_degrees() const
	{
		return current_physics().iAngle * 180.0 / boost::math::constants::pi<double>();
	}

	const vec3& physical_object::velocity() const
	{
		return current_physics().iVelocity;
	}

	const vec3& physical_object::acceleration() const
	{
		return current_physics().iAcceleration;
	}

	const vec3& physical_object::spin_radians() const
	{
		return current_physics().iSpin;
	}

	vec3 physical_object::spin_degrees() const
	{
		return current_physics().iSpin * 180.0 / boost::math::constants::pi<scalar>();
	}

	scalar physical_object::mass() const
	{
		return current_physics().iMass;
	}

	void physical_object::set_origin(const vec3& aOrigin)
	{
		iOrigin = aOrigin;
	}

	void physical_object::set_position(const vec3& aPosition)
	{
		current_physics().iPosition = aPosition;
	}

	void physical_object::set_angle_radians(const vec3& aAngle)
	{
		current_physics().iAngle = aAngle;
	}

	void physical_object::set_angle_degrees(const vec3& aAngle)
	{
		current_physics().iAngle = aAngle * boost::math::constants::pi<scalar>() / 180.0;
	}

	void physical_object::set_velocity(const vec3& aVelocity)
	{
		current_physics().iVelocity = aVelocity;
	}

	void physical_object::set_acceleration(const vec3& aAcceleration)
	{
		current_physics().iAcceleration = aAcceleration;
	}

	void physical_object::set_spin_radians(const vec3& aSpin)
	{
		current_physics().iSpin = aSpin;
	}

	void physical_object::set_spin_degrees(const vec3& aSpin)
	{
		current_physics().iSpin = aSpin * boost::math::constants::pi<scalar>() / 180.0;
	}

	void physical_object::set_mass(scalar aMass) 
	{
		current_physics().iMass = aMass;
	}

	const physical_object::aabb_type& physical_object::aabb() const
	{
		return iAxisAlignedBoundingBox;
	}

	bool physical_object::collided(const i_physical_object& aOther) const
	{
		/* todo */
		(void)aOther;
		return false;
	}

	bool physical_object::update(const optional_time_point& aNow, const vec3& aForce)
	{
		bool updated = false;
		if (iTimeOfLastUpdate == boost::none)
		{
			iTimeOfLastUpdate = aNow;
			updated = true;
		}
		next_physics() = current_physics();
		if (iTimeOfLastUpdate != boost::none && aNow != boost::none)
			updated = apply_physics((*aNow - *iTimeOfLastUpdate).count() * std::chrono::steady_clock::period::num / static_cast<double>(std::chrono::steady_clock::period::den), aForce) || updated;
		else
			updated = apply_physics(1.0, aForce) || updated;
		iTimeOfLastUpdate = aNow;
		current_physics() = next_physics();
		return updated;
	}

	const physical_object::physics& physical_object::current_physics() const
	{
		if (iCurrentPhysics == boost::none)
			iCurrentPhysics = physics{};
		return *iCurrentPhysics;
	}

	physical_object::physics& physical_object::current_physics()
	{
		return const_cast<physics&>(const_cast<const physical_object*>(this)->current_physics());
	}

	const physical_object::physics& physical_object::next_physics() const
	{
		if (iNextPhysics == boost::none)
			iNextPhysics = physics{};
		return *iNextPhysics;
	}

	physical_object::physics& physical_object::next_physics()
	{
		return const_cast<physics&>(const_cast<const physical_object*>(this)->next_physics());
	}

	bool physical_object::apply_physics(double aElapsedTime, const vec3& aForce)
	{
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
		next_physics().iVelocity = current_physics().iVelocity + ((current_physics().iMass == 0 ? vec3{} : aForce / current_physics().iMass) + (rotation * current_physics().iAcceleration)) * vec3(aElapsedTime, aElapsedTime, aElapsedTime); // v = u + at
		next_physics().iPosition = current_physics().iPosition + vec3(1.0, 1.0, 1.0) * (current_physics().iVelocity * aElapsedTime + ((next_physics().iVelocity - current_physics().iVelocity) * aElapsedTime / 2.0));
		next_physics().iAngle = (current_physics().iAngle + current_physics().iSpin * aElapsedTime) % (2.0 * boost::math::constants::pi<scalar>());
		return next_physics().iPosition != current_physics().iPosition || next_physics().iAngle != current_physics().iAngle;
	}
}