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
	inline shape<MixinInterface>::shape() :
		iContainer{ nullptr }, iCurrentFrame{ 0 }
	{
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const colour& aColour) :
		iContainer{ nullptr }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aColour));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const i_texture& aTexture, const optional_rect& aTextureRect) :
		iContainer{ nullptr }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aTexture, aTextureRect));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const i_image& aImage, const optional_rect& aTextureRect) :
		iContainer{ nullptr }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(texture(aImage), aTextureRect));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer) :
		iContainer{ &aContainer }, iCurrentFrame{ 0 }
	{
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const colour& aColour) :
		iContainer{ &aContainer }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aColour));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_rect& aTextureRect) :
		iContainer{ &aContainer }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(aTexture, aTextureRect));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_image& aImage, const optional_rect& aTextureRect) :
		iContainer{ &aContainer }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::frame>(texture(aImage), aTextureRect));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const shape& aOther) :
		iContainer{ aOther.iContainer },
		iFrames{ aOther.iFrames },
		iAnimation{ aOther.iAnimation },
		iCurrentFrame{ aOther.iCurrentFrame },
		iTimeOfLastUpdate{ aOther.iTimeOfLastUpdate },
		iOrigin{ aOther.iOrigin },
		iPosition{ aOther.iPosition },
		iExtents{ aOther.iExtents },
		iTransformationMatrix{aOther.iTransformationMatrix}
	{
	}

	template <typename MixinInterface>
	inline const vec3_list& shape<MixinInterface>::vertices() const
	{
		if (iVertices = boost::none)
		{
			auto r = bounding_box_2d();
			iVertices = vec3_list{ r.top_left().to_vec3(), r.top_right().to_vec3(), r.bottom_right().to_vec3(), r.bottom_left().to_vec3() };
		}
		return *iVertices;
	}

	template <typename MixinInterface>
	inline const typename shape<MixinInterface>::face_list& shape<MixinInterface>::faces() const
	{
		if (iFaces.empty())
			iFaces = face_list{ { 0, 1, 2 },{ 0, 3, 2 } };
		return iFaces;
	}

	template <typename MixinInterface>
	inline bool shape<MixinInterface>::has_transformation_matrix() const
	{
		return iTransformationMatrix != boost::none;
	}

	template <typename MixinInterface>
	inline mat44 shape<MixinInterface>::transformation_matrix() const
	{
		if (iTransformationMatrix != boost::none)
			return *iTransformationMatrix;
		return mat44{ { 1.0, 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0, 0.0 }, { position().x, position().y, position().z, 1.0 } };
	}
	
	template <typename MixinInterface>
	inline vec3_list shape<MixinInterface>::transformed_vertices() const
	{
		vec3_list result;
		result.reserve(vertices().size());
		for (auto const& v : vertices())
			result.push_back((transformation_matrix() * vec4 { v.x, v.y, v.z, 0.0 }).xyz);
		return result;
	}

	template <typename MixinInterface>
	inline const i_shape_container& shape<MixinInterface>::container() const
	{
		if (iContainer != nullptr)
			return *iContainer;
		throw no_shape_container();
	}

	template <typename MixinInterface>
	inline i_shape_container& shape<MixinInterface>::container()
	{
		if (iContainer != nullptr)
			return *iContainer;
		throw no_shape_container();
	}

	template <typename MixinInterface>
	inline bool shape<MixinInterface>::is_tag() const
	{
		return iTagOf.first != nullptr;
	}

	template <typename MixinInterface>
	inline i_shape& shape<MixinInterface>::tag_of() const
	{
		if (iTagOf.first == nullptr)
			throw not_a_tag();
		return *iTagOf.first;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_tag_of(i_shape& aTagOf, const vec3& aOffset)
	{
		iTagOf.first = &aTagOf;
		iTagOf.second = aOffset;
	}

	template <typename MixinInterface>
	inline const vec3& shape<MixinInterface>::tag_offset() const
	{
		if (iTagOf.first == nullptr)
			throw not_a_tag();
		return iTagOf.second;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_tag_offset(const vec3& aOffset)
	{
		if (iTagOf.first == nullptr)
			throw not_a_tag();
		iTagOf.second = aOffset;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::unset_tag_of()
	{
		iTagOf.first = nullptr;
		iTagOf.second = vec3{};
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
	inline vec3 shape<MixinInterface>::origin() const
	{
		return iOrigin;
	}

	template <typename MixinInterface>
	inline vec3 shape<MixinInterface>::position() const
	{
		return is_tag() ? tag_of().position() + tag_offset() : iPosition;
	}

	template <typename MixinInterface>
	inline vec3 shape<MixinInterface>::extents() const
	{
		if (iExtents != boost::none)
			return *iExtents;
		else if (frame_count() > 0 && current_frame().has_extents())
			return vec3{ current_frame().extents().cx, current_frame().extents().cy, 0.0 };
		else
			return vec3{};
	}

	template <typename MixinInterface>
	inline rect shape<MixinInterface>::bounding_box_2d() const
	{
		return rect{ point{ origin() - extents() / 2.0 + position() }, size{ extents() } };
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
	inline void shape<MixinInterface>::set_origin(const vec3& aOrigin)
	{
		iOrigin = aOrigin;
		clear_vertices_cache();
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_position(const vec3& aPosition)
	{
		iPosition = aPosition;
		clear_vertices_cache();
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::clear_extents()
	{
		iExtents = boost::none;
		clear_vertices_cache();
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_extents(const vec3& aExtents)
	{
		iExtents = aExtents;
		clear_vertices_cache();
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::clear_transformation_matrix()
	{
		iTransformationMatrix = boost::none;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_transformation_matrix(const mat33& aTransformationMatrix)
	{
		iTransformationMatrix = mat44{
			{ aTransformationMatrix[0][0], aTransformationMatrix[0][1], aTransformationMatrix[0][2], 0.0 },
			{ aTransformationMatrix[1][0], aTransformationMatrix[1][1], aTransformationMatrix[1][2], 0.0 },
			{ aTransformationMatrix[2][0], aTransformationMatrix[2][1], aTransformationMatrix[2][2], 0.0 },
			{ 0.0, 0.0, 0.0, 1.0 } };
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_transformation_matrix(const mat44& aTransformationMatrix)
	{
		iTransformationMatrix = aTransformationMatrix;
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
		if (current_frame().texture() != boost::none)
		{
			if (current_frame().texture_rect() == boost::none)
				aGraphicsContext.draw_texture(*this, *current_frame().texture());
			else
				aGraphicsContext.draw_texture(*this, *current_frame().texture(), *current_frame().texture_rect());
		}
		else if (current_frame().colour() != boost::none)
		{
			aGraphicsContext.fill_shape(*this, *current_frame().colour());
		}
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::clear_vertices_cache()
	{
		iVertices = boost::none;
	}
}