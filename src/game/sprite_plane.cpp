// sprite_plane.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include "../hid/native/i_native_surface.hpp"

namespace neogfx
{
	class sprite_plane::physics_thread : public neolib::thread
	{
	public:
		physics_thread(sprite_plane& aOwner) : neolib::thread{ "neogfx::sprite_plane::physics_thread" }, iOwner{ aOwner }
		{
			start();
		}
	public:
		void exec() override
		{
			while (!finished())
			{
				iOwner.update_objects();
				yield();
			}
		}
	private:
		sprite_plane& iOwner;
	};

	sprite_plane::sprite_plane() : 
		iUpdater{ app::instance(), [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			if (snapshot())
				update();
		}, 10 },
		iEnableDynamicUpdate{ false }, 
		iEnableZSorting{ false }, 
		iNeedsSorting{ false }, 
		iG{ 6.67408e-11 }, 
		iStepInterval{ chrono::to_flicks(0.010).count() }, 
		iUpdateTime{ 0ull },
		iUpdatedSinceLastSnapshot{ false },
		iTakingSnapshot{ false },
		iPhysicsThread{ std::make_unique<physics_thread>(*this) }
	{
	}

	sprite_plane::sprite_plane(i_widget& aParent) :
		widget{ aParent }, 
		iUpdater{ app::instance(), [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			if (snapshot())
				update();
		}, 10 },
		iEnableDynamicUpdate{ false }, 
		iEnableZSorting{ false }, 
		iNeedsSorting{ false }, iG{ 6.67408e-11 }, 
		iStepInterval{ chrono::to_flicks(0.010).count() }, 
		iUpdateTime{ 0ull },
		iUpdatedSinceLastSnapshot{ false },
		iTakingSnapshot{ false },
		iPhysicsThread{ std::make_unique<physics_thread>(*this) }
	{
	}

	sprite_plane::sprite_plane(i_layout& aLayout) :
		widget{ aLayout }, 
		iUpdater{ app::instance(), [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			if (snapshot())
				update();
		}, 10 },
		iEnableDynamicUpdate{ false }, 
		iEnableZSorting{ false }, 
		iNeedsSorting{ false }, 
		iG{ 6.67408e-11 }, 
		iStepInterval{ chrono::to_flicks(0.010).count() }, 
		iUpdateTime{ 0ull },
		iUpdatedSinceLastSnapshot{ false },
		iTakingSnapshot{ false },
		iPhysicsThread{ std::make_unique<physics_thread>(*this) }
	{
	}

	sprite_plane::~sprite_plane()
	{
		iPhysicsThread->abort();
	}

	logical_coordinate_system sprite_plane::logical_coordinate_system() const
	{
		if (widget::has_logical_coordinate_system())
			return widget::logical_coordinate_system();
		return neogfx::logical_coordinate_system::AutomaticGame;
	}

	void sprite_plane::paint(graphics_context& aGraphicsContext) const
	{	
		std::lock_guard<std::recursive_mutex> lock(iUpdateMutex); // todo: remove this lock once snapshot data added
		aGraphicsContext.clear_depth_buffer();
		painting_sprites.trigger(aGraphicsContext);
		sort_shapes();
		for (auto s : iRenderBuffer)
		{
			if (s->killed())
				continue;
			if (s->bounding_box_2d().intersection(client_rect()).empty())
				continue;
			s->paint(aGraphicsContext);
		}
		aGraphicsContext.flush();
		sprites_painted.trigger(aGraphicsContext);
	}

