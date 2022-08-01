// graphics_operations.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
            case SetViewport: return "SetViewport";
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

        bool batchable(const text_color& lhs, const text_color& rhs)
        {
            if (lhs.index() != rhs.index())
                return false;
            if (std::holds_alternative<color>(lhs))
                return true;
            return lhs == rhs;
        }

        bool batchable(const text_effect& lhs, const text_effect& rhs)
        {
            return batchable(lhs.color(), rhs.color()) && lhs.type() == rhs.type();
        }

        template <typename T>
        inline bool batchable(const std::optional<T>& lhs, const std::optional<T>& rhs)
        {
            return !!lhs == !!rhs && (lhs == std::nullopt || batchable(*lhs, *rhs));
        }

        template <typename T>
        inline bool batchable(const optional<T>& lhs, const optional<T>& rhs)
        {
            return !!lhs == !!rhs && (lhs == std::nullopt || batchable(*lhs, *rhs));
        }

        bool batchable(i_glyph_text const& lhsText, i_glyph_text const& rhsText, glyph const& lhs, glyph const& rhs)
        {
            // either whitespace?
            if (is_whitespace(lhs) || is_whitespace(rhs))
                return true;
            // ensure emoji cannot allow subpixel and non-subpixel glyphs in same batch
            if (is_emoji(lhs) != is_emoji(rhs))
                return false;
            // are both emoji?
            if (is_emoji(lhs))
                return true;
            // neither are emoji...
            if (subpixel(lhs) != subpixel(rhs))
                return false;
            const i_glyph_texture& leftGlyphTexture = lhsText.glyph_texture(lhs);
            const i_glyph_texture& rightGlyphTexture = rhsText.glyph_texture(rhs);
            if (leftGlyphTexture.subpixel() != rightGlyphTexture.subpixel())
                return false;
            return true;
        };

        bool batchable(text_attributes const& aLeft, text_attributes const& aRight)
        {
            if (!batchable(aLeft.ink(), aRight.ink()))
                return false;
            if (!batchable(aLeft.paper(), aRight.paper()))
                return false;
            if (!batchable(aLeft.effect(), aRight.effect()))
                return false;
            return true;
        }

        bool batchable(const operation& aLeft, const operation& aRight)
        {
            auto const leftOp = static_cast<operation_type>(aLeft.index());
            auto const rightOp = static_cast<operation_type>(aRight.index());
            if (leftOp != rightOp)
            {
                return false;
            }
            switch (leftOp)
            {
            case operation_type::SetPixel:
            case operation_type::DrawPixel:
            case operation_type::DrawMesh:
            case operation_type::DrawGlyph:
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
                return left.fill.index() == right.fill.index() && std::holds_alternative<color>(left.fill);
            }
            case operation_type::FillCheckerRect:
            {
                auto& left = static_variant_cast<const fill_checker_rect&>(aLeft);
                auto& right = static_variant_cast<const fill_checker_rect&>(aRight);
                return left.fill1.index() == right.fill1.index() && std::holds_alternative<color>(left.fill1) &&
                    left.fill2.index() == right.fill2.index() && std::holds_alternative<color>(left.fill2);
            }
            case operation_type::FillShape:
            {
                auto& left = static_variant_cast<const fill_shape&>(aLeft);
                auto& right = static_variant_cast<const fill_shape&>(aRight);
                return left.fill.index() == right.fill.index() && std::holds_alternative<color>(left.fill);
            }
            case operation_type::FillPath:
            {
                auto& left = static_variant_cast<const fill_path&>(aLeft);
                auto& right = static_variant_cast<const fill_path&>(aRight);
                return left.fill.index() == right.fill.index() && std::holds_alternative<color>(left.fill);
            }
            default:
                return false;
            }
        }
    }
}