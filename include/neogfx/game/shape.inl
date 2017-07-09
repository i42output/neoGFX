// shape.inl
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

namespace neogfx
{
	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const colour& aColour) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aColour));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_rect& aTextureRect) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aTexture, aTextureRect));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_image& aImage, const optional_rect& aTextureRect) :
		iContainer{aContainer},
		iCurrentFrame{0},
		iZPos{0.0},
		iScale{1.0, 1.0}
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(texture(aImage), aTextureRect));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const shape& aOther) :
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

	template <typename MixinInterface>
	inline i_shape_container& shape<MixinInterface>::container() const
	{
		return iContainer;
	}

	template <typename MixinInterface>
	inline bool shape<MixinInterface>::has_buddy() const
	{
		return container().has_buddy(*this);
	}

	template <typename MixinInterface>
	inline i_shape& shape<MixinInterface>::buddy() const
	{
		if (!has_buddy())
			throw no_buddy();
		return container().buddy(*this);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_buddy(i_shape& aBuddy, const vec3& aBuddyOffset)
	{
		container().set_buddy(*this, aBuddy, aBuddyOffset);
	}

	template <typename MixinInterface>
	inline const vec3& shape<MixinInterface>::buddy_offset() const
	{
		return container().buddy_offset(*this);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_buddy_offset(const vec3& aBuddyOffset)
	{
		container().set_buddy_offset(*this, aBuddyOffset);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::unset_buddy()
	{
		container().unset_buddy(*this);
	}

	template <typename MixinInterface>
	inline i_shape::frame_index shape<MixinInterface>::frame_count() const
	{
		return iFrames.size();
	}

	template <typename MixinInterface>
	inline const i_frame& shape<MixinInterface>::frame(frame_index aFrameIndex) const
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	template <typename MixinInterface>
	inline i_frame& shape<MixinInterface>::frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::add_frame(i_frame& aFrame)
	{
		iFrames.push_back(std::shared_ptr<i_frame>(std::shared_ptr<i_frame>(), &aFrame));
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::add_frame(std::shared_ptr<i_frame> aFrame)
	{
		iFrames.push_back(aFrame);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::replace_frame(frame_index aFrameIndex, i_frame& aFrame)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames[aFrameIndex] = std::shared_ptr<i_frame>(std::shared_ptr<i_frame>(), &aFrame);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::replace_frame(frame_index aFrameIndex, std::shared_ptr<i_frame> aFrame)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames[aFrameIndex] = aFrame;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::remove_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames.erase(iFrames.begin() + aFrameIndex);
		if (iCurrentFrame >= frame_count())
			iCurrentFrame = 0;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_texture_rect_for_all_frames(const optional_rect& aTextureRect)
	{
		for (auto& t : iFrames)
			t->set_texture_rect(aTextureRect);
	}

	template <typename MixinInterface>
	inline const i_shape::animation_frames& shape<MixinInterface>::animation() const
	{
		return iAnimation;
	}

	template <typename MixinInterface>
	inline const i_frame& shape<MixinInterface>::current_frame() const
	{
		return frame(iCurrentFrame);
	}

	template <typename MixinInterface>
	inline i_frame& shape<MixinInterface>::current_frame()
	{
		return frame(iCurrentFrame);
	}

	template <typename MixinInterface>
	inline point shape<MixinInterface>::origin() const
	{
		return iOrigin;
	}

	template <typename MixinInterface>
	inline point shape<MixinInterface>::position() const
	{
		return iPosition;
	}

	template <typename MixinInterface>
	inline vec3 shape<MixinInterface>::position_3D() const
	{
		auto xy = position();
		return vec3{xy.x, xy.y, iZPos};
	}

	template <typename MixinInterface>
	inline rect shape<MixinInterface>::bounding_box() const
	{
		if (iBoundingBox != boost::none)
			return *iBoundingBox;
		else if (frame_count() > 0 && current_frame().has_extents())
			return rect{ origin() - current_frame().extents() / size{2.0}, current_frame().extents() };
		else
			return rect{ origin(), size{} };
	}

	template <typename MixinInterface>
	inline const vec2& shape<MixinInterface>::scale() const
	{
		return iScale;
	}

	template <typename MixinInterface>
	inline bool shape<MixinInterface>::has_transformation_matrix() const
	{
		return iTransformationMatrix != boost::none;
	}

	template <typename MixinInterface>
	inline mat33 shape<MixinInterface>::transformation_matrix() const
	{
		if (iTransformationMatrix != boost::none)
			return *iTransformationMatrix;
		return mat33{ { 1.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0 },{ position().x, position().y, 1.0 } };
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_animation(const animation_frames& aAnimation)
	{
		iAnimation = aAnimation;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_current_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iCurrentFrame = aFrameIndex;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_origin(const point& aOrigin)
	{
		iOrigin = aOrigin;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_position(const point& aPosition)
	{
		iPosition = aPosition;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_position_3D(const vec3& aPosition3D)
	{
		iPosition.x = aPosition3D[0];
		iPosition.y = aPosition3D[1];
		iZPos = aPosition3D[2];
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_bounding_box(const optional_rect& aBoundingBox)
	{
		iBoundingBox = aBoundingBox;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_scale(const vec2& aScale)
	{
		iScale = aScale;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_transformation_matrix(const optional_matrix33& aTransformationMatrix)
	{
		iTransformationMatrix = aTransformationMatrix;
	}
	
	template <typename MixinInterface>
	inline std::size_t shape<MixinInterface>::vertex_count(bool aIncludeCentre) const
	{
		return aIncludeCentre ? 1 : 0;
	}
	
	template <typename MixinInterface>
	inline vec3_list shape<MixinInterface>::vertices(bool aIncludeCentre) const
	{
		vec3_list result;
		result.reserve(vertex_count(aIncludeCentre));
		if (aIncludeCentre)
			result.push_back(origin().to_vector3());
		return result;
	}

	template <typename MixinInterface>
	inline vec3_list shape<MixinInterface>::transformed_vertices(bool aIncludeCentre) const
	{
		vec3_list result = vertices(aIncludeCentre);
		auto tm = transformation_matrix();
		for (auto& vertex : result)
			vertex = tm * vertex;
		return result;
	}

	template <typename MixinInterface>
	inline bool shape<MixinInterface>::update(const optional_time_interval& aNow)
	{
		/* todo: animate frames */
		iTimeOfLastUpdate = aNow;
		return false;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::paint(graphics_context& aGraphicsContext) const
	{
		if (frame_count() == 0)
			return;
		auto m = transformed_vertices(current_frame().texture() == boost::none);
		if (current_frame().texture() != boost::none)
		{
			if (current_frame().texture_rect() == boost::none)
				aGraphicsContext.draw_texture(texture_map{ {m[0][0], m[0][1]}, {m[1][0], m[1][1]}, {m[2][0], m[2][1]}, {m[3][0], m[3][1]} }, *current_frame().texture());
			else
				aGraphicsContext.draw_texture(texture_map{ {m[0][0], m[0][1]}, {m[1][0], m[1][1]}, {m[2][0], m[2][1]}, {m[3][0], m[3][1]} }, *current_frame().texture(), *current_frame().texture_rect());
		}
		else if (current_frame().colour() != boost::none)
		{
			aGraphicsContext.fill_shape(m, *current_frame().colour());
		}
	}
}