// shape.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/game/shape.hpp>
#include <neogfx/gfx/graphics_context.hpp>

namespace neogfx
{
	shape::shape() :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame { 0 }, iKilled{ false }
	{
	}

	shape::shape(const colour& aColour) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		iFrames.push_back(std::make_shared<neogfx::shape_frame>(aColour));
	}

	shape::shape(const i_texture& aTexture, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(aTexture, optional_rect{}, aAnimationInfo);
	}

	shape::shape(const i_image& aImage, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(texture{ aImage }, optional_rect{}, aAnimationInfo);
	}

	shape::shape(const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(aTexture, aTextureRect, aAnimationInfo);
	}

	shape::shape(const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ nullptr }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(texture{ aImage }, aTextureRect, aAnimationInfo);
	}

	shape::shape(i_shape_container& aContainer) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
	}

	shape::shape(i_shape_container& aContainer, const colour& aColour) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		iFrames.push_back(std::make_shared<neogfx::shape_frame>(aColour));
	}

	shape::shape(i_shape_container& aContainer, const i_texture& aTexture, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(aTexture, optional_rect{} , aAnimationInfo);
	}

	shape::shape(i_shape_container& aContainer, const i_image& aImage, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(texture{ aImage }, optional_rect{}, aAnimationInfo);
	}

	shape::shape(i_shape_container& aContainer, const i_texture& aTexture, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(aTexture, aTextureRect, aAnimationInfo);
	}

	shape::shape(i_shape_container& aContainer, const i_image& aImage, const rect& aTextureRect, const optional_animation_info& aAnimationInfo) :
		iContainer{ &aContainer }, iRepeatAnimation{ true }, iAnimationFrame{ 0 }, iCurrentFrame{ 0 }, iKilled{ false }
	{
		init_frames(texture{ aImage }, aTextureRect, aAnimationInfo);
	}

	shape::shape(const shape& aOther) :
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
		iTransformationMatrix{aOther.iTransformationMatrix},
		iKilled{false}
	{
	}

	object_category shape::category() const
	{
		return object_category::Shape;
	}

	const i_shape& shape::as_shape() const
	{
		return *this;
	}

	i_shape& shape::as_shape()
	{
		return *this;
	}

	const i_collidable_object& shape::as_collidable_object() const
	{
		throw not_a_collidable_object();
	}

	i_collidable_object& shape::as_collidable_object()
	{
		throw not_a_collidable_object();
	}

	const i_physical_object& shape::as_physical_object() const
	{
		throw not_a_physical_object();
	}

	i_physical_object& shape::as_physical_object()
	{
		throw not_a_physical_object();
	}

	bool shape::killed() const
	{
		return iKilled;
	}

	void shape::kill()
	{
		iKilled = true;
	}

	vertex_list_pointer shape::vertices() const
	{
		if (iVertices != nullptr)
			return iVertices;
		if (iDefaultVertices == nullptr)
			iDefaultVertices = std::make_shared<vertex_list>();
		if (iDefaultVertices->empty())
		{
			auto r = bounding_box_2d(false).with_centred_origin();
			iDefaultVertices->assign(
				{ vertex{ r.top_left().to_vec3(), vec2{ 0.0, 0.0 } },
				vertex{ r.top_right().to_vec3(), vec2{ 1.0, 0.0 } },
				vertex{ r.bottom_right().to_vec3(), vec2{ 1.0, 1.0 } },
				vertex{ r.bottom_left().to_vec3(), vec2{ 0.0, 1.0 } } });
		}
		return iDefaultVertices;
	}

	texture_list_pointer shape::textures() const
	{
		if (frame_count() > 0)
			return shape_frame(0).textures();
		else
			return texture_list_pointer{};
	}

	face_list shape::faces() const
	{
		if (!iFaces.empty())
		{
			return iActiveFaces.empty() ? iFaces : iActiveFaces;
		}
		if (iDefaultFaces.empty())
		{
			iDefaultFaces = std::make_shared<face_list::container>();
			iDefaultFaces.faces().assign({ face{ 0, 1, 2 }, face{ 0, 3, 2 } });
		}
		return iDefaultFaces;
	}

	face_list shape::active_faces() const
	{
		return iActiveFaces;
	}

	void shape::activate_faces(face_list aActiveFaces) const
	{
		iActiveFaces = aActiveFaces;
	}

	mat44 shape::transformation_matrix() const
	{
		if (iTransformationMatrix == std::nullopt)
			return mat44{ { 1.0, 0.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 }, { position().x, position().y, position().z, 1.0 } };
		return *iTransformationMatrix;
	}
	
	const vertex_list& shape::transformed_vertices() const
	{
		if (!iTransformedVertices.empty())
			return iTransformedVertices;
		iTransformedVertices.reserve(vertices()->size());
		for (auto const& v : *vertices())
			iTransformedVertices.push_back(vertex{ (transformation_matrix() * vec4 { v.coordinates.x, v.coordinates.y, v.coordinates.z, 1.0 }).xyz, v.textureCoordinates });
		return iTransformedVertices;
	}

	void shape::set_vertices(vertex_list_pointer aVertices)
	{
		iVertices = aVertices;
	}

	void shape::set_textures(texture_list_pointer aTextures)
	{
		if (iFrames.empty())
			iFrames.push_back(std::make_shared<neogfx::shape_frame>(aTextures));
		else
			for (auto& f : iFrames)
				f->set_textures(aTextures);
	}

	void shape::set_faces(face_list aFaces)
	{
		iFaces = aFaces;
	}

	const i_shape_container& shape::container() const
	{
		if (iContainer != nullptr)
			return *iContainer;
		throw no_shape_container();
	}

	i_shape_container& shape::container()
	{
		if (iContainer != nullptr)
			return *iContainer;
		throw no_shape_container();
	}

	bool shape::is_tag() const
	{
		return iTagOf.first != nullptr;
	}

	i_shape& shape::tag_of() const
	{
		if (iTagOf.first == nullptr)
			throw not_a_tag();
		return *iTagOf.first;
	}

	void shape::set_tag_of(i_shape& aTagOf, const vec3& aOffset)
	{
		iTagOf.first = &aTagOf;
		iTagOf.second = aOffset;
	}

	const vec3& shape::tag_offset() const
	{
		if (iTagOf.first == nullptr)
			throw not_a_tag();
		return iTagOf.second;
	}

	void shape::set_tag_offset(const vec3& aOffset)
	{
		if (iTagOf.first == nullptr)
			throw not_a_tag();
		iTagOf.second = aOffset;
	}

	void shape::unset_tag_of()
	{
		iTagOf.first = nullptr;
		iTagOf.second = vec3{};
	}

	i_shape::frame_index shape::frame_count() const
	{
		return iFrames.size();
	}

	const i_shape_frame& shape::shape_frame(frame_index aFrameIndex) const
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	i_shape_frame& shape::shape_frame(frame_index aFrameIndex)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		return *iFrames[aFrameIndex];
	}

	void shape::add_frame(i_shape_frame& aFrame)
	{
		iFrames.push_back(std::shared_ptr<i_shape_frame>(std::shared_ptr<i_shape_frame>(), &aFrame));
	}

	void shape::add_frame(std::shared_ptr<i_shape_frame> aFrame)
	{
		iFrames.push_back(aFrame);
	}

	void shape::replace_frame(frame_index aFrameIndex, i_shape_frame& aFrame)
	{
		if (aFrameIndex >= iFrames.size())
			throw bad_frame_index();
		iFrames[aFrameIndex] = std::shared_ptr<i_shape_frame>(std::shared_ptr<i_shape_frame>(), &aFrame);
	}

	void shape::replace_frame(frame_index aFrameIndex, std::shared_ptr<i_shape_frame> aFrame)
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

	const i_shape::animation_frames& shape::animation() const
	{
		return iAnimation;
	}

	bool shape::repeat_animation() const
	{
		return iRepeatAnimation;
	}

	typename const shape::animation_frame& shape::current_animation_frame() const
	{
		static const animation_frame sNullAnimationFrame{ 0, 0 };
		if (!animation().empty())
			return iAnimationFrame < iAnimation.size() ? iAnimation[iAnimationFrame] : iAnimation.back();
		else
			return sNullAnimationFrame;
	}

	bool shape::has_animation_finished() const
	{
		return iAnimation.empty() || (iAnimationFrame == iAnimation.size() && !repeat_animation());
	}

	void shape::animation_finished()
	{
	}

	typename shape::frame_index shape::current_frame_index() const
	{
		return has_animation_finished() ? iCurrentFrame : current_animation_frame().first;
	}

	const i_shape_frame& shape::current_frame() const
	{
		return shape_frame(current_frame_index());
	}

	i_shape_frame& shape::current_frame()
	{
		return shape_frame(current_frame_index());
	}

	vec3 shape::origin() const
	{
		return iOrigin;
	}

	vec3 shape::position() const
	{
		return is_tag() ? tag_of().position() + tag_offset() : iPosition;
	}

	vec3 shape::extents() const
	{
		if (iExtents != std::nullopt)
			return *iExtents;
		else if (frame_count() > 0 && current_frame().has_extents())
			return vec3{ current_frame().extents().cx, current_frame().extents().cy, 0.0 };
		else
			return vec3{};
	}

	rect shape::bounding_box_2d(bool aWithPosition) const
	{
		return rect{ point{ origin() - extents() / 2.0 + (aWithPosition ? position() : vec3{}) }, size{ extents() } };
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

	void shape::set_origin(const vec3& aOrigin)
	{
		iOrigin = aOrigin;
		clear_vertices_cache();
	}

	void shape::set_position(const vec3& aPosition)
	{
		iPosition = aPosition;
		clear_vertices_cache();
	}

	void shape::clear_extents()
	{
		iExtents = std::nullopt;
		clear_vertices_cache();
	}

	void shape::set_extents(const vec3& aExtents)
	{
		iExtents = aExtents;
		clear_vertices_cache();
	}

	bool shape::has_transformation_matrix() const
	{
		return iTransformationMatrix != std::nullopt;
	}

	void shape::clear_transformation_matrix()
	{
		iTransformationMatrix = std::nullopt;
	}

	void shape::set_transformation_matrix(const mat33& aTransformationMatrix)
	{
		set_transformation_matrix(mat44{
			{ aTransformationMatrix[0][0], aTransformationMatrix[0][1], aTransformationMatrix[0][2], 0.0 },
			{ aTransformationMatrix[1][0], aTransformationMatrix[1][1], aTransformationMatrix[1][2], 0.0 },
			{ aTransformationMatrix[2][0], aTransformationMatrix[2][1], aTransformationMatrix[2][2], 0.0 },
			{ 0.0, 0.0, 0.0, 1.0 } });
	}

	void shape::set_transformation_matrix(const mat44& aTransformationMatrix)
	{
		iTransformationMatrix = aTransformationMatrix;
	}
	
	bool shape::update(time_interval aNow)
	{
		bool updated = false;
		if (iTimeOfLastUpdate == std::nullopt)
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

	void shape::paint(graphics_context& aGraphicsContext) const
	{
		if (frame_count() == 0)
			return;
		if (current_frame().textures() != nullptr)
			aGraphicsContext.draw_textures(*this, current_frame().textures(), current_frame().colour() && std::holds_alternative<colour>(*current_frame().colour()) ? static_variant_cast<colour>(*current_frame().colour()) : optional_colour{});
		else if (current_frame().colour() != std::nullopt)
			aGraphicsContext.fill_shape(*this, to_brush(*current_frame().colour()));
	}

	void shape::clear_vertices_cache()
	{
		if (iDefaultVertices != nullptr)
			iDefaultVertices->clear();
		iTransformedVertices.clear();
	}

	void shape::init_frames(const i_texture& aTexture, const optional_rect& aTextureRect, const optional_animation_info& aAnimationInfo)
	{
		init_frames(to_texture_list_pointer(aTexture, aTextureRect), aAnimationInfo);
	}

	void shape::init_frames(texture_list_pointer aTextures, const optional_animation_info& aAnimationInfo)
	{
		if (aAnimationInfo == std::nullopt)
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