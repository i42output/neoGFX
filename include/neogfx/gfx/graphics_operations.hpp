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

#include <vector>
#include <ranges>

#include <neolib/core/variant.hpp>

#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/path.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/filter.hpp>

namespace neogfx
{
    namespace graphics_operation
    {
        struct set_logical_coordinate_system
        {
            logical_coordinate_system system;
        };

        struct set_origin
        {
            point origin;
        };

        struct set_logical_coordinates
        {
            logical_coordinates coordinates;
        };

        struct set_viewport
        {
            std::optional<rect> viewport;
        };

        struct set_view_transformation
        {
            std::optional<mat33> viewTransformation;
        };

        struct scissor_on
        {
            rect rect;
        };

        struct scissor_off
        {
        };

        struct snap_to_pixel_on
        {
        };

        struct snap_to_pixel_off
        {
        };

        struct set_opacity
        {
            double opacity;
        };

        struct set_blending_mode
        {
            blending_mode blendingMode;
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
            scalar factor;
            uint16_t pattern;
            scalar position;
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
            color color;
        };

        struct clear_depth_buffer
        {
        };

        struct clear_stencil_buffer
        {
        };

        struct clear_gradient
        {
        };

        struct set_gradient
        {
            gradient gradient;
        };

        struct set_pixel
        {
            point point;
            color color;
        };

        struct draw_pixel
        {
            point point;
            color color;
        };

        struct draw_line
        {
            point from;
            point to;
            pen pen;
        };

        struct draw_triangle
        {
            point p0;
            point p1;
            point p2;
            pen pen;
            brush fill;
        };

        struct draw_rect
        {
            rect rect;
            pen pen;
            brush fill;
        };

        struct draw_rounded_rect
        {
            rect rect;
            vec4 radius;
            pen pen;
            brush fill;
        };

        struct draw_checker_rect
        {
            rect rect;
            size squareSize;
            pen pen;
            brush fill1;
            brush fill2;
        };

        struct draw_circle
        {
            point center;
            dimension radius;
            pen pen;
            brush fill;
        };

        struct draw_ellipse
        {
            point center;
            dimension radiusA;
            dimension radiusB;
            pen pen;
            brush fill;
        };

        struct draw_pie
        {
            point center;
            dimension radius;
            angle startAngle;
            angle endAngle;
            pen pen;
            brush fill;
        };

        struct draw_arc
        {
            point center;
            dimension radius;
            angle startAngle;
            angle endAngle;
            pen pen;
            brush fill;
        };

        struct draw_cubic_bezier
        {
            point p0;
            point p1;
            point p2;
            point p3;
            pen pen;
        };

        struct draw_path
        {
            path path;
            pen pen;
            brush fill;
        };

        struct draw_shape
        {
            game::mesh mesh;
            vec3 position;
            pen pen;
            brush fill;
        };

        struct draw_entities
        {
            game::i_ecs& ecs;
            game::scene_layer layer;
            mat44 transformation;
        };

        struct draw_glyphs
        {
            vec3 point;
            glyph_text glyphText;
            glyph_text::const_iterator begin;
            glyph_text::const_iterator end;
            text_format_spans attributes;
            bool showMnemonics;
        };

        struct draw_mesh
        {
            game::mesh mesh;
            game::material material;
            mat44 transformation;
            std::optional<game::filter> filter;
        };

        typedef std::variant<std::monostate,
            set_logical_coordinate_system,
            set_logical_coordinates,
            set_origin,
            set_viewport,
            set_view_transformation,
            scissor_on,
            scissor_off,
            snap_to_pixel_on,
            snap_to_pixel_off,
            set_opacity,
            set_blending_mode,
            set_smoothing_mode,
            push_logical_operation,
            pop_logical_operation,
            line_stipple_on,
            line_stipple_off,
            subpixel_rendering_on,
            subpixel_rendering_off,
            clear,
            clear_depth_buffer,
            clear_stencil_buffer,
            clear_gradient,
            set_gradient,
            set_pixel,
            draw_pixel,
            draw_line,
            draw_triangle,
            draw_rect,
            draw_rounded_rect,
            draw_checker_rect,
            draw_circle,
            draw_ellipse,
            draw_pie,
            draw_arc,
            draw_cubic_bezier,
            draw_path,
            draw_shape,
            draw_entities,
            draw_glyphs,
            draw_mesh
        > operation;

        enum operation_type
        {
            Invalid = 0,
            SetLogicalCoordinateSystem,
            SetLogicalCoordinates,
            SetOrigin,
            SetViewport,
            SetViewTransformation,
            ScissorOn,
            ScissorOff,
            SnapToPixelOn,
            SnapToPixelOff,
            SetOpacity,
            SetBlendingMode,
            SetSmoothingMode,
            PushLogicalOperation,
            PopLogicalOperation,
            LineStippleOn,
            LineStippleOff,
            SubpixelRenderingOn,
            SubpixelRenderingOff,
            Clear,
            ClearDepthBuffer,
            ClearStencilBuffer,
            ClearGradient,
            SetGradient,
            SetPixel,
            DrawPixel,
            DrawLine,
            DrawTriangle,
            DrawRect,
            DrawRoundedRect,
            DrawCheckerRect,
            DrawCircle,
            DrawEllipse,
            DrawPie,
            DrawArc,
            DrawCubicBezier,
            DrawPath,
            DrawShape,
            DrawEntities,
            DrawGlyph,
            DrawMesh
        };

        std::string to_string(operation_type aOpType);

        bool batchable(const operation& aLeft, const operation& aRight);
        bool batchable(text_format const& aLeft, text_format const& aRight);
        bool batchable(i_glyph_text const& lhsText, i_glyph_text const& rhsText, glyph_char const& lhs, glyph_char const& rhs);

        typedef std::vector<operation> queue;
        typedef std::ranges::subrange<operation const*> batch;
    }
}