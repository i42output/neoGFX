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

#include "neogfx.hpp"
#include <chrono>
#include "app.hpp"
#include "sprite_plane.hpp"

namespace neogfx
{
	sprite_plane::sprite_plane()
	{
	}

	sprite_plane::sprite_plane(i_widget& aParent) :
		widget(aParent)
	{
		surface().native_surface().rendering_check([this]()
		{
			if (update_objects())
				update();
		}, this);
	}

	sprite_plane::sprite_plane(i_layout& aLayout) :
		widget(aLayout)
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
		for (const auto& s : iSprites)
			s->paint(aGraphicsContext);
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
		iSimpleSprites.push_back(sprite());
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_texture& aTexture, const optional_rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(aTexture, aTextureRect);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_image& aImage, const optional_rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(aImage, aTextureRect);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	void sprite_plane::add_object(i_physical_object& aObject)
	{
		iObjects.push_back(std::shared_ptr<i_physical_object>(std::shared_ptr<i_physical_object>(), &aObject));
	}

	void sprite_plane::add_object(std::shared_ptr<i_physical_object> aObject)
	{
		iObjects.push_back(aObject);
	}

	void sprite_plane::add_earth()
	{
		auto& earth = create_object();
		earth.set_origin({0.0, 6371000.0, 0.0});
		earth.set_mass(5.972e24);
	}

	i_physical_object& sprite_plane::create_object()
	{
		iSimpleObjects.push_back(physical_object());
		add_object(iSimpleObjects.back());
		return iSimpleObjects.back();
	}

	const sprite_plane::sprite_list& sprite_plane::sprites() const
	{
		return iSprites;
	}

	sprite_plane::sprite_list& sprite_plane::sprites()
	{
		return iSprites;
	}

	bool sprite_plane::update_objects()
	{
		applying_physics.trigger();
		auto now = std::chrono::steady_clock::now();
		bool updated = false;
		for (auto& s : iSprites)
			updated = (s->update(now) || updated);
		for (auto& o : iObjects)
			updated = (o->update(now) || updated);
		physics_applied.trigger();
		return updated;
	}
}