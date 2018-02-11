// graphics_operations.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/variant.hpp>
#include <neolib/vecarray.hpp>
#include <neogfx/core/geometry.hpp>
#include <neogfx/core/path.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/../../src/gfx/native/i_native_texture.hpp>
#include <neogfx/../../src/gfx/text/native/i_native_font_face.hpp>

namespace neogfx
{
	namespace graphics_operation
	{
		struct set_logical_coordinate_system
		{
			logical_coordinate_system system;
		};

		struct set_logical_coordinates
		{
			std::pair<vec2, vec2> coordinates;
		};

		struct scissor_on
		{
			rect rect;
		};

		struct scissor_off
		{
		};

		struct clip_to_rect
		{
			rect rect;
		};

		struct clip_to_path
		{
			path path;
			dimension pathOutline;
		};

		struct reset_clip
		{
		};

		struct set_opacity
		{
			double opacity;
		};

		struct set_smoothing_mode
		{
			smoothing_mode smoothingMode;
		};

		struct push_logical_operation
		{
			logical_operation logicalOperation;
		};

		struct pop_logical_operation
		{
		};

		struct line_stipple_on
		{
			uint32_t factor;
			uint16_t pattern;
		};

		struct line_stipple_off
		{
		};

		struct subpixel_rendering_on
		{
		};

		struct subpixel_rendering_off
		{
		};

		struct clear
		{
			colour colour;
		};

		struct clear_depth_buffer
		{
		};

		struct set_pixel
		{
			point point;
			colour colour;
		};

		struct draw_pixel
		{
			point point;
			colour colour;
		};

		struct draw_line
		{
			point from;
			point to;
			pen pen;
		};

		struct draw_rect
		{
			rect rect;
			pen pen;
		};

		struct draw_rounded_rect
		{
			rect rect;
			dimension radius;
			pen pen;
		};

		struct draw_circle
		{
			point centre;
			dimension radius;
			pen pen;
			angle startAngle;
		};

		struct draw_arc
		{
			point centre;
			dimension radius;
			angle startAngle;
			angle endAngle;
			pen pen;
		};

		struct draw_path
		{
			path path;
			pen pen;
		};

		struct draw_shape
		{
			neogfx::mesh mesh;
			pen pen;
		};

		struct fill_rect
		{
			rect rect;
			brush fill;
		};

		struct fill_rounded_rect
		{
			rect rect;
			dimension radius;
			brush fill;
		};

		struct fill_circle
		{
			point centre;
			dimension radius;
			brush fill;
		};

		struct fill_arc
		{
			point centre;
			dimension radius;
			angle startAngle;
			angle endAngle;
			brush fill;
		};

		struct fill_path
		{
			path path;
			brush fill;
		};

		struct fill_shape
		{
			neogfx::mesh mesh;
			brush fill;
		};

		struct draw_glyph
		{
			vec3 point;
			glyph glyph;
			font font;
			text_appearance appearance;
		};

		struct draw_textures
		{
			neogfx::mesh mesh;
			optional_colour colour;
			shader_effect shaderEffect;
		};

		typedef neolib::variant <
			set_logical_coordinate_system,
			set_logical_coordinates,
			scissor_on,
			scissor_off,
			clip_to_rect,
			clip_to_path,
			reset_clip,
			set_opacity,
			set_smoothing_mode,
			push_logical_operation,
			pop_logical_operation,
			line_stipple_on,
			line_stipple_off,
			subpixel_rendering_on,
			subpixel_rendering_off,
			clear,
			clear_depth_buffer,
			set_pixel,
			draw_pixel,
			draw_line,
			draw_rect,
			draw_rounded_rect,
			draw_circle,
			draw_arc,
			draw_path,
			draw_shape,
			fill_rect,
			fill_rounded_rect,
			fill_circle,
			fill_arc,
			fill_path,
			fill_shape,
			draw_glyph,
			draw_textures
		> operation;

		enum operation_type
		{
			Invalid = 0,
			SetLogicalCoordinateSystem,
			SetLogicalCoordinates,
			ScissorOn,
			ScissorOff,
			ClipToRect,
			ClipToPath,
			ResetClip,
			SetOpacity,
			SetSmoothingMode,
			PushLogicalOperation,
			PopLogicalOperation,
			LineStippleOn,
			LineStippleOff,
			SubpixelRenderingOn,
			SubpixelRenderingOff,
			Clear,
			ClearDepthBuffer,
			SetPixel,
			DrawPixel,
			DrawLine,
			DrawRect,
			DrawRoundedRect,
			DrawCircle,
			DrawArc,
			DrawPath,
			DrawShape,
			FillRect,
			FillRoundedRect,
			FillCircle,
			FillArc,
			FillPath,
			FillShape,
			DrawGlyph,
			DrawTextures
		};

