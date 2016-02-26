// sprite.cpp
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

#include "neogfx.hpp"
#include <boost/math/constants/constants.hpp>
#include "sprite.hpp"

namespace neogfx
{
	sprite::sprite(time_unit_e aTimeUnit) : iTimeUnit{ aTimeUnit }, iScale{1.0, 1.0}, iAngle{}, iSpin{}
	{
	}

	sprite::sprite(const i_texture& aTexture, const optional_rect& aTextureRect, time_unit_e aTimeUnit) : iTimeUnit{ aTimeUnit }, iScale{1.0, 1.0}, iAngle{}, iSpin{}
	{
		iTextures.emplace_back(aTexture, aTextureRect);
	}

	void sprite::add_frame(const i_texture& aTexture, const optional_rect& aTextureRect)
	{
		iTextures.emplace_back(aTexture, aTextureRect);
	}

	void sprite::replace_frame(frame_index aFrameIndex, const i_texture& aTexture, const optional_rect& aTextureRect)
	{
		if (aFrameIndex >= iTextures.size())
			throw bad_frame_index();
		iTextures[aFrameIndex].first = aTexture;
		iTextures[aFrameIndex].second = aTextureRect;
	}
	
	void sprite::remove_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iTextures.size())
			throw bad_frame_index();
		iTextures.erase(iTextures.begin() + aFrameIndex);
	}
	
	void sprite::set_texture_rect(frame_index aFrameIndex, const optional_rect& aTextureRect)
	{
		if (aFrameIndex >= iTextures.size())
			throw bad_frame_index();
		iTextures[aFrameIndex].second = aTextureRect;
	}

	void sprite::set_texture_rect_for_all_frames(const optional_rect& aTextureRect)
	{
		for (auto& t : iTextures)
			t.second = aTextureRect;
	}

	i_sprite::time_unit_e sprite::time_unit() const
	{
		return iTimeUnit;
	}

	const i_sprite::frame_list& sprite::animation() const
	{
		return iAnimation;
	}

	const point& sprite::origin() const
	{
		return iOrigin;
	}

	const point& sprite::position() const
	{
		return iPosition;
	}

	const optional_size& sprite::size() const
	{
		return iSize;
	}

	const vector2& sprite::scale() const
	{
		return iScale;
	}

	scalar sprite::angle_radians() const
	{
		return iAngle;
	}

	scalar sprite::angle_degrees() const
	{
		return iAngle * 180.0 / boost::math::constants::pi<double>();
	}

	const vector2& sprite::velocity() const
	{
		return iVelocity;
	}

	const vector2& sprite::acceleration() const
	{
		return iAcceleration;
	}

	scalar sprite::spin_radians() const
	{
		return iSpin;
	}

	scalar sprite::spin_degrees() const
	{
		return iSpin * 180.0 / boost::math::constants::pi<scalar>();
	}

	const optional_path& sprite::path() const
	{
		return iPath;
	}

	const matrix33& sprite::transformation() const
	{
		return iTransformation;
	}

	void sprite::set_time_unit(time_unit_e aTimeUnit)
	{
		iTimeUnit = aTimeUnit;
	}

	void sprite::set_animation(const frame_list& aAnimation)
	{
		iAnimation = aAnimation;
	}

	void sprite::set_origin(const point& aOrigin)
	{
		iOrigin = aOrigin;
	}

	void sprite::set_position(const point& aPosition)
	{
		iPosition = aPosition;
	}

	void sprite::set_size(const optional_size& aSize)
	{
		iSize = aSize;
	}

	void sprite::set_scale(const vector2& aScale)
	{
		iScale = aScale;
	}

	void sprite::set_angle_radians(scalar aAngle)
	{
		iAngle = aAngle;
	}

	void sprite::set_angle_degrees(scalar aAngle)
	{
		iAngle = aAngle * boost::math::constants::pi<scalar>() / 180.0;
	}

	void sprite::set_velocity(const vector2& aVelocity)
	{
		iVelocity = aVelocity;
	}

	void sprite::set_acceleration(const vector2& aAcceleration)
	{
		iAcceleration = aAcceleration;
	}

	void sprite::set_spin_radians(scalar aSpin)
	{
		iSpin = aSpin;
	}

	void sprite::set_spin_degrees(scalar aSpin)
	{
		iSpin = aSpin * boost::math::constants::pi<scalar>() / 180.0;
	}

	void sprite::set_path(const optional_path& aPath)
	{
		iPath = aPath;
	}

	void sprite::update()
	{
		/* todo */
	}

	void sprite::paint(graphics_context& aGraphicsContext) const
	{
		/* todo */
	}
}