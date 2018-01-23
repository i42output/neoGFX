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
#include <boost/functional/hash.hpp>
#include <neolib/timer.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/game/chrono.hpp>
#include <neogfx/game/sprite.hpp>
#include <neogfx/game/aabb_quadtree.hpp>
#include <neogfx/game/aabb_octree.hpp>

namespace neogfx
{
	class sprite_plane : public widget, public i_shape_container
	{
	public:
		typedef i_physical_object::step_time_interval step_time_interval;
	public:
		event<step_time_interval> applying_physics;
		event<step_time_interval> physics_applied;
		event<graphics_context&> painting_sprites;
		event<graphics_context&> sprites_painted;
		event<i_object&, i_object&> object_collision;
		event<i_object&> object_clicked;
	public:
		typedef i_physical_object::time_interval time_interval;
		typedef i_physical_object::optional_time_interval optional_time_interval;
		typedef i_physical_object::optional_step_time_interval optional_step_time_interval;
	public:
		typedef std::shared_ptr<i_object> object_pointer;
		typedef std::vector<object_pointer> object_list;
		typedef std::vector<i_shape*> shape_list;
		typedef aabb_quadtree<> broad_phase_collision_tree_2d;
		typedef aabb_octree<> broad_phase_collision_tree_3d;
	private:
		typedef std::list<sprite, boost::fast_pool_allocator<sprite>> simple_sprite_list;
		typedef std::list<physical_object, boost::fast_pool_allocator<physical_object>> simple_object_list;
	public:
		sprite_plane();
		sprite_plane(i_widget& aParent);
		sprite_plane(i_layout& aLayout);
		~sprite_plane();
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const;
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
	public:
		virtual const i_widget& as_widget() const;
		virtual i_widget& as_widget();
	public:
		void pause_physics_while_not_rendering(bool aPausePhysicsWhileNotRendering);
		bool dynamic_update_enabled() const;
		void enable_dynamic_update(bool aEnableDynamicUpdate);
		void enable_z_sorting(bool aEnableZSorting);
	public:
		void add_sprite(i_sprite& aObject);
		void add_sprite(std::shared_ptr<i_sprite> aObject);
		void add_physical_object(i_physical_object& aObject);
		void add_physical_object(std::shared_ptr<i_physical_object> aObject);
		void add_shape(i_shape& aObject);
		void add_shape(std::shared_ptr<i_shape> aObject);
	public:
		i_sprite& create_sprite();
		i_sprite& create_sprite(const i_texture& aTexture);
		i_sprite& create_sprite(const i_image& aImage);
		i_sprite& create_sprite(const i_texture& aTexture, const rect& aTextureRect);
		i_sprite& create_sprite(const i_image& aImage, const rect& aTextureRect);
	public:
		scalar gravitational_constant() const;
		void set_gravitational_constant(scalar aG);
		const optional_vec3& uniform_gravity() const;
		void set_uniform_gravity(const optional_vec3& aUniformGravity = vec3{ 0.0, -9.80665, 0.0});
		i_physical_object& create_earth(); ///< adds gravity by simulating the earth, groundlevel at y = 0;
		i_physical_object& create_physical_object();
		const optional_step_time_interval& physics_time() const;
		void set_physics_time(const optional_step_time_interval& aTime);
		step_time_interval physics_step_interval() const;
		void set_physics_step_interval(step_time_interval aStepInterval);
	public:
		void reserve(std::size_t aCapacity);
		const object_list& objects() const;
		void add_object(std::shared_ptr<i_object> aObject);
	public:
		bool is_collision_tree_2d() const;
		bool is_collision_tree_3d() const;
		const broad_phase_collision_tree_2d& collision_tree_2d() const;
		broad_phase_collision_tree_2d& collision_tree_2d();
		const broad_phase_collision_tree_3d& collision_tree_3d() const;
		broad_phase_collision_tree_3d& collision_tree_3d();
	public:
		uint64_t update_time() const;
	private:
		void do_add_object(std::shared_ptr<i_object> aObject);
		void sort_shapes() const;
		void sort_objects();
		bool update_objects();
	private:
		neolib::callback_timer iUpdater;
		bool iPausePhysicsWhileNotRendering;
		bool iEnableDynamicUpdate;
		bool iEnableZSorting;
		bool iNeedsSorting;
		scalar iG;
		optional_vec3 iUniformGravity;
		optional_step_time_interval iPhysicsTime;
		step_time_interval iStepInterval;
		object_list iObjects;
		object_list iNewObjects;
		mutable shape_list iRenderBuffer;
		simple_sprite_list iSimpleSprites; ///< Simple sprites created by this widget (pointers to which will be available in the main sprite list)
		simple_object_list iSimpleObjects;
		mutable bool iWaitForRender;
		bool iUpdatingObjects;
		object_list::iterator iLastCollidable;
		mutable boost::optional<broad_phase_collision_tree_2d> iBroadPhaseCollisionTree2d;
		mutable boost::optional<broad_phase_collision_tree_3d> iBroadPhaseCollisionTree3d;
		uint64_t iUpdateTime;
	};
}