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

	template <typename MixinInterface = i_shape>
	class shape : public MixinInterface
	{
	private:
		typedef std::vector<std::shared_ptr<i_frame>> frame_list;
	public:
		typedef i_mesh::triangle triangle;
		typedef i_mesh::vertex_list vertex_list;
		typedef i_mesh::vertex_index vertex_index;
		typedef i_mesh::face face;
		typedef i_mesh::face_list face_list;
	public:
		typedef i_shape::frame_index frame_index;
		typedef i_shape::time_interval time_interval;
		typedef i_shape::animation_frames animation_frames;
		typedef i_shape::time_interval time_interval;
		typedef i_shape::optional_time_interval optional_time_interval;
	public:
		shape();
		shape(const colour& aColour);
		shape(const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		shape(const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
		shape(i_shape_container& aContainer);
		shape(i_shape_container& aContainer, const colour& aColour);
		shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		shape(i_shape_container& aContainer, const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
		shape(const shape& aOther);
		// object
	public:
		object_category category() const override;
		// mesh
	public:
		const vertex_list& vertices() const override;
		const face_list& faces() const override;
		mat44 transformation_matrix() const override;
		vertex_list transformed_vertices() const override;
		// container
	public:
		const i_shape_container& container() const override;
		i_shape_container& container() override;
		// tag
	public:
		bool is_tag() const override;
		i_shape& tag_of() const override;
		void set_tag_of(i_shape& aTagOf, const vec3& aOffset = vec3{}) override;
		const vec3& tag_offset() const override;
		void set_tag_offset(const vec3& aOffset) override;
		void unset_tag_of() override;
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
		vec3 origin() const override;
		vec3 position() const override;
		vec3 extents() const override;
		rect bounding_box_2d(bool aWithPosition = true) const override;
		void set_animation(const animation_frames& aAnimation) override;
		void set_current_frame(frame_index aFrameIndex) override;
		void set_origin(const vec3& aOrigin) override;
		void set_position(const vec3& aPosition) override;
		void clear_extents() override;
		void set_extents(const vec3& aExtents) override;
		bool has_transformation_matrix() const override;
		void clear_transformation_matrix() override;
		void set_transformation_matrix(const mat33& aTransformationMatrix) override;
		void set_transformation_matrix(const mat44& aTransformationMatrix) override;
		// rendering
	public:
		bool update(const optional_time_interval& aNow = optional_time_interval{}) override;
		void paint(graphics_context& aGraphicsContext) const override;
		// udates
	public:
		virtual void clear_vertices_cache();
		// helpers
	public:
		using i_shape::set_origin;
		using i_shape::set_position;
		using i_shape::set_extents;
		// attributes
	private:
		i_shape_container* iContainer;
		frame_list iFrames;
		animation_frames iAnimation;
		frame_index iCurrentFrame;
		optional_time_interval iTimeOfLastUpdate;
		vec3 iOrigin;
		vec3 iPosition;
		optional_vec3 iExtents;
		optional_mat44 iTransformationMatrix;
		std::pair<i_shape*, vec3> iTagOf;
		mutable boost::optional<vertex_list> iVertices;
		mutable face_list iFaces;
	};
}

#include "shape.inl"

#include "rectangle.hpp"
