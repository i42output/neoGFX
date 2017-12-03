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
	class shape_frame : public i_shape_frame
	{
	public:
		shape_frame(const colour_or_gradient& aColour) : iColour(aColour) {}
		shape_frame(const colour_or_gradient& aColour, const mat33& aTransformation) : iColour(aColour), iTransformation(aTransformation) {}
		shape_frame(const i_texture& aTexture) : iTextures{ to_texture_list_pointer(aTexture) } {}
		shape_frame(const i_texture& aTexture, const mat33& aTransformation) : iTextures{ to_texture_list_pointer(aTexture) }, iTransformation(aTransformation) {}
		shape_frame(const i_texture& aTexture, const optional_rect& aTextureRect) : iTextures{ to_texture_list_pointer(aTexture, aTextureRect) } {}
		shape_frame(const i_texture& aTexture, const optional_rect& aTextureRect, const mat33& aTransformation) : iTextures{ to_texture_list_pointer(aTexture, aTextureRect) }, iTransformation(aTransformation) {}
		shape_frame(texture_list_pointer aTextures) : iTextures{ aTextures } {}
		shape_frame(texture_list_pointer aTextures, const mat33& aTransformation) : iTextures{ aTextures }, iTransformation(aTransformation) {}
	public:
		bool has_extents() const override { return iTextures != nullptr && !iTextures->empty(); }
		size extents() const override { return has_extents() ? (*iTextures)[0].second != boost::none ? *(*iTextures)[0].second : (*iTextures)[0].first->extents() : size{}; }
		const optional_colour_or_gradient& colour() const override { return iColour; }
		void set_colour(const optional_colour_or_gradient& aColour) override { iColour = aColour; }
		texture_list_pointer textures() const override { return iTextures; }
		void set_textures(texture_list_pointer aTextures) override { iTextures = aTextures; }
		const optional_mat33& transformation() const override { return iTransformation; }
		void set_transformation(const optional_mat33& aTransformation) override { iTransformation = aTransformation; }
	private:
		optional_colour_or_gradient iColour;
		texture_list_pointer iTextures;
		optional_mat33 iTransformation;
	};

	template <typename MixinInterface = i_shape>
	class shape : public MixinInterface
	{
	private:
		typedef std::vector<std::shared_ptr<i_shape_frame>> frame_list;
	public:
		typedef i_shape::frame_index frame_index;
		typedef i_shape::time_interval time_interval;
		typedef i_shape::animation_frame animation_frame;
		typedef i_shape::animation_frames animation_frames;
		typedef i_shape::time_interval time_interval;
		typedef i_shape::optional_time_interval optional_time_interval;
	public:
		struct animation_info
		{
			struct texture_sheet_item
			{
				point offset;
				size extents;
			};
			std::vector<texture_sheet_item> items;
			uint32_t count;
			time_interval time;
			bool repeat;
		};
		typedef boost::optional<animation_info> optional_animation_info;
	public:
		shape();
		shape(const colour& aColour);
		shape(const i_texture& aTexture, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(const i_image& aImage, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(i_shape_container& aContainer);
		shape(i_shape_container& aContainer, const colour& aColour);
		shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(i_shape_container& aContainer, const i_image& aImage, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(i_shape_container& aContainer, const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(i_shape_container& aContainer, const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo = optional_animation_info());
		shape(const shape& aOther);
		// object
	public:
		object_category category() const override;
		const i_shape& as_shape() const override;
		i_shape& as_shape() override;
		// mesh
	public:
		vertex_list_pointer vertices() const override;
		texture_list_pointer textures() const override;
		face_list faces() const override;
		mat44 transformation_matrix() const override;
		const vertex_list& transformed_vertices() const override;
	public:
		void set_vertices(vertex_list_pointer aVertices) override;
		void set_textures(texture_list_pointer aTextures) override;
		void set_faces(face_list aFaces) override;
	public:
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
		const i_shape_frame& shape_frame(frame_index aFrameIndex) const override;
		i_shape_frame& shape_frame(frame_index aFrameIndex) override;
		void add_frame(i_shape_frame& aFrame) override;
		void add_frame(std::shared_ptr<i_shape_frame> aFrame) override;
		void replace_frame(frame_index aFrameIndex, i_shape_frame& aFrame) override;
		void replace_frame(frame_index aFrameIndex, std::shared_ptr<i_shape_frame> aFrame) override;
		void remove_frame(frame_index aFrameIndex) override;
		// geometry
	public:
		const animation_frames& animation() const override;
		bool repeat_animation() const override;
		const animation_frame& current_animation_frame() const override;
		bool has_animation_finished() const override;
		void animation_finished() override;
		frame_index current_frame_index() const override;
		const i_shape_frame& current_frame() const override;
		i_shape_frame& current_frame();
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
		bool update(time_interval aNow) override;
		void paint(graphics_context& aGraphicsContext) const override;
		// udates
	public:
		virtual void clear_vertices_cache();
		// helpers
	public:
		using i_shape::set_origin;
		using i_shape::set_position;
		using i_shape::set_extents;
		// implementation
	private:
		void init_frames(const i_texture& aTexture, const optional_rect& aTextureRect, const optional_animation_info& aAnimationInfo);
		void init_frames(texture_list_pointer aTextures, const optional_animation_info& aAnimationInfo);
		// attributes
	private:
		i_shape_container* iContainer;
		frame_list iFrames;
		animation_frames iAnimation;
		bool iRepeatAnimation;
		frame_index iAnimationFrame;
		frame_index iCurrentFrame;
		optional_time_interval iTimeOfLastUpdate;
		vec3 iOrigin;
		vec3 iPosition;
		optional_vec3 iExtents;
		optional_mat44 iTransformationMatrix;
		std::pair<i_shape*, vec3> iTagOf;
		mutable vertex_list_pointer iDefaultVertices;
		mutable face_list iDefaultFaces;
		mutable vertex_list_pointer iVertices;
		mutable face_list iFaces;
		mutable vertex_list iTransformedVertices;
	};
}

#include "shape.inl"

#include "rectangle.hpp"
