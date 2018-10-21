// opengl_graphics_context.cpp
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

#include <neogfx/neogfx.hpp>
#include <boost/math/constants/constants.hpp>
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/text/i_glyph_texture.hpp>
#include <neogfx/game/shape_factory.hpp>
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include "../../hid/native/i_native_surface.hpp"
#include "i_native_texture.hpp"
#include "../text/native/i_native_font_face.hpp"
#include "opengl_graphics_context.hpp"
#include "opengl_renderer.hpp" // todo: remove this #include when base class interface abstraction complete

namespace neogfx
{
	namespace 
	{
		template <typename T>
		class partition_iterator
		{
		public:
			typedef T value_type;
		public:
			partition_iterator(const value_type* aBegin, const value_type* aEnd, std::size_t aPartitionCount = 2u, std::size_t aSkipAmount = 2u, bool aRepeat = false) :
				iBegin{ aBegin }, iEnd{ aEnd }, iPartitionCount{ aPartitionCount }, iSkipAmount{ std::min<std::size_t>(aEnd - aBegin, aSkipAmount) }, iRepeat{ aRepeat }, iNext { aBegin }, iPass{ 1u }
			{
			}
			partition_iterator() :
				iBegin{ nullptr }, iEnd{ nullptr}, iPartitionCount{ 0u }, iSkipAmount{ 0u }, iRepeat{ false}, iNext{ nullptr }, iPass{ 1u }
			{
			}
		public:
			std::size_t partition_count() const
			{
				return iPartitionCount;
			}
			std::size_t skip_amount() const
			{
				return iSkipAmount;
			}
			std::size_t pass() const
			{
				return iPass;
			}
		public:
			partition_iterator& operator++()
			{
				if (static_cast<std::size_t>(iEnd - iNext) <= iSkipAmount)
				{
					if (iPass < iPartitionCount)
						iNext = iBegin + (iRepeat ? iPass % iSkipAmount : iPass);
					else
						iNext = iEnd;
					++iPass;
				}
				else
					iNext += iSkipAmount;
				return *this;
			}
			const value_type& operator*() const
			{
				return *iNext;
			}
			bool operator==(const graphics_operation::operation* aTest) const
			{
				return iNext == aTest;
			}
			bool operator!=(const graphics_operation::operation* aTest) const
			{
				return !operator==(aTest);
			}
		private:
			const value_type* iBegin;
			const value_type* iEnd;
			std::size_t iPartitionCount;
			std::size_t iSkipAmount;
			bool iRepeat;
			const value_type* iNext;
			std::size_t iPass;
		};

		inline std::size_t scanline_offsets(const text_effect& aEffect)
		{
			if (aEffect.type() != text_effect_type::Outline)
				return 1u;
			else
				return static_cast<std::size_t>(aEffect.width() * 2.0 + 1.0);
		}

		inline std::size_t offsets(const text_effect& aEffect)
		{
			return scanline_offsets(aEffect) * scanline_offsets(aEffect);
		}

		inline std::size_t barrier_partitions(const text_effect& aEffect)
		{
			return offsets(aEffect) * 2u;
		}

		inline GLenum path_shape_to_gl_mode(path::shape_type_e aShape)
		{
			switch (aShape)
			{
			case path::Quads:
				return GL_QUADS;
			case path::Lines:
				return GL_LINES;
			case path::LineLoop:
				return GL_LINE_LOOP;
			case path::LineStrip:
				return GL_LINE_STRIP;
			case path::ConvexPolygon:
				return GL_TRIANGLE_FAN;
			default:
				return GL_POINTS;
			}
		}

		inline GLenum path_shape_to_gl_mode(const path& aPath)
		{
			return path_shape_to_gl_mode(aPath.shape());
		}

		inline double pixel_adjust(const dimension aWidth)
		{
			return static_cast<uint32_t>(aWidth) % 2 == 1 ? 0.5 : 0.0;
		}

		inline double pixel_adjust(const pen& aPen)
		{
			return pixel_adjust(aPen.width());
		}

		inline std::vector<xyz> line_loop_to_lines(const std::vector<xyz>& aLineLoop)
		{
			std::vector<xyz> result;
			result.reserve(aLineLoop.size() * 2);
			for (auto v = aLineLoop.begin(); v != aLineLoop.end(); ++v)
			{
				result.push_back(*v);
				if (v != aLineLoop.begin() && v != aLineLoop.end() - 1)
					result.push_back(*v);
			}
			return result;
		}

		struct with_textures_t {} with_textures;

		class use_vertex_arrays_instance
		{
		public:
			struct not_enough_room : std::invalid_argument { not_enough_room() : std::invalid_argument("neogfx::use_vertex_arrays_instance::not_enough_room") {} };
			struct invalid_draw_count : std::invalid_argument { invalid_draw_count() : std::invalid_argument("neogfx::use_vertex_arrays_instance::invalid_draw_count") {} };
			struct cannot_use_barrier : std::invalid_argument { cannot_use_barrier() : std::invalid_argument("neogfx::use_vertex_arrays_instance::cannot_use_barrier") {} };
		public:
			typedef opengl_standard_vertex_arrays::vertex_array::value_type value_type;
			typedef opengl_standard_vertex_arrays::vertex_array::const_iterator const_iterator;
			typedef opengl_standard_vertex_arrays::vertex_array::iterator iterator;
		public:
			use_vertex_arrays_instance(opengl_graphics_context& aParent, GLenum aMode, std::size_t aNeed = 0u, bool aUseBarrier = false) :
				iParent{ aParent }, iUse{ aParent.rendering_engine().vertex_arrays() }, iMode{ aMode }, iWithTextures{ false }, iStart{ static_cast<GLint>(vertices().size()) }, iUseBarrier{ aUseBarrier }
			{
				if (!room_for(aNeed) || aUseBarrier || is_new_transformation(optional_mat44{}))
					execute();
				set_transformation(optional_mat44{});
				if (!room_for(aNeed))
					throw not_enough_room();
			}
			use_vertex_arrays_instance(opengl_graphics_context& aParent, GLenum aMode, const optional_mat44& aTransformation, std::size_t aNeed = 0u, bool aUseBarrier = false) :
				iParent{ aParent }, iUse{ aParent.rendering_engine().vertex_arrays() }, iMode{ aMode }, iWithTextures{ false }, iStart{ static_cast<GLint>(vertices().size()) }, iUseBarrier{ aUseBarrier }
			{
				if (!room_for(aNeed) || aUseBarrier || is_new_transformation(aTransformation))
					execute();
				set_transformation(aTransformation);
				if (!room_for(aNeed))
					throw not_enough_room();
			}
			use_vertex_arrays_instance(opengl_graphics_context& aParent, GLenum aMode, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false) :
				iParent{ aParent }, iUse{ aParent.rendering_engine().vertex_arrays() }, iMode{ aMode }, iWithTextures{ true }, iStart{ static_cast<GLint>(vertices().size()) }, iUseBarrier{ aUseBarrier }
			{
				if (!room_for(aNeed) || aUseBarrier || is_new_transformation(optional_mat44{}))
					execute();
				set_transformation(optional_mat44{});
				if (!room_for(aNeed))
					throw not_enough_room();
			}
			use_vertex_arrays_instance(opengl_graphics_context& aParent, GLenum aMode, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false) :
				iParent{ aParent }, iUse{ aParent.rendering_engine().vertex_arrays() }, iMode{ aMode }, iWithTextures{ true }, iStart{ static_cast<GLint>(vertices().size()) }, iUseBarrier{ aUseBarrier }
			{
				if (!room_for(aNeed) || aUseBarrier || is_new_transformation(aTransformation))
					execute();
				set_transformation(aTransformation);
				if (!room_for(aNeed))
					throw not_enough_room();
			}
			~use_vertex_arrays_instance()
			{
				draw();
			}
		public:
			std::size_t primitive_vertex_count() const
			{
				switch (mode())
				{
				case GL_TRIANGLES:
					return 3;
				case GL_QUADS: // two triangles
					return 6;
				case GL_LINES:
					return 2;
				case GL_POINTS:
					return 1;
				case GL_TRIANGLE_FAN:
				case GL_LINE_LOOP:
				case GL_LINE_STRIP:
				case GL_TRIANGLE_STRIP:
				default:
					return 0;
				}
			}
		public:
			const_iterator begin() const
			{
				return vertices().begin() + static_cast<std::size_t>(iStart);
			}
			iterator begin()
			{
				return vertices().begin() + static_cast<std::size_t>(iStart);
			}
			const_iterator end() const
			{
				return vertices().end();
			}
			iterator end()
			{
				return vertices().end();
			}
			bool empty() const
			{
				return vertices().size() == static_cast<std::size_t>(iStart);
			}
			const value_type& operator[](std::size_t aOffset) const
			{
				return *(begin() + aOffset);
			}
			value_type& operator[](std::size_t aOffset)
			{
				return *(begin() + aOffset);
			}
		public:
			void push_back(const value_type& aVertex)
			{
				if (!room_for(1))
					execute();
				vertices().push_back(aVertex);
			}
			template <typename Iter>
			iterator insert(const_iterator aPos, Iter aFirst, Iter aLast)
			{
				if (room_for(std::distance(aFirst, aLast)))
					return vertices().insert(aPos, aFirst, aLast);
				else
				{
					execute();
					if (!room_for(std::distance(aFirst, aLast)))
						vertices().reserve(std::distance(aFirst, aLast));
					return vertices().insert(vertices().begin(), aFirst, aLast);
				}
			}
		public:
			std::size_t room() const
			{
				return vertices().capacity() - vertices().size();
			}
			bool room_for(std::size_t aAmount) const
			{
				auto pvc = primitive_vertex_count();
				if (pvc != 0)
					aAmount = std::max(aAmount, (pvc - ((vertices().size() - iStart) % pvc)) % pvc);
				return room() >= aAmount;
			}
			void execute()
			{
				draw();
				iUse.execute();
				iUse.vertices().clear();
				iStart = 0;
			}
			void draw()
			{
				draw(vertices().size() - static_cast<std::size_t>(iStart));
			}
			void draw(std::size_t aCount, std::size_t aBarrierPartitions = 2)
			{
				if (static_cast<std::size_t>(iStart) + aCount > vertices().size())
					throw invalid_draw_count();
				if (static_cast<std::size_t>(iStart) == vertices().size())
					return;
				if (!iWithTextures)
					iParent.rendering_engine().vertex_arrays().instantiate(iParent, iParent.rendering_engine().active_shader_program());
				else
					iParent.rendering_engine().vertex_arrays().instantiate_with_texture_coords(iParent, iParent.rendering_engine().active_shader_program());
				if (!iUseBarrier && mode() == translated_mode())
				{
					glCheck(glDrawArrays(translated_mode(), iStart, static_cast<GLsizei>(aCount)));
					iStart += aCount;
				}
				else
				{
					if (iUseBarrier)
					{
						glCheck(glTextureBarrier());
					}
					auto pvc = primitive_vertex_count();
					auto chunk = std::max(pvc, (aCount / pvc / aBarrierPartitions) * pvc);
					while (aCount > 0)
					{
						auto amount = std::min(chunk, aCount);
						glCheck(glDrawArrays(translated_mode(), iStart, static_cast<GLsizei>(amount)));
						iStart += amount;
						aCount -= amount;
						if (iUseBarrier)
						{
							glCheck(glTextureBarrier());
						}
					}
				}
			}
		private:
			bool is_new_transformation(const optional_mat44& aTransformation) const
			{
				return iUse.transformation() != aTransformation;
			}
			const optional_mat44& transformation() const
			{
				return iUse.transformation();
			}
			void set_transformation(const optional_mat44& aTransformation)
			{
				iUse.set_transformation(aTransformation);
			}
			const opengl_standard_vertex_arrays::vertex_array& vertices() const
			{
				return iUse.vertices();
			}
			opengl_standard_vertex_arrays::vertex_array& vertices()
			{
				return iUse.vertices();
			}
			GLenum translated_mode() const
			{
				switch (iMode)
				{
				case GL_QUADS:
					return GL_TRIANGLES;
				default:
					return iMode;
				}
			}
			GLenum mode() const
			{
				return iMode;
			}
		private:
			opengl_graphics_context& iParent;
			opengl_standard_vertex_arrays::use iUse;
			GLenum iMode;
			bool iWithTextures;
			GLint iStart;
			bool iUseBarrier;
		};

