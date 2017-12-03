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

#include <neogfx/gfx/graphics_context.hpp>

namespace neogfx
{
	template <typename MixinInterface>
	inline shape<MixinInterface>::shape() :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame { 0 }
	{
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const colour& aColour) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::shape_frame>(aColour));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const i_texture& aTexture, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(aTexture, optional_rect{}, aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const i_image& aImage, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(texture{ aImage }, optional_rect{}, aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(aTexture, aTextureRect, aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(texture{ aImage }, aTextureRect, aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const colour& aColour) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		iFrames.push_back(std::make_shared<neogfx::shape_frame>(aColour));
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(aTexture, optional_rect{} , aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_image& aImage, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(texture{ aImage }, optional_rect{}, aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(aTexture, aTextureRect, aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(i_shape_container& aContainer, const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }
	{
		init_frames(texture{ aImage }, aTextureRect, aAnimationInfo);
	}

	template <typename MixinInterface>
	inline shape<MixinInterface>::shape(const shape& aOther) :
		iContainer{ aOther.iContainer },
		iFrames{ aOther.iFrames },
		iAnimation{ aOther.iAnimation },
		iRepeatAnimation{ aOther.iRepeatAnimation },
		iAnimationFrame{ aOther.iAnimationFrame },
		iCurrentFrame{ aOther.iCurrentFrame },
		iTimeOfLastUpdate{ aOther.iTimeOfLastUpdate },
		iOrigin{ aOther.iOrigin },
		iPosition{ aOther.iPosition },
		iExtents{ aOther.iExtents },
		iTransformationMatrix{aOther.iTransformationMatrix}
	{
	}

	template <typename MixinInterface>
	inline object_category shape<MixinInterface>::category() const
	{
		return object_category::Shape;
	}

	template <typename MixinInterface>
	inline const i_shape& shape<MixinInterface>::as_shape() const
	{
		return *this;
	}

	template <typename MixinInterface>
	inline i_shape& shape<MixinInterface>::as_shape()
	{
		return *this;
	}

	template <typename MixinInterface>
	inline vertex_list_pointer shape<MixinInterface>::vertices() const
	{
		if (iVertices != nullptr)
			return iVertices;
		if (iDefaultVertices == nullptr)
			iDefaultVertices = std::make_shared<vertex_list>();
		if (iDefaultVertices->empty())
		{
			auto r = bounding_box_2d(false);
			iDefaultVertices->assign(
				{ vertex{ r.top_left().to_vec3(), vec2{ 0.0, 0.0 } },
				vertex{ r.top_right().to_vec3(), vec2{ 1.0, 0.0 } },
				vertex{ r.bottom_right().to_vec3(), vec2{ 1.0, 1.0 } },
				vertex{ r.bottom_left().to_vec3(), vec2{ 0.0, 1.0 } } });
		}
		return iDefaultVertices;
	}

	template <typename MixinInterface>
	inline texture_list_pointer shape<MixinInterface>::textures() const
	{
		return shape_frame(0).textures();
	}

	template <typename MixinInterface>
	inline face_list_pointer shape<MixinInterface>::faces() const
	{
		if (iFaces != nullptr)
			return iFaces;
		if (iDefaultFaces == nullptr)
			iDefaultFaces = std::make_shared<face_list>();
		if (iDefaultFaces->empty())
			iDefaultFaces->assign({ face{ 0, 1, 2 }, face{ 0, 3, 2 } });
		return iDefaultFaces;
	}

	template <typename MixinInterface>
	inline mat44 shape<MixinInterface>::transformation_matrix() const
	{
		if (iTransformationMatrix == boost::none)
			return mat44{ { 1.0, 0.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 }, { position().x, position().y, position().z, 1.0 } };
		return *iTransformationMatrix;
	}
	
	template <typename MixinInterface>
	inline const vertex_list& shape<MixinInterface>::transformed_vertices() const
	{
		if (!iTransformedVertices.empty())
			return iTransformedVertices;
		iTransformedVertices.reserve(vertices()->size());
		for (auto const& v : *vertices())
			iTransformedVertices.push_back(vertex{ (transformation_matrix() * vec4 { v.coordinates.x, v.coordinates.y, v.coordinates.z, 1.0 }).xyz, v.textureCoordinates });
		return iTransformedVertices;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_vertices(vertex_list_pointer aVertices)
	{
		iVertices = aVertices;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_textures(texture_list_pointer aTextures)
	{
		for (auto& f : iFrames)
			f->set_textures(aTextures);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::set_faces(face_list_pointer aFaces)
	{
		iFaces = aFaces;
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
	inline const i_shape_frame& shape<MixinInterface>::shape_frame(frame_index aFrameIndex) const
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	template <typename MixinInterface>
	inline i_shape_frame& shape<MixinInterface>::shape_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::add_frame(i_shape_frame& aFrame)
	{
		iFrames.push_back(std::shared_ptr<i_shape_frame>(std::shared_ptr<i_shape_frame>(), &aFrame));
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::add_frame(std::shared_ptr<i_shape_frame> aFrame)
	{
		iFrames.push_back(aFrame);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::replace_frame(frame_index aFrameIndex, i_shape_frame& aFrame)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames[aFrameIndex] = std::shared_ptr<i_shape_frame>(std::shared_ptr<i_shape_frame>(), &aFrame);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::replace_frame(frame_index aFrameIndex, std::shared_ptr<i_shape_frame> aFrame)
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
	inline const i_shape::animation_frames& shape<MixinInterface>::animation() const
	{
		return iAnimation;
	}

	template <typename MixinInterface>
	inline bool shape<MixinInterface>::repeat_animation() const
	{
		return iRepeatAnimation;
	}

	template <typename MixinInterface>
	inline typename const shape<MixinInterface>::animation_frame& shape<MixinInterface>::current_animation_frame() const
	{
		static const animation_frame sNullAnimationFrame{ 0, 0 };
		if (!animation().empty())
			return iAnimationFrame < iAnimation.size() ? iAnimation[iAnimationFrame] : iAnimation.back();
		else
			return sNullAnimationFrame;
	}

	template <typename MixinInterface>
	inline bool shape<MixinInterface>::has_animation_finished() const
	{
		return iAnimation.empty() || (iAnimationFrame == iAnimation.size() && !repeat_animation());
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::animation_finished()
	{
	}

	template <typename MixinInterface>
	inline typename shape<MixinInterface>::frame_index shape<MixinInterface>::current_frame_index() const
	{
		return has_animation_finished() ? iCurrentFrame : current_animation_frame().first;
	}

	template <typename MixinInterface>
	inline const i_shape_frame& shape<MixinInterface>::current_frame() const
	{
		return shape_frame(current_frame_index());
	}

	template <typename MixinInterface>
	inline i_shape_frame& shape<MixinInterface>::current_frame()
	{
		return shape_frame(current_frame_index());
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
	inline rect shape<MixinInterface>::bounding_box_2d(bool aWithPosition) const
	{
		return rect{ point{ origin() - extents() / 2.0 + (aWithPosition ? position() : vec3{}) }, size{ extents() } };
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
	inline bool shape<MixinInterface>::has_transformation_matrix() const
	{
		return iTransformationMatrix != boost::none;
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
	inline bool shape<MixinInterface>::update(time_interval aNow)
	{
		bool updated = false;
		if (iTimeOfLastUpdate == boost::none)
			iTimeOfLastUpdate = aNow;
		auto left = aNow - *iTimeOfLastUpdate;
		while (!has_animation_finished() && left >= current_animation_frame().second)
		{
			left -= current_animation_frame().second;
			*iTimeOfLastUpdate += current_animation_frame().second;
			++iAnimationFrame;
			if (iAnimationFrame == iAnimation.size())
			{
				if (repeat_animation())
					iAnimationFrame = 0;
				else
					animation_finished();
			}
			updated = true;
		}
		return updated;
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::paint(graphics_context& aGraphicsContext) const
	{
		if (frame_count() == 0)
			return;
		if (current_frame().textures() != nullptr)
			aGraphicsContext.draw_textures(*this, current_frame().textures(), current_frame().colour() && current_frame().colour()->is<colour>() ? static_variant_cast<colour>(*current_frame().colour()) : optional_colour{});
		else if (current_frame().colour() != boost::none)
			aGraphicsContext.fill_shape(*this, to_brush(*current_frame().colour()));
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::clear_vertices_cache()
	{
		if (iDefaultVertices != nullptr)
			iDefaultVertices->clear();
		iTransformedVertices.clear();
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::init_frames(const i_texture& aTexture, const optional_rect& aTextureRect, const optional_animation_info& aAnimationInfo)
	{
		init_frames(to_texture_list_pointer(aTexture, aTextureRect), aAnimationInfo);
	}

	template <typename MixinInterface>
	inline void shape<MixinInterface>::init_frames(texture_list_pointer aTextures, const optional_animation_info& aAnimationInfo)
	{
		if (aAnimationInfo == boost::none)
		{
			iFrames.push_back(std::make_shared<neogfx::shape_frame>(aTextures));
		}
		else
		{
			iFrames.reserve(aAnimationInfo->count);
			for (std::size_t i = 0; i < aAnimationInfo->count; ++i)
				iFrames.push_back(std::make_shared<neogfx::shape_frame>(std::make_shared<texture_list>()));
			std::vector<point> pos;
			pos.reserve(aTextures->size());
			for (std::size_t i = 0; i < aTextures->size(); ++i)
				pos.push_back(aAnimationInfo->items[i].offset);
			iRepeatAnimation = aAnimationInfo->repeat;
			frame_index frameIndex = 0;
			while (frameIndex < aAnimationInfo->count)
			{
				for (texture_list::size_type textureSourceIndex = 0; textureSourceIndex < aTextures->size(); ++textureSourceIndex)
				{
					auto const& textureSource = (*aTextures)[textureSourceIndex];
					auto sheetRect = textureSource.second ? *textureSource.second : rect{ point{}, textureSource.first->extents() };
					iFrames[frameIndex]->textures()->push_back(texture_source{ textureSource.first, rect{ pos[textureSourceIndex], aAnimationInfo->items[textureSourceIndex].extents } });
					pos[textureSourceIndex].x += aAnimationInfo->items[textureSourceIndex].extents.cx;
					if (pos[textureSourceIndex].x >= sheetRect.right())
					{
						pos[textureSourceIndex].x = aAnimationInfo->items[textureSourceIndex].offset.x;
						pos[textureSourceIndex].y += aAnimationInfo->items[textureSourceIndex].extents.cy;
					}
				}
				iAnimation.push_back(std::make_pair(frameIndex, aAnimationInfo->time));
				++frameIndex;
			}
		}
	}
}