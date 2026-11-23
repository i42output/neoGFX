// d3d12_graphics_backend.hpp
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
#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../i_graphics_backend.hpp"
#include "d3d12.hpp"

namespace neogfx
{
    // A Direct3D 12 implementation of the graphics API calls made by the shared native code (see i_graphics_backend), modelled
    // on vulkan_graphics_backend: the calls' OpenGL semantics are kept (state set is current until changed and the work is
    // ordered as the calls are made) by recording the work into a frame's command list, which is submitted when a surface is
    // presented and waited for only when the host needs it done (see execute()); the next frame is recorded meanwhile (see
    // FramesInFlight) and a mapped buffer whose contents are discarded while the GPU may still be reading them gets new storage
    // (see discard_buffer). Each texture's state is tracked and a transition barrier recorded when it is used differently; a
    // device local buffer is transitioned from the common state (to which buffers decay after each command list) when first
    // used by a command list.
    // n.b. as with Vulkan, so that images are row for row as they are with OpenGL, viewports and scissor rectangles are used as
    // given, clip space y is negated and depth mapped from [-w, w] to [0, w] in the vertex shader and front faces are swapped;
    // the flip to the window's orientation is done once, when the surface is drawn to its swapchain's back buffer.

    // a descriptor in one of the backend's CPU (not shader visible) descriptor heaps (see d3d12_descriptor_allocator)
    struct d3d12_descriptor
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu = {};
        std::uint32_t heap = ~0u;
        std::uint32_t index = 0u;

