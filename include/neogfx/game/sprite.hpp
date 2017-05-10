// sprite.hpp
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
#include "shape.hpp"
#include "physical_object.hpp"
#include "i_sprite.hpp"

namespace neogfx
{
	class sprite : public shape, public i_sprite
	{
		// types
	public:
		using i_sprite::time_point;
		using i_sprite::optional_time_point;
		// construction
	public:
		sprite(i_shape_container& aContainer);
		sprite(i_shape_container& aContainer, const colour& aColour);
		sprite(i_shape_container& aContainer, const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		sprite(i_shape_container& aContainer, const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
		sprite(const sprite& aOther);
		// container/buddy
	public:
		i_shape_container& container() const override;
		bool has_buddy() const override;
		i_shape& buddy() const override;
		void set_buddy(i_shape& aBuddy, const vec3& aBuddyOffset = vec3{}) override;
		const vec3& buddy_offset() const override;
		void set_buddy_offset(const vec3& aBuddyOffset) override;
		void unset_buddy() override;
		// animation
	public:
		frame_index frame_count() const override;
		const i_frame& frame(frame_index aFrameIndex) const override;
		i_frame& frame(frame_index aFrameIndex) override;
		void add_frame(i_frame& aFrame) override;
		void add_frame(std::shared_ptr<i_frame> aFrame) override;
		void replace_frame(frame_index aFrameIndex, i_frame& aFrame) override;
		void replace_frame(frame_index aFrameIndex, std::shared_ptr<i_frame> aFrame) override;
		void remove_frame(frame_index aFrameIndex) override;
		void set_texture_rect_for_all_frames(const optional_rect& aTextureRect) override;
		// geometry
	public:
		const animation_frames& animation() const override;
		const i_frame& current_frame() const override;
		i_frame& current_frame() override;
		point origin() const override;
		point position() const override;
		vec3 position_3D() const override;
		rect bounding_box() const override;
		const vec2& scale() const override;
		bool has_transformation_matrix() const override;
		mat33 transformation_matrix() const override;
		const optional_path& path() const override;
		void set_animation(const animation_frames& aAnimation) override;
		void set_current_frame(frame_index aFrameIndex) override;
		void set_origin(const point& aOrigin) override;
		void set_position(const point& aPosition) override;
		void set_position_3D(const vec3& aPosition3D) override;
		void set_bounding_box(const optional_rect& aBoundingBox) override;
		void set_scale(const vec2& aScale) override;
		void set_transformation_matrix(const optional_mat33& aTransformationMatrix) override;
		void set_path(const optional_path& aPath) override;
		// physics
	public:
		const i_physical_object& physics() const override;
		i_physical_object& physics() override;
		bool update(const optional_time_point& aNow, const vec3& aForce) override;
		// rendering
	public:
		std::size_t vertex_count(bool aIncludeCentre = false) const override;
		vec3_list vertices(bool aIncludeCentre = false) const override;
		vec3_list transformed_vertices(bool aIncludeCentre = false) const override;
		bool update(const optional_time_point& aNow) override;
		void paint(graphics_context& aGraphicsContext) const override;
		// attributes
	private:
		optional_path iPath;
		physical_object iObject;
	};
}