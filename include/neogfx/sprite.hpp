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

#include "neogfx.hpp"
#include "physical_object.hpp"
#include "texture.hpp"
#include "i_image.hpp"
#include "i_sprite.hpp"

namespace neogfx
{
	class sprite : public i_sprite
	{
	public:
		typedef i_physical_object::time_point time_point;
		typedef i_physical_object::optional_time_point optional_time_point;
	private:
		typedef std::vector<std::pair<texture, optional_rect>> texture_list;
	public:
		sprite();
		sprite(const i_texture& aTexture, const optional_rect& aTextureRect);
		sprite(const i_image& aImage, const optional_rect& aTextureRect);
		sprite(const sprite& aOther);
	public:
		virtual void add_frame(const i_texture& aTexture, const optional_rect& aTextureRect);
		virtual void replace_frame(frame_index aFrameIndex, const i_texture& aTexture, const optional_rect& aTextureRect);
		virtual void remove_frame(frame_index aFrameIndex);
		virtual void set_texture_rect(frame_index aFrameIndex, const optional_rect& aTextureRect);
		virtual void set_texture_rect_for_all_frames(const optional_rect& aTextureRect);
	public:
		virtual const frame_list& animation() const;
		virtual frame_index current_frame() const;
		virtual point position() const;
		virtual neogfx::size size() const;
		virtual const vec2& scale() const;
		virtual const optional_path& path() const;
		virtual mat33 transformation() const;
		virtual void set_animation(const frame_list& aAnimation);
		virtual void set_current_frame(frame_index aFrameIndex);
		virtual void set_position(const point& aPosition);
		virtual void set_size(const optional_size& aSize, bool aCentreOrigin = true);
		virtual void set_scale(const vec2& aScale);
		virtual void set_path(const optional_path& aPath);
		virtual void set_transformation(const optional_matrix33& aTransformation);
	public:
		virtual const i_physical_object& physics() const;
		virtual i_physical_object& physics();
	public:
		virtual bool update(const optional_time_point& aNow = optional_time_point(), const vec3& aForce = vec3{});
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		texture_list iTextures;
		frame_list iAnimation;
		frame_index iCurrentFrame;
		optional_size iSize;
		vec2 iScale;
		optional_path iPath;
		mutable optional_mat33 iTransformation;
		physical_object iObject;
	};
}