		inline std::string to_string(operation_type aOpType)
		{
			switch (aOpType)
			{
			case Invalid: return "Invalid";
			case SetLogicalCoordinateSystem: return "SetLogicalCoordinateSystem";
			case SetLogicalCoordinates: return "SetLogicalCoordinates";
			case ScissorOn: return "ScissorOn";
			case ScissorOff: return "ScissorOff";
			case ClipToRect: return "ClipToRect";
			case ClipToPath: return "ClipToPath";
			case ResetClip: return "ResetClip";
			case SetOpacity: return "SetOpacity";
			case SetSmoothingMode: return "SetSmoothingMode";
			case PushLogicalOperation: return "PushLogicalOperation";
			case PopLogicalOperation: return "PopLogicalOperation";
			case LineStippleOn: return "LineStippleOn";
			case LineStippleOff: return "LineStippleOff";
			case SubpixelRenderingOn: return "SubpixelRenderingOn";
			case SubpixelRenderingOff: return "SubpixelRenderingOff";
			case Clear: return "Clear";
			case ClearDepthBuffer: return "ClearDepthBuffer";
			case SetPixel: return "SetPixel";
			case DrawPixel: return "DrawPixel";
			case DrawLine: return "DrawLine";
			case DrawRect: return "DrawRect";
			case DrawRoundedRect: return "DrawRoundedRect";
			case DrawCircle: return "DrawCircle";
			case DrawArc: return "DrawArc";
			case DrawPath: return "DrawPath";
			case DrawShape: return "DrawShape";
			case FillRect: return "FillRect";
			case FillRoundedRect: return "FillRoundedRect";
			case FillCircle: return "FillCircle";
			case FillArc: return "FillArc";
			case FillPath: return "FillPath";
			case FillShape: return "FillShape";
			case DrawGlyph: return "DrawGlyph";
			case DrawTextures: return "DrawTextures";
			default: return "";
			}
		}

		bool inline batchable(const operation& aLeft, const operation& aRight)
		{
			if (aLeft.which() != aRight.which())
				return false;
			switch (static_cast<operation_type>(aLeft.which()))
			{
			case operation_type::SetPixel:
			case operation_type::DrawPixel:
			case operation_type::DrawTextures:
				return true;
			case operation_type::DrawLine:
			{
				auto& left = static_variant_cast<const draw_line&>(aLeft);
				auto& right = static_variant_cast<const draw_line&>(aRight);
				return left.pen.width() == right.pen.width() &&
					left.pen.anti_aliased() == right.pen.anti_aliased();
			}
			case operation_type::FillRect:
			{
				auto& left = static_variant_cast<const fill_rect&>(aLeft);
				auto& right = static_variant_cast<const fill_rect&>(aRight);
				return left.fill.which() == right.fill.which() && left.fill.is<colour>();
			}
			case operation_type::FillShape:
			{
				auto& left = static_variant_cast<const fill_shape&>(aLeft);
				auto& right = static_variant_cast<const fill_shape&>(aRight);
				return left.fill.which() == right.fill.which() && left.fill.is<colour>();
			}
			case operation_type::DrawGlyph:
			{
				auto& left = static_variant_cast<const draw_glyph&>(aLeft);
				auto& right = static_variant_cast<const draw_glyph&>(aRight);
				if (left.glyph.is_emoji() || right.glyph.is_emoji())
					return false;
				if (left.appearance.ink().which() != right.appearance.ink().which() || !left.appearance.ink().is<colour>())
					return false;
				if (left.appearance.has_effect() != right.appearance.has_effect())
					return false;
				if (left.appearance.has_effect() && (left.appearance.effect().type() != right.appearance.effect().type() || left.appearance.effect().width() != right.appearance.effect().width()))
					return false;
				const i_glyph_texture& leftGlyphTexture = !left.glyph.use_fallback() ? left.font.native_font_face().glyph_texture(left.glyph) :
					left.glyph.fallback_font(left.font).native_font_face().glyph_texture(left.glyph);
				const i_glyph_texture& rightGlyphTexture = !right.glyph.use_fallback() ? right.font.native_font_face().glyph_texture(right.glyph) :
					right.glyph.fallback_font(right.font).native_font_face().glyph_texture(right.glyph);
				return leftGlyphTexture.texture().native_texture()->handle() == rightGlyphTexture.texture().native_texture()->handle() &&
					left.glyph.subpixel() == right.glyph.subpixel();
			}
			default:
				return false;
			}
		}

		typedef neolib::vecarray<operation, 16, -1, neolib::nocheck> batch;

		typedef std::deque<batch, boost::fast_pool_allocator<batch>> queue;
	}
}