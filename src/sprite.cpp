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
	sprite::sprite() :
		iCurrentFrame(0),
		iScale{1.0, 1.0},
		iAngle{}
	{
	}

	sprite::sprite(const i_texture& aTexture, const optional_rect& aTextureRect) : 
		iCurrentFrame(0),
		iScale{1.0, 1.0},
		iAngle{}
	{
		iTextures.emplace_back(aTexture, aTextureRect);
	}

	sprite::sprite(const i_image& aImage, const optional_rect& aTextureRect) :
		iCurrentFrame(0),
		iScale{1.0, 1.0},
		iAngle{}
	{
		iTextures.emplace_back(aImage, aTextureRect);
	}

	sprite::sprite(const sprite& aOther) :
		iTextures(aOther.iTextures),
		iAnimation(aOther.iAnimation),
		iCurrentFrame(aOther.iCurrentFrame),
		iOrigin(aOther.iOrigin),
		iPosition(aOther.iPosition),
		iSize(aOther.iSize),
		iScale(aOther.iScale),
		iAngle(aOther.iAngle),
		iCurrentPhysics(aOther.iCurrentPhysics),
		iNextPhysics(aOther.iNextPhysics),
		iPath(aOther.iPath),
		iTransformation(aOther.iTransformation)
	{
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

	const i_sprite::frame_list& sprite::animation() const
	{
		return iAnimation;
	}

	i_sprite::frame_index sprite::current_frame() const
	{
		return iCurrentFrame;
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
		return next_physics().iVelocity;
	}

	const vector2& sprite::acceleration() const
	{
		return next_physics().iAcceleration;
	}

	scalar sprite::spin_radians() const
	{
		return next_physics().iSpin;
	}

	scalar sprite::spin_degrees() const
	{
		return next_physics().iSpin * 180.0 / boost::math::constants::pi<scalar>();
	}

	const optional_path& sprite::path() const
	{
		return iPath;
	}

	const matrix33& sprite::transformation() const
	{
		return iTransformation;
	}

	void sprite::set_animation(const frame_list& aAnimation)
	{
		iAnimation = aAnimation;
	}

	void sprite::set_current_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iTextures.size())
			throw bad_frame_index();
		iCurrentFrame = aFrameIndex;
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
		next_physics().iVelocity = aVelocity;
	}

	void sprite::set_acceleration(const vector2& aAcceleration)
	{
		next_physics().iAcceleration = aAcceleration;
	}

	void sprite::set_spin_radians(scalar aSpin)
	{
		next_physics().iSpin = aSpin;
	}

	void sprite::set_spin_degrees(scalar aSpin)
	{
		next_physics().iSpin = aSpin * boost::math::constants::pi<scalar>() / 180.0;
	}

	void sprite::set_path(const optional_path& aPath)
	{
		iPath = aPath;
	}

	bool sprite::update(const optional_time_point& aNow)
	{
		if (iTimeOfLastUpdate == boost::none)
			iTimeOfLastUpdate = aNow;
		bool updated = false;
		if (iTimeOfLastUpdate != boost::none && aNow != boost::none)
			updated = apply_physics((*aNow - *iTimeOfLastUpdate).count() * std::chrono::steady_clock::period::num / static_cast<double>(std::chrono::steady_clock::period::den));
		else
			updated = apply_physics(1.0);
		iTimeOfLastUpdate = aNow;
		current_physics() = next_physics();
		return updated;
	}

	void sprite::paint(graphics_context& aGraphicsContext) const
	{
		if (iSize != boost::none)
		{
			if (iTextures[iCurrentFrame].second == boost::none)
				aGraphicsContext.draw_texture(rect(iPosition, *iSize), iTextures[iCurrentFrame].first);
			else
				aGraphicsContext.draw_texture(rect(iPosition, *iSize), iTextures[iCurrentFrame].first, *iTextures[iCurrentFrame].second);
		}
		else
		{
			if (iTextures[iCurrentFrame].second == boost::none)
				aGraphicsContext.draw_texture(iPosition, iTextures[iCurrentFrame].first);
			else
				aGraphicsContext.draw_texture(iPosition, iTextures[iCurrentFrame].first, *iTextures[iCurrentFrame].second);
		}
	}

	const sprite::physics& sprite::current_physics() const
	{
		if (iCurrentPhysics == boost::none)
		{
			if (iNextPhysics != boost::none)
				iCurrentPhysics = iNextPhysics;
			else
				iCurrentPhysics = physics{};
		}
		return *iCurrentPhysics;
	}

	sprite::physics& sprite::current_physics()
	{
		return const_cast<physics&>(const_cast<const sprite*>(this)->current_physics());
	}

	const sprite::physics& sprite::next_physics() const
	{
		if (iNextPhysics == boost::none)
			iNextPhysics = physics{};
		return *iNextPhysics;
	}

	sprite::physics& sprite::next_physics()
	{
		return const_cast<physics&>(const_cast<const sprite*>(this)->next_physics());
	}

	bool sprite::apply_physics(double aElapsedTime)
	{
		next_physics().iVelocity = (current_physics().iVelocity + current_physics().iAcceleration * vector2(aElapsedTime, aElapsedTime)); // v = u + at
		point oldPosition = iPosition;
		iPosition += vector2(1.0, 1.0) * (current_physics().iVelocity * aElapsedTime + ((next_physics().iVelocity - current_physics().iVelocity) * aElapsedTime / 2.0));
		return iPosition != oldPosition;
	}
}