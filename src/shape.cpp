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
	shape::shape(i_shape_container& aContainer) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
	}

	shape::shape(i_shape_container& aContainer, const colour& aColour) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aColour));
	}

	shape::shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_rect& aTextureRect) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aTexture, aTextureRect));
	}

	shape::shape(i_shape_container& aContainer, const i_image& aImage, const optional_rect& aTextureRect) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(texture(aImage), aTextureRect));
	}

	shape::shape(const shape& aOther) :
		iContainer{aOther.iContainer},
		iFrames{aOther.iFrames},
		iAnimation{aOther.iAnimation},
		iCurrentFrame{aOther.iCurrentFrame},
		iTimeOfLastUpdate{aOther.iTimeOfLastUpdate},
		iBoundingBox{aOther.iBoundingBox},
		iZPos{aOther.iZPos},
		iScale{aOther.iScale},
		iTransformationMatrix{aOther.iTransformationMatrix}
	{
	}

	i_shape_container& shape::container() const
	{
		return iContainer;
	}

	bool shape::has_buddy() const
	{
		return container().has_buddy(*this);
	}

	i_shape& shape::buddy() const
	{
		if (!has_buddy())
			throw no_buddy();
		return container().buddy(*this);
	}

	void shape::set_buddy(i_shape& aBuddy, const vec3& aBuddyOffset)
	{
		container().set_buddy(*this, aBuddy, aBuddyOffset);
	}

	const vec3& shape::buddy_offset() const
	{
		return container().buddy_offset(*this);
	}

	void shape::set_buddy_offset(const vec3& aBuddyOffset)
	{
		container().set_buddy_offset(*this, aBuddyOffset);
	}

	void shape::unset_buddy()
	{
		container().unset_buddy(*this);
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

	vec3 shape::position_3D() const
	{
		auto xy = position();
		return vec3{xy.x, xy.y, iZPos};
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
		return mat33{ { 1.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0 },{ position().x, position().y, 1.0 } };
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

	void shape::set_position_3D(const vec3& aPosition3D)
	{
		iPosition.x = aPosition3D[0];
		iPosition.y = aPosition3D[1];
		iZPos = aPosition3D[2];
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
		auto m = map();
		for (auto& vertex : m)
			vertex = tm * vertex;
		if (current_frame().texture() != boost::none)
		{
			if (current_frame().texture_rect() == boost::none)
				aGraphicsContext.draw_texture(texture_map{ {m[0][0], m[0][1]}, {m[1][0], m[1][1]}, {m[2][0], m[2][1]}, {m[3][0], m[3][1]} }, *current_frame().texture());
			else
				aGraphicsContext.draw_texture(texture_map{ {m[0][0], m[0][1]}, {m[1][0], m[1][1]}, {m[2][0], m[2][1]}, {m[3][0], m[3][1]} }, *current_frame().texture(), *current_frame().texture_rect());
		}
		else if (current_frame().colour() != boost::none)
		{
			aGraphicsContext.fill_solid_shape(position() - origin(), m, *current_frame().colour());
		}
	}
}