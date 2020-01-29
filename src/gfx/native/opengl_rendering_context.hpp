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
            disable_anti_alias(opengl_rendering_context& aParent) : scoped_anti_alias{ aParent, neogfx::smoothing_mode::None }
            {
            }
        };
        class scoped_multisample
        {
        public:
            scoped_multisample(opengl_rendering_context& aParent, bool aMultisample) : iParent(aParent), iOldMultiSample(aParent.multisample())
            {
                iParent.set_multisample(aMultisample);
            }
            ~scoped_multisample()
            {
                iParent.set_multisample(iOldMultiSample);
            }
        private:
            opengl_rendering_context& iParent;
            bool iOldMultiSample;
        };
        class disable_multisample : public scoped_multisample
        {
        public:
            disable_multisample(opengl_rendering_context& aParent) : scoped_multisample{ aParent, false }
            {
            }
        };
        struct mesh_drawable
        {
            game::mesh_filter const* filter;
            game::mesh_renderer const* renderer;
            mat44 transformation;
            game::entity_id entity;
            bool drawn;
            mesh_drawable(
                game::mesh_filter const& filter, 
                game::mesh_renderer const& renderer,
                mat44 const& transformation = mat44::identity(),
                game::entity_id entity = game::null_entity
                ) :
                filter{ &filter },
                renderer{ &renderer },
                transformation{ transformation },
                entity{ entity },
                drawn{ false }
            {}
        };
        struct patch_drawable
        {
            vertices xyz;
            vertices_2d uv;
            struct item
            {
                struct no_texture : std::logic_error { no_texture() : std::logic_error{ "neogfx::opengl_rendering_context::patch_drawable::item::no_texture" } {} };
                mesh_drawable* mesh;
                vertices::size_type offsetVertices;
                vertices_2d::size_type offsetTextureVertices;
                game::material const* material;
                game::faces const* faces;
                item(mesh_drawable& mesh, vertices::size_type offsetVertices, vertices_2d::size_type offsetTextureVertices, game::faces const& faces) :
                    mesh{ &mesh }, offsetVertices{ offsetVertices }, offsetTextureVertices{ offsetTextureVertices }, material{ &mesh.renderer->material }, faces{ &faces }                {}
                item(mesh_drawable& mesh, vertices::size_type offsetVertices, vertices_2d::size_type offsetTextureVertices, game::material const& material, game::faces const& faces) :
                    mesh{ &mesh }, offsetVertices{ offsetVertices }, offsetTextureVertices{ offsetTextureVertices }, material{ &material }, faces{ &faces }                {}
                bool has_texture() const
                {
                    if (material->texture != std::nullopt)
                        return true;
                    else if (material->sharedTexture != std::nullopt)
                        return true;
                    else if (mesh->renderer->material.texture != std::nullopt)
                        return true;
                    else if (mesh->renderer->material.sharedTexture != std::nullopt)
                        return true;
                    else
                        return false;
                }
                game::texture const& texture() const
                {
                    if (material->texture != std::nullopt)
                        return *material->texture;
                    else if (material->sharedTexture != std::nullopt)
                        return *material->sharedTexture->ptr;
                    else if (mesh->renderer->material.texture != std::nullopt)
                        return *mesh->renderer->material.texture;
                    else if (mesh->renderer->material.sharedTexture != std::nullopt)
                        return *mesh->renderer->material.sharedTexture->ptr;
                    else
                        throw no_texture();
                }
            };
            std::vector<item> items;
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
        const graphics_operation::queue& queue() const override;
        graphics_operation::queue& queue() override;
        void enqueue(const graphics_operation::operation& aOperation) override;
        void flush() override;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem);
        neogfx::logical_coordinates logical_coordinates() const override;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates);
        vec2 offset() const override;
        void set_offset(const optional_vec2& aOffset) override;
        bool snap_to_pixel() const;
        void set_snap_to_pixel(bool aSnapToPixel);
        void scissor_on(const rect& aRect);
        void scissor_off();
        const optional_rect& scissor_rect() const;
        bool multisample() const;
        void set_multisample(bool aMultisample);
        void enable_sample_shading(double aSampleShadingRate);
        void disable_sample_shading();
        void set_opacity(double aOpacity);
        neogfx::blending_mode blending_mode() const;
        void set_blending_mode(neogfx::blending_mode aBlendingMode);
        neogfx::smoothing_mode smoothing_mode() const;
        void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode);
        void push_logical_operation(logical_operation aLogicalOperation);
        void pop_logical_operation();
        void line_stipple_on(uint32_t aFactor, uint16_t aPattern, scalar aPosition);
        void line_stipple_off();
        bool is_subpixel_rendering_on() const;
        void subpixel_rendering_on();
        void subpixel_rendering_off();
        void clear(const color& aColor);
        void clear_depth_buffer();
        void clear_stencil_buffer();
        void set_pixel(const point& aPoint, const color& aColor);
        void draw_pixel(const point& aPoint, const color& aColor);
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
        void draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const mat44& aTransformation);
        void draw_mesh(const game::mesh_filter& aMeshFilter, const game::mesh_renderer& aMeshRenderer, const mat44& aTransformation);
        void draw_meshes(mesh_drawable* aFirst, mesh_drawable* aLast, const mat44& aTransformation);
        void draw_patch(patch_drawable& aPatch);
    public:
        neogfx::subpixel_format subpixel_format() const override;
    private:
        void apply_scissor();
        void apply_logical_operation();
    private:
        i_rendering_engine& iRenderingEngine;
        const i_render_target& iTarget;
        const i_widget* iWidget;
        mutable std::optional<neogfx::logical_coordinate_system> iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        bool iMultisample;
        std::optional<double> iSampleShadingRate;
        double iOpacity;
        std::optional<neogfx::blending_mode> iBlendingMode;
        std::optional<neogfx::smoothing_mode> iSmoothingMode;
        bool iSubpixelRendering;
        std::vector<logical_operation> iLogicalOperationStack;
        std::list<use_shader_program> iShaderProgramStack;
        std::vector<rect> iScissorRects;
        mutable optional_rect iScissorRect;
        GLint iPreviousTexture;
        font iLastDrawGlyphFallbackFont;
        std::optional<uint8_t> iLastDrawGlyphFallbackFontIndex;
        sink iSink;
        scoped_render_target iSrt;
        use_shader_program iUseDefaultShaderProgram;
        optional_vec2 iOffset;
        bool iSnapToPixel;
    };
}