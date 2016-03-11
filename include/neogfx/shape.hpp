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

#include "neogfx.hpp"
#include "colour.hpp"
#include "texture.hpp"
#include "i_image.hpp"
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
		virtual bool has_extents() const { return iTextureRect != boost::none; }
		virtual size extents() const { return iTextureRect != boost::none ? iTextureRect->extents() : size{}; }
		virtual const optional_colour& colour() const { return iColour; }
		virtual void set_colour(const optional_colour& aColour) { iColour = aColour; }
		virtual const optional_texture& texture() const { return iTexture; }
		virtual void set_texture(const optional_texture& aTexture) { iTexture = aTexture; }
		virtual const optional_rect& texture_rect() const { return iTextureRect; }
		virtual void set_texture_rect(const optional_rect& aTextureRect) { iTextureRect = aTextureRect; }
		virtual const optional_mat33& transformation() const { return iTransformation; }
		virtual void set_transformation(const optional_mat33& aTransformation) { iTransformation = aTransformation; }
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
		shape();
		shape(const colour& aColour);
		shape(const i_texture& aTexture, const optional_rect& aTextureRect);
		shape(const i_image& aImage, const optional_rect& aTextureRect);
		shape(const shape& aOther);
	public:
		virtual frame_index frame_count() const;
		virtual const i_frame& frame(frame_index aFrameIndex) const;
		virtual i_frame& frame(frame_index aFrameIndex);
		virtual void add_frame(i_frame& aFrame);
		virtual void add_frame(std::shared_ptr<i_frame> aFrame);
		virtual void replace_frame(frame_index aFrameIndex, i_frame& aFrame);
		virtual void replace_frame(frame_index aFrameIndex, std::shared_ptr<i_frame> aFrame);
		virtual void remove_frame(frame_index aFrameIndex);
		virtual void set_texture_rect_for_all_frames(const optional_rect& aTextureRect);
	public:
		virtual const animation_frames& animation() const;
		virtual const i_frame& current_frame() const;
		virtual i_frame& current_frame();
		virtual point origin() const;
		virtual point position() const;
		virtual rect bounding_box() const;
		virtual const vec2& scale() const;
		virtual bool has_transformation_matrix() const;
		virtual mat33 transformation_matrix() const;
		virtual void set_animation(const animation_frames& aAnimation);
		virtual void set_current_frame(frame_index aFrameIndex);
		virtual void set_origin(const point& aOrigin);
		virtual void set_position(const point& aPosition);
		virtual void set_bounding_box(const optional_rect& aBoundingBox);
		virtual void set_scale(const vec2& aScale);
		virtual void set_transformation_matrix(const optional_matrix33& aTransformationMatrix);
	public:
		virtual bool update(const optional_time_point& aNow = optional_time_point());
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		frame_list iFrames;
		animation_frames iAnimation;
		frame_index iCurrentFrame;
		optional_time_point iTimeOfLastUpdate;
		point iOrigin;
		point iPosition;
		optional_rect iBoundingBox;
		vec2 iScale;
		mutable optional_mat33 iTransformationMatrix;
	};
}