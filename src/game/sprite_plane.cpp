// sprite_plane.cpp
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

#include <neogfx/neogfx.hpp>
#include <numeric>
#include <chrono>
#include <neogfx/app/app.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include "../hid/native/i_native_surface.hpp"

namespace neogfx
{
	sprite_plane::sprite_plane() : 
		iUpdater{ app::instance(), [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			if (update_objects())
				update();
		}, 10 },
		iEnableZSorting{ false }, iNeedsSorting{ false }, iG{ 6.67408e-11 }, iStepInterval{ 10 }, iWaitForRender{ false }
	{
	}

	sprite_plane::sprite_plane(i_widget& aParent) :
		widget{ aParent }, 
		iUpdater{ app::instance(), [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			if (update_objects())
				update();
		}, 10 },
		iEnableZSorting{ false }, iNeedsSorting{ false }, iG{ 6.67408e-11 }, iStepInterval{ 10 }, iWaitForRender{ false }
	{
	}

	sprite_plane::sprite_plane(i_layout& aLayout) :
		widget{ aLayout }, 
		iUpdater{ app::instance(), [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			if (update_objects())
				update();
		}, 10 },
		iEnableZSorting{ false }, iNeedsSorting{ false }, iG{ 6.67408e-11 }, iStepInterval{ 10 }, iWaitForRender{ false }
	{
	}

	sprite_plane::~sprite_plane()
	{
	}

	logical_coordinate_system sprite_plane::logical_coordinate_system() const
	{
		return neogfx::logical_coordinate_system::AutomaticGame;
	}

	void sprite_plane::paint(graphics_context& aGraphicsContext) const
	{	
		painting_sprites.trigger(aGraphicsContext);
		sort_shapes();
		for (auto s : iRenderBuffer)
		{
			if (s->destroyed())
				continue;
			if (s->bounding_box_2d().intersection(client_rect()).empty())
				continue;
			s->paint(aGraphicsContext);
		}
		sprites_painted.trigger(aGraphicsContext);
		iWaitForRender = false;
	}

	const i_widget& sprite_plane::as_widget() const
	{
		return *this;
	}

	i_widget& sprite_plane::as_widget()
	{
		return *this;
	}

	void sprite_plane::enable_z_sorting(bool aEnableZSorting)
	{
		iEnableZSorting = aEnableZSorting;
	}

	void sprite_plane::add_sprite(i_sprite& aObject)
	{
		iObjects.push_back(std::shared_ptr<i_object>(std::shared_ptr<i_object>(), &aObject.physics())); // todo: using aliasing ctor here; not quite happy with the i_object based class hierarchy at present
		iRenderBuffer.push_back(static_cast<i_shape*>(&aObject));
		iNeedsSorting = true;
	}

	void sprite_plane::add_sprite(std::shared_ptr<i_sprite> aObject)
	{
		iObjects.push_back(std::shared_ptr<i_object>(aObject, &aObject->physics())); // todo: using aliasing ctor here; not quite happy with the i_object based class hierarchy at present
		iRenderBuffer.push_back(static_cast<i_shape*>(&*aObject));
		iNeedsSorting = true;
	}

	void sprite_plane::add_physical_object(i_physical_object& aObject)
	{
		iObjects.push_back(std::shared_ptr<i_physical_object>(std::shared_ptr<i_physical_object>(), &aObject));
		iNeedsSorting = true;
	}

	void sprite_plane::add_physical_object(std::shared_ptr<i_physical_object> aObject)
	{
		iObjects.push_back(aObject);
		iNeedsSorting = true;
	}

	void sprite_plane::add_shape(i_shape& aObject)
	{
		iObjects.push_back(std::shared_ptr<i_shape>(std::shared_ptr<i_shape>(), &aObject));
		iRenderBuffer.push_back(&aObject);
		iNeedsSorting = true;
	}

	void sprite_plane::add_shape(std::shared_ptr<i_shape> aObject)
	{
		iObjects.push_back(aObject);
		iRenderBuffer.push_back(&*aObject);
		iNeedsSorting = true;
	}

