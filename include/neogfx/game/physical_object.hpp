// physical_object.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include "i_physical_object.hpp"

namespace neogfx
{
	class physical_object : public virtual i_physical_object
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
		typedef std::optional<physics> optional_physics;
	public:
		physical_object();
		physical_object(const physical_object& aOther);
	public:
		object_category category() const override;
		const i_shape& as_shape() const override;
		i_shape& as_shape() override;
		const i_collidable_object& as_collidable_object() const override;
		i_collidable_object& as_collidable_object() override;
		const i_physical_object& as_physical_object() const override;
		i_physical_object& as_physical_object() override;
		bool killed() const override;
		void kill() override;
	public:
		vec3 origin() const override;
		vec3 position() const override;
		vec3 angle_radians() const override;
		vec3 angle_degrees() const override;
		vec3 velocity() const override;
		vec3 acceleration() const override;
		vec3 spin_radians() const override;
		vec3 spin_degrees() const override;
		scalar mass() const override;
		void set_origin(const vec3& aOrigin) override;
		void set_position(const vec3& aPosition) override;
		void set_angle_radians(const vec3& aAngle) override;
		void set_angle_degrees(const vec3& aAngle) override;
		void set_velocity(const vec3& aVelocity) override;
		void set_acceleration(const vec3& aAcceleration) override;
		void set_spin_radians(const vec3& aSpin) override;
		using i_physical_object::set_spin_radians;
		void set_spin_degrees(const vec3& aSpin) override;
		using i_physical_object::set_spin_degrees;
		void set_mass(scalar aMass) override;
	public:
		void clear_vertices_cache() override;
	public:
		const neogfx::aabb& aabb() const override;
		const neogfx::aabb& saved_aabb() const override;
		void save_aabb() override;
		void clear_saved_aabb() override;
		bool collidable() const override;
		bool has_collided(const i_collidable_object& aOther) const override;
		void collided(i_collidable_object& aOther) override;
	public:
		uint32_t collision_update_id() const override;
		void set_collision_update_id(uint32_t aCollisionUpdateId) override;
	public:
		void clear_aabb_cache() override;
	public:
		bool update(const optional_time_interval& aNow, const vec3& aForce) override;
		const optional_time_interval& update_time() const override;
		void set_update_time(const optional_time_interval& aLastUpdateTime) override;
	private:
		const physics& current_physics() const;
		physics& current_physics();
		const physics& next_physics() const;
		physics& next_physics();
		bool apply_physics(double aElapsedTime, const vec3& aForce);
	private:
		vec3 iOrigin;
		optional_time_interval iTimeOfLastUpdate;
		mutable optional_physics iCurrentPhysics;
		mutable optional_physics iNextPhysics;
		mutable optional_aabb iAabb;
		mutable optional_aabb iSavedAabb;
		bool iKilled;
		uint32_t iCollisionUpdateId;
	};
}