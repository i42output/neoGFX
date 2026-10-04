// opengl_graphics_backend.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include "../i_graphics_backend.hpp"
#include "opengl.hpp"

namespace neogfx
{
    // the OpenGL calls made by the shared native code (see i_graphics_backend)
    class opengl_graphics_backend : public i_graphics_backend
    {
    public:
        opengl_graphics_backend(neogfx::renderer aRenderer);
        ~opengl_graphics_backend();
    public:
        neogfx::renderer renderer() const final;
        void initialize() final;
        void cleanup() final;
        void finish() final;
        void execute() final;
    public:
        std::unique_ptr<texture_manager> create_texture_manager() final;
        ref_ptr<i_shader_program> create_standard_shader_program() final;
        void* create_shader_program_object() final;
        void destroy_shader_program_object(void* aShaderProgramObject) final;
        void* create_shader_object(shader_type aShaderType) final;
        void destroy_shader_object(void* aShaderObject) final;
    public:
        neogfx::viewport viewport() const final;
        std::optional<rect> scissor() const final;
    public:
        gpu_buffer create_buffer(std::size_t aSize, bool aDeviceLocal) final;
        void destroy_buffer(gpu_buffer aBuffer) final;
        void* map_buffer(gpu_buffer aBuffer, std::size_t aSize) final;
        void flush_buffer(gpu_buffer aBuffer, std::size_t aOffset, std::size_t aSize) final;
        void unmap_buffer(gpu_buffer aBuffer) final;
        bool discard_buffer(gpu_buffer aBuffer) final;
        void write_buffer(gpu_buffer aBuffer, std::size_t aOffset, void const* aData, std::size_t aSize) final;
        void copy_buffer(gpu_buffer aSource, gpu_buffer aDestination, std::size_t aSize) final;
    public:
        gpu_vertex_array create_vertex_array() final;
        void destroy_vertex_array(gpu_vertex_array aVertexArray) final;
        gpu_vertex_array bound_vertex_array() const final;
        void bind_vertex_array(gpu_vertex_array aVertexArray) final;
        void set_vertex_attribute(i_shader_program const& aProgram, std::string const& aName, gpu_buffer aBuffer,
            std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset) final;
        void set_vertex_attribute(std::uint32_t aLocation, gpu_buffer aBuffer,
            std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset) final;
        void set_index_buffer(gpu_buffer aBuffer) final;
    public:
        void draw_arrays(gpu_primitive aPrimitive, std::size_t aFirst, std::size_t aCount) final;
        void draw_elements(gpu_primitive aPrimitive, std::size_t aFirstIndex, std::size_t aCount) final;
        void texture_barrier() final;
    public:
        void enable_scissor(bool aEnable) final;
        void set_scissor(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight) final;
        void enable_multisample(bool aEnable) final;
        void set_sample_shading(std::optional<double> const& aSampleShadingRate) final;
        void set_front_face(neogfx::front_face aFrontFace) final;
        void set_face_culling(neogfx::face_culling aCulling, bool aFlipped) final;
        void set_blending_mode(neogfx::blending_mode aBlendingMode) final;
        void set_xor_blending() final;
        void enable_smoothing(bool aEnable) final;
        void clear(color const& aColor) final;
        void clear_depth_buffer() final;
        void clear_stencil_buffer(std::int32_t aValue) final;
        void apply_stencil(bool aEnabled, bool aUpdating, std::int32_t aRef) final;
        bool depth_test_enabled() const final;
        void enable_depth_test(bool aEnable) final;
    public:
        void set_texture_filter(i_texture const& aTexture, texture_sampling aSampling) final;
        void set_texture_linear_clamp(i_texture const& aTexture) final;
        void set_active_texture_unit(std::uint32_t aTextureUnit) final;
        void unbind_texture_unit(std::uint32_t aTextureUnit) final;
    public:
        bool scene_shadows_available(i_standard_shader_program& aProgram) final;
        void draw_scene_shadow_maps(i_standard_shader_program& aProgram, native_scene_buffer& aSceneBuffer,
            std::vector<mat44> const& aViews, std::int32_t aDirectionalView,
            std::vector<std::optional<gpu_mesh_range>> const& aMeshes, std::vector<std::optional<scene_shadow_alpha_test>> const& aAlphaTests,
            std::uint32_t aModelTableBase) final;
        void draw_scene_background(i_standard_shader_program& aProgram, mat44 const& aNdcToClip, mat44 const& aClipToWorld,
            i_texture const* aBackgroundTexture, i_texture const* aEnvironment, double aBlur, double aIntensity) final;
    private:
        neogfx::renderer iRenderer;
    };
}
