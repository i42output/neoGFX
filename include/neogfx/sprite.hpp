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
#include "texture.hpp"
#include "i_image.hpp"
#include "i_sprite.hpp"

namespace neogfx
{
	class sprite : public i_sprite
	{
	private:
		typedef std::vector<std::pair<texture, optional_rect>> texture_list;
		struct physics
		{
			vector2 iVelocity;
			vector2 iAcceleration;
			scalar iSpin;
		};
		typedef boost::optional<physics> optional_physics;
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
		virtual const point& origin() const;
		virtual const point& position() const;
		virtual const optional_size& size() const;
		virtual const vector2& scale() const;
		virtual scalar angle_radians() const;
		virtual scalar angle_degrees() const;
		virtual const vector2& velocity() const;
		virtual const vector2& acceleration() const;
		virtual scalar spin_radians() const;
		virtual scalar spin_degrees() const;
		virtual const optional_path& path() const;
		virtual const matrix33& transformation() const;
		virtual void set_animation(const frame_list& aAnimation);
		virtual void set_current_frame(frame_index aFrameIndex);
		virtual void set_origin(const point& aOrigin);
		virtual void set_position(const point& aPosition);
		virtual void set_size(const optional_size& aSize);
		virtual void set_scale(const vector2& aScale);
		virtual void set_angle_radians(scalar aAngle);
		virtual void set_angle_degrees(scalar aAngle);
		virtual void set_velocity(const vector2& aVelocity);
		virtual void set_acceleration(const vector2& aAcceleration);
		virtual void set_spin_radians(scalar aSpin);
		virtual void set_spin_degrees(scalar aSpin);
		virtual void set_path(const optional_path& aPath);
	public:
		virtual bool update(const optional_time_point& aNow = optional_time_point());
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		const physics& current_physics() const;
		physics& current_physics();
		const physics& next_physics() const;
		physics& next_physics();
		bool apply_physics(double aElapsedTime);
	private:
		texture_list iTextures;
		frame_list iAnimation;
		frame_index iCurrentFrame;
		point iOrigin;
		point iPosition;
		optional_size iSize;
		vector2 iScale;
		scalar iAngle;
		optional_time_point iTimeOfLastUpdate;
		mutable optional_physics iCurrentPhysics;
		mutable optional_physics iNextPhysics;
		optional_path iPath;
		mutable matrix33 iTransformation;
	};
}