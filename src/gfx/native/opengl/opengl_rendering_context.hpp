// opengl_rendering_context.hpp
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

#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/entity_info.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/animation_filter.hpp>
#include <neogfx/game/rigid_body.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_render_cache.hpp>
#include "opengl.hpp"
#include "opengl_error.hpp"
#include "opengl_helpers.hpp"
#include "use_vertex_arrays.hpp"

namespace neogfx
{
    class i_widget;

    class opengl_rendering_context : public i_rendering_context
    {
    public:
        class standard_batching : public i_vertex_provider
        {
        public:
            standard_batching()
            {
                static auto& sVertexBuffer = service<i_rendering_engine>().allocate_vertex_buffer(*this);
            }
        public:
            bool cacheable() const override
            {
                return false;
            }
            const game::component<game::mesh_render_cache>& cache() const override
            {
                throw not_cacheable();
            }
            game::component<game::mesh_render_cache>& cache() override
            {
                throw not_cacheable();
            }
        };
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
        class scoped_blending_mode
        {
        public:
            scoped_blending_mode(opengl_rendering_context& aParent, neogfx::blending_mode aBlendigMode) :
                iParent{ aParent }, iPreviousBlendingMode{ aParent.blending_mode() }
            {
                iParent.set_blending_mode(aBlendigMode);
            }
            ~scoped_blending_mode()
            {
                iParent.set_blending_mode(iPreviousBlendingMode);
            }
        private:
            opengl_rendering_context& iParent;
            neogfx::blending_mode iPreviousBlendingMode;
        };
        typedef std::variant<blur_filter> filter; // todo: more filters to follow
        template <typename Filter>
        class scoped_filter
        {
        public:
            scoped_filter(opengl_rendering_context& aParent, Filter const& aFilter) :
                iParent{ aParent },
                iBufferRect{ point{}, aFilter.region.extents() + size{ aFilter.radius * 2.0 } },
                iBuffers{ std::move(create_ping_pong_buffers(aParent, iBufferRect.extents())) },
                iRenderTarget{ front_buffer() }
            {
                front_buffer().set_origin(-aFilter.region.top_left() + point{ aFilter.radius, aFilter.radius });
                iParent.iFilters.push_back(aFilter);
            }
            ~scoped_filter()
            {
                front_buffer().set_origin({});
                Filter const& filter = std::get<Filter>(iParent.iFilters.back());
                {
                    iRenderTarget.emplace(back_buffer());
                    if constexpr (std::is_same_v<Filter, blur_filter>)
                        back_buffer().blur(iBufferRect, front_buffer(), iBufferRect, filter.radius, filter.algorithm, filter.parameter1, filter.parameter2);
                }
                iRenderTarget = {};
                scoped_blending_mode sbm{ iParent, neogfx::blending_mode::Blit };
                rect const drawRect { filter.region.top_left() - point{ filter.radius, filter.radius }, iBufferRect.extents() };
                iParent.draw_texture(drawRect, back_buffer().render_target().target_texture(), iBufferRect);
                iParent.iFilters.pop_back();
            }
        public: 
            i_graphics_context const& front_buffer() const
            {
                return *iBuffers.buffer1;
            }
            i_graphics_context const& back_buffer() const
            {
                return *iBuffers.buffer2;
            }
        private:
            opengl_rendering_context& iParent;
            rect iBufferRect;
            ping_pong_buffers iBuffers;
            std::optional<scoped_render_target> iRenderTarget;
        };
        struct draw_glyph
        {
            vec3 point;
            i_glyph_text* glyphText;
            glyph_char const* glyphChar;
            text_format const* appearance;
            bool showMnemonics;
        };
        struct mesh_drawable
        {
            game::mesh_filter const* filter;
            game::mesh_renderer const* renderer;
            optional_mat44 transformation;
            game::entity_id entity;
            mesh_drawable(
                game::mesh_filter const& filter, 
                game::mesh_renderer const& renderer,
                optional_mat44 const& transformation = {},
                game::entity_id entity = game::null_entity
                ) :
                filter{ &filter },
                renderer{ &renderer },
                transformation{ transformation },
                entity{ entity }
            {}
        };
        struct patch_drawable
        {
            struct no_texture : std::logic_error { no_texture() : std::logic_error{ "neogfx::opengl_rendering_context::patch_drawable::no_texture" } {} };
            static bool has_texture(const game::mesh_renderer& meshRenderer, const game::material& material)
            {
                if (material.texture != std::nullopt)
                    return true;
                else if (material.sharedTexture != std::nullopt)
                    return true;
                else if (meshRenderer.material.texture != std::nullopt)
                    return true;
                else if (meshRenderer.material.sharedTexture != std::nullopt)
                    return true;
                else
                    return false;
            }
            static game::texture const& texture(const game::mesh_renderer& meshRenderer, const game::material& material)
            {
                if (material.texture != std::nullopt)
                    return *material.texture;
                else if (material.sharedTexture != std::nullopt)
                    return *material.sharedTexture->ptr;
                else if (meshRenderer.material.texture != std::nullopt)
                    return *meshRenderer.material.texture;
                else if (meshRenderer.material.sharedTexture != std::nullopt)
                    return *meshRenderer.material.sharedTexture->ptr;
                else
                    throw no_texture();
            }
            i_vertex_provider* provider;
            struct item
            {
                mesh_drawable* meshDrawable;
                typedef opengl_vertex_buffer<>::vertex_array vertices;
                vertices::size_type vertexArrayIndexStart;
                vertices::size_type vertexArrayIndexEnd;
                game::material const* material;
                game::faces const* faces;
                item(mesh_drawable& meshDrawable, vertices::size_type vertexArrayIndexStart, vertices::size_type vertexArrayIndexEnd) :
                    meshDrawable{ &meshDrawable }, vertexArrayIndexStart{ vertexArrayIndexStart }, vertexArrayIndexEnd{ vertexArrayIndexEnd }, material{ &meshDrawable.renderer->material }, faces{ nullptr } {}
                item(mesh_drawable& meshDrawable, vertices::size_type vertexArrayIndexStart, vertices::size_type vertexArrayIndexEnd, game::faces const& faces) :
                    meshDrawable{ &meshDrawable }, vertexArrayIndexStart{ vertexArrayIndexStart }, vertexArrayIndexEnd{ vertexArrayIndexEnd }, material{ &meshDrawable.renderer->material }, faces{ &faces } {}
                item(mesh_drawable& meshDrawable, vertices::size_type vertexArrayIndexStart, vertices::size_type vertexArrayIndexEnd, game::material const& material) :
                    meshDrawable{ &meshDrawable }, vertexArrayIndexStart{ vertexArrayIndexStart }, vertexArrayIndexEnd{ vertexArrayIndexEnd }, material{ &material }, faces{ nullptr } {}
                item(mesh_drawable& meshDrawable, vertices::size_type vertexArrayIndexStart, vertices::size_type vertexArrayIndexEnd, game::material const& material, game::faces const& faces) :
                    meshDrawable{ &meshDrawable }, vertexArrayIndexStart{ vertexArrayIndexStart }, vertexArrayIndexEnd{ vertexArrayIndexEnd }, material{ &material }, faces{ &faces } {}
                bool has_texture() const
                {
                    return patch_drawable::has_texture(*meshDrawable->renderer, *material);
                }
                game::texture const& texture() const
                {
                    return patch_drawable::texture(*meshDrawable->renderer, *material);
                }
            };
            std::vector<item> items;
        };
        typedef game::scoped_component_lock<game::entity_info, game::mesh_renderer, game::mesh_render_cache, game::mesh_filter, game::animation_filter, game::rigid_body> ecs_render_lock;
        typedef std::optional<ecs_render_lock> optional_ecs_render_lock;
    public:
        opengl_rendering_context(const i_render_target& aTarget, blending_mode aBlendingMode = blending_mode::Default);
        opengl_rendering_context(const i_render_target& aTarget, const i_widget& aWidget, blending_mode aBlendingMode = blending_mode::Default);
        opengl_rendering_context(const opengl_rendering_context& aOther);
        ~opengl_rendering_context();
    public:
        std::unique_ptr<i_rendering_context> clone() const override;
    public:
        i_rendering_engine& rendering_engine() const override;
        const i_render_target& render_target() const override;
        rect rendering_area(bool aConsiderScissor = true) const override;
    public:
        const graphics_operation::queue& queue() const override;
        graphics_operation::queue& queue() override;
        void enqueue(const graphics_operation::operation& aOperation) override;
        void flush() override;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const override;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem);
        neogfx::logical_coordinates logical_coordinates() const override;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates);
        point origin() const;
        void set_origin(const point& aOrigin);
        vec2 offset() const override;
        void set_offset(const optional_vec2& aOffset) override;
        bool gradient_set() const override;
        void apply_gradient(i_gradient_shader& aShader) override;
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
        bool logical_operation_active() const;
        void push_logical_operation(logical_operation aLogicalOperation);
        void pop_logical_operation();
        void line_stipple_on(stipple const& aStipple);
        void line_stipple_off();
        void set_gradient(const gradient& aGradient);
        void clear_gradient();
        bool is_subpixel_rendering_on() const;
        void subpixel_rendering_on();
        void subpixel_rendering_off();
        void clear(const color& aColor);
        void clear_depth_buffer();
        void clear_stencil_buffer();
        void set_pixel(const point& aPoint, const color& aColor);
        void draw_pixel(const point& aPoint, const color& aColor);
        void draw_pixels(const graphics_operation::batch& aFillRectOps);
        void draw_line(const point& aFrom, const point& aTo, const pen& aPen);
        void draw_triangles(const graphics_operation::batch& aDrawTriangleOps);
        void draw_rects(const graphics_operation::batch& aDrawRectOps);
        void draw_rounded_rects(const graphics_operation::batch& aDrawRoundedRectOps);
        void draw_ellipse_rects(const graphics_operation::batch& aDrawEllipseRectOps);
        void draw_checker_rects(const graphics_operation::batch& aDrawCheckerRectOps);
        void draw_circles(const graphics_operation::batch& aDrawCircleOps);
        void draw_ellipses(const graphics_operation::batch& aDrawEllipseOps);
        void draw_pies(const graphics_operation::batch& aDrawPieOps);
        void draw_arcs(const graphics_operation::batch& aDrawArcOps);
        void draw_cubic_bezier(const point& aP0, const point& aP1, const point& aP2, const point& aP3, const pen& aPen);
        void draw_path(const path& aPath, const pen& aPen);
        void draw_shapes(const graphics_operation::batch& aDrawShapeOps);
        void draw_shape(const game::mesh& aMesh, const vec3& aPosition, const pen& aPen);
        void draw_entities(game::i_ecs& aEcs, game::scene_layer aLayer, const mat44& aTransformation);
        void fill_path(const path& aPath, const brush& aFill);
        void fill_shape(const game::mesh& aMesh, const vec3& aPosition, const brush& aFill);
        void draw_glyphs(const graphics_operation::batch& aDrawGlyphOps);
        void draw_glyphs(const draw_glyph* aBegin, const draw_glyph* aEnd);
        void draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const mat44& aTransformation, const std::optional<game::filter>& aFilter = {});
        void draw_mesh(const game::mesh_filter& aMeshFilter, const game::mesh_renderer& aMeshRenderer, const mat44& aTransformation);
        void draw_meshes(optional_ecs_render_lock& aLock, i_vertex_provider& aVertexProvider, mesh_drawable* aFirst, mesh_drawable* aLast, const mat44& aTransformation);
        void draw_patch(patch_drawable& aPatch, const mat44& aTransformation);
        void draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor = {}, shader_effect aShaderEffect = shader_effect::None);
    public:
        neogfx::subpixel_format subpixel_format() const override;
    private:
        void apply_scissor();
        void apply_logical_operation();
    private:
        i_rendering_engine& iRenderingEngine;
        const i_render_target& iTarget;
        const i_widget* iWidget;
        graphics_operation::queue iQueue;
        bool iInFlush;
        mutable std::optional<neogfx::logical_coordinate_system> iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        point iOrigin;
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
        std::optional<std::uint8_t> iLastDrawGlyphFallbackFontIndex;
        sink iSink;
        optional_vec2 iOffset;
        bool iSnapToPixel;
        bool iSnapToPixelUsesOffset;
        std::optional<gradient> iGradient;
        std::vector<filter> iFilters;
        use_shader_program iUseDefaultShaderProgram; // must be last
    private:
        static standard_batching& as_vertex_provider()
        {
            static standard_batching sProvider;
            return sProvider;
        }
    };
}