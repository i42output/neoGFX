// i_physical_object.hpp
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

#include <neogfx/neogfx.hpp>
#include <chrono>
#include <boost/optional.hpp>
#include <neogfx/core/geometry.hpp>
#include "i_object.hpp"
#include "i_collidable.hpp"

namespace neogfx
{
	class i_physical_object : public i_object, public i_collidable
	{
		// types
	public:
		typedef scalar time_interval;
		typedef boost::optional<time_interval> optional_time_interval;
		typedef int64_t step_time_interval;
		typedef boost::optional<step_time_interval> optional_step_time_interval;
		// lifetime
	public:
		virtual ~i_physical_object() {}
		// physics
	public:
		virtual vec3 origin() const = 0;
		virtual vec3 position() const = 0;
		virtual vec3 angle_radians() const = 0;
		virtual vec3 angle_degrees() const = 0;
		virtual vec3 velocity() const = 0;
		virtual vec3 acceleration() const = 0;
		virtual vec3 spin_radians() const = 0;
		virtual vec3 spin_degrees() const = 0;
		virtual scalar mass() const = 0;
		virtual void set_origin(const vec3& aOrigin) = 0;
		virtual void set_position(const vec3& aPosition) = 0;
		virtual void set_angle_radians(const vec3& aAngle) = 0;
		virtual void set_angle_degrees(const vec3& aAngle) = 0;
		virtual void set_velocity(const vec3& aVelocity) = 0;
		virtual void set_acceleration(const vec3& aAcceleration) = 0;
		virtual void set_spin_radians(const vec3& aSpin) = 0;
		virtual void set_spin_degrees(const vec3& aSpin) = 0;
		virtual void set_mass(scalar aMass) = 0;
		// object
	public:
		virtual void clear_vertices_cache() = 0;
		virtual bool update(const optional_time_interval& aNow, const vec3& aForce) = 0;
		virtual const optional_time_interval& update_time() const = 0;
		virtual void set_update_time(const optional_time_interval& aLastUpdateTime) = 0;
		// helpers
	public:
		void set_angle_radians(scalar aAngle)
		{
			set_angle_radians(vec3{ 0.0, 0.0, aAngle });
		}
		void set_angle_degrees(scalar aAngle)
		{
			set_angle_degrees(vec3{ 0.0, 0.0, aAngle });
		}
		void set_spin_radians(scalar aSpin)
		{
			set_spin_radians(vec3{ 0.0, 0.0, aSpin });
		}
		void set_spin_degrees(scalar aSpin)
		{
			set_spin_degrees(vec3{ 0.0, 0.0, aSpin });
		}
	};

	inline rect to_rect(const aabb& aAabb)
	{
		return rect{ point{aAabb.min.x, aAabb.min.y}, point{ aAabb.max.x, aAabb.max.y } };
	}

	inline i_physical_object::step_time_interval to_step_time(i_physical_object::time_interval aTime, i_physical_object::step_time_interval aStepInterval)
	{
		auto ms = static_cast<i_physical_object::step_time_interval>(aTime * 1000.0);
		return ms - (ms % aStepInterval);
	}

	inline i_physical_object::step_time_interval to_step_time(const i_physical_object::optional_time_interval& aTime, i_physical_object::step_time_interval aStepInterval)
	{
		if (aTime)
			return to_step_time(*aTime, aStepInterval);
		else
			return 0;
	}

	inline i_physical_object::time_interval from_step_time(i_physical_object::step_time_interval aStepTime)
	{
		return aStepTime / 1000.0;
	}
}