	i_sprite& sprite_plane::create_sprite()
	{
		iSimpleSprites.push_back(sprite{});
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_texture& aTexture)
	{
		iSimpleSprites.emplace_back(aTexture);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_image& aImage)
	{
		iSimpleSprites.emplace_back(aImage);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_texture& aTexture, const rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(aTexture, aTextureRect);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_image& aImage, const rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(aImage, aTextureRect);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	scalar sprite_plane::gravitational_constant() const
	{
		return iG;
	}

	void sprite_plane::set_gravitational_constant(scalar aG)
	{
		iG = aG;
	}

	const optional_vec3& sprite_plane::uniform_gravity() const
	{
		return iUniformGravity;
	}

	void sprite_plane::set_uniform_gravity(const optional_vec3& aUniformGravity)
	{
		iUniformGravity = aUniformGravity;
	}

	i_physical_object& sprite_plane::create_physical_object()
	{
		iSimpleObjects.push_back(physical_object{});
		add_physical_object(iSimpleObjects.back());
		return iSimpleObjects.back();
	}

	i_physical_object& sprite_plane::create_earth()
	{
		auto& earth = create_physical_object();
		earth.set_position({ 0.0, -6371000.0, 0.0 });
		earth.set_mass(5.972e24);
		return earth;
	}

	const sprite_plane::optional_step_time_interval& sprite_plane::physics_time() const
	{
		return iPhysicsTime;
	}

	void sprite_plane::set_physics_time(const optional_step_time_interval& aTime)
	{
		iPhysicsTime = aTime;
	}

	sprite_plane::step_time_interval sprite_plane::physics_step_interval() const
	{
		return iStepInterval;
	}

	void sprite_plane::set_physics_step_interval(step_time_interval aStepInterval)
	{
		iStepInterval = aStepInterval;
	}

	void sprite_plane::reserve(std::size_t aCapacity)
	{
		iObjects.reserve(aCapacity);
	}

	const sprite_plane::object_list& sprite_plane::objects() const
	{
		return iObjects;
	}

	void sprite_plane::sort_shapes() const
	{
		if (iNeedsSorting && iEnableZSorting)
		{
			std::stable_sort(iRenderBuffer.begin(), iRenderBuffer.end(), [this](i_shape* left, i_shape* right) -> bool
			{
				if (left->destroyed() != right->destroyed())
					return left->destroyed() < right->destroyed();
				else
					return left->position().z < right->position().z;
			});
			while (!iRenderBuffer.empty() && iRenderBuffer.back()->destroyed())
				iRenderBuffer.pop_back();
		}
	}

	void sprite_plane::sort_objects()
	{
		if (iNeedsSorting)
		{
			sort_shapes();
			std::stable_sort(iObjects.begin(), iObjects.end(), [this](const object_pointer& left, const object_pointer& right) -> bool
			{
				if (left->destroyed() != right->destroyed())
					return left->destroyed() < right->destroyed();
				else if (left->category() == object_category::Shape || right->category() == object_category::Shape)
				{
					if (left->category() != right->category())
						return left->category() < right->category();
					else
						return &left < &right;
				}
				else
				{
					const i_physical_object& leftObject = static_cast<const i_physical_object&>(*left);
					const i_physical_object& rightObject = static_cast<const i_physical_object&>(*right);
					return leftObject.mass() > rightObject.mass();
				}
			});
			while (!iObjects.empty() && iObjects.back()->destroyed())
				iObjects.pop_back();
			iNeedsSorting = false;
		}
	}

	bool sprite_plane::update_objects()
	{
		if (iWaitForRender)
			return false;
		auto nowClock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
		auto now = to_step_time(nowClock.count() * .001, physics_step_interval());
		if (!iPhysicsTime)
			iPhysicsTime = now;
		if (*iPhysicsTime == now)
			return false;
		bool updated = false;
		while (*iPhysicsTime <= now)
		{
			applying_physics.trigger(*iPhysicsTime);
			sort_objects();
			if (iG != 0.0)
			{
				for (auto& i1 : iObjects)
				{
					vec3 totalForce;
					if (i1->category() == object_category::Shape)
						break;
					i_physical_object& o1 = static_cast<i_physical_object&>(*i1);
					if (o1.mass() == 0.0)
						break;
					if (iUniformGravity != boost::none)
						totalForce = *iUniformGravity * o1.mass();
					for (auto& i2 : iObjects)
					{
						if (i2->category() == object_category::Shape)
							break;
						i_physical_object& o2 = static_cast<i_physical_object&>(*i2);
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
					updated = (o1.update(from_step_time(*iPhysicsTime), totalForce) || updated);
				}
			}
			for (auto& i1 : iObjects)
			{
				if (i1->destroyed())
				{
					iNeedsSorting = true;
					continue;
				}
				if (i1->category() == object_category::Shape)
					break;
				i_physical_object& o1 = static_cast<i_physical_object&>(*i1);
				for (auto& i2 : iObjects)
				{
					if (i2->destroyed())
					{
						iNeedsSorting = true;
						continue;
					}
					if (i2->category() == object_category::Shape)
						break;
					i_physical_object& o2 = static_cast<i_physical_object&>(*i2);
					if (&o2 == &o1)
						continue;
					if (&o1 < &o2 && o1.has_collided(o2))
					{
						o1.collided(o2);
						o2.collided(o1);
						object_collision.trigger(o1, o2);
					}
				}
			}
			physics_applied.trigger(*iPhysicsTime);
			*iPhysicsTime += physics_step_interval();
		}
		for (auto& s : iRenderBuffer)
		{
			updated = s->update(from_step_time(now)) || updated;
		}
		if (updated)
			iWaitForRender = true;
		return updated;
	}
}