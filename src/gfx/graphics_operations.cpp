// graphics_operations.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/graphics_operations.hpp>
#include "native/i_native_texture.hpp"

namespace neogfx
{
	namespace graphics_operation
	{
		std::string to_string(operation_type aOpType)
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
			case DrawTexture: return "DrawTexture";
			default: return "";
			}
		}

		bool batchable(const operation& aLeft, const operation& aRight)
		{
			if (aLeft.index() != aRight.index())
				return false;
			switch (static_cast<operation_type>(aLeft.index()))
			{
			case operation_type::SetPixel:
			case operation_type::DrawPixel:
			case operation_type::DrawTexture:
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
				return left.fill.index() == right.fill.index() && std::holds_alternative<colour>(left.fill);
			}
			case operation_type::FillShape:
			{
				auto& left = static_variant_cast<const fill_shape&>(aLeft);
				auto& right = static_variant_cast<const fill_shape&>(aRight);
				return left.fill.index() == right.fill.index() && std::holds_alternative<colour>(left.fill);
			}
			case operation_type::DrawGlyph:
			{
				auto& left = static_variant_cast<const draw_glyph&>(aLeft);
				auto& right = static_variant_cast<const draw_glyph&>(aRight);
				if (left.glyph.is_emoji() || right.glyph.is_emoji())
 					return false;
				if (left.appearance.ink().index() != right.appearance.ink().index() || !std::holds_alternative<colour>(left.appearance.ink()))
					return false;
				if (left.appearance.has_effect() != right.appearance.has_effect())
					return false;
				if (left.appearance.has_effect() && (left.appearance.effect().type() != right.appearance.effect().type() || left.appearance.effect().width() != right.appearance.effect().width()))
					return false;
				if (left.glyph.subpixel() != right.glyph.subpixel())
					return false;
				const i_glyph_texture& leftGlyphTexture = glyph_texture(left);
				const i_glyph_texture& rightGlyphTexture = glyph_texture(right);
				if (leftGlyphTexture.texture().native_texture()->handle() != rightGlyphTexture.texture().native_texture()->handle())
					return false;
				if (leftGlyphTexture.subpixel() != rightGlyphTexture.subpixel())
					return false;
				return true;
			}
			default:
				return false;
			}
		}
	}
}