		class use_vertex_arrays
		{
		public:
			use_vertex_arrays(opengl_graphics_context& aParent, GLenum aMode, std::size_t aNeed = 0u, bool aUseBarrier = false)
			{
				instantiate(aParent, aMode, aNeed, aUseBarrier);
			}
			use_vertex_arrays(opengl_graphics_context& aParent, GLenum aMode, const optional_mat44& aTransformation, std::size_t aNeed = 0u, bool aUseBarrier = false)
			{
				instantiate(aParent, aMode, aTransformation, aNeed, aUseBarrier);
			}
			use_vertex_arrays(opengl_graphics_context& aParent, GLenum aMode, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false)
			{
				instantiate(aParent, aMode, with_textures, aNeed, aUseBarrier);
			}
			use_vertex_arrays(opengl_graphics_context& aParent, GLenum aMode, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false)
			{
				instantiate(aParent, aMode, aTransformation, with_textures, aNeed, aUseBarrier);
			}
			~use_vertex_arrays()
			{
			}
		public:
			use_vertex_arrays_instance& instance()
			{
				return *iInstance;
			}
			template <typename... Args>
			void instantiate(Args&&... args)
			{
				thread_local std::weak_ptr<use_vertex_arrays_instance> tInstance;
				if (tInstance.expired())
				{
					iInstance = std::make_shared<use_vertex_arrays_instance>(std::forward<Args>(args)...);
					tInstance = iInstance;
				}
				else
					iInstance = tInstance.lock();
			}
		private:
			std::shared_ptr<use_vertex_arrays_instance> iInstance;
		};
	}

	opengl_graphics_context::opengl_graphics_context(const i_native_surface& aSurface) :
		iRenderingEngine(service<i_rendering_engine>::instance()), 
		iSurface(aSurface), 
		iLogicalCoordinateSystem(aSurface.logical_coordinate_system()),
		iLogicalCoordinates(aSurface.logical_coordinates()), 
		iSmoothingMode(neogfx::smoothing_mode::None),
		iSubpixelRendering(rendering_engine().is_subpixel_rendering_on()),
		iClipCounter(0),
		iLineStippleActive(false)
	{
		rendering_engine().activate_context(iSurface);
		rendering_engine().activate_shader_program(*this, rendering_engine().default_shader_program());
		set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
	}

	opengl_graphics_context::opengl_graphics_context(const i_native_surface& aSurface, const i_widget& aWidget) :
		iRenderingEngine(service<i_rendering_engine>::instance()),
		iSurface(aSurface), 
		iLogicalCoordinateSystem(aWidget.logical_coordinate_system()),
		iLogicalCoordinates(aSurface.logical_coordinates()),
		iSmoothingMode(neogfx::smoothing_mode::None),
		iSubpixelRendering(rendering_engine().is_subpixel_rendering_on()),
		iClipCounter(0),
		iLineStippleActive(false)
	{
		rendering_engine().activate_context(iSurface);
		rendering_engine().activate_shader_program(*this, rendering_engine().default_shader_program());
		set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
	}

	opengl_graphics_context::opengl_graphics_context(const opengl_graphics_context& aOther) :
		iRenderingEngine(aOther.iRenderingEngine), 
		iSurface(aOther.iSurface), 
		iLogicalCoordinateSystem(aOther.iLogicalCoordinateSystem),
		iLogicalCoordinates(aOther.iLogicalCoordinates),
		iSmoothingMode(aOther.iSmoothingMode), 
		iSubpixelRendering(aOther.iSubpixelRendering),
		iClipCounter(0),
		iLineStippleActive(false)
	{
		rendering_engine().activate_context(iSurface);
		rendering_engine().activate_shader_program(*this, rendering_engine().default_shader_program());
		set_smoothing_mode(iSmoothingMode);
	}

	opengl_graphics_context::~opengl_graphics_context()
	{
		flush();
	}

	i_rendering_engine& opengl_graphics_context::rendering_engine()
	{
		return iRenderingEngine;
	}

	const i_native_surface& opengl_graphics_context::surface() const
	{
		return iSurface;
	}

	neogfx::logical_coordinate_system opengl_graphics_context::logical_coordinate_system() const
	{
		return iLogicalCoordinateSystem;
	}

	void opengl_graphics_context::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
	{
		iLogicalCoordinateSystem = aSystem;
	}

	const std::pair<vec2, vec2>& opengl_graphics_context::logical_coordinates() const
	{
		return get_logical_coordinates(surface().surface_size(), iLogicalCoordinateSystem, iLogicalCoordinates);
	}

