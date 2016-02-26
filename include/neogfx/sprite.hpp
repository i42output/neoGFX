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
#include "i_sprite.hpp"

namespace neogfx
{
	class sprite : public i_sprite
	{
	private:
		typedef std::vector<std::pair<texture, optional_rect>> texture_list;
	public:
		sprite(time_unit_e aTimeUnit = TimeUnitSecond);
		sprite(const i_texture& aTexture, const optional_rect& aTextureRect, time_unit_e aTimeUnit = TimeUnitSecond);
	public:
		virtual void add_frame(const i_texture& aTexture, const optional_rect& aTextureRect);
		virtual void replace_frame(frame_index aFrameIndex, const i_texture& aTexture, const optional_rect& aTextureRect);
		virtual void remove_frame(frame_index aFrameIndex);
		virtual void set_texture_rect(frame_index aFrameIndex, const optional_rect& aTextureRect);
		virtual void set_texture_rect_for_all_frames(const optional_rect& aTextureRect);
	public:
		virtual time_unit_e time_unit() const;
		virtual const frame_list& animation() const;
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
		virtual void set_time_unit(time_unit_e aTimeUnit);
		virtual void set_animation(const frame_list& aAnimation);
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
		virtual void update();
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		time_unit_e iTimeUnit;
		texture_list iTextures;
		frame_list iAnimation;
		point iOrigin;
		point iPosition;
		optional_size iSize;
		vector2 iScale;
		scalar iAngle;
		vector2 iVelocity;
		vector2 iAcceleration;
		scalar iSpin;
		optional_path iPath;
		mutable matrix33 iTransformation;
	};
}