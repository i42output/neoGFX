// i_graphics_backend.hpp
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

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_render_target.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/i_standard_shader_program.hpp>
#include <neogfx/gfx/texture_manager.hpp>

namespace neogfx
{
    class native_scene_buffer;

    // The graphics API calls made by the code shared by the native backends (native_rendering_context,
    // native_renderer, native_buffer, native_vertex_buffer, native_scene_buffer and native_triangle_renderer).
    // n.b. the shared code is the code that was written for OpenGL: it keeps its names and the semantics of each
    // call are those of the OpenGL calls it replaces (an immediate, ordered state machine); a backend for an
    // explicit API (Vulkan) records and orders the work itself.

    using gpu_buffer = std::uintptr_t;
    using gpu_vertex_array = std::uintptr_t;
    constexpr gpu_buffer no_gpu_buffer = 0u;
    constexpr gpu_vertex_array no_gpu_vertex_array = 0u;

    enum class gpu_primitive : std::uint32_t
    {
        Triangles,
        TriangleStrip
    };

    enum class gpu_attribute_type : std::uint32_t
    {
        Float,
        Double,
        UnsignedByte,
        UnsignedShort
    };

    template <typename T>
    struct gpu_attribute_type_of {};
    template <>
    struct gpu_attribute_type_of<float> { static constexpr gpu_attribute_type type = gpu_attribute_type::Float; };
    template <>
    struct gpu_attribute_type_of<double> { static constexpr gpu_attribute_type type = gpu_attribute_type::Double; };
    template <>
    struct gpu_attribute_type_of<std::uint8_t> { static constexpr gpu_attribute_type type = gpu_attribute_type::UnsignedByte; };
    template <>
    struct gpu_attribute_type_of<std::uint16_t> { static constexpr gpu_attribute_type type = gpu_attribute_type::UnsignedShort; };

    // a scene mesh's vertices and indices in a scene buffer (see native_scene_buffer)
    struct gpu_mesh_range
    {
        std::uint32_t vertexStart;
        std::uint32_t vertexEnd;
        std::uint32_t indexStart;
        std::uint32_t indexEnd;
    };

    // a scene mesh's alpha test (glTF alpha mode MASK) when drawn into the shadow maps: its base colour texture, that
    // texture's wrap transform, the alpha cutoff and the wrapping
    using scene_shadow_alpha_test = std::tuple<i_texture const*, vec4f, float, texture_wrap, texture_wrap>;

    // the shadow map atlas (see standard-pbr.frag): view 0 (the directional light's) is the bottom left quarter; views 1 to 48
    // (six cube faces for each of up to eight point lights) are 512 square tiles in the other quarters
    namespace scene_shadow_atlas
    {
        constexpr std::int32_t AtlasSize = 4096;
        constexpr std::int32_t DirectionalSize = 2048;
        constexpr std::int32_t FaceSize = 512;
        constexpr std::uint32_t MaxShadowCastingPointLights = 8u;

        inline std::array<std::int32_t, 4> tile(std::uint32_t aView)
        {
            if (aView == 0u)
                return { 0, 0, DirectionalSize, DirectionalSize };
            auto const face = aView - 1u;
            auto const quadrant = 1u + face / 16u;
            auto const local = face % 16u;
            return {
                static_cast<std::int32_t>((quadrant % 2u) * DirectionalSize + (local % 4u) * FaceSize),
                static_cast<std::int32_t>((quadrant / 2u) * DirectionalSize + (local / 4u) * FaceSize),
                FaceSize, FaceSize };
        }
    }

