// shape.cpp
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
#include "shape.hpp"

namespace neogfx
{
	shape::shape() :
		iCurrentFrame(0),
		iScale{ 1.0, 1.0 }
	{
	}

	shape::shape(const colour& aColour) :
		iCurrentFrame(0),
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aColour));
	}

	shape::shape(const i_texture& aTexture, const optional_rect& aTextureRect) : 
		iCurrentFrame(0),
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aTexture, aTextureRect));
	}

	shape::shape(const i_image& aImage, const optional_rect& aTextureRect) :
		iCurrentFrame(0),
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(texture(aImage), aTextureRect));
	}

	shape::shape(const shape& aOther) :
		iFrames(aOther.iFrames),
		iAnimation(aOther.iAnimation),
		iCurrentFrame(aOther.iCurrentFrame),
		iTimeOfLastUpdate(aOther.iTimeOfLastUpdate),
		iBoundingBox(aOther.iBoundingBox),
		iScale(aOther.iScale),
		iTransformationMatrix(aOther.iTransformationMatrix)
	{
	}

	shape::frame_index shape::frame_count() const
	{
		return iFrames.size();
	}

	const i_frame& shape::frame(frame_index aFrameIndex) const
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	i_frame& shape::frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	void shape::add_frame(i_frame& aFrame)
	{
		iFrames.push_back(std::shared_ptr<i_frame>(std::shared_ptr<i_frame>(), &aFrame));
	}

	void shape::add_frame(std::shared_ptr<i_frame> aFrame)
	{
		iFrames.push_back(aFrame);
	}

	void shape::replace_frame(frame_index aFrameIndex, i_frame& aFrame)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames[aFrameIndex] = std::shared_ptr<i_frame>(std::shared_ptr<i_frame>(), &aFrame);
	}

	void shape::replace_frame(frame_index aFrameIndex, std::shared_ptr<i_frame> aFrame)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames[aFrameIndex] = aFrame;
	}

	void shape::remove_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames.erase(iFrames.begin() + aFrameIndex);
		if (iCurrentFrame >= frame_count())
			iCurrentFrame = 0;
	}

	void shape::set_texture_rect_for_all_frames(const optional_rect& aTextureRect)
	{
		for (auto& t : iFrames)
			t->set_texture_rect(aTextureRect);
	}

	const shape::animation_frames& shape::animation() const
	{
		return iAnimation;
	}

	const i_frame& shape::current_frame() const
	{
		return frame(iCurrentFrame);
	}

	i_frame& shape::current_frame()
	{
		return frame(iCurrentFrame);
	}

	point shape::origin() const
	{
		return iOrigin;
	}

	point shape::position() const
	{
		return iPosition;
	}

	rect shape::bounding_box() const
	{
		if (iBoundingBox != boost::none)
			return *iBoundingBox;
		else if (frame_count() > 0 && current_frame().has_extents())
			return rect{ origin() - current_frame().extents() / size{2.0}, current_frame().extents() };
		else
			return rect{ origin(), size{} };
	}

	const vec2& shape::scale() const
	{
		return iScale;
	}

	bool shape::has_transformation_matrix() const
	{
		return iTransformationMatrix != boost::none;
	}

	mat33 shape::transformation_matrix() const
	{
		if (iTransformationMatrix != boost::none)
			return *iTransformationMatrix;
		return mat33{};
	}

	void shape::set_animation(const animation_frames& aAnimation)
	{
		iAnimation = aAnimation;
	}

	void shape::set_current_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iCurrentFrame = aFrameIndex;
	}

	void shape::set_origin(const point& aOrigin)
	{
		iOrigin = aOrigin;
	}

	void shape::set_position(const point& aPosition)
	{
		iPosition = aPosition;
	}

	void shape::set_bounding_box(const optional_rect& aBoundingBox)
	{
		iBoundingBox = aBoundingBox;
	}

	void shape::set_scale(const vec2& aScale)
	{
		iScale = aScale;
	}

	void shape::set_transformation_matrix(const optional_matrix33& aTransformationMatrix)
	{
		iTransformationMatrix = aTransformationMatrix;
	}
	
	bool shape::update(const optional_time_point& aNow)
	{
		/* todo: animate frames */
		return false;
	}

	void shape::paint(graphics_context& aGraphicsContext) const
	{
		if (frame_count() == 0)
			return;
		auto tm = transformation_matrix();
		rect r = bounding_box();
		texture_map3 map = { r.top_left().to_vector3(), r.top_right().to_vector3(), r.bottom_right().to_vector3(), r.bottom_left().to_vector3() };
		for (auto& vertex : map)
			vertex = tm * vertex;
		if (current_frame().texture() != boost::none)
		{
			if (current_frame().texture_rect() == boost::none)
				aGraphicsContext.draw_texture(texture_map{ {map[0][0], map[0][1]}, {map[1][0], map[1][1]}, {map[2][0], map[2][1]}, {map[3][0], map[3][1]} }, *current_frame().texture());
			else
				aGraphicsContext.draw_texture(texture_map{ {map[0][0], map[0][1]}, {map[1][0], map[1][1]}, {map[2][0], map[2][1]}, {map[3][0], map[3][1]} }, *current_frame().texture(), *current_frame().texture_rect());
		}
	}
}