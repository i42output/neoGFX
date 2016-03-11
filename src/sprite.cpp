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
		iScale{1.0, 1.0}
	{
	}

	sprite::sprite(const i_texture& aTexture, const optional_rect& aTextureRect) : 
		iCurrentFrame(0),
		iScale{1.0, 1.0}
	{
		iTextures.emplace_back(aTexture, aTextureRect);
	}

	sprite::sprite(const i_image& aImage, const optional_rect& aTextureRect) :
		iCurrentFrame(0),
		iScale{1.0, 1.0}
	{
		iTextures.emplace_back(aImage, aTextureRect);
	}

	sprite::sprite(const sprite& aOther) :
		iTextures(aOther.iTextures),
		iAnimation(aOther.iAnimation),
		iCurrentFrame(aOther.iCurrentFrame),
		iSize(aOther.iSize),
		iScale(aOther.iScale),
		iPath(aOther.iPath),
		iTransformation(aOther.iTransformation),
		iObject(aOther.iObject)
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

	point sprite::position() const
	{
		return point{ physics().position()[0], physics().position()[1] };
	}

	size sprite::size() const
	{
		if (iSize != boost::none)
			return *iSize;
		else
			return iTextures[iCurrentFrame].first.extents();
	}

	const vec2& sprite::scale() const
	{
		return iScale;
	}

	const optional_path& sprite::path() const
	{
		return iPath;
	}

	mat33 sprite::transformation() const
	{
		if (iTransformation != boost::none)
			return *iTransformation;
		auto az = physics().angle_radians()[2];
		auto pos = physics().position();
		return mat33{ { std::cos(az), -std::sin(az), 0.0 }, { std::sin(az), std::cos(az), 0.0 }, { pos[0], pos[1], 1.0 } };
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

	void sprite::set_position(const point& aPosition)
	{
		physics().set_position(aPosition.to_vector3());
	}

	void sprite::set_size(const optional_size& aSize, bool aCentreOrigin)
	{
		iSize = aSize;
	}

	void sprite::set_scale(const vec2& aScale)
	{
		iScale = aScale;
	}

	void sprite::set_path(const optional_path& aPath)
	{
		iPath = aPath;
	}

	void sprite::set_transformation(const optional_matrix33& aTransformation)
	{
		iTransformation = aTransformation;
	}

	const i_physical_object& sprite::physics() const
	{
		return iObject;
	}

	i_physical_object& sprite::physics()
	{
		return iObject;
	}

	bool sprite::update(const optional_time_point& aNow, const vec3& aForce)
	{
		return physics().update(aNow, aForce);
	}

	void sprite::paint(graphics_context& aGraphicsContext) const
	{
		auto tm = transformation();
		if (iSize != boost::none)
		{
			rect r{ vec2{-size().cx / 2.0, -size().cy / 2.0} + vec2{physics().origin().xy}, size() };
			texture_map3 map = { r.top_left().to_vector3(), r.top_right().to_vector3(), r.bottom_right().to_vector3(), r.bottom_left().to_vector3() };
			for (auto& vertex : map)
				vertex = tm * vertex;
			if (iTextures[iCurrentFrame].second == boost::none)
				aGraphicsContext.draw_texture(texture_map{ {map[0][0], map[0][1]}, {map[1][0], map[1][1]}, {map[2][0], map[2][1]}, {map[3][0], map[3][1]} }, iTextures[iCurrentFrame].first);
			else
				aGraphicsContext.draw_texture(texture_map{ {map[0][0], map[0][1]}, {map[1][0], map[1][1]}, {map[2][0], map[2][1]}, {map[3][0], map[3][1]} }, iTextures[iCurrentFrame].first, *iTextures[iCurrentFrame].second);
		}
		else
		{
			rect r{ vec2{-size().cx / 2.0, -size().cy / 2.0} + vec2{physics().origin().xy}, size() };
			texture_map3 map = { r.top_left().to_vector3(), r.top_right().to_vector3(), r.bottom_right().to_vector3(), r.bottom_left().to_vector3() };
			for (auto& vertex : map)
				vertex = tm * vertex;
			if (iTextures[iCurrentFrame].second == boost::none)
				aGraphicsContext.draw_texture(texture_map{ {map[0][0], map[0][1]}, {map[1][0], map[1][1]}, {map[2][0], map[2][1]}, {map[3][0], map[3][1]} }, iTextures[iCurrentFrame].first);
			else
				aGraphicsContext.draw_texture(texture_map{ {map[0][0], map[0][1]}, {map[1][0], map[1][1]}, {map[2][0], map[2][1]}, {map[3][0], map[3][1]} }, iTextures[iCurrentFrame].first, *iTextures[iCurrentFrame].second);
		}
	}
}