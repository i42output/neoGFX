// vulkan_graphics_backend.hpp
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
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../i_graphics_backend.hpp"
#include "vulkan.hpp"

namespace neogfx
{
    // A Vulkan 1.3 implementation of the graphics API calls made by the shared native code (see i_graphics_backend). The
    // calls' OpenGL semantics are kept: state set is current until changed and the work is ordered as the calls are made.
    // To do so the work is recorded into one command buffer (dynamic rendering, begun when first needed and ended when
    // something that cannot be done while rendering is needed: a copy, a barrier, a change of target) which is submitted,
    // and waited for, when the host needs it done (see execute()) and when a surface is presented. Images are kept in the
    // general layout (only a swapchain's images change layout) and a memory barrier precedes each rendering and each copy.
    // n.b. so that images are row for row as they are with OpenGL (render to texture then sample, glyph render output
    // reads, read backs and the shadow map bias all depend on it) viewports and scissor rectangles are used as given
    // (no flip), front faces are swapped and clip space depth is mapped from [-w, w] to [0, w] in the vertex shader; the
    // flip to the window's orientation is done once, when the surface is copied to its swapchain image.

    struct vulkan_buffer
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0u;
        void* mapping = nullptr;
        bool deviceLocal = false;
        bool coherent = true;
    };

    struct vulkan_sampler_state
    {
        bool magLinear = true;
        bool minLinear = true;
        bool mipmap = false;
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

        auto operator<=>(vulkan_sampler_state const&) const = default;
    };

    struct vulkan_image
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        std::uint32_t width = 0u;
        std::uint32_t height = 0u;
        std::uint32_t mipLevels = 1u;
        std::uint32_t texelSize = 4u;
        vulkan_sampler_state sampler;
    };

    // the bytes of a uniform block (see basic_vulkan_shader_program); changed when its generation does
    struct vulkan_uniform_block
    {
        std::vector<std::uint8_t> data;
        std::uint64_t generation = 1u;
    };

    // a shader object (see i_rendering_engine::create_shader_object)
    struct vulkan_shader_object
    {
        shader_type type;
        std::vector<std::uint32_t> spirv;
        VkShaderModule module = VK_NULL_HANDLE;
    };

    struct vulkan_vertex_input_layout
    {
        struct attribute
        {
            std::uint32_t location;
            std::uint32_t binding;
            VkFormat format;
            std::uint32_t offset;
            auto operator<=>(attribute const&) const = default;
        };
        std::vector<attribute> attributes;
        std::vector<std::uint32_t> strides; // per binding
        auto operator<=>(vulkan_vertex_input_layout const&) const = default;
    };

    struct vulkan_pipeline_key
    {
        vulkan_vertex_input_layout vertexInput;
        VkFormat colorFormat = VK_FORMAT_UNDEFINED;
        VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        std::uint32_t blend = 0u;       // see vulkan_graphics_backend::blend_key
        bool colorWrite = true;
        float sampleShading = 0.0f;
        auto operator<=>(vulkan_pipeline_key const&) const = default;
    };

    // a shader program object (see i_rendering_engine::create_shader_program_object): what drawing with the program needs
    // (filled in by basic_vulkan_shader_program when it is linked)
    struct vulkan_program
    {
        struct uniform_block_binding
        {
            std::uint32_t binding;
            VkShaderStageFlags stages;
            vulkan_uniform_block const* block;
        };
        struct sampler_binding
        {
            std::uint32_t binding;
            VkShaderStageFlags stages;
            bool multisample;
            std::int32_t textureUnit;
        };
        struct storage_binding
        {
            std::uint32_t binding;
            VkShaderStageFlags stages;
            std::function<gpu_buffer()> buffer;
        };
        struct vertex_input
        {
            std::uint32_t location;
            shader_data_type type;
        };

        bool linked = false;
        std::array<VkShaderModule, static_cast<std::size_t>(shader_type::COUNT)> modules = {};
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        std::vector<uniform_block_binding> uniformBlocks;
        std::vector<sampler_binding> samplers;
        std::vector<storage_binding> storageBuffers;
        std::map<std::string, vertex_input> vertexInputs;
        std::map<vulkan_pipeline_key, VkPipeline> pipelines;
    };

    // a swapchain (see vulkan_surface)
    struct vulkan_swapchain
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkExtent2D extent = {};
        std::vector<VkImage> images;
        std::vector<VkSemaphore> renderingFinished;
        VkSemaphore imageAcquired = VK_NULL_HANDLE;
        std::unique_ptr<vulkan_image> resolved;
        std::uint64_t vsyncGeneration = 0u;
        bool outOfDate = false;
    };

    class vulkan_graphics_backend : public i_graphics_backend
    {
    public:
        struct failed_to_initialize : std::runtime_error { failed_to_initialize(std::string const& aReason) : std::runtime_error{ "neogfx::vulkan_graphics_backend::failed_to_initialize: " + aReason } {} };
        struct no_target : std::logic_error { no_target() : std::logic_error{ "neogfx::vulkan_graphics_backend::no_target" } {} };
        struct no_program : std::logic_error { no_program() : std::logic_error{ "neogfx::vulkan_graphics_backend::no_program" } {} };
        struct no_vertex_array : std::logic_error { no_vertex_array() : std::logic_error{ "neogfx::vulkan_graphics_backend::no_vertex_array" } {} };
    public:
        static constexpr std::uint32_t MaxTextureUnits = 32u;
        // descriptor bindings of the generated GLSL (see basic_vulkan_shader_program): a stage's uniform block binding is its
        // shader_type, an SSBO's is StorageBindingBase + its id and the samplers' start at SamplerBindingBase
        static constexpr std::uint32_t StorageBindingBase = 16u;
        static constexpr std::uint32_t SamplerBindingBase = 32u;
    public:
        vulkan_graphics_backend();
        ~vulkan_graphics_backend();
    public:
        // n.b. null once destroyed (objects outliving the rendering engine have nothing to give their Vulkan objects back to)
        static vulkan_graphics_backend* instance();
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
        // the Vulkan objects (see vulkan_texture, vulkan_surface and basic_vulkan_shader_program)
    public:
        bool alive() const;
        VkInstance vk_instance() const;
        VkPhysicalDevice physical_device() const;
        VkDevice device() const;
        VkFormat depth_stencil_format() const;
        VkSampleCountFlagBits sample_count(std::uint32_t aSamples) const;
        bool format_supports(VkFormat aFormat, VkFormatFeatureFlags aFeatures) const;
    public:
        std::unique_ptr<vulkan_image> create_image(std::uint32_t aWidth, std::uint32_t aHeight, VkFormat aFormat, std::uint32_t aTexelSize,
            VkSampleCountFlagBits aSamples = VK_SAMPLE_COUNT_1_BIT, std::uint32_t aMipLevels = 1u);
        std::unique_ptr<vulkan_image> create_depth_stencil_image(std::uint32_t aWidth, std::uint32_t aHeight, VkSampleCountFlagBits aSamples);
        void destroy_image(std::unique_ptr<vulkan_image>& aImage);
        // aRowLength: texels per row of the data (0 means aWidth); aRowPitch: bytes per row of the data
        void upload_image(vulkan_image& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight,
            void const* aData, std::size_t aRowPitch);
        void clear_image(vulkan_image& aImage, std::array<float, 4> const& aColor);
        void generate_mipmaps(vulkan_image& aImage);
        // n.b. waits for the work recorded so far
        void read_image(vulkan_image const& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight, void* aData);
    public:
        // the target drawn to (see vulkan_texture::activate_target and vulkan_surface::do_activate_target)
        void set_render_target(vulkan_image* aColor, vulkan_image* aDepthStencil);
        void release_render_target(vulkan_image const* aColor);
        void set_viewport(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight);
        void enable_blending(bool aEnable);
        void set_depth_compare(VkCompareOp aCompareOp);
    public:
        void bind_texture(std::uint32_t aTextureUnit, vulkan_image const* aImage);
        vulkan_image const* bound_texture(std::uint32_t aTextureUnit) const;
    public:
        void use_program(vulkan_program* aProgram);
        // GLSL (Vulkan) to SPIR-V; throws (i_rendering_engine::shader_program_error) on failure
        std::vector<std::uint32_t> compile_shader(shader_type aType, std::string const& aSource, std::string const& aName) const;
        VkShaderModule create_shader_module(std::vector<std::uint32_t> const& aSpirv);
        void destroy_shader_module(VkShaderModule aModule);
        // n.b. the program's pipelines and descriptor layouts (it is about to be (re)linked or destroyed)
        void release_program(vulkan_program& aProgram);
    public:
        bool vsync() const;
        void set_vsync(bool aEnable);
        void create_surface_swapchain(vulkan_swapchain& aSwapchain, std::uint32_t aWidth, std::uint32_t aHeight);
        void destroy_swapchain(vulkan_swapchain& aSwapchain, bool aDestroySurface);
        // submits the work recorded so far, the last part of which copies the (resolved) source to the swapchain image
        void present(vulkan_swapchain& aSwapchain, vulkan_image& aSource, std::uint32_t aWidth, std::uint32_t aHeight);
    private:
        struct vertex_array
        {
            struct attribute
            {
                gpu_buffer buffer;
                VkFormat format;
                std::uint32_t stride;
                std::uint32_t offset;
            };
            std::map<std::uint32_t, attribute> attributes;
            gpu_buffer indexBuffer = no_gpu_buffer;
        };
        struct transient_chunk
        {
            vulkan_buffer buffer;
            VkDeviceSize used = 0u;
        };
        struct transient_allocation
        {
            VkBuffer buffer;
            VkDeviceSize offset;
            void* mapping;
        };
        struct state
        {
            bool scissorEnabled = false;
            VkRect2D scissor = {};
            VkViewport viewport = {};
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
            VkCompareOp depthCompare = VK_COMPARE_OP_LESS;
            bool stencilTest = false;
            VkCompareOp stencilCompare = VK_COMPARE_OP_ALWAYS;
            VkStencilOp stencilPassOp = VK_STENCIL_OP_KEEP;
            std::uint32_t stencilReference = 0u;
            std::uint32_t stencilCompareMask = 0xFFu;
            std::uint32_t stencilWriteMask = 0xFFu;
        };
        struct shadow_resources;
        struct background_resources;
    private:
        void create_instance();
        void create_device();
        void create_defaults();
        std::uint32_t memory_type(std::uint32_t aTypeBits, VkMemoryPropertyFlags aRequired, VkMemoryPropertyFlags aPreferred = 0u) const;
        vulkan_buffer allocate_buffer(VkDeviceSize aSize, VkBufferUsageFlags aUsage, bool aMapped);
        void free_buffer(vulkan_buffer& aBuffer);
        transient_allocation allocate_transient(VkDeviceSize aSize, VkDeviceSize aAlignment);
        void defer(std::function<void()> aDestroy);
        VkCommandBuffer command_buffer();
        void submit(VkSemaphore aWait = VK_NULL_HANDLE, VkSemaphore aSignal = VK_NULL_HANDLE);
        void begin_rendering();
        void end_rendering();
        void memory_barrier();
        void image_barrier(VkImage aImage, VkImageAspectFlags aAspect, VkImageLayout aOldLayout, VkImageLayout aNewLayout);
        VkSampler sampler(vulkan_sampler_state const& aState, std::uint32_t aMipLevels);
        std::uint32_t blend_key() const;
        VkPipeline pipeline(vulkan_program& aProgram, vulkan_vertex_input_layout const& aVertexInput);
        void prepare_draw();
        void apply_dynamic_state(bool aForce);
        void apply_viewport_and_scissor();
        void push_descriptors(vulkan_program& aProgram);
        vulkan_image const& texture_unit_image(std::uint32_t aTextureUnit, bool aMultisample) const;
        VkExtent2D target_extent() const;
        shadow_resources& shadows(i_standard_shader_program& aProgram);
        background_resources& background();
        void invalidate_bindings();
    private:
        VkInstance iInstance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT iDebugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice iPhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties iProperties = {};
        VkPhysicalDeviceMemoryProperties iMemoryProperties = {};
        VkDevice iDevice = VK_NULL_HANDLE;
        std::uint32_t iQueueFamily = 0u;
        VkQueue iQueue = VK_NULL_HANDLE;
        PFN_vkCmdPushDescriptorSetKHR iCmdPushDescriptorSet = nullptr;
        bool iSampleRateShading = false;
        VkFormat iDepthStencilFormat = VK_FORMAT_UNDEFINED;
        VkCommandPool iCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer iCommandBuffer = VK_NULL_HANDLE;
        VkFence iFence = VK_NULL_HANDLE;
        bool iRecording = false;
        std::vector<std::function<void()>> iDeferred;
        std::vector<transient_chunk> iTransient;
        std::size_t iTransientChunk = 0u;
        std::map<std::pair<vulkan_sampler_state, std::uint32_t>, VkSampler> iSamplers;
        std::unique_ptr<vulkan_image> iDummyTexture;
        std::unique_ptr<vulkan_image> iDummyTextureMS;
        vulkan_buffer iZeroBuffer;
        // the target and whether it is being rendered to
        vulkan_image* iColor = nullptr;
        vulkan_image* iDepthStencil = nullptr;
        bool iRendering = false;
        // state
        state iState;
        bool iDynamicStateValid = false;
        VkPipeline iBoundPipeline = VK_NULL_HANDLE;
        vulkan_program* iProgram = nullptr;
        vertex_array* iVertexArray = nullptr;
        std::array<vulkan_image const*, MaxTextureUnits> iTextureUnits = {};
        // n.b. a uniform block's bytes are uploaded once for all the draws they are unchanged for
        std::unordered_map<vulkan_uniform_block const*, std::pair<std::uint64_t, transient_allocation>> iUploadedUniformBlocks;
        bool iVsync = true;
        std::uint64_t iVsyncGeneration = 1u;
        std::unique_ptr<shadow_resources> iShadows;
        std::unique_ptr<background_resources> iBackground;
    };
}
