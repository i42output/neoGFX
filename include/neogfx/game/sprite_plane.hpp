// sprite_plane.hpp
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
#include <boost/pool/pool_alloc.hpp>
#include <neolib/timer.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include "sprite.hpp"

namespace neogfx
{
	class sprite_plane : public widget, public i_shape_container
	{
	public:
		event<> applying_physics;
		event<> physics_applied;
		event<graphics_context&> painting_sprites;
		event<graphics_context&> sprites_painted;
	public:
		typedef std::vector<std::shared_ptr<i_shape>> shape_list;
		typedef std::vector<std::shared_ptr<i_sprite>> sprite_list;
		typedef std::vector<std::shared_ptr<i_physical_object>> object_list;
		typedef std::map<const i_shape*, std::pair<i_shape*, vec3>> buddy_list;
	private:
		typedef std::list<sprite, boost::fast_pool_allocator<sprite>> simple_sprite_list;
		typedef std::list<physical_object, boost::fast_pool_allocator<physical_object>> simple_object_list;
	public:
		struct no_buddy : std::logic_error { no_buddy() : std::logic_error("neogfx::sprite_plane::no_buddy") {} };
		struct buddy_exists : std::logic_error { buddy_exists() : std::logic_error("neogfx::sprite_plane::buddy_exists") {} };
	public:
		sprite_plane();
		sprite_plane(i_widget& aParent);
		sprite_plane(i_layout& aLayout);
		~sprite_plane();
	public:
		virtual void parent_changed();
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const;
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual const i_widget& as_widget() const;
		virtual i_widget& as_widget();
	public:
		virtual bool has_buddy(const i_shape& aShape) const;
		virtual i_shape& buddy(const i_shape& aShape) const;
		virtual void set_buddy(const i_shape& aShape, i_shape& aBuddy, const vec3& aBuddyOffset = vec3{});
		virtual const vec3& buddy_offset(const i_shape& aShape) const;
		virtual void set_buddy_offset(const i_shape& aShape, const vec3& aBuddyOffset);
		virtual void unset_buddy(const i_shape& aShape);
	public:
		void enable_z_sorting(bool aEnableZSorting);
	public:
		void add_shape(i_shape& aShape);
		void add_shape(std::shared_ptr<i_shape> aShape);
	public:
		void add_sprite(i_sprite& aSprite);
		void add_sprite(std::shared_ptr<i_sprite> aSprite);
		i_sprite& create_sprite();
		i_sprite& create_sprite(const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		i_sprite& create_sprite(const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
	public:
		scalar gravitational_constant() const;
		void set_gravitational_constant(scalar aG);
		const optional_vec3& uniform_gravity() const;
		void set_uniform_gravity(const optional_vec3& aUniformGravity = vec3{ 0.0, -9.80665, 0.0});
		void add_object(i_physical_object& aObject);
		void add_object(std::shared_ptr<i_physical_object> aObject);
		i_physical_object& create_earth(); ///< adds gravity by simulating the earth, groundlevel at y = 0;
		i_physical_object& create_object();
	public:
		const shape_list& shapes() const;
		shape_list& shapes();
		const sprite_list& sprites() const;
		sprite_list& sprites();
		const object_list& objects() const;
		object_list& objects();
		const buddy_list& buddies() const;
		buddy_list& buddies();
	private:
		bool update_objects();
	private:
		sink iSink;
		bool iEnableZSorting;
		scalar iG;
		optional_vec3 iUniformGravity;
		shape_list iShapes;
		sprite_list iSprites;
		object_list iObjects;
		buddy_list iBuddies;
		simple_sprite_list iSimpleSprites; ///< Simple sprites created by this widget (pointers to which will be available in the main sprite list)
		simple_object_list iSimpleObjects;
		mutable std::vector<i_shape*> iRenderBuffer;
		mutable std::vector<i_physical_object*> iUpdateBuffer;
	};
}