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
		iEnableZSorting(false),
		iG(6.67408e-11)
	{
	}

	sprite_plane::sprite_plane(i_widget& aParent) :
		widget(aParent), iEnableZSorting(false), iG(6.67408e-11)
	{
		surface().native_surface().rendering_check([this]()
		{
			if (update_objects())
				update();
		}, this);
	}

	sprite_plane::sprite_plane(i_layout& aLayout) :
		widget(aLayout), iEnableZSorting(false), iG(6.67408e-11)
	{
		surface().native_surface().rendering_check([this]()
		{
			if (update_objects())
				update();
		}, this);
	}

	sprite_plane::~sprite_plane()
	{
		if (has_surface() && !surface().destroyed())
			surface().native_surface().rendering_check.unsubscribe(this);
	}

	void sprite_plane::parent_changed()
	{
		widget::parent_changed();
		surface().native_surface().rendering_check([this]()
		{
			if (update_objects())
				update();
		}, this);
	}

	logical_coordinate_system sprite_plane::logical_coordinate_system() const
	{
		return neogfx::logical_coordinate_system::AutomaticGame;
	}

	void sprite_plane::paint(graphics_context& aGraphicsContext) const
	{	
		painting_sprites.trigger(aGraphicsContext);
		if (iEnableZSorting)
		{
			iRenderBuffer.reserve(iShapes.size() + iSprites.size());
			iRenderBuffer.clear();
			for (const auto& s : iShapes)
			{
				if ((s->bounding_box() + s->position()).intersection(client_rect()).empty())
					continue;
				iRenderBuffer.push_back(&*s);
			}
			for (const auto& s : iSprites)
			{
				if ((s->bounding_box() + s->position()).intersection(client_rect()).empty())
					continue;
				iRenderBuffer.push_back(&*s);
			}
			std::stable_sort(iRenderBuffer.begin(), iRenderBuffer.end(), [](i_shape* left, i_shape* right) ->bool
			{
				return left->position_3D()[2] < right->position_3D()[2];
			});
			for (auto s : iRenderBuffer)
				s->paint(aGraphicsContext);
		}
		else
		{
			for (const auto& s : iShapes)
			{
				if ((s->bounding_box() + s->position()).intersection(client_rect()).empty())
					continue;
				s->paint(aGraphicsContext);
			}
			for (const auto& s : iSprites)
			{
				if ((s->bounding_box() + s->position()).intersection(client_rect()).empty())
					continue;
				s->paint(aGraphicsContext);
			}
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
		iShapes.push_back(std::shared_ptr<i_shape>(std::shared_ptr<i_shape>(), &aShape));
	}

	void sprite_plane::add_shape(std::shared_ptr<i_shape> aShape)
	{
		iShapes.push_back(aShape);
	}

	void sprite_plane::add_sprite(i_sprite& aSprite)
	{
		iSprites.push_back(std::shared_ptr<i_sprite>(std::shared_ptr<i_sprite>(), &aSprite));
	}

	void sprite_plane::add_sprite(std::shared_ptr<i_sprite> aSprite)
	{
		iSprites.push_back(aSprite);
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
		iObjects.push_back(std::shared_ptr<i_physical_object>(std::shared_ptr<i_physical_object>(), &aObject));
	}

	void sprite_plane::add_object(std::shared_ptr<i_physical_object> aObject)
	{
		iObjects.push_back(aObject);
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

	const sprite_plane::shape_list& sprite_plane::shapes() const
	{
		return iShapes;
	}

	sprite_plane::shape_list& sprite_plane::shapes()
	{
		return iShapes;
	}

	const sprite_plane::sprite_list& sprite_plane::sprites() const
	{
		return iSprites;
	}

	sprite_plane::sprite_list& sprite_plane::sprites()
	{
		return iSprites;
	}

	const sprite_plane::object_list& sprite_plane::objects() const
	{
		return iObjects;
	}

	sprite_plane::object_list& sprite_plane::objects()
	{
		return iObjects;
	}

	const sprite_plane::buddy_list& sprite_plane::buddies() const
	{
		return iBuddies;
	}

	sprite_plane::buddy_list& sprite_plane::buddies()
	{
		return iBuddies;
	}

	bool sprite_plane::update_objects()
	{
		applying_physics.trigger();
		auto now = std::chrono::steady_clock::now();
		bool updated = false;
		iUpdateBuffer.reserve(iSprites.size() + iObjects.size());
		iUpdateBuffer.clear();
		for (const auto& s : iSprites)
			iUpdateBuffer.push_back(&s->physics());
		for (const auto& s : iObjects)
			iUpdateBuffer.push_back(&*s);
		std::stable_sort(iUpdateBuffer.begin(), iUpdateBuffer.end(), [](i_physical_object* left, i_physical_object* right) ->bool
		{
			return left->mass() > right->mass();
		});
		for (auto& o2 : iUpdateBuffer)
		{
			vec3 totalForce;
			if (o2->mass() == 0.0)
				continue;
			if (iUniformGravity != boost::none)
				totalForce = *iUniformGravity * o2->mass();
			else if (iG != 0.0)
			{
				for (auto& o1 : iUpdateBuffer)
				{
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
			updated = (o2->update(now, totalForce) || updated);
		}
		physics_applied.trigger();
		return updated;
	}
}