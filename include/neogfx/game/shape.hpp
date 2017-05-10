// shape.hpp
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
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/i_image.hpp>
#include "i_shape.hpp"

namespace neogfx
{
	class frame : public i_frame
	{
	public:
		frame(const neogfx::colour& aColour) : iColour(aColour) {}
		frame(const neogfx::colour& aColour, const mat33& aTransformation) : iColour(aColour), iTransformation(aTransformation) {}
		frame(const i_texture& aTexture) : iTexture(aTexture) {}
		frame(const i_texture& aTexture, const mat33& aTransformation) : iTexture(aTexture), iTransformation(aTransformation) {}
		frame(const i_texture& aTexture, const optional_rect& aTextureRect) : iTexture(aTexture), iTextureRect(aTextureRect) {}
		frame(const i_texture& aTexture, const optional_rect& aTextureRect, const mat33& aTransformation) : iTexture(aTexture), iTextureRect(aTextureRect), iTransformation(aTransformation) {}
	public:
		bool has_extents() const override { return iTextureRect != boost::none; }
		size extents() const override { return iTextureRect != boost::none ? iTextureRect->extents() : size{}; }
		const optional_colour& colour() const override { return iColour; }
		void set_colour(const optional_colour& aColour) override { iColour = aColour; }
		const optional_texture& texture() const override { return iTexture; }
		void set_texture(const optional_texture& aTexture) override { iTexture = aTexture; }
		const optional_rect& texture_rect() const override { return iTextureRect; }
		void set_texture_rect(const optional_rect& aTextureRect) override { iTextureRect = aTextureRect; }
		const optional_mat33& transformation() const override { return iTransformation; }
		void set_transformation(const optional_mat33& aTransformation) override { iTransformation = aTransformation; }
	private:
		optional_colour iColour;
		optional_texture iTexture;
		optional_rect iTextureRect;
		optional_mat33 iTransformation;
	};

	class shape : public i_shape
	{
	private:
		typedef std::vector<std::shared_ptr<i_frame>> frame_list;
	public:
		shape(i_shape_container& aContainer);
		shape(i_shape_container& aContainer, const colour& aColour);
		shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		shape(i_shape_container& aContainer, const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
		shape(const shape& aOther);
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
		i_frame& current_frame();
		point origin() const override;
		point position() const override;
		vec3 position_3D() const override;
		rect bounding_box() const override;
		const vec2& scale() const override;
		bool has_transformation_matrix() const override;
		mat33 transformation_matrix() const override;
		void set_animation(const animation_frames& aAnimation) override;
		void set_current_frame(frame_index aFrameIndex) override;
		void set_origin(const point& aOrigin) override;
		void set_position(const point& aPosition) override;
		void set_position_3D(const vec3& aPosition3D) override;
		void set_bounding_box(const optional_rect& aBoundingBox) override;
		void set_scale(const vec2& aScale) override;
		void set_transformation_matrix(const optional_matrix33& aTransformationMatrix) override;
		// rendering
	public:
		std::size_t vertex_count(bool aIncludeCentre = false) const override;
		vec3_list vertices(bool aIncludeCentre = false) const override;
		vec3_list transformed_vertices(bool aIncludeCentre = false) const override;
		bool update(const optional_time_point& aNow = optional_time_point()) override;
		void paint(graphics_context& aGraphicsContext) const override;
		// attributes
	private:
		i_shape_container& iContainer;
		frame_list iFrames;
		animation_frames iAnimation;
		frame_index iCurrentFrame;
		optional_time_point iTimeOfLastUpdate;
		point iOrigin;
		point iPosition;
		coordinate iZPos;
		optional_rect iBoundingBox;
		vec2 iScale;
		mutable optional_mat33 iTransformationMatrix;
	};
}

#include "rectangle.hpp"