	void sprite_plane::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e)
	{
		if (aButton == mouse_button::Left)
		{
			std::vector<i_collidable_object*> picked;
			if (is_collision_tree_2d())
				collision_tree_2d().pick(aPosition.to_vec2(), picked);
			else
				collision_tree_3d().pick(aPosition.to_vec2(), picked);
			for (auto p : picked)
				object_clicked.trigger(*p);
		}
	}

	const i_widget& sprite_plane::as_widget() const
	{
		return *this;
	}

	i_widget& sprite_plane::as_widget()
	{
		return *this;
	}

	bool sprite_plane::dynamic_update_enabled() const
	{
		return iEnableDynamicUpdate;
	}

	void sprite_plane::enable_dynamic_update(bool aEnableDynamicUpdate)
	{
		iEnableDynamicUpdate = aEnableDynamicUpdate;
	}

	void sprite_plane::enable_z_sorting(bool aEnableZSorting)
	{
		iEnableZSorting = aEnableZSorting;
	}

	void sprite_plane::add_sprite(i_sprite& aObject)
	{
		add_object(std::shared_ptr<i_object>(std::shared_ptr<i_object>(), &aObject));
	}

	void sprite_plane::add_sprite(std::shared_ptr<i_sprite> aObject)
	{
		add_object(aObject);
	}

	void sprite_plane::add_physical_object(i_physical_object& aObject)
	{
		add_object(std::shared_ptr<i_physical_object>(std::shared_ptr<i_physical_object>(), &aObject));
	}

	void sprite_plane::add_physical_object(std::shared_ptr<i_physical_object> aObject)
	{
		add_object(aObject);
	}

	void sprite_plane::add_shape(i_shape& aObject)
	{
		add_object(std::shared_ptr<i_shape>(std::shared_ptr<i_shape>(), &aObject));
	}

	void sprite_plane::add_shape(std::shared_ptr<i_shape> aObject)
	{
		add_object(aObject);
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
		earth.set_position(vec3{ 0.0, -6371000.0, 0.0 });
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

	void sprite_plane::add_object(std::shared_ptr<i_object> aObject)
	{
		iNewObjects.push_back(aObject);
	}

	bool sprite_plane::is_collision_tree_2d() const
	{
		return iBroadPhaseCollisionTree2d != boost::none || iBroadPhaseCollisionTree3d == boost::none;
	}

	bool sprite_plane::is_collision_tree_3d() const 
	{
		return iBroadPhaseCollisionTree3d != boost::none;
	}
	
	const sprite_plane::broad_phase_collision_tree_2d& sprite_plane::collision_tree_2d() const
	{
		if (iBroadPhaseCollisionTree2d == boost::none)
		{
			iBroadPhaseCollisionTree2d.emplace();
			iBroadPhaseCollisionTree3d = boost::none;
			for (auto o : iObjects)
				if (o->category() == object_category::Sprite || o->category() == object_category::PhysicalObject)
					iBroadPhaseCollisionTree2d->insert(o->as_physical_object());
		}
		return *iBroadPhaseCollisionTree2d;
	}

	sprite_plane::broad_phase_collision_tree_2d& sprite_plane::collision_tree_2d()
	{
		return const_cast<broad_phase_collision_tree_2d&>(const_cast<const sprite_plane*>(this)->collision_tree_2d());
	}

	const sprite_plane::broad_phase_collision_tree_3d& sprite_plane::collision_tree_3d() const
	{
		if (iBroadPhaseCollisionTree3d == boost::none)
		{
			iBroadPhaseCollisionTree3d.emplace();
			iBroadPhaseCollisionTree2d = boost::none;
			for (auto o : iObjects)
				if (o->category() == object_category::Sprite || o->category() == object_category::PhysicalObject)
					iBroadPhaseCollisionTree3d->insert(o->as_physical_object());
		}
		return *iBroadPhaseCollisionTree3d;
	}

	sprite_plane::broad_phase_collision_tree_3d& sprite_plane::collision_tree_3d()
	{
		return const_cast<broad_phase_collision_tree_3d&>(const_cast<const sprite_plane*>(this)->collision_tree_3d());
	}

	void sprite_plane::do_add_object(std::shared_ptr<i_object> aObject)
	{
		iObjects.push_back(aObject);
		if (aObject->category() == object_category::Sprite || aObject->category() == object_category::PhysicalObject)
			is_collision_tree_2d() ? 
				collision_tree_2d().insert(aObject->as_physical_object()) :
				collision_tree_3d().insert(aObject->as_physical_object());
		if (aObject->category() == object_category::Sprite || aObject->category() == object_category::Shape)
			iRenderBuffer.push_back(&aObject->as_shape());
		iNeedsSorting = true;
	}

	void sprite_plane::sort_shapes() const
	{
		if (iNeedsSorting)
		{
			std::stable_sort(iRenderBuffer.begin(), iRenderBuffer.end(), [this](i_shape* left, i_shape* right) -> bool
			{
				if (left->killed() != right->killed())
					return left->killed() < right->killed();
				else if (iEnableZSorting)
					return left->position().z < right->position().z;
				else
					return false;
			});
			while (!iRenderBuffer.empty() && iRenderBuffer.back()->killed())
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
				if (left->killed() != right->killed())
					return left->killed() < right->killed();
				else if (left->category() == object_category::Shape || right->category() == object_category::Shape)
				{
					if (left->category() != right->category())
						return left->category() < right->category();
					else
						return &left < &right;
				}
				else
				{
					const auto& leftObject = (*left).as_physical_object();
					const auto& rightObject = (*right).as_physical_object();
					return leftObject.mass() > rightObject.mass();
				}
			});
			for (auto i = iCollisions.begin(); i != iCollisions.end();)
				if (i->first->killed() || i->second->killed())
					i = iCollisions.erase(i);
				else
					++i;
			while (!iObjects.empty() && iObjects.back()->killed())
			{
				if (iObjects.back()->category() == object_category::Sprite || iObjects.back()->category() == object_category::PhysicalObject)
					is_collision_tree_2d() ?
					collision_tree_2d().remove(iObjects.back()->as_collidable_object()) :
					collision_tree_3d().remove(iObjects.back()->as_collidable_object());
				iObjects.pop_back();
			}
			iNeedsSorting = false;
		}
	}

	void sprite_plane::update_objects()
	{
		std::lock_guard<std::recursive_mutex> lock(iUpdateMutex);
		iUpdateTime = 0ull;
		auto nowClock = std::chrono::duration_cast<chrono::flicks>(std::chrono::high_resolution_clock::now().time_since_epoch());
		auto now = to_step_time(chrono::to_seconds(nowClock), physics_step_interval());
		if (!iPhysicsTime)
			iPhysicsTime = now;
		if (*iPhysicsTime == now)
			return;
		bool updated = false;
		while (*iPhysicsTime <= now)
		{
			auto updateStartTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
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
					if (iUniformGravity != boost::none)
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
			auto process_collisions = [this, &updated](auto& tree)
			{
				if (dynamic_update_enabled())
					tree.dynamic_update(iObjects.begin(), iObjects.end());
				else
					tree.full_update(iObjects.begin(), iObjects.end());
				tree.collisions(iObjects.begin(), iObjects.end(),
					[this, &updated](i_collidable_object& o1, i_collidable_object& o2)
				{
					iCollisions.insert(std::make_pair(&o1, &o2));
					updated = true;
				});
			};
			if (is_collision_tree_2d())
				process_collisions(collision_tree_2d());
			else
				process_collisions(collision_tree_3d());
			for (auto& s : iRenderBuffer)
			{
				updated = s->update(from_step_time(*iPhysicsTime)) || updated;
				if (s->killed())
					iNeedsSorting = true;
			}
			iUpdateTime = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()) - updateStartTime).count();
			physics_applied.trigger(*iPhysicsTime);
			*iPhysicsTime += physics_step_interval();
		}
		iUpdatedSinceLastSnapshot = iUpdatedSinceLastSnapshot || updated;
	}

	bool sprite_plane::snapshot()
	{
		if (iTakingSnapshot)
			return false;
		neolib::scoped_flag sf{ iTakingSnapshot };

		bool updated = false;
		
		std::lock_guard<std::recursive_mutex> lock(iUpdateMutex);

		taking_snapshot.trigger(*iPhysicsTime);

		if (iUpdatedSinceLastSnapshot || !iNewObjects.empty())
		{
			updated = true;
			iUpdatedSinceLastSnapshot = false;
			for (const auto& c : iCollisions)
			{
				if (!c.first->collidable() || !c.second->collidable())
					continue;
				c.first->collided(*c.second);
				c.second->collided(*c.first);
				object_collision.trigger(*c.first, *c.second);
				if (c.first->killed() || c.second->killed())
					iNeedsSorting = true;
			}
			iCollisions.clear();
			for (auto& o : iNewObjects)
				do_add_object(o);
			iNewObjects.clear();
			if (iNeedsSorting)
				sort_objects();
		}
		
		return updated;
	}

	uint64_t sprite_plane::update_time() const
	{
		return iUpdateTime;
	}
}