        bool valid() const { return heap != ~0u; }
    };

    class d3d12_descriptor_allocator
    {
    public:
        void initialize(ID3D12Device* aDevice, D3D12_DESCRIPTOR_HEAP_TYPE aType, std::uint32_t aHeapSize = 1024u);
        void clear();
        d3d12_descriptor allocate();
        void free(d3d12_descriptor& aDescriptor);
    private:
        ID3D12Device* iDevice = nullptr;
        D3D12_DESCRIPTOR_HEAP_TYPE iType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        std::uint32_t iHeapSize = 0u;
        std::uint32_t iIncrement = 0u;
        std::vector<com_ptr<ID3D12DescriptorHeap>> iHeaps;
        std::uint32_t iUsed = 0u; // of the last heap
        std::vector<d3d12_descriptor> iFree;
    };

    struct d3d12_buffer
    {
        // storage replaced by discard_buffer, reused once the GPU is done with it (the submission retired is complete)
        struct spare
        {
            com_ptr<ID3D12Resource> resource;
            void* mapping = nullptr;
            std::uint64_t retired = 0u;
        };

        com_ptr<ID3D12Resource> resource;
        std::uint64_t size = 0u;
        // a mapped buffer is in an upload heap, persistently mapped (and in the generic read state); else in the default heap
        void* mapping = nullptr;
        bool deviceLocal = false;
        // a device local buffer's state in the command list numbered stateRecording (buffers decay to the common state when the
        // GPU completes a command list, so in any other command list the state is the common state)
        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
        std::uint64_t stateRecording = 0u;
        std::vector<spare> spares;
    };

    struct d3d12_sampler_state
    {
        bool magLinear = true;
        bool minLinear = true;
        bool mipmap = false;
        D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

        auto operator<=>(d3d12_sampler_state const&) const = default;
    };

    struct d3d12_image
    {
        com_ptr<ID3D12Resource> resource;
        // the format of the resource's render target or depth stencil view, and of its shader resource view (they differ for the
        // shadow map atlas, whose resource is typeless)
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN;
        bool depthStencil = false;
        std::uint32_t samples = 1u;
        std::uint32_t width = 0u;
        std::uint32_t height = 0u;
        std::uint32_t mipLevels = 1u;
        std::uint32_t texelSize = 4u;
        // all subresources are in this state (outside of generate_mipmaps)
        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
        d3d12_descriptor srv;
        d3d12_descriptor rtv;
        d3d12_descriptor dsv;
        d3d12_sampler_state sampler;
    };

    // the bytes of a uniform block (see basic_d3d12_shader_program); changed when its generation does
    struct d3d12_uniform_block
    {
        std::vector<std::uint8_t> data;
        std::uint64_t generation = 1u;
    };

    using d3d12_bytecode = std::shared_ptr<std::vector<std::uint8_t> const>;

    // a shader object (see i_rendering_engine::create_shader_object): its Direct3D 12 GLSL (see d3d12_glsl), compiled to SPIR-V
    // (unoptimized: for reflection and for translation to HLSL) and the DXBC of the variants compiled so far (see
    // d3d12_graphics_backend::shader_bytecode)
    struct d3d12_shader_object
    {
        shader_type type;
        std::string name;
        std::string glsl;
        std::vector<std::uint32_t> spirv;
        bool usesNumSamples = false;
        // keyed by sample count (if gl_NumSamples is used, else 0) and the scaled vertex inputs (see d3d12_glsl_scaled_inputs)
        std::map<std::pair<std::uint32_t, std::uint32_t>, d3d12_bytecode> bytecode;
    };

    struct d3d12_input_layout
    {
        struct element
        {
            std::uint32_t location;
            std::uint32_t slot;
            DXGI_FORMAT format;
            std::uint32_t offset;
            auto operator<=>(element const&) const = default;
        };
        std::vector<element> elements;
        // the locations whose attributes are unsigned integers to be converted to floating point (see d3d12_glsl_scaled_inputs)
        std::uint32_t scaledLocations = 0u;
        auto operator<=>(d3d12_input_layout const&) const = default;
    };

    struct d3d12_pipeline_key
    {
        d3d12_input_layout inputLayout;
        DXGI_FORMAT colorFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_UNKNOWN;
        std::uint32_t samples = 1u;
        std::uint32_t blend = 0u;       // see d3d12_graphics_backend::blend_key
        bool colorWrite = true;
        bool depthTest = false;
        bool depthWrite = true;
        D3D12_COMPARISON_FUNC depthCompare = D3D12_COMPARISON_FUNC_LESS;
        bool stencilTest = false;
        D3D12_COMPARISON_FUNC stencilCompare = D3D12_COMPARISON_FUNC_ALWAYS;
        D3D12_STENCIL_OP stencilPassOp = D3D12_STENCIL_OP_KEEP;
        std::uint8_t stencilReadMask = 0xFFu;
        std::uint8_t stencilWriteMask = 0xFFu;
        D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE;
        bool frontCounterClockwise = false;
        auto operator<=>(d3d12_pipeline_key const&) const = default;
    };

    // a shader program object (see i_rendering_engine::create_shader_program_object): what drawing with the program needs
    // (filled in by basic_d3d12_shader_program when it is linked); its root signature's parameters are, in order, a root CBV
    // for each uniform block, a root SRV for each SSBO and, if it has samplers, a descriptor table of their textures (SRVs) and
    // one of their samplers (each texture and sampler at the slot binding - SamplerBindingBase)
    struct d3d12_program
    {
        struct uniform_block_binding
        {
            std::uint32_t binding;
            d3d12_uniform_block const* block;
        };
        struct sampler_binding
        {
            std::uint32_t binding;
            bool multisample;
            std::int32_t textureUnit;
        };
        struct storage_binding
        {
            std::uint32_t binding;
            std::function<gpu_buffer()> buffer;
        };
        struct vertex_input
        {
            std::uint32_t location;
            shader_data_type type;
        };

        bool linked = false;
        std::array<d3d12_shader_object*, static_cast<std::size_t>(shader_type::COUNT)> shaders = {};
        std::vector<uniform_block_binding> uniformBlocks;
        std::vector<sampler_binding> samplers;
        std::uint32_t samplerSlots = 0u;
        std::vector<storage_binding> storageBuffers;
        std::map<std::string, vertex_input> vertexInputs;
        com_ptr<ID3D12RootSignature> rootSignature;
        std::map<d3d12_pipeline_key, com_ptr<ID3D12PipelineState>> pipelines;
    };

    // a window's swapchain (see d3d12_surface)
    struct d3d12_swapchain
    {
        HWND window = nullptr;
        com_ptr<IDXGISwapChain3> swapchain;
        std::vector<com_ptr<ID3D12Resource>> buffers;
        std::vector<d3d12_descriptor> rtvs;
        DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
        std::uint32_t width = 0u;
        std::uint32_t height = 0u;
        bool tearing = false;
        std::unique_ptr<d3d12_image> resolved;
    };

    class d3d12_graphics_backend : public i_graphics_backend
    {
    public:
        struct failed_to_initialize : std::runtime_error { failed_to_initialize(std::string const& aReason) : std::runtime_error{ "neogfx::d3d12_graphics_backend::failed_to_initialize: " + aReason } {} };
        struct no_target : std::logic_error { no_target() : std::logic_error{ "neogfx::d3d12_graphics_backend::no_target" } {} };
        struct no_program : std::logic_error { no_program() : std::logic_error{ "neogfx::d3d12_graphics_backend::no_program" } {} };
        struct no_vertex_array : std::logic_error { no_vertex_array() : std::logic_error{ "neogfx::d3d12_graphics_backend::no_vertex_array" } {} };
    public:
        // as vulkan_graphics_backend: frames recorded while the GPU executes earlier ones
        static constexpr std::uint32_t FramesInFlight = 2u;
        static constexpr std::uint32_t MaxTextureUnits = 32u;
        // the registers of the generated GLSL's resources (see basic_d3d12_shader_program and vulkan_glsl): a stage's uniform
        // block's is its shader_type (b), an SSBO's StorageBindingBase + its id (t) and the samplers' start at SamplerBindingBase
        // (t and s)
        static constexpr std::uint32_t StorageBindingBase = 16u;
        static constexpr std::uint32_t SamplerBindingBase = 32u;
    public:
        d3d12_graphics_backend();
        ~d3d12_graphics_backend();
    public:
        // n.b. null once destroyed (objects outliving the rendering engine have nothing to give their Direct3D objects back to)
        static d3d12_graphics_backend* instance();
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
        // n.b. a texture can't be a render target and a shader resource at the same time (so subpixel glyphs, which are blended
        // with the render target by the glyph shader, are not rendered; see native_rendering_context::subpixel_format)
        bool has_texture_barrier() const final { return false; }
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
        // the Direct3D objects (see d3d12_texture, d3d12_surface and basic_d3d12_shader_program)
    public:
        bool alive() const;
        ID3D12Device* device() const;
        std::uint32_t sample_count(std::uint32_t aSamples) const;
    public:
        std::unique_ptr<d3d12_image> create_image(std::uint32_t aWidth, std::uint32_t aHeight, DXGI_FORMAT aFormat, std::uint32_t aTexelSize,
            std::uint32_t aSamples = 1u, std::uint32_t aMipLevels = 1u);
        std::unique_ptr<d3d12_image> create_depth_stencil_image(std::uint32_t aWidth, std::uint32_t aHeight, std::uint32_t aSamples);
        void destroy_image(std::unique_ptr<d3d12_image>& aImage);
        // aRowPitch: bytes per row of the data
        void upload_image(d3d12_image& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight,
            void const* aData, std::size_t aRowPitch);
        void clear_image(d3d12_image& aImage, std::array<float, 4> const& aColor);
        void generate_mipmaps(d3d12_image& aImage);
        // n.b. waits for the work recorded so far
        void read_image(d3d12_image& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight, void* aData);
    public:
        // the target drawn to (see d3d12_texture::activate_target and d3d12_surface::do_activate_target)
        void set_render_target(d3d12_image* aColor, d3d12_image* aDepthStencil);
        void release_render_target(d3d12_image const* aColor);
        void set_viewport(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight);
        void enable_blending(bool aEnable);
        void set_depth_compare(D3D12_COMPARISON_FUNC aCompare);
    public:
        void bind_texture(std::uint32_t aTextureUnit, d3d12_image* aImage);
        d3d12_image* bound_texture(std::uint32_t aTextureUnit) const;
    public:
        void use_program(d3d12_program* aProgram);
        // GLSL (Vulkan/Direct3D 12) to SPIR-V (unoptimized); throws (i_rendering_engine::shader_program_error) on failure
        std::vector<std::uint32_t> compile_spirv(shader_type aType, std::string const& aSource, std::string const& aName) const;
        // the DXBC of a shader object's variant (compiled when first needed); throws on failure
        d3d12_bytecode shader_bytecode(d3d12_shader_object& aShader, std::uint32_t aSamples, std::uint32_t aScaledLocations);
        // the program's root signature (see d3d12_program), created when it is linked
        void create_root_signature(d3d12_program& aProgram);
        // n.b. the program's pipelines and root signature (it is about to be (re)linked or destroyed)
        void release_program(d3d12_program& aProgram);
    public:
        bool vsync() const;
        void set_vsync(bool aEnable);
        void destroy_swapchain(d3d12_swapchain& aSwapchain);
        // submits the work recorded so far, the last part of which draws the (resolved) source to the swapchain's back buffer, and
        // presents it
        void present(d3d12_swapchain& aSwapchain, d3d12_image& aSource, std::uint32_t aWidth, std::uint32_t aHeight);
    private:
        struct vertex_array
        {
            struct attribute
            {
                gpu_buffer buffer;
                DXGI_FORMAT format;
                bool scaled;
                std::uint32_t stride;
                std::uint32_t offset;
            };
            std::map<std::uint32_t, attribute> attributes;
            gpu_buffer indexBuffer = no_gpu_buffer;
        };
        struct transient_chunk
        {
            d3d12_buffer buffer;
            std::uint64_t used = 0u;
        };
        struct transient_allocation
        {
            ID3D12Resource* resource;
            std::uint64_t offset;
            void* mapping;
            D3D12_GPU_VIRTUAL_ADDRESS address;
        };
        struct visible_heap
        {
            com_ptr<ID3D12DescriptorHeap> heap;
            std::uint32_t used = 0u;
        };
        struct descriptor_range
        {
            D3D12_CPU_DESCRIPTOR_HANDLE cpu;
            D3D12_GPU_DESCRIPTOR_HANDLE gpu;
        };
        struct state
        {
            bool scissorEnabled = false;
            std::array<std::int32_t, 4> scissor = {};   // x, y, width, height
            std::array<std::int32_t, 4> viewport = {};  // x, y, width, height
            bool multisample = true;
            std::optional<double> sampleShading;
            neogfx::front_face frontFace = neogfx::front_face::CounterClockwise;
            neogfx::face_culling faceCulling = neogfx::face_culling::None;
            bool faceCullingFlipped = false;
            bool blending = false;
            neogfx::blending_mode blendingMode = neogfx::blending_mode::Blit;
            bool xorBlending = false;
            bool colorWrite = true;
            bool depthTest = false;
            bool depthWrite = true;
            D3D12_COMPARISON_FUNC depthCompare = D3D12_COMPARISON_FUNC_LESS;
            bool stencilTest = false;
            D3D12_COMPARISON_FUNC stencilCompare = D3D12_COMPARISON_FUNC_ALWAYS;
            D3D12_STENCIL_OP stencilPassOp = D3D12_STENCIL_OP_KEEP;
            std::uint32_t stencilReference = 0u;
            std::uint8_t stencilCompareMask = 0xFFu;
            std::uint8_t stencilWriteMask = 0xFFu;
        };
        struct shadow_resources;
        struct background_resources;
        struct blit_resources;
    private:
        void load_libraries();
        void create_device();
        void create_defaults();
        // the on-disk cache's folder: the "dx_cache" sub-folder of the application's settings folder (std::nullopt if not available)
        std::optional<std::filesystem::path> const& cache_folder() const;
        // HLSL to DXBC (cached on disk); aNumSamples: SPIRV-Cross's specialization constant 0 (gl_NumSamples), if not 0
        d3d12_bytecode compile_hlsl(shader_type aType, std::string const& aHlsl, std::string const& aName, std::uint32_t aNumSamples = 0u,
            char const* aEntryPoint = "main") const;
        com_ptr<ID3D12RootSignature> create_root_signature(D3D12_ROOT_SIGNATURE_DESC const& aDesc);
        d3d12_buffer allocate_buffer(std::uint64_t aSize, bool aMapped);
        transient_allocation allocate_transient(std::uint64_t aSize, std::uint64_t aAlignment);
        descriptor_range allocate_descriptors(std::uint32_t aCount);
        void defer(std::function<void()> aDestroy);
        ID3D12GraphicsCommandList* command_list();
        void submit();
        void wait_for(std::uint64_t aSerial);
        void wait_for_frames();
        void poll_frames();
        void retire_frame(std::uint32_t aFrame);
        void transition(d3d12_image& aImage, D3D12_RESOURCE_STATES aState);
        void transition(d3d12_buffer& aBuffer, D3D12_RESOURCE_STATES aState);
        void transition(ID3D12Resource* aResource, D3D12_RESOURCE_STATES aBefore, D3D12_RESOURCE_STATES aAfter, std::uint32_t aSubresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
        void flush_barriers();
        void bind_descriptor_heaps();
        void bind_render_target();
        // the sampler and its (CPU) descriptor (created when first needed)
        std::uint32_t sampler(d3d12_sampler_state const& aState, std::uint32_t aMipLevels);
        // a table of samplers (by sampler()) in the shader visible sampler heap
        D3D12_GPU_DESCRIPTOR_HANDLE sampler_table(std::vector<std::uint32_t> const& aSamplers);
        // a table of the images' shader resource views in the shader visible descriptor heap
        D3D12_GPU_DESCRIPTOR_HANDLE texture_table(std::vector<d3d12_image*> const& aImages);
        std::uint32_t blend_key() const;
        D3D12_BLEND_DESC blend_desc(std::uint32_t aBlend, bool aColorWrite) const;
        ID3D12PipelineState* pipeline(d3d12_program& aProgram, d3d12_input_layout const& aInputLayout);
        // everything for a draw but the draw; false if nothing is to be drawn
        bool prepare_draw(gpu_primitive aPrimitive, bool aIndexed);
        void apply_viewport_and_scissor();
        d3d12_image& texture_unit_image(std::uint32_t aTextureUnit, bool aMultisample);
        std::array<std::uint32_t, 2> target_extent() const;
        shadow_resources& shadows(i_standard_shader_program& aProgram);
        background_resources& background();
        blit_resources& blits();
        void invalidate_bindings();
        void report_statistics();
    private:
        // per-frame statistics, reported once a second (to std::cerr) when NEOGFX_D3D12_STATS is set
        struct statistics
        {
            bool enabled = false;
            std::chrono::steady_clock::time_point periodStart;
            std::uint64_t frames = 0u;
            std::uint64_t submits = 0u;
            std::uint64_t executes = 0u;
            std::uint64_t readbacks = 0u;
            std::chrono::steady_clock::duration fenceWait = {};
            std::chrono::steady_clock::duration present = {};
            std::uint64_t barriers = 0u;
            std::uint64_t draws = 0u;
            std::uint64_t pipelineBinds = 0u;
            std::uint64_t pipelinesCreated = 0u;
            std::uint64_t uploads = 0u;
            std::uint64_t uploadBytes = 0u;
            std::uint64_t transientBytes = 0u;
            std::uint64_t descriptors = 0u;
            std::uint64_t discards = 0u;
            std::uint64_t swapchainsCreated = 0u;
        };
        HMODULE iD3d12 = nullptr;
        HMODULE iDxgi = nullptr;
        HMODULE iD3dCompiler = nullptr;
        pD3DCompile iD3DCompile = nullptr;
        com_ptr<IDXGIFactory4> iFactory;
        com_ptr<IDXGIAdapter1> iAdapter;
        com_ptr<ID3D12Device> iDevice;
        com_ptr<ID3D12CommandQueue> iQueue;
        com_ptr<ID3D12Fence> iFence;
        HANDLE iFenceEvent = nullptr;
        bool iTearing = false;
        mutable std::optional<std::filesystem::path> iCacheFolder;
        d3d12_descriptor_allocator iRtvDescriptors;
        d3d12_descriptor_allocator iDsvDescriptors;
        d3d12_descriptor_allocator iSrvDescriptors;
        d3d12_descriptor_allocator iSamplerDescriptors;
        std::uint32_t iSrvIncrement = 0u;
        std::uint32_t iSamplerIncrement = 0u;
        std::uint32_t iRtvIncrement = 0u;
        // the samplers (see sampler()) and the shader visible sampler heap: tables of them, each created once (the heap is reset,
        // after waiting for the GPU, if it fills)
        std::map<std::pair<d3d12_sampler_state, bool>, std::uint32_t> iSamplerIds;
        std::vector<d3d12_descriptor> iSamplers;
        com_ptr<ID3D12DescriptorHeap> iSamplerHeap;
        std::uint32_t iSamplerHeapUsed = 0u;
        std::uint64_t iSamplerHeapGeneration = 0u;
        std::map<std::vector<std::uint32_t>, std::uint32_t> iSamplerTables;
        // render target views of mip levels (see generate_mipmaps), reused (they are read when recorded)
        std::vector<d3d12_descriptor> iScratchRtvs;
        std::uint32_t iNextScratchRtv = 0u;
        struct frame
        {
            com_ptr<ID3D12CommandAllocator> allocator;
            bool inFlight = false;
            std::uint64_t serial = 0u;
            std::vector<std::function<void()>> deferred;
            std::vector<transient_chunk> transient;
            std::size_t transientChunk = 0u;
            std::vector<visible_heap> descriptorHeaps;
            std::size_t descriptorHeap = 0u;
        };
        std::array<frame, FramesInFlight> iFrames;
        std::uint32_t iFrame = 0u;
        // submissions: the last submitted and the last the GPU is known to have completed (fence values)
        std::uint64_t iSubmitted = 0u;
        std::uint64_t iCompleted = 0u;
        com_ptr<ID3D12GraphicsCommandList> iCommandList;
        bool iRecording = false;
        // the command lists recorded (see d3d12_buffer::stateRecording)
        std::uint64_t iRecordings = 0u;
        std::vector<D3D12_RESOURCE_BARRIER> iBarriers;
        std::unique_ptr<d3d12_image> iDummyTexture;
        std::unique_ptr<d3d12_image> iDummyTextureMS;
        d3d12_buffer iZeroBuffer;
        // the target
        d3d12_image* iColor = nullptr;
        d3d12_image* iDepthStencil = nullptr;
        // the command list's bindings (invalidated when it is begun; see invalidate_bindings)
        bool iTargetBound = false;
        bool iHeapsBound = false;
        bool iViewportValid = false;
        ID3D12RootSignature* iBoundRootSignature = nullptr;
        ID3D12PipelineState* iBoundPipeline = nullptr;
        std::optional<std::uint32_t> iBoundStencilReference;
        // state
        state iState;
        d3d12_program* iProgram = nullptr;
        vertex_array* iVertexArray = nullptr;
        std::array<d3d12_image*, MaxTextureUnits> iTextureUnits = {};
        // n.b. a uniform block's bytes are uploaded once for all the draws they are unchanged for (in this frame's transient memory)
        std::unordered_map<d3d12_uniform_block const*, std::pair<std::uint64_t, D3D12_GPU_VIRTUAL_ADDRESS>> iUploadedUniformBlocks;
        bool iVsync = true;
        std::unique_ptr<shadow_resources> iShadows;
        std::unique_ptr<background_resources> iBackground;
        std::unique_ptr<blit_resources> iBlits;
        statistics iStatistics;
    };
}
