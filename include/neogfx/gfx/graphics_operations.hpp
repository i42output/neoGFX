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
            point center;
            dimension radius;
            pen pen;
            angle startAngle;
        };

        struct draw_arc
        {
            point center;
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
            game::mesh mesh;
            vec3 position;
            pen pen;
        };

        struct draw_entities
        {
            game::i_ecs& ecs;
            int32_t layer;
            mat44 transformation;
        };

        struct fill_rect
        {
            rect rect;
            brush fill;
            scalar zpos;
        };

        struct fill_rounded_rect
        {
            rect rect;
            dimension radius;
            brush fill;
        };

        struct fill_circle
        {
            point center;
            dimension radius;
            brush fill;
        };

        struct fill_arc
        {
            point center;
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
            game::mesh mesh;
            vec3 position;
            brush fill;
        };

        struct draw_glyph
        {
            vec3 point;
            glyph_ex glyph;
            text_appearance appearance;
        };

        struct draw_mesh
        {
            game::mesh mesh;
            game::material material;
            mat44 transformation;
            std::optional<game::filter> filter;
        };

        typedef neolib::variant<
            set_logical_coordinate_system,
            set_logical_coordinates,
            set_origin,
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
            draw_rect,
            draw_rounded_rect,
            draw_circle,
            draw_arc,
            draw_path,
            draw_shape,
            draw_entities,
            fill_rect,
            fill_rounded_rect,
            fill_circle,
            fill_arc,
            fill_path,
            fill_shape,
            draw_glyph,
            draw_mesh
        > operation;

        enum operation_type
        {
            Invalid = 0,
            SetLogicalCoordinateSystem,
            SetLogicalCoordinates,
            SetOrigin,
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
            DrawRect,
            DrawRoundedRect,
            DrawCircle,
            DrawArc,
            DrawPath,
            DrawShape,
            DrawEntities,
            FillRect,
            FillRoundedRect,
            FillCircle,
            FillArc,
            FillPath,
            FillShape,
            DrawGlyph,
            DrawMesh
        };

        std::string to_string(operation_type aOpType);

        bool batchable(const operation& aLeft, const operation& aRight);

        typedef std::vector<operation> queue;
        typedef std::pair<operation const*, operation const*> batch;
    }
}