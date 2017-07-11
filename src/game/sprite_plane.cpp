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
		iEnableZSorting{ false }, iNeedsSorting{ false }, iG { 6.67408e-11 }, iStepInterval{ 10 }
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
		iEnableZSorting{ false }, iNeedsSorting{ false }, iG { 6.67408e-11 }, iStepInterval{ 10 }
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
		iEnableZSorting{ false }, iNeedsSorting{ false }, iG { 6.67408e-11 }, iStepInterval{ 10 }
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
			if ((s->bounding_box() + s->position()).intersection(client_rect()).empty())
				continue;
			s->paint(aGraphicsContext);
		}
		sprites_painted.trigger(aGraphicsContext);
	}

	const i_widget& sprite_plane::as_widget() const
	{
		return *this;
	}

	i_widget& sprite_plane::as_widget()
	{
		return *this;
	}

	bool sprite_plane::has_buddy(const i_shape& aShape) const
	{
		return iBuddies.find(&aShape) != iBuddies.end();
	}

	i_shape& sprite_plane::buddy(const i_shape& aShape) const
	{
		auto b = iBuddies.find(&aShape);
		if (b == iBuddies.end())
			throw no_buddy();
		return *b->second.first;
	}

	void sprite_plane::set_buddy(const i_shape& aShape, i_shape& aBuddy, const vec3& aBuddyOffset)
	{
		if (has_buddy(aShape))
			throw buddy_exists();
		iBuddies.emplace(&aShape, std::make_pair(&aBuddy, aBuddyOffset));
	}

	const vec3& sprite_plane::buddy_offset(const i_shape& aShape) const
	{
		auto b = iBuddies.find(&aShape);
		if (b == iBuddies.end())
			throw no_buddy();
		return b->second.second;
	}

	void sprite_plane::set_buddy_offset(const i_shape& aShape, const vec3& aBuddyOffset)
	{
		auto b = iBuddies.find(&aShape);
		if (b == iBuddies.end())
			throw no_buddy();
		b->second.second = aBuddyOffset;
	}

	void sprite_plane::unset_buddy(const i_shape& aShape)
	{
		auto b = iBuddies.find(&aShape);
		if (b == iBuddies.end())
			throw no_buddy();
		iBuddies.erase(b);
	}

	void sprite_plane::enable_z_sorting(bool aEnableZSorting)
	{
		iEnableZSorting = aEnableZSorting;
	}

	void sprite_plane::add_shape(i_shape& aShape)
	{
		iItems.push_back(std::shared_ptr<i_shape>(std::shared_ptr<i_shape>(), &aShape));
		iRenderBuffer.push_back(&aShape);
		iNeedsSorting = true;
	}

	void sprite_plane::add_shape(std::shared_ptr<i_shape> aShape)
	{
		iItems.push_back(aShape);
		iRenderBuffer.push_back(&*aShape);
		iNeedsSorting = true;
	}

	void sprite_plane::add_sprite(i_sprite& aSprite)
	{
		iItems.push_back(std::shared_ptr<i_sprite>(std::shared_ptr<i_sprite>(), &aSprite));
		iRenderBuffer.push_back(&aSprite);
		iNeedsSorting = true;
	}

	void sprite_plane::add_sprite(std::shared_ptr<i_sprite> aSprite)
	{
		iItems.push_back(aSprite);
		iRenderBuffer.push_back(&*aSprite);
		iNeedsSorting = true;
	}

	i_sprite& sprite_plane::create_sprite()
	{
		iSimpleSprites.push_back(sprite(*this));
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_texture& aTexture, const optional_rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(*this, aTexture, aTextureRect);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_image& aImage, const optional_rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(*this, aImage, aTextureRect);
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

	void sprite_plane::add_object(i_physical_object& aObject)
	{
		iItems.push_back(std::shared_ptr<i_physical_object>(std::shared_ptr<i_physical_object>(), &aObject));
		iNeedsSorting = true;
	}

	void sprite_plane::add_object(std::shared_ptr<i_physical_object> aObject)
	{
		iItems.push_back(aObject);
		iNeedsSorting = true;
	}

	i_physical_object& sprite_plane::create_object()
	{
		iSimpleObjects.push_back(physical_object());
		add_object(iSimpleObjects.back());
		return iSimpleObjects.back();
	}

	i_physical_object& sprite_plane::create_earth()
	{
		auto& earth = create_object();
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
		iItems.reserve(aCapacity);
	}

	const sprite_plane::item_list& sprite_plane::items() const
	{
		return iItems;
	}

	const sprite_plane::buddy_list& sprite_plane::buddies() const
	{
		return iBuddies;
	}

	sprite_plane::buddy_list& sprite_plane::buddies()
	{
		return iBuddies;
	}

	void sprite_plane::sort_shapes() const
	{
		if (iNeedsSorting && iEnableZSorting)
		{
			std::stable_sort(iRenderBuffer.begin(), iRenderBuffer.end(), [this](i_shape* left, i_shape* right) -> bool
			{
				return left->position_3D()[2] < right->position_3D()[2];
			});
		}
	}

	void sprite_plane::sort_objects()
	{
		if (iNeedsSorting)
		{
			sort_shapes();
			std::stable_sort(iItems.begin(), iItems.end(), [this](const item& left, const item& right) -> bool
			{
				if (left.which() != right.which())
					return left.which() < right.which();
				else if (left.is<shape_pointer>())
					return &left < &right;
				else
				{
					const i_physical_object* leftObject = &(left.is<sprite_pointer>() ? static_variant_cast<const sprite_pointer&>(left)->physics() : *static_variant_cast<const object_pointer&>(left));
					const i_physical_object* rightObject = &(right.is<sprite_pointer>() ? static_variant_cast<const sprite_pointer&>(right)->physics() : *static_variant_cast<const object_pointer&>(right));
					return leftObject->mass() > rightObject->mass();
				}
			});
			iNeedsSorting = false;
		}
	}

	bool sprite_plane::update_objects()
	{
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
			for (auto& i2 : iItems)
			{
				vec3 totalForce;
				if (i2.is<shape_pointer>())
					break;
				i_physical_object* o2 = &(i2.is<sprite_pointer>() ? static_variant_cast<sprite_pointer&>(i2)->physics() : *static_variant_cast<object_pointer&>(i2));
				if (o2->mass() == 0.0)
					continue;
				if (iUniformGravity != boost::none)
					totalForce = *iUniformGravity * o2->mass();
				else if (iG != 0.0)
				{
					for (auto& i1 : iItems)
					{
						if (i1.is<shape_pointer>())
							break;
						i_physical_object* o1 = &(i1.is<sprite_pointer>() ? static_variant_cast<sprite_pointer&>(i1)->physics() : *static_variant_cast<object_pointer&>(i1));
						if (o1 == o2)
							continue;
						if (o1->collided(*o2))
							continue;
						vec3 force;
						vec3 r12 = o2->position() - o1->position();
						if (r12.magnitude() > 0.0)
							force = -iG * o1->mass() * o2->mass() * r12 / std::pow(r12.magnitude(), 3.0);
						if (force.magnitude() < 1.0e-6)
							break;
						totalForce += force;
					}
				}
				updated = (o2->update(from_step_time(*iPhysicsTime), totalForce) || updated);
			}
			physics_applied.trigger(*iPhysicsTime);
			*iPhysicsTime += physics_step_interval();
		}
		return updated;
	}
}