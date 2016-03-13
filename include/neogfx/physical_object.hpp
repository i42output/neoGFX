// physical_object.hpp
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
#pragma once

#include "neogfx.hpp"
#include "i_physical_object.hpp"

namespace neogfx
{
	class physical_object : public i_physical_object
	{
	private:
		struct physics
		{
			vec3 iPosition;
			vec3 iAngle;
			vec3 iVelocity;
			vec3 iAcceleration;
			vec3 iSpin;
			scalar iMass;
		};
		typedef boost::optional<physics> optional_physics;
	public:
		physical_object();
		physical_object(const physical_object& aOther);
	public:
		virtual const vec3& origin() const;
		virtual const vec3& position() const;
		virtual const vec3& angle_radians() const;
		virtual vec3 angle_degrees() const;
		virtual const vec3& velocity() const;
		virtual const vec3& acceleration() const;
		virtual const vec3& spin_radians() const;
		virtual vec3 spin_degrees() const;
		virtual scalar mass() const;
		virtual void set_origin(const vec3& aOrigin);
		virtual void set_position(const vec3& aPosition);
		virtual void set_angle_radians(const vec3& aAngle);
		virtual void set_angle_degrees(const vec3& aAngle);
		virtual void set_velocity(const vec3& aVelocity);
		virtual void set_acceleration(const vec3& aAcceleration);
		virtual void set_spin_radians(const vec3& aSpin);
		virtual void set_spin_degrees(const vec3& aSpin);
		virtual void set_mass(scalar aMass);
	public:
		virtual const aabb_type& aabb() const;
		virtual bool collided(const i_physical_object& aOther) const;
		virtual bool update(const optional_time_point& aNow, const vec3& aForce);
	private:
		const physics& current_physics() const;
		physics& current_physics();
		const physics& next_physics() const;
		physics& next_physics();
		bool apply_physics(double aElapsedTime, const vec3& aForce);
	private:
		vec3 iOrigin;
		mutable aabb_type iAxisAlignedBoundingBox;
		optional_time_point iTimeOfLastUpdate;
		mutable optional_physics iCurrentPhysics;
		mutable optional_physics iNextPhysics;
	};
}