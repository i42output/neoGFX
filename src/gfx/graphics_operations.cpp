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
            case SnapToPixelOn: return "SnapToPixelOn";
            case SnapToPixelOff: return "SnapToPixelOff";
            case SetOpacity: return "SetOpacity";
            case SetBlendingMode: return "SetBlendingMode";
            case SetSmoothingMode: return "SetSmoothingMode";
            case PushLogicalOperation: return "PushLogicalOperation";
            case PopLogicalOperation: return "PopLogicalOperation";
            case LineStippleOn: return "LineStippleOn";
            case LineStippleOff: return "LineStippleOff";
            case SubpixelRenderingOn: return "SubpixelRenderingOn";
            case SubpixelRenderingOff: return "SubpixelRenderingOff";
            case Clear: return "Clear";
            case ClearDepthBuffer: return "ClearDepthBuffer";
            case ClearStencilBuffer: return "ClearStencilBuffer";
            case SetPixel: return "SetPixel";
            case DrawPixel: return "DrawPixel";
            case DrawLine: return "DrawLine";
            case DrawRect: return "DrawRect";
            case DrawRoundedRect: return "DrawRoundedRect";
            case DrawCircle: return "DrawCircle";
            case DrawArc: return "DrawArc";
            case DrawPath: return "DrawPath";
            case DrawShape: return "DrawShape";
            case DrawEntities: return "DrawEntities";
            case FillRect: return "FillRect";
            case FillRoundedRect: return "FillRoundedRect";
            case FillCircle: return "FillCircle";
            case FillArc: return "FillArc";
            case FillPath: return "FillPath";
            case FillShape: return "FillShape";
            case DrawGlyph: return "DrawGlyph";
            case DrawMesh: return "DrawMesh";
            default: return "";
            }
        }

        template <typename T>
        inline bool batchable(const std::optional<T>& lhs, const std::optional<T>& rhs)
        {
            return !!lhs == !!rhs && (lhs == std::nullopt || batchable(*lhs, *rhs));
        }

        bool batchable(const text_colour& lhs, const text_colour& rhs)
        {
            if (lhs.index() != rhs.index())
                return false;
            if (std::holds_alternative<colour>(lhs))
                return true;
            return lhs == rhs;
        }

        bool batchable(const text_effect& lhs, const text_effect& rhs)
        {
            return batchable(lhs.colour(), rhs.colour());
        }

        bool batchable(const operation& aLeft, const operation& aRight)
        {
            if (aLeft.index() != aRight.index())
                return false;
            switch (static_cast<operation_type>(aLeft.index()))
            {
            case operation_type::SetPixel:
            case operation_type::DrawPixel:
            case operation_type::DrawMesh:
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
            case operation_type::FillPath:
            {
                auto& left = static_variant_cast<const fill_path&>(aLeft);
                auto& right = static_variant_cast<const fill_path&>(aRight);
                return left.fill.index() == right.fill.index() && std::holds_alternative<colour>(left.fill);
            }
            case operation_type::DrawGlyph:
            {
                auto& left = static_variant_cast<const draw_glyph&>(aLeft);
                auto& right = static_variant_cast<const draw_glyph&>(aRight);
                // ensure emoji cannot allow subpixel and non-subpixel glyphs in same batch
                if (left.glyph.is_emoji() != right.glyph.is_emoji())
                    return false;
                // are both emoji?
                if (left.glyph.is_emoji())
                    return true;
                // neither are emoji...
                if (left.glyph.subpixel() != right.glyph.subpixel())
                    return false;
                const i_glyph_texture& leftGlyphTexture = left.glyph.glyph_texture();
                const i_glyph_texture& rightGlyphTexture = right.glyph.glyph_texture();
                if (leftGlyphTexture.subpixel() != rightGlyphTexture.subpixel())
                    return false;
                if (!batchable(left.appearance.ink(), right.appearance.ink()))
                    return false;
                if (!batchable(left.appearance.paper(), right.appearance.paper()))
                    return false;
                if (!batchable(left.appearance.effect(), right.appearance.effect()))
                    return false;
                return true;
            }
            default:
                return false;
            }
        }
    }
}