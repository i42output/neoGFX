// opengl_rendering_context.hpp
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
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include "opengl.hpp"
#include "opengl_error.hpp"
#include "opengl_helpers.hpp"

namespace neogfx
{
    class i_widget;

    class opengl_rendering_context : public i_rendering_context
    {
    public:
        class scoped_anti_alias
        {
        public:
            scoped_anti_alias(opengl_rendering_context& aParent, neogfx::smoothing_mode aNewSmoothingMode) : iParent(aParent), iOldSmoothingMode(aParent.smoothing_mode())
            {
                iParent.set_smoothing_mode(aNewSmoothingMode);
            }
            ~scoped_anti_alias()
            {
                iParent.set_smoothing_mode(iOldSmoothingMode);
            }
        private:
            opengl_rendering_context& iParent;
            neogfx::smoothing_mode iOldSmoothingMode;
        };
        class disable_anti_alias : public scoped_anti_alias
        {
        public:
            disable_anti_alias(opengl_rendering_context& aParent) : scoped_anti_alias(aParent, neogfx::smoothing_mode::None)
            {
            }
        };
    public:
        opengl_rendering_context(const i_render_target& aTarget, blending_mode aBlendingMode = blending_mode::Default);
        opengl_rendering_context(const i_render_target& aTarget, const i_widget& aWidget, blending_mode aBlendingMode = blending_mode::Default);
        opengl_rendering_context(const opengl_rendering_context& aOther);
        ~opengl_rendering_context();
    public:
        std::unique_ptr<i_rendering_context> clone() const override;
    public:
        i_rendering_engine& rendering_engine() override;
        const i_render_target& render_target() const override;
        const i_render_target& render_target() override;
        rect rendering_area(bool aConsiderScissor = true) const override;
    public:
        void enqueue(const graphics_operation::operation& aOperation) override;
        void flush() override;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem);
        neogfx::logical_coordinates logical_coordinates() const override;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates);
        vec2 offset() const override;
        void set_offset(const optional_vec2& aOffset) override;
        const optional_vec2& pixel_adjust() const;
        void set_pixel_adjust(const optional_vec2& aAdjust);
        void scissor_on(const rect& aRect);
        void scissor_off();
        const optional_rect& scissor_rect() const;
        void clip_to(const rect& aRect);
        void clip_to(const path& aPath, dimension aPathOutline);
        void reset_clip();
        void apply_clip();
        void set_opacity(double aOpacity);
        neogfx::blending_mode blending_mode() const;
        void set_blending_mode(neogfx::blending_mode aBlendingMode);
        neogfx::smoothing_mode smoothing_mode() const;
        void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode);
        void push_logical_operation(logical_operation aLogicalOperation);
        void pop_logical_operation();
        void line_stipple_on(uint32_t aFactor, uint16_t aPattern);
        void line_stipple_off();
        bool is_subpixel_rendering_on() const;
        void subpixel_rendering_on();
        void subpixel_rendering_off();
        void clear(const colour& aColour);
        void clear_depth_buffer();
        void clear_stencil_buffer();
        void set_pixel(const point& aPoint, const colour& aColour);
        void draw_pixel(const point& aPoint, const colour& aColour);
        void draw_line(const point& aFrom, const point& aTo, const pen& aPen);
        void draw_rect(const rect& aRect, const pen& aPen);
        void draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen);
        void draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, angle aStartAngle);
        void draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen);
        void draw_path(const path& aPath, const pen& aPen);
        void draw_shape(const game::mesh& aMesh, const pen& aPen);
        void draw_entities(game::i_ecs& aEcs, const mat44& aTransformation);
        void fill_rect(const rect& aRect, const brush& aFill, scalar aZpos = 0.0);
        void fill_rect(const graphics_operation::batch& aFillRectOps);
        void fill_rounded_rect(const rect& aRect, dimension aRadius, const brush& aFill);
        void fill_circle(const point& aCentre, dimension aRadius, const brush& aFill);
        void fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill);
        void fill_path(const path& aPath, const brush& aFill);
        void fill_shape(const game::mesh& aMesh, const brush& aFill);
        void fill_shape(const graphics_operation::batch& aFillShapeOps);
        void draw_glyph(const graphics_operation::batch& aDrawGlyphOps);
        bool draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const mat44& aTransformation);
        bool draw_mesh(const game::mesh_filter& aMeshFilter, const game::mesh_renderer& aMeshRenderer, const mat44& aTransformation);
        bool draw_patch(const vertices_t& aVertices, const vertices_2d_t& aTextureVertices, const game::material& aMaterial, const game::faces_t& aFaces);
    public:
        neogfx::subpixel_format subpixel_format() const override;
    private:
        std::size_t max_operations(const graphics_operation::operation& aOperation);
        void apply_scissor();
        void apply_logical_operation();
        xyz to_shader_vertex(const point& aPoint, coordinate aZ = 0.0) const;
    private:
        i_rendering_engine& iRenderingEngine;
        const i_render_target& iTarget;
        const i_widget* iWidget;
        graphics_operation::queue iQueue;
        mutable std::optional<neogfx::logical_coordinate_system> iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        double iOpacity;
        neogfx::blending_mode iBlendingMode;
        neogfx::smoothing_mode iSmoothingMode; 
        bool iSubpixelRendering;
        std::vector<logical_operation> iLogicalOperationStack;
        std::list<use_shader_program> iShaderProgramStack;
        uint32_t iClipCounter;
        std::vector<rect> iScissorRects;
        mutable optional_rect iScissorRect;
        GLint iPreviousTexture;
        font iLastDrawGlyphFallbackFont;
        std::optional<uint8_t> iLastDrawGlyphFallbackFontIndex;
        vertices_2d_t iTempTextureCoords;
        sink iSink;
        scoped_render_target iSrt;
        use_shader_program iUseDefaultShaderProgram;
        optional_vec2 iOffset;
        optional_vec2 iPixelAdjust;
    };
}