    class i_graphics_backend
    {
    public:
        virtual ~i_graphics_backend() = default;
        // device
    public:
        virtual neogfx::renderer renderer() const = 0;
        virtual void initialize() = 0;
        virtual void cleanup() = 0;
        // wait for all submitted work to complete (glFinish)
        virtual void finish() = 0;
        // complete the work recorded so far before the host reuses memory the GPU may still be reading (e.g. vertices
        // cleared or reclaimed); n.b. a no-op for OpenGL, whose calls are (as far as the host can tell) immediate
        virtual void execute() = 0;
        // objects
    public:
        virtual std::unique_ptr<texture_manager> create_texture_manager() = 0;
        virtual ref_ptr<i_shader_program> create_standard_shader_program() = 0;
        virtual void* create_shader_program_object() = 0;
        virtual void destroy_shader_program_object(void* aShaderProgramObject) = 0;
        virtual void* create_shader_object(shader_type aShaderType) = 0;
        virtual void destroy_shader_object(void* aShaderObject) = 0;
        // the active target's viewport and scissor
    public:
        virtual neogfx::viewport viewport() const = 0;
        virtual std::optional<rect> scissor() const = 0;
        // buffers (see native_buffer): a buffer is either mapped (persistently, written through the mapping) or device local
        // (written with write_buffer)
    public:
        virtual gpu_buffer create_buffer(std::size_t aSize, bool aDeviceLocal) = 0;
        virtual void destroy_buffer(gpu_buffer aBuffer) = 0;
        virtual void* map_buffer(gpu_buffer aBuffer, std::size_t aSize) = 0;
        virtual void flush_buffer(gpu_buffer aBuffer, std::size_t aOffset, std::size_t aSize) = 0;
        virtual void unmap_buffer(gpu_buffer aBuffer) = 0;
        // a mapped buffer's contents are no longer wanted (see native_buffer::clear): returns true if the buffer now has
        // different storage (so is to be mapped again) because the GPU may still be reading the old (cf. buffer orphaning)
        virtual bool discard_buffer(gpu_buffer aBuffer) = 0;
        virtual void write_buffer(gpu_buffer aBuffer, std::size_t aOffset, void const* aData, std::size_t aSize) = 0;
        virtual void copy_buffer(gpu_buffer aSource, gpu_buffer aDestination, std::size_t aSize) = 0;
        // vertex arrays (see native_vertex_array): vertex attributes and the index buffer apply to the bound vertex array
    public:
        virtual gpu_vertex_array create_vertex_array() = 0;
        virtual void destroy_vertex_array(gpu_vertex_array aVertexArray) = 0;
        virtual gpu_vertex_array bound_vertex_array() const = 0;
        virtual void bind_vertex_array(gpu_vertex_array aVertexArray) = 0;
        // the attribute of the program's vertex input variable of that name (ignored if it has none)
        virtual void set_vertex_attribute(i_shader_program const& aProgram, std::string const& aName, gpu_buffer aBuffer,
            std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset) = 0;
        virtual void set_vertex_attribute(std::uint32_t aLocation, gpu_buffer aBuffer,
            std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset) = 0;
        virtual void set_index_buffer(gpu_buffer aBuffer) = 0;
        // drawing (with the active shader program, the bound vertex array and the current state)
    public:
        virtual void draw_arrays(gpu_primitive aPrimitive, std::size_t aFirst, std::size_t aCount) = 0;
        virtual void draw_elements(gpu_primitive aPrimitive, std::size_t aFirstIndex, std::size_t aCount) = 0;
        // make what has been drawn so far visible to what is drawn next (glTextureBarrier)
        virtual void texture_barrier() = 0;
        // fixed function state (see native_rendering_context)
    public:
        virtual void enable_scissor(bool aEnable) = 0;
        virtual void set_scissor(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight) = 0;
        virtual void enable_multisample(bool aEnable) = 0;
        virtual void set_sample_shading(std::optional<double> const& aSampleShadingRate) = 0;
        virtual void set_front_face(neogfx::front_face aFrontFace) = 0;
        // n.b. aFlipped: the projection is flipped so the face culled is the opposite of the one requested
        virtual void set_face_culling(neogfx::face_culling aCulling, bool aFlipped) = 0;
        virtual void set_blending_mode(neogfx::blending_mode aBlendingMode) = 0;
        virtual void set_xor_blending() = 0;
        virtual void enable_smoothing(bool aEnable) = 0;
        // the clears are scissored, and clear() honours the colour mask
        virtual void clear(color const& aColor) = 0;
        virtual void clear_depth_buffer() = 0;
        virtual void clear_stencil_buffer(std::int32_t aValue) = 0;
        virtual void apply_stencil(bool aEnabled, bool aUpdating, std::int32_t aRef) = 0;
        virtual bool depth_test_enabled() const = 0;
        virtual void enable_depth_test(bool aEnable) = 0;
        // textures (see i_texture::bind)
    public:
        // the filtering of the texture just bound (its sampling, or a material's)
        virtual void set_texture_filter(i_texture const& aTexture, texture_sampling aSampling) = 0;
        // bilinear filtering, clamped to the edges (e.g. the PBR environment texture)
        virtual void set_texture_linear_clamp(i_texture const& aTexture) = 0;
        virtual void set_active_texture_unit(std::uint32_t aTextureUnit) = 0;
        virtual void unbind_texture_unit(std::uint32_t aTextureUnit) = 0;
        // scene meshes (see native_rendering_context::draw_scene_meshes)
    public:
        virtual bool scene_shadows_available(i_standard_shader_program& aProgram) = 0;
        // depth only, into the shadow map atlas (see scene_shadow_atlas), which is then bound to reserved_texture_unit::PbrShadow;
        // n.b. the state is unchanged
        virtual void draw_scene_shadow_maps(i_standard_shader_program& aProgram, native_scene_buffer& aSceneBuffer,
            std::vector<mat44> const& aViews, std::int32_t aDirectionalView,
            std::vector<std::optional<gpu_mesh_range>> const& aMeshes, std::vector<std::optional<scene_shadow_alpha_test>> const& aAlphaTests,
            std::uint32_t aModelTableBase) = 0;
        // the scene's background (see i_pbr_shader::set_background), from the background texture if there is one, otherwise
        // from the environment texture; n.b. the state is unchanged
        virtual void draw_scene_background(i_standard_shader_program& aProgram, mat44 const& aNdcToClip, mat44 const& aClipToWorld,
            i_texture const* aBackgroundTexture, i_texture const* aEnvironment, double aBlur, double aIntensity) = 0;
    };

    // the rendering engine's backend
    i_graphics_backend& graphics_backend();
    // the frame being rendered: counted when the non-cacheable vertex buffers are cleared (see
    // native_renderer::clear_non_cacheable_vertex_buffers); blocks reclaimed in a buffer are reused frames later (see native_buffer)
    std::uint64_t native_frame();
}