	void opengl_graphics_context::set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates) const
	{
		iLogicalCoordinates = aCoordinates;
	}

	void opengl_graphics_context::enqueue(const graphics_operation::operation& aOperation)
	{
		if (iQueue.second.empty())
			iQueue.second.push_back(0);
		bool sameBatch = (iQueue.first.empty() || graphics_operation::batchable(iQueue.first.back(), aOperation)) && iQueue.first.size() - iQueue.second.back() < max_operations(aOperation);
		if (!sameBatch)
			iQueue.second.push_back(iQueue.first.size());
		iQueue.first.push_back(aOperation);
	}

	void opengl_graphics_context::flush()
	{
		if (iQueue.first.empty())
			return;
		iQueue.second.push_back(iQueue.first.size());
		auto endIndex = std::prev(iQueue.second.end());
		for (auto startIndex = iQueue.second.begin(); startIndex != endIndex; ++startIndex)
		{
			graphics_operation::batch opBatch{ &*iQueue.first.begin() + *startIndex, &*iQueue.first.begin() + *std::next(startIndex) };
			switch (opBatch.first->index())
			{
			case graphics_operation::operation_type::SetLogicalCoordinateSystem:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					set_logical_coordinate_system(static_variant_cast<const graphics_operation::set_logical_coordinate_system&>(*op).system);
				break;
			case graphics_operation::operation_type::SetLogicalCoordinates:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					set_logical_coordinates(static_variant_cast<const graphics_operation::set_logical_coordinates&>(*op).coordinates);
				break;
			case graphics_operation::operation_type::ScissorOn:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					scissor_on(static_variant_cast<const graphics_operation::scissor_on&>(*op).rect);
				break;
			case graphics_operation::operation_type::ScissorOff:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					(void)op;
					scissor_off();
				}
				break;
			case graphics_operation::operation_type::ClipToRect:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					clip_to(static_variant_cast<const graphics_operation::clip_to_rect&>(*op).rect);
				break;
			case graphics_operation::operation_type::ClipToPath:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					clip_to(static_variant_cast<const graphics_operation::clip_to_path&>(*op).path, static_variant_cast<const graphics_operation::clip_to_path&>(*op).pathOutline);
				break;
			case graphics_operation::operation_type::ResetClip:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					(void)op;
					reset_clip();
				}
				break;
			case graphics_operation::operation_type::SetSmoothingMode:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					set_smoothing_mode(static_variant_cast<const graphics_operation::set_smoothing_mode&>(*op).smoothingMode);
				break;
			case graphics_operation::operation_type::PushLogicalOperation:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					push_logical_operation(static_variant_cast<const graphics_operation::push_logical_operation&>(*op).logicalOperation);
				break;
			case graphics_operation::operation_type::PopLogicalOperation:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					(void)op;
					pop_logical_operation();
				}
				break;
			case graphics_operation::operation_type::LineStippleOn:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					line_stipple_on(static_variant_cast<const graphics_operation::line_stipple_on&>(*op).factor, static_variant_cast<const graphics_operation::line_stipple_on&>(*op).pattern);
				break;
			case graphics_operation::operation_type::LineStippleOff:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					(void)op;
					line_stipple_off();
				}
				break;
			case graphics_operation::operation_type::SubpixelRenderingOn:
				subpixel_rendering_on();
				break;
			case graphics_operation::operation_type::SubpixelRenderingOff:
				subpixel_rendering_off();
				break;
			case graphics_operation::operation_type::Clear:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					clear(static_variant_cast<const graphics_operation::clear&>(*op).colour);
				break;
			case graphics_operation::operation_type::ClearDepthBuffer:
				clear_depth_buffer();
				break;
			case graphics_operation::operation_type::SetPixel:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					set_pixel(static_variant_cast<const graphics_operation::set_pixel&>(*op).point, static_variant_cast<const graphics_operation::set_pixel&>(*op).colour);
				break;
			case graphics_operation::operation_type::DrawPixel:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					draw_pixel(static_variant_cast<const graphics_operation::draw_pixel&>(*op).point, static_variant_cast<const graphics_operation::draw_pixel&>(*op).colour);
				break;
			case graphics_operation::operation_type::DrawLine:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_line&>(*op);
					draw_line(args.from, args.to, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawRect:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_rect&>(*op);
					draw_rect(args.rect, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawRoundedRect:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_rounded_rect&>(*op);
					draw_rounded_rect(args.rect, args.radius, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawCircle:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_circle&>(*op);
					draw_circle(args.centre, args.radius, args.pen, args.startAngle);
				}
				break;
			case graphics_operation::operation_type::DrawArc:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_arc&>(*op);
					draw_arc(args.centre, args.radius, args.startAngle, args.endAngle, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawPath:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_path&>(*op);
					draw_path(args.path, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawShape:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_shape&>(*op);
					draw_shape(args.mesh, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawEntities:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_entities&>(*op);
					draw_entities(args.ecs, args.transformation);
				}
				break;
			case graphics_operation::operation_type::FillRect:
				fill_rect(opBatch);
				break;
			case graphics_operation::operation_type::FillRoundedRect:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::fill_rounded_rect&>(*op);
					fill_rounded_rect(args.rect, args.radius, args.fill);
				}
				break;
			case graphics_operation::operation_type::FillCircle:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::fill_circle&>(*op);
					fill_circle(args.centre, args.radius, args.fill);
				}
				break;
			case graphics_operation::operation_type::FillArc:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
				{
					const auto& args = static_variant_cast<const graphics_operation::fill_arc&>(*op);
					fill_arc(args.centre, args.radius, args.startAngle, args.endAngle, args.fill);
				}
				break;
			case graphics_operation::operation_type::FillPath:
				for (auto op = opBatch.first; op != opBatch.second; ++op)
					fill_path(static_variant_cast<const graphics_operation::fill_path&>(*op).path, static_variant_cast<const graphics_operation::fill_path&>(*op).fill);
				break;
			case graphics_operation::operation_type::FillShape:
				fill_shape(opBatch);
				break;
			case graphics_operation::operation_type::DrawGlyph:
				draw_glyph(opBatch);
				break;
			case graphics_operation::operation_type::DrawMesh:
				{
					use_shader_program usp{ *this, iRenderingEngine, rendering_engine().texture_shader_program() };
					for (auto op = opBatch.first; op != opBatch.second; ++op)
					{
						const auto& args = static_variant_cast<const graphics_operation::draw_mesh&>(*op);
						draw_mesh(args.mesh, args.material, args.transformation, args.shaderEffect);
					}
				}
				break;
			}
		}
		iQueue.first.clear();
		iQueue.second.clear();
	}

	void opengl_graphics_context::scissor_on(const rect& aRect)
	{
		if (iScissorRect == std::nullopt)
		{
			glCheck(glEnable(GL_SCISSOR_TEST));
			iScissorRect = aRect;
		}
		iScissorRects.push_back(*iScissorRect);
		iScissorRect = iScissorRect->intersection(aRect);
		apply_scissor();
	}

	void opengl_graphics_context::scissor_off()
	{
		auto previousScissorRect = iScissorRects.back();
		iScissorRects.pop_back();
		if (iScissorRects.empty())
		{
			glCheck(glDisable(GL_SCISSOR_TEST));
			iScissorRect = std::nullopt;
		}
		else
		{
			iScissorRect = previousScissorRect;
			apply_scissor();
		}
	}

	const optional_rect& opengl_graphics_context::scissor_rect() const
	{
		return iScissorRect;
	}

	void opengl_graphics_context::apply_scissor()
	{
		auto sr = *scissor_rect();
		GLint x = static_cast<GLint>(std::ceil(sr.x));
		GLint y = static_cast<GLint>(std::ceil(rendering_area(false).cy - sr.cy - sr.y));
		GLsizei cx = static_cast<GLsizei>(std::ceil(sr.cx));
		GLsizei cy = static_cast<GLsizei>(std::ceil(sr.cy));
		glCheck(glScissor(x, y, cx, cy));
	}

	void opengl_graphics_context::clip_to(const rect& aRect)
	{
		if (iClipCounter++ == 0)
		{
			glCheck(glClear(GL_STENCIL_BUFFER_BIT));
			glCheck(glEnable(GL_STENCIL_TEST));
		}
		glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		glCheck(glDepthMask(GL_FALSE));
		glCheck(glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP));  // draw 1s on test fail (always)
		glCheck(glStencilMask(static_cast<GLuint>(-1)));
		glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
		fill_rect(rendering_area(), colour::White);
		glCheck(glStencilFunc(GL_NEVER, 1, static_cast<GLuint>(-1)));
		fill_rect(aRect, colour::White);
		glCheck(glStencilFunc(GL_NEVER, 1, static_cast<GLuint>(-1)));
		glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		glCheck(glDepthMask(GL_TRUE));
		glCheck(glStencilMask(0x00));
		// draw only where stencil's value is 1
		glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
	}

	void opengl_graphics_context::clip_to(const path& aPath, dimension aPathOutline)
	{
		if (iClipCounter++ == 0)
		{
			glCheck(glClear(GL_STENCIL_BUFFER_BIT));
			glCheck(glEnable(GL_STENCIL_TEST));
		}
		glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		glCheck(glDepthMask(GL_FALSE));
		glCheck(glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP));  // draw 1s on test fail (always)
		glCheck(glStencilMask(static_cast<GLuint>(-1)));
		glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
		fill_rect(rendering_area(), colour::White);
		glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				auto vertices = aPath.to_vertices(aPath.paths()[i]);

				use_vertex_arrays vertexArrays{ *this, path_shape_to_gl_mode(aPath) };
				for (const auto& v : vertices)
				{
					vertexArrays.instance().push_back(opengl_standard_vertex_arrays::vertex{
						v, std::array<uint8_t, 4>{{0xFF, 0xFF, 0xFF, 0xFF}}
						});
				}
			}
		}
		if (aPathOutline != 0)
		{
			glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
			path innerPath = aPath;
			innerPath.deflate(aPathOutline);
			for (std::size_t i = 0; i < innerPath.paths().size(); ++i)
			{
				if (innerPath.paths()[i].size() > 2)
				{
					auto vertices = aPath.to_vertices(innerPath.paths()[i]);

					use_vertex_arrays vertexArrays{ *this, path_shape_to_gl_mode(innerPath) };
					for (const auto& v : vertices)
					{
						vertexArrays.instance().push_back(opengl_standard_vertex_arrays::vertex{
							v, std::array<uint8_t, 4>{{0xFF, 0xFF, 0xFF, 0xFF}}
							});
					}
				}
			}
		}
		glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		glCheck(glDepthMask(GL_TRUE));
		glCheck(glStencilMask(0x00));
		// draw only where stencil's value is 1
		glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
	}

	void opengl_graphics_context::reset_clip()
	{
		if (--iClipCounter == 0)
		{
			glCheck(glDisable(GL_STENCIL_TEST));
		}
	}

	smoothing_mode opengl_graphics_context::smoothing_mode() const
	{
		return iSmoothingMode;
	}

	void opengl_graphics_context::set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode)
	{
		iSmoothingMode = aSmoothingMode;
		if (iSmoothingMode == neogfx::smoothing_mode::AntiAlias)
		{
			glCheck(glEnable(GL_LINE_SMOOTH));
			glCheck(glEnable(GL_POLYGON_SMOOTH));
		}
		else
		{
			glCheck(glDisable(GL_LINE_SMOOTH));
			glCheck(glDisable(GL_POLYGON_SMOOTH));
		}
	}

	void opengl_graphics_context::push_logical_operation(logical_operation aLogicalOperation)
	{
		iLogicalOperationStack.push_back(aLogicalOperation);
		apply_logical_operation();
	}

	void opengl_graphics_context::pop_logical_operation()
	{
		if (!iLogicalOperationStack.empty())
			iLogicalOperationStack.pop_back();
		apply_logical_operation();
	}

	void opengl_graphics_context::apply_logical_operation()
	{
		if (iLogicalOperationStack.empty() || iLogicalOperationStack.back() == logical_operation::None)
		{
			glCheck(glDisable(GL_COLOR_LOGIC_OP));
		}
		else
		{
			glCheck(glEnable(GL_COLOR_LOGIC_OP));
			switch (iLogicalOperationStack.back())
			{
			case logical_operation::Xor:
				glCheck(glLogicOp(GL_XOR));
				break;
			}
		}	
	}

	void opengl_graphics_context::gradient_on(const gradient& aGradient, const rect& aBoundingBox)
	{
		basic_rect<float> boundingBox{ aBoundingBox };
		iShaderProgramStack.emplace_back(*this, iRenderingEngine, rendering_engine().gradient_shader_program());
		rendering_engine().gradient_shader_program().set_uniform_variable("posViewportTop", static_cast<float>(logical_coordinates().first.y));
		rendering_engine().gradient_shader_program().set_uniform_variable("posTopLeft", boundingBox.top_left().x, boundingBox.top_left().y);
		rendering_engine().gradient_shader_program().set_uniform_variable("posBottomRight", boundingBox.bottom_right().x, boundingBox.bottom_right().y);
		rendering_engine().gradient_shader_program().set_uniform_variable("nGradientDirection", static_cast<int>(aGradient.direction()));
		rendering_engine().gradient_shader_program().set_uniform_variable("radGradientAngle", std::holds_alternative<double>(aGradient.orientation()) ? static_cast<float>(static_variant_cast<double>(aGradient.orientation())) : 0.0f);
		rendering_engine().gradient_shader_program().set_uniform_variable("nGradientStartFrom", std::holds_alternative<gradient::corner_e>(aGradient.orientation()) ? static_cast<int>(static_variant_cast<gradient::corner_e>(aGradient.orientation())) : -1);
		rendering_engine().gradient_shader_program().set_uniform_variable("nGradientSize", static_cast<int>(aGradient.size()));
		rendering_engine().gradient_shader_program().set_uniform_variable("nGradientShape", static_cast<int>(aGradient.shape()));
		basic_vector<float, 2> gradientExponents = (aGradient.exponents() != std::nullopt ? *aGradient.exponents() : vec2{2.0, 2.0});
		rendering_engine().gradient_shader_program().set_uniform_variable("exponents", gradientExponents.x, gradientExponents.y);
		basic_point<float> gradientCentre = (aGradient.centre() != std::nullopt ? *aGradient.centre() : point{});
		rendering_engine().gradient_shader_program().set_uniform_variable("posGradientCentre", gradientCentre.x, gradientCentre.y);
		auto combinedStops = aGradient.combined_stops();
		iGradientStopPositions.reserve(combinedStops.size());
		iGradientStopColours.reserve(combinedStops.size());
		iGradientStopPositions.clear();
		iGradientStopColours.clear();
		for (const auto& stop : combinedStops)
		{
			iGradientStopPositions.push_back(static_cast<float>(stop.first));
			iGradientStopColours.push_back(std::array<float, 4>{ {stop.second.red<float>(), stop.second.green<float>(), stop.second.blue<float>(), stop.second.alpha<float>()}});
		}
		rendering_engine().gradient_shader_program().set_uniform_variable("nStopCount", static_cast<int>(iGradientStopPositions.size()));
		rendering_engine().gradient_shader_program().set_uniform_variable("nFilterSize", static_cast<int>(opengl_renderer::GRADIENT_FILTER_SIZE));
		auto filter = static_gaussian_filter<float, opengl_renderer::GRADIENT_FILTER_SIZE>(static_cast<float>(aGradient.smoothness() * 10.0));
		// todo: remove the following cast when gradient textures abstracted in rendering engine base class interface
		auto& gradientTextures = static_cast<opengl_renderer&>(iRenderingEngine).gradient_textures(); 
		glCheck(glActiveTexture(GL_TEXTURE2));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, gradientTextures[0]));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, iGradientStopPositions.size(), 1, GL_RED, GL_FLOAT, &iGradientStopPositions[0]));
		glCheck(glActiveTexture(GL_TEXTURE3));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, gradientTextures[1]));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, iGradientStopColours.size(), 1, GL_RGBA, GL_FLOAT, &iGradientStopColours[0]));
		glCheck(glActiveTexture(GL_TEXTURE4));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, gradientTextures[2]));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, opengl_renderer::GRADIENT_FILTER_SIZE, opengl_renderer::GRADIENT_FILTER_SIZE, GL_RED, GL_FLOAT, &filter[0][0]));
		rendering_engine().gradient_shader_program().set_uniform_variable("texStopPositions", 2);
		rendering_engine().gradient_shader_program().set_uniform_variable("texStopColours", 3);
		rendering_engine().gradient_shader_program().set_uniform_variable("texFilter", 4);
		glCheck(glActiveTexture(GL_TEXTURE1));
	}

	void opengl_graphics_context::gradient_off()
	{
		iShaderProgramStack.pop_back();
		glCheck(glDisable(GL_TEXTURE_RECTANGLE));
	}

	void opengl_graphics_context::line_stipple_on(uint32_t aFactor, uint16_t aPattern)
	{
		// TODO: glLineStipple unavailable in 3.2
		iLineStippleActive = true;
	}

	void opengl_graphics_context::line_stipple_off()
	{
		// TODO: glLineStipple unavailable in 3.2
		iLineStippleActive = false;
	}

	bool opengl_graphics_context::is_subpixel_rendering_on() const
	{
		return iSubpixelRendering;
	}

	void opengl_graphics_context::subpixel_rendering_on()
	{
		iSubpixelRendering = true;
	}

	void opengl_graphics_context::subpixel_rendering_off()
	{
		iSubpixelRendering = false;
	}

	void opengl_graphics_context::clear(const colour& aColour)
	{
		disable_anti_alias daa(*this);
		fill_rect(rendering_area(), aColour);
	}

	void opengl_graphics_context::clear_depth_buffer()
	{
		glCheck(glClear(GL_DEPTH_BUFFER_BIT));
	}

	void opengl_graphics_context::set_pixel(const point& aPoint, const colour& aColour)
	{
		/* todo: faster alternative to this... */
		disable_anti_alias daa(*this);
		draw_pixel(aPoint, aColour.with_alpha(0xFF));
	}

	void opengl_graphics_context::draw_pixel(const point& aPoint, const colour& aColour)
	{
		/* todo: faster alternative to this... */
		fill_rect(rect{ aPoint, size{1.0, 1.0} }, aColour);
	}

	void opengl_graphics_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen)
	{
		if (std::holds_alternative<gradient>(aPen.colour()))
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, rect{ aFrom, aTo });
		}

		double pixelAdjust = pixel_adjust(aPen);
		auto penColour = std::holds_alternative<colour>(aPen.colour()) ?
			std::array<uint8_t, 4>{{
				static_variant_cast<colour>(aPen.colour()).red(),
				static_variant_cast<colour>(aPen.colour()).green(),
				static_variant_cast<colour>(aPen.colour()).blue(),
				static_variant_cast<colour>(aPen.colour()).alpha()}} :
			std::array<uint8_t, 4>{};

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		{
			use_vertex_arrays vertexArrays{ *this, GL_LINES };
			vertexArrays.instance().push_back(opengl_standard_vertex_arrays::vertex{ xyz{aFrom.x + pixelAdjust, aFrom.y + pixelAdjust}, penColour });
			vertexArrays.instance().push_back(opengl_standard_vertex_arrays::vertex{ xyz{aTo.x + pixelAdjust, aTo.y + pixelAdjust}, penColour });
		}
		glCheck(glLineWidth(1.0f));

		if (std::holds_alternative<gradient>(aPen.colour()))
			gradient_off();
	}

	void opengl_graphics_context::draw_rect(const rect& aRect, const pen& aPen)
	{
		if (std::holds_alternative<gradient>(aPen.colour()))
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, aRect);
		}

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		{
			use_vertex_arrays vertexArrays{ *this, GL_LINES, 8 };
			back_insert_rect_vertices(vertexArrays.instance(), aRect, pixel_adjust(aPen), rect_type::Outline);
			for (auto& v : vertexArrays.instance())
				v.rgba = colour_to_vec4f(std::holds_alternative<colour>(aPen.colour()) ?
					std::array <uint8_t, 4>{{
						static_variant_cast<colour>(aPen.colour()).red(),
						static_variant_cast<colour>(aPen.colour()).green(),
						static_variant_cast<colour>(aPen.colour()).blue(),
						static_variant_cast<colour>(aPen.colour()).alpha()}} :
					std::array <uint8_t, 4>{});
		}
		glCheck(glLineWidth(1.0f));

		if (std::holds_alternative<gradient>(aPen.colour()))
			gradient_off();
	}

	void opengl_graphics_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen)
	{
		if (std::holds_alternative<gradient>(aPen.colour()))
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, aRect);
		}

		double pixelAdjust = pixel_adjust(aPen);
		auto vertices = rounded_rect_vertices(aRect + point{ pixelAdjust, pixelAdjust }, aRadius, false);

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		{
			use_vertex_arrays vertexArrays{ *this, GL_LINE_LOOP };
			for (const auto& v : vertices)
				vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
					std::array <uint8_t, 4>{{
						static_variant_cast<colour>(aPen.colour()).red(),
						static_variant_cast<colour>(aPen.colour()).green(),
						static_variant_cast<colour>(aPen.colour()).blue(),
						static_variant_cast<colour>(aPen.colour()).alpha()}} :
					std::array <uint8_t, 4>{}});
		}
		glCheck(glLineWidth(1.0f));

		if (std::holds_alternative<gradient>(aPen.colour()))
			gradient_off();
	}

	void opengl_graphics_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, angle aStartAngle)
	{
		if (std::holds_alternative<gradient>(aPen.colour()))
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, rect{ aCentre - size{aRadius, aRadius}, size{aRadius * 2.0, aRadius * 2.0 } });
		}

		auto vertices = circle_vertices(aCentre, aRadius, aStartAngle, false);

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		{
			use_vertex_arrays vertexArrays{ *this, GL_LINE_LOOP, vertices.size() };
			for (const auto& v : vertices)
				vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
					std::array <uint8_t, 4>{{
						static_variant_cast<colour>(aPen.colour()).red(),
						static_variant_cast<colour>(aPen.colour()).green(),
						static_variant_cast<colour>(aPen.colour()).blue(),
						static_variant_cast<colour>(aPen.colour()).alpha()}} :
					std::array <uint8_t, 4>{}});
		}
		glCheck(glLineWidth(1.0f));

		if (std::holds_alternative<gradient>(aPen.colour()))
			gradient_off();
	}

	void opengl_graphics_context::draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen)
	{
		if (std::holds_alternative<gradient>(aPen.colour()))
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, rect{ aCentre - size{ aRadius, aRadius }, size{ aRadius * 2.0, aRadius * 2.0 } });
		}

		auto vertices = line_loop_to_lines(arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, false));;

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		{
			use_vertex_arrays vertexArrays{ *this, GL_LINES, vertices.size() };
			for (const auto& v : vertices)
				vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
					std::array <uint8_t, 4>{ {
						static_variant_cast<colour>(aPen.colour()).red(),
						static_variant_cast<colour>(aPen.colour()).green(),
						static_variant_cast<colour>(aPen.colour()).blue(),
						static_variant_cast<colour>(aPen.colour()).alpha()}} :
					std::array <uint8_t, 4>{} });
		}
		glCheck(glLineWidth(1.0f));

		if (std::holds_alternative<gradient>(aPen.colour()))
			gradient_off();
	}

	void opengl_graphics_context::draw_path(const path& aPath, const pen& aPen)
	{
		if (std::holds_alternative<gradient>(aPen.colour()))
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, aPath.bounding_rect());
		}

		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				if (aPath.shape() == path::ConvexPolygon)
					clip_to(aPath, aPen.width());

				auto vertices = aPath.to_vertices(aPath.paths()[i]);

				{
					use_vertex_arrays vertexArrays{ *this, path_shape_to_gl_mode(aPath.shape()), vertices.size() };
					for (const auto& v : vertices)
						vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
							std::array <uint8_t, 4>{{
								static_variant_cast<colour>(aPen.colour()).red(),
								static_variant_cast<colour>(aPen.colour()).green(),
								static_variant_cast<colour>(aPen.colour()).blue(),
								static_variant_cast<colour>(aPen.colour()).alpha()}} :
							std::array <uint8_t, 4>{}});
				}
				if (aPath.shape() == path::ConvexPolygon)
					reset_clip();
			}
		}

		if (std::holds_alternative<gradient>(aPen.colour()))
			gradient_off();
	}

	void opengl_graphics_context::draw_shape(const game::mesh& aMesh, const pen& aPen)
	{
		auto const& vertices = aMesh.vertices;

		if (std::holds_alternative<gradient>(aPen.colour()))
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, bounding_rect(aMesh));
		}

		{
			use_vertex_arrays vertexArrays{ *this, GL_LINE_LOOP, vertices.size() };
			for (const auto& v : vertices)
				vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
					std::array <uint8_t, 4>{{
						static_variant_cast<colour>(aPen.colour()).red(),
						static_variant_cast<colour>(aPen.colour()).green(),
						static_variant_cast<colour>(aPen.colour()).blue(),
						static_variant_cast<colour>(aPen.colour()).alpha()}} :
					std::array <uint8_t, 4>{}});
		}

		if (std::holds_alternative<gradient>(aPen.colour()))
			gradient_off();
	}

	void opengl_graphics_context::draw_entities(const game::i_ecs& aEcs, const mat44& aTransformation)
	{
		aEcs.component<game::rigid_body>().take_snapshot();
		auto rigidBodiesSnapshot = aEcs.component<game::rigid_body>().snapshot();
		auto const& rigidBodies = rigidBodiesSnapshot.data();
		std::unique_ptr<use_shader_program> usp;
		std::unique_ptr<use_vertex_arrays> uva;
		bool withTexture = false;
		for (auto entity : aEcs.component<game::mesh_renderer>().entities())
		{
			auto const& meshFilter = aEcs.component<game::mesh_filter>().entity_record(entity);
			auto const& meshRenderer = aEcs.component<game::mesh_renderer>().entity_record(entity);
			bool renderTexture = (meshRenderer.material.texture != std::nullopt);
			if (usp == nullptr || renderTexture != withTexture)
			{
				usp = nullptr;
				if (renderTexture)
					usp = std::make_unique<use_shader_program>(*this, iRenderingEngine, rendering_engine().texture_shader_program());
				else
					usp = std::make_unique<use_shader_program>(*this, iRenderingEngine, rendering_engine().default_shader_program());
			}
			if (uva == nullptr || renderTexture != withTexture)
			{
				uva = nullptr;
				if (renderTexture)
					uva = std::make_unique<use_vertex_arrays>(*this, GL_TRIANGLES, with_textures);
				else
					uva = std::make_unique<use_vertex_arrays>(*this, GL_TRIANGLES);
			}
			withTexture = renderTexture;
			draw_mesh(
				meshFilter,
				meshRenderer,
				rigidBodies.has_entity_record(entity) ?
					aTransformation * to_transformation_matrix(rigidBodies.entity_record(entity)) : aTransformation,
				shader_effect::None);
		}
	}

	void opengl_graphics_context::fill_rect(const rect& aRect, const brush& aFill)
	{
		graphics_operation::operation op{ graphics_operation::fill_rect{ aRect, aFill } };
		fill_rect(graphics_operation::batch{ &op, &op + 1 });
	}

	void opengl_graphics_context::fill_rect(const graphics_operation::batch& aFillRectOps)
	{
		auto& firstOp = static_variant_cast<const graphics_operation::fill_rect&>(*aFillRectOps.first);

		if (std::holds_alternative<gradient>(firstOp.fill))
			gradient_on(static_variant_cast<const gradient&>(firstOp.fill), firstOp.rect);

		{
			use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, 6u * (aFillRectOps.second - aFillRectOps.first)};

			for (auto op = aFillRectOps.first; op != aFillRectOps.second; ++op)
			{
				auto& drawOp = static_variant_cast<const graphics_operation::fill_rect&>(*op);
				auto newVertices = back_insert_rect_vertices(vertexArrays.instance(), drawOp.rect, 0.0, rect_type::FilledTriangles);
				for (auto i = newVertices; i != vertexArrays.instance().end(); ++i)
					i->rgba = colour_to_vec4f(std::holds_alternative<colour>(drawOp.fill) ?
						std::array<uint8_t, 4>{{
							static_variant_cast<const colour&>(drawOp.fill).red(),
							static_variant_cast<const colour&>(drawOp.fill).green(),
							static_variant_cast<const colour&>(drawOp.fill).blue(),
							static_variant_cast<const colour&>(drawOp.fill).alpha()}} :
						std::array<uint8_t, 4>{});
			}
		}

		if (std::holds_alternative<gradient>(firstOp.fill))
			gradient_off();
	}

	void opengl_graphics_context::fill_rounded_rect(const rect& aRect, dimension aRadius, const brush& aFill)
	{
		if (aRect.empty())
			return;

		use_shader_program usp{ *this, iRenderingEngine, rendering_engine().default_shader_program() };

		if (std::holds_alternative<gradient>(aFill))
			gradient_on(static_variant_cast<const gradient&>(aFill), aRect);

		auto vertices = rounded_rect_vertices(aRect, aRadius, true);
		
		{
			use_vertex_arrays vertexArrays{ *this, GL_TRIANGLE_FAN, vertices.size() };
			for (const auto& v : vertices)
			{
				vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
					std::array <uint8_t, 4>{{
						static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
					std::array <uint8_t, 4>{}});
			}
		}

		if (std::holds_alternative<gradient>(aFill))
			gradient_off();
	}

	void opengl_graphics_context::fill_circle(const point& aCentre, dimension aRadius, const brush& aFill)
	{
		if (std::holds_alternative<gradient>(aFill))
			gradient_on(static_variant_cast<const gradient&>(aFill), rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

		auto vertices = circle_vertices(aCentre, aRadius, 0.0, true);

		{
			use_vertex_arrays vertexArrays{ *this, GL_TRIANGLE_FAN, vertices.size() };
			for (const auto& v : vertices)
			{
				vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
					std::array <uint8_t, 4>{{
						static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
					std::array <uint8_t, 4>{}});
			}
		}

		if (std::holds_alternative<gradient>(aFill))
			gradient_off();
	}

	void opengl_graphics_context::fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill)
	{
		if (std::holds_alternative<gradient>(aFill))
			gradient_on(static_variant_cast<const gradient&>(aFill), rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

		auto vertices = arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, true);

		{
			use_vertex_arrays vertexArrays{ *this, GL_TRIANGLE_FAN, vertices.size() };
			for (const auto& v : vertices)
			{
				vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
					std::array <uint8_t, 4>{{
						static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
					std::array <uint8_t, 4>{}});
			}
		}

		if (std::holds_alternative<gradient>(aFill))
			gradient_off();
	}

	void opengl_graphics_context::fill_path(const path& aPath, const brush& aFill)
	{
		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				clip_to(aPath, 0.0);
				point min = aPath.paths()[i][0];
				point max = min;
				for (auto const& pt : aPath.paths()[i])
				{
					min = min.min(pt);
					max = max.max(pt);
				}

				if (std::holds_alternative<gradient>(aFill))
					gradient_on(static_variant_cast<const gradient&>(aFill), rect{ point{ min.x, min.y }, size{ max.x - min.y, max.y - min.y } });

				auto vertices = aPath.to_vertices(aPath.paths()[i]);

				{
					use_vertex_arrays vertexArrays{ *this, path_shape_to_gl_mode(aPath.shape()), vertices.size() };
					for (const auto& v : vertices)
					{
						vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
							std::array <uint8_t, 4>{{
								static_variant_cast<const colour&>(aFill).red(),
								static_variant_cast<const colour&>(aFill).green(),
								static_variant_cast<const colour&>(aFill).blue(),
								static_variant_cast<const colour&>(aFill).alpha()}} :
							std::array <uint8_t, 4>{}});
					}
				}

				reset_clip();

				if (std::holds_alternative<gradient>(aFill))
					gradient_off();
			}
		}
	}

	void opengl_graphics_context::fill_shape(const graphics_operation::batch& aFillShapeOps)
	{
		auto& firstOp = static_variant_cast<const graphics_operation::fill_shape&>(*aFillShapeOps.first);

		if (std::holds_alternative<gradient>(firstOp.fill))
		{
			auto const& vertices = firstOp.mesh.vertices;
			vec3 min = vertices[0].xyz;
			vec3 max = min;
			for (auto const& v : vertices)
			{
				min.x = std::min(min.x, v.x);
				max.x = std::max(max.x, v.x);
				min.y = std::min(min.y, v.y);
				max.y = std::max(max.y, v.y);
			}
			gradient_on(static_variant_cast<const gradient&>(firstOp.fill),
				rect{
					point{ min.x, min.y },
					size{ max.x - min.y, max.y - min.y } });
		}

		{
			use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES };
			for (auto op = aFillShapeOps.first; op != aFillShapeOps.second; ++op)
			{
				auto& drawOp = static_variant_cast<const graphics_operation::fill_shape&>(*op);
				auto const& vertices = drawOp.mesh.vertices;
				auto const& uv = drawOp.mesh.uv;
				for (auto const& f : drawOp.mesh.faces)
				{
					for (auto vi : f)
					{
						auto const& v = vertices[vi];
						vertexArrays.instance().push_back({
							v,
							std::holds_alternative<colour>(drawOp.fill) ?
								std::array <uint8_t, 4>{{
									static_variant_cast<const colour&>(drawOp.fill).red(),
									static_variant_cast<const colour&>(drawOp.fill).green(),
									static_variant_cast<const colour&>(drawOp.fill).blue(),
									static_variant_cast<const colour&>(drawOp.fill).alpha()}} :
								std::array <uint8_t, 4>{},
							uv[vi]});
					}
				}
			}
		}

		if (std::holds_alternative<gradient>(firstOp.fill))
			gradient_off();
	}

	namespace
	{
		void texture_vertices(const size& aTextureStorageSize, const rect& aTextureRect, const std::pair<vec2, vec2>& aLogicalCoordinates, std::vector<vec2>& aResult)
		{
			rect normalizedRect = aTextureRect / aTextureStorageSize;
			aResult.emplace_back(normalizedRect.top_left().x, normalizedRect.top_left().y);
			aResult.emplace_back(normalizedRect.top_right().x, normalizedRect.top_right().y);
			aResult.emplace_back(normalizedRect.bottom_right().x, normalizedRect.bottom_right().y);
			aResult.emplace_back(normalizedRect.bottom_left().x, normalizedRect.bottom_left().y);
			if (aLogicalCoordinates.first.y < aLogicalCoordinates.second.y)
			{
				std::swap(aResult[0][1], aResult[2][1]);
				std::swap(aResult[1][1], aResult[3][1]);
			}
		}
	}

	std::size_t opengl_graphics_context::max_operations(const graphics_operation::operation& aOperation)
	{
		auto need = 1u;
		if (std::holds_alternative<graphics_operation::draw_glyph>(aOperation))
		{
			need = 6u;
			auto& drawGlyphOp = static_variant_cast<const graphics_operation::draw_glyph&>(aOperation);
			if (drawGlyphOp.appearance.has_effect() && drawGlyphOp.appearance.effect().type() == text_effect_type::Outline)
				need += 6u * static_cast<uint32_t>(std::ceil((drawGlyphOp.appearance.effect().width() * 2 + 1) * (drawGlyphOp.appearance.effect().width() * 2 + 1)));
		}
		return rendering_engine().vertex_arrays().capacity() / need;
	}

	void opengl_graphics_context::draw_glyph(const graphics_operation::batch& aDrawGlyphOps)
	{
		auto& firstOp = static_variant_cast<const graphics_operation::draw_glyph&>(*aDrawGlyphOps.first);

		if (firstOp.glyph.is_emoji())
		{
			use_shader_program usp{ *this, iRenderingEngine, rendering_engine().default_shader_program() };
			auto const& emojiAtlas = rendering_engine().font_manager().emoji_atlas();
			auto const& emojiTexture = emojiAtlas.emoji_texture(firstOp.glyph.value()).as_sub_texture();
			draw_mesh(
				to_ecs_component(rect{ firstOp.point, glyph_extents(firstOp) }),
				game::material{ 
					{}, 
					{}, 
					{}, 
					to_ecs_component(emojiTexture)
				}, 
				mat44::identity(),
				shader_effect::None);
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			return;
		}

		const i_glyph_texture& firstGlyphTexture = glyph_texture(firstOp);

		auto need = 6u * (aDrawGlyphOps.second - aDrawGlyphOps.first);
		if (firstOp.appearance.has_effect() && firstOp.appearance.effect().type() == text_effect_type::Outline)
			need += 6u * static_cast<uint32_t>(std::ceil((firstOp.appearance.effect().width() * 2 + 1) * (firstOp.appearance.effect().width() * 2 + 1))) * (aDrawGlyphOps.second - aDrawGlyphOps.first);

		use_vertex_arrays vertexArrays{ *this, GL_QUADS, with_textures, need, firstOp.glyph.subpixel() && firstGlyphTexture.subpixel() };
		
		bool hasEffects = firstOp.appearance.has_effect();

		for (uint32_t pass = (hasEffects ? 1 : 2); pass <= 2; ++pass)
		{
			partition_iterator<graphics_operation::operation> start;
			if (pass == 1 && hasEffects)
				start = partition_iterator<graphics_operation::operation>{ aDrawGlyphOps.first, aDrawGlyphOps.second, barrier_partitions(firstOp.appearance.effect()), 2, firstOp.appearance.effect().type() == text_effect_type::Outline };
			else
				start = partition_iterator<graphics_operation::operation>{ aDrawGlyphOps.first, aDrawGlyphOps.second, 2 };
			for (auto op = start; op != aDrawGlyphOps.second; ++op)
			{
				auto& drawOp = static_variant_cast<const graphics_operation::draw_glyph&>(*op);

				const font& glyphFont = service<i_font_manager>::instance().font_from_id(drawOp.glyphFont);
				const i_glyph_texture& glyphTexture = glyph_texture(drawOp);

				vec3 glyphOrigin(
					drawOp.point.x + glyphTexture.placement().x,
					logical_coordinates().first.y < logical_coordinates().second.y ? 
						drawOp.point.y + (glyphTexture.placement().y + -glyphFont.descender()) :
						drawOp.point.y + glyphFont.height() - (glyphTexture.placement().y + -glyphFont.descender()) - glyphTexture.texture().extents().cy,
					drawOp.point.z);

				iTempTextureCoords.clear();
				texture_vertices(glyphTexture.texture().atlas_texture().storage_extents(), rect{ glyphTexture.texture().atlas_location().top_left(), glyphTexture.texture().extents() } + point{ 1.0, 1.0 }, logical_coordinates(), iTempTextureCoords);

				rect outputRect{ point{glyphOrigin}, glyphTexture.texture().extents() };

				if (drawOp.appearance.has_effect() && pass == 1)
				{
					hasEffects = true;
					if (drawOp.appearance.effect().type() == text_effect_type::Outline)
					{
						auto effectColour = std::holds_alternative<colour>(drawOp.appearance.effect().colour()) ?
							std::array <uint8_t, 4>{{
								static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).red(),
								static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).green(),
								static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).blue(),
								static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).alpha()}} :
							std::array <uint8_t, 4>{};
						auto scanlineOffsetCount = scanline_offsets(drawOp.appearance.effect());
						auto barrierPartitionCount = barrier_partitions(drawOp.appearance.effect());
						auto y = ((op.pass() - 1) % (barrierPartitionCount / 2)) / scanlineOffsetCount - drawOp.appearance.effect().width();
						auto x = ((op.pass() - 1) % (barrierPartitionCount / 2)) % scanlineOffsetCount - drawOp.appearance.effect().width();
						rect effectRect = outputRect + point{ x, y };
						vertexArrays.instance().push_back({ effectRect.top_left().to_vec3(glyphOrigin.z), effectColour, iTempTextureCoords[0] });
						vertexArrays.instance().push_back({ effectRect.bottom_left().to_vec3(glyphOrigin.z), effectColour, iTempTextureCoords[3] });
						vertexArrays.instance().push_back({ effectRect.top_right().to_vec3(glyphOrigin.z), effectColour, iTempTextureCoords[1] });
						vertexArrays.instance().push_back({ effectRect.top_right().to_vec3(glyphOrigin.z), effectColour, iTempTextureCoords[1] });
						vertexArrays.instance().push_back({ effectRect.bottom_right().to_vec3(glyphOrigin.z), effectColour, iTempTextureCoords[2] });
						vertexArrays.instance().push_back({ effectRect.bottom_left().to_vec3(glyphOrigin.z), effectColour, iTempTextureCoords[3] });
					}
				}
				else if (pass == 2)
				{
					auto ink = std::holds_alternative<colour>(drawOp.appearance.ink()) ?
						std::array <uint8_t, 4>{{
							static_variant_cast<const colour&>(drawOp.appearance.ink()).red(),
							static_variant_cast<const colour&>(drawOp.appearance.ink()).green(),
							static_variant_cast<const colour&>(drawOp.appearance.ink()).blue(),
							static_variant_cast<const colour&>(drawOp.appearance.ink()).alpha()}} :
						std::array <uint8_t, 4>{};
					vertexArrays.instance().push_back({ outputRect.top_left().to_vec3(glyphOrigin.z), ink, iTempTextureCoords[0] });
					vertexArrays.instance().push_back({ outputRect.bottom_left().to_vec3(glyphOrigin.z), ink, iTempTextureCoords[3] });
					vertexArrays.instance().push_back({ outputRect.top_right().to_vec3(glyphOrigin.z), ink, iTempTextureCoords[1] });
					vertexArrays.instance().push_back({ outputRect.top_right().to_vec3(glyphOrigin.z), ink, iTempTextureCoords[1] });
					vertexArrays.instance().push_back({ outputRect.bottom_right().to_vec3(glyphOrigin.z), ink, iTempTextureCoords[2] });
					vertexArrays.instance().push_back({ outputRect.bottom_left().to_vec3(glyphOrigin.z), ink, iTempTextureCoords[3] });
				}
			}
		}

		if (vertexArrays.instance().empty())
			return;

		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousTexture));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		if (firstOp.glyph.subpixel() && firstGlyphTexture.subpixel())
		{
			glCheck(glActiveTexture(GL_TEXTURE2));
			glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, reinterpret_cast<GLuint>(iSurface.rendering_target_texture_handle())));
			glCheck(glActiveTexture(GL_TEXTURE1));
		}

		if (std::holds_alternative<gradient>(firstOp.appearance.ink()))
			gradient_on(
				static_variant_cast<const gradient&>(firstOp.appearance.ink()), 
				rect{ 
					point{ 
						vertexArrays.instance()[0].xyz[0], 
						vertexArrays.instance()[0].xyz[1]}, 
					point{
						vertexArrays.instance()[2].xyz[0],
						vertexArrays.instance()[2].xyz[1]}});

		glCheck(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLuint>(firstGlyphTexture.texture().native_texture()->handle())));

		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		disable_anti_alias daa(*this);

		use_shader_program usp{ *this, iRenderingEngine, rendering_engine().glyph_shader_program(firstOp.glyph.subpixel() && firstGlyphTexture.subpixel())};

		for (uint32_t pass = (hasEffects ? 1 : 2); pass <= 2; ++pass)
		{
			auto& shader = rendering_engine().active_shader_program();

			rendering_engine().vertex_arrays().instantiate_with_texture_coords(*this, shader);

			bool guiCoordinates = (logical_coordinates().first.y > logical_coordinates().second.y);
			shader.set_uniform_variable("guiCoordinates", guiCoordinates);
			shader.set_uniform_variable("outputExtents", static_cast<float>(iSurface.surface_size().cx), static_cast<float>(iSurface.surface_size().cy));
			
			shader.set_uniform_variable("glyphTexture", 1);

			if (firstOp.glyph.subpixel() && firstGlyphTexture.subpixel())
				shader.set_uniform_variable("outputTexture", 2);

			glCheck(glTextureBarrier());

			if (pass == 1)
			{
				std::size_t count = (aDrawGlyphOps.second - aDrawGlyphOps.first) * vertexArrays.instance().primitive_vertex_count();
				if (firstOp.appearance.has_effect())
				{
					count *= offsets(firstOp.appearance.effect());
					switch (firstOp.appearance.effect().type())
					{
					case text_effect_type::None:
						continue;
					case text_effect_type::Outline:
						break;
					case text_effect_type::Glow:
					case text_effect_type::Shadow:
						{/*
							const i_glyph_texture& glyphTexture = drawOp.glyph.glyph_texture();
							shader.set_uniform_variable("glyphOrigin",
								static_cast<float>(vertexArrays.instance()[index].st[0] * glyphTexture.texture().atlas_texture().storage_extents().cx),
								static_cast<float>(vertexArrays.instance()[index].st[1] * glyphTexture.texture().atlas_texture().storage_extents().cy));

							if (guiCoordinates)
							{
								shader.set_uniform_variable("effectRect",
									vec4{
									vertexArrays.instance()[index].xyz[0],
									vertexArrays.instance()[index + 2].xyz[1] - 1.0,
									vertexArrays.instance()[index + 2].xyz[0],
									vertexArrays.instance()[index].xyz[1] - 1.0 });
								shader.set_uniform_variable("glyphRect",
									vec4{
									vertexArrays.instance()[index + 4].xyz[0],
									vertexArrays.instance()[index + 4 + 2].xyz[1] - 1.0,
									vertexArrays.instance()[index + 4 + 2].xyz[0],
									vertexArrays.instance()[index + 4].xyz[1] - 1.0 });
							}
							else
							{
								shader.set_uniform_variable("effectRect",
									vec4{
									vertexArrays.instance()[index].xyz[0],
									vertexArrays.instance()[index].xyz[1],
									vertexArrays.instance()[index + 2].xyz[0],
									vertexArrays.instance()[index + 2].xyz[1] });
								shader.set_uniform_variable("glyphRect",
									vec4{
									vertexArrays.instance()[index + 4].xyz[0],
									vertexArrays.instance()[index + 4].xyz[1],
									vertexArrays.instance()[index + 4 + 2].xyz[0],
									vertexArrays.instance()[index + 4 + 2].xyz[1] });
							}
							shader.set_uniform_variable("effectWidth", static_cast<int>(drawOp.appearance.effect().width()));
							glCheck(glDrawArrays(GL_QUADS, index, 4));
							index += 4;
						*/}
						break;
					}
					shader.set_uniform_variable("subpixel", static_cast<int>(firstGlyphTexture.subpixel()));
					shader.set_uniform_variable("effect", static_cast<int>(firstOp.appearance.effect().type()));
					vertexArrays.instance().draw(count, barrier_partitions(firstOp.appearance.effect()));
				}
			}
			else if (pass == 2)
			{
				shader.set_uniform_variable("subpixel", static_cast<int>(firstGlyphTexture.subpixel()));
				shader.set_uniform_variable("effect", 0);
			}
		}

		vertexArrays.instance().execute();

		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(iPreviousTexture)));

		if (std::holds_alternative<gradient>(firstOp.appearance.ink()))
			gradient_off();
	}

	void opengl_graphics_context::draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const mat44& aTransformation, shader_effect aShaderEffect)
	{
		draw_mesh(game::mesh_filter{ { &aMesh }, {}, {} }, game::mesh_renderer{ aMaterial, {} }, aTransformation, aShaderEffect);
	}
	
	void opengl_graphics_context::draw_mesh(const game::mesh_filter& aMeshFilter, const game::mesh_renderer& aMeshRenderer, const mat44& aTransformation, shader_effect aShaderEffect)
	{
		colour colourizationColour{ 0xFF, 0xFF, 0xFF, 0xFF };
		if (aMeshRenderer.material.colour != std::nullopt)
			colourizationColour = aMeshRenderer.material.colour->rgba;

		auto const transformation = aTransformation * (aMeshFilter.transformation != std::nullopt ? *aMeshFilter.transformation : mat44::identity());

		auto const& vertices = transformation * (aMeshFilter.mesh != std::nullopt ? *aMeshFilter.mesh : *aMeshFilter.sharedMesh.ptr).vertices;
		auto const& uv = aMeshFilter.mesh != std::nullopt ? aMeshFilter.mesh->uv : aMeshFilter.sharedMesh.ptr->uv;
		auto const& faces = aMeshFilter.mesh != std::nullopt ? aMeshFilter.mesh->faces : aMeshFilter.sharedMesh.ptr->faces;

		auto const& material = aMeshRenderer.material;
		auto const& patches = aMeshRenderer.patches; // todo

		if (material.texture != std::nullopt)
		{
			use_shader_program usp{ *this, iRenderingEngine, rendering_engine().texture_shader_program() };
			rendering_engine().active_shader_program().set_uniform_variable("effect", static_cast<int>(aShaderEffect));

			glCheck(glActiveTexture(GL_TEXTURE1));
			glCheck(glEnable(GL_BLEND));
			glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			GLint previousTexture;
			glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			{
				use_vertex_arrays vertexArrays { *this, GL_TRIANGLES, with_textures };
				if (!vertexArrays.instance().room_for(faces.size()))
					vertexArrays.instance().execute();

				GLuint textureHandle = 0;
				const i_sub_texture* subTexture = nullptr;
				vec2 textureStorageExtents;
				vec2 uvFixupCoefficient;
				vec2 uvFixupOffset;
				bool first = true;
				bool newTexture = false;
				for (auto const& face : faces)
				{
					auto const& texture = *service<i_texture_manager>::instance().find_texture(material.texture->id.cookie());

					if (first || textureHandle != reinterpret_cast<GLuint>(texture.native_texture()->handle()))
					{
						newTexture = true;
						textureStorageExtents = texture.storage_extents().to_vec2();
						if (texture.type() == texture_type::Texture)
							subTexture = nullptr;
						else
							subTexture = &texture.as_sub_texture();
						if (!subTexture)
						{
							uvFixupCoefficient = texture.extents().to_vec2();
							uvFixupOffset = vec2{ 1.0, 1.0 };
						}
						else
						{
							uvFixupCoefficient = subTexture->extents().to_vec2();
							uvFixupOffset = subTexture->atlas_location().top_left().to_vec2();
						}
						textureHandle = reinterpret_cast<GLuint>(texture.native_texture()->handle());
						glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.sampling() == texture_sampling::NormalMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
						glCheck(glBindTexture(GL_TEXTURE_2D, textureHandle));
						if (first)
							rendering_engine().active_shader_program().set_uniform_variable("tex", 1);
					}

					if (newTexture)
					{
						newTexture = false;
						if (!first)
							vertexArrays.instance().execute();
					}

					for (auto faceVertexIndex : face)
					{
						auto const& faceVertex = vertices[faceVertexIndex];
						auto const& faceUv = (uv[faceVertexIndex] * uvFixupCoefficient + uvFixupOffset) / textureStorageExtents;
						vertexArrays.instance().push_back(
							opengl_standard_vertex_arrays::vertex{
								faceVertex,
								std::array<uint8_t, 4>{ {
									colourizationColour.red(),
									colourizationColour.green(),
									colourizationColour.blue(),
									colourizationColour.alpha()}},
								faceUv
							});
					}

					first = false;
				}
			}

			glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));
		}
		else
		{
			use_shader_program usp{ *this, iRenderingEngine, rendering_engine().default_shader_program() };

			use_vertex_arrays vertexArrays { *this, GL_TRIANGLES };
			if (!vertexArrays.instance().room_for(faces.size()))
				vertexArrays.instance().execute();

			for (auto const& face : faces)
			{
				for (auto faceVertexIndex : face)
				{
					auto const& faceVertex = vertices[faceVertexIndex];
					vertexArrays.instance().push_back(
						opengl_standard_vertex_arrays::vertex{
							faceVertex,
							std::array<uint8_t, 4>{ {
								colourizationColour.red(),
								colourizationColour.green(),
								colourizationColour.blue(),
								colourizationColour.alpha()}}
						});
				}
			}
		}
	}

	xyz opengl_graphics_context::to_shader_vertex(const point& aPoint, coordinate aZ) const
	{
		return xyz{{ aPoint.x, aPoint.y, aZ }};
	}
}