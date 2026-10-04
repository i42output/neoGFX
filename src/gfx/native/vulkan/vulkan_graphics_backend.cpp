// vulkan_graphics_backend.cpp
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

#include <neogfx/neogfx.hpp>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <shaderc/shaderc.hpp>

#include <neogfx/gfx/i_texture.hpp>
#include "../i_native_texture.hpp"
#include "../native_vertex.hpp"
#include "vulkan_texture_manager.hpp"
#include "vulkan_shader_program.hpp"
#include "vulkan_graphics_backend.hpp"

namespace neogfx
{
    namespace
    {
        vulkan_graphics_backend* sInstance;

        constexpr VkDeviceSize TransientChunkSize = 8u * 1024u * 1024u;

        VkShaderStageFlagBits to_vk_stage(shader_type aType)
        {
            switch (aType)
            {
            case shader_type::Compute:
                return VK_SHADER_STAGE_COMPUTE_BIT;
            case shader_type::Vertex:
            default:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case shader_type::TessellationControl:
                return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case shader_type::TessellationEvaluation:
                return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case shader_type::Geometry:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            case shader_type::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            }
        }

        VkPrimitiveTopology to_vk_topology(gpu_primitive aPrimitive)
        {
            switch (aPrimitive)
            {
            case gpu_primitive::Triangles:
            default:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case gpu_primitive::TriangleStrip:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            }
        }

        VkFormat to_vk_attribute_format(std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized)
        {
            static constexpr VkFormat sFloat[] = { VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT };
            static constexpr VkFormat sDouble[] = { VK_FORMAT_R64_SFLOAT, VK_FORMAT_R64G64_SFLOAT, VK_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_R64G64B64A64_SFLOAT };
            static constexpr VkFormat sUnorm8[] = { VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM };
            static constexpr VkFormat sUscaled8[] = { VK_FORMAT_R8_USCALED, VK_FORMAT_R8G8_USCALED, VK_FORMAT_R8G8B8_USCALED, VK_FORMAT_R8G8B8A8_USCALED };
            static constexpr VkFormat sUnorm16[] = { VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM };
            static constexpr VkFormat sUscaled16[] = { VK_FORMAT_R16_USCALED, VK_FORMAT_R16G16_USCALED, VK_FORMAT_R16G16B16_USCALED, VK_FORMAT_R16G16B16A16_USCALED };
            auto const index = std::clamp<std::uint32_t>(aArity, 1u, 4u) - 1u;
            switch (aType)
            {
            case gpu_attribute_type::Float:
            default:
                return sFloat[index];
            case gpu_attribute_type::Double:
                return sDouble[index];
            case gpu_attribute_type::UnsignedByte:
                // n.b. as OpenGL, an integer attribute not normalized is converted to floating point
                return aNormalized ? sUnorm8[index] : sUscaled8[index];
            case gpu_attribute_type::UnsignedShort:
                return aNormalized ? sUnorm16[index] : sUscaled16[index];
            }
        }

        // the format of the zero attribute a vertex input variable without an attribute reads (OpenGL: a disabled attribute)
        VkFormat zero_attribute_format(shader_data_type aType)
        {
            switch (aType)
            {
            case shader_data_type::Int:
            case shader_data_type::IVec2:
            case shader_data_type::IVec3:
            case shader_data_type::IVec4:
                return VK_FORMAT_R32G32B32A32_SINT;
            case shader_data_type::Uint:
            case shader_data_type::UVec2:
            case shader_data_type::UVec3:
            case shader_data_type::UVec4:
                return VK_FORMAT_R32G32B32A32_UINT;
            default:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            }
        }

        bool has_stencil(VkFormat aFormat)
        {
            return aFormat == VK_FORMAT_D24_UNORM_S8_UINT || aFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || aFormat == VK_FORMAT_D16_UNORM_S8_UINT;
        }

        vulkan_image* image_of(i_texture const& aTexture)
        {
            return reinterpret_cast<vulkan_image*>(aTexture.native_texture().native_handle());
        }

        vulkan_buffer* buffer_of(gpu_buffer aBuffer)
        {
            return reinterpret_cast<vulkan_buffer*>(aBuffer);
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT aSeverity, VkDebugUtilsMessageTypeFlagsEXT,
            VkDebugUtilsMessengerCallbackDataEXT const* aData, void*)
        {
            if (aSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                service<debug::logger>() << neolib::logger::severity::Debug << "Vulkan: " << aData->pMessage << std::endl;
            return VK_FALSE;
        }

        // GLSL (Vulkan) for the shadow maps (see opengl_graphics_backend: scene_shadows) and the scene background
        // (scene_background); n.b. the same, except: uniforms are push constants or a uniform block, the rectangle
        // samplers are 2D samplers and clip space depth is mapped to [0, w]
        std::string const sShadowVertexShader =
            "#version 460 core\n"
            "layout (location = 0) in vec3 VertexPosition;\n"
            "layout (location = 1) in vec4 VertexColor;\n"
            "layout (location = 2) in vec2 VertexTextureCoord;\n"
            "layout (location = 11) in float VertexModel;\n"
            "layout (location = 12) in vec4 VertexJoints;\n"
            "layout (location = 13) in vec4 VertexWeights;\n"
            "layout(std430, set = 0, binding = %MATRICES%) buffer SSBO_bModelMatrices { mat4 bModelMatrices[]; };\n"
            "layout(std430, set = 0, binding = %TABLE%) buffer SSBO_bModelTable { uint bModelTable[]; };\n"
            "layout(push_constant) uniform ShadowParameters\n"
            "{\n"
            "    mat4 uViewProjection;\n"
            "    vec4 uTextureTransform;\n"
            "    ivec2 uTextureWrap;\n"
            "    uint uModelTableBase;\n"
            "    int uAlphaTest;\n"
            "    float uAlphaCutoff;\n"
            "};\n"
            "layout (location = 0) out vec2 TexCoord;\n"
            "layout (location = 1) out float Alpha;\n"
            "void main()\n"
            "{\n"
            "    TexCoord = VertexTextureCoord;\n"
            "    Alpha = VertexColor.a;\n"
            "    vec4 position = vec4(VertexPosition, 1.0);\n"
            "    if (VertexModel > 0.0)\n"
            "    {\n"
            "        uint first = bModelTable[uModelTableBase + uint(VertexModel + 0.5)];\n"
            "        if (any(greaterThan(VertexWeights, vec4(0.0))))\n"
            "        {\n"
            "            vec4 skinned = vec4(0.0);\n"
            "            for (int i = 0; i < 4; ++i)\n"
            "                if (VertexWeights[i] > 0.0)\n"
            "                    skinned += VertexWeights[i] * (bModelMatrices[first + 1u + uint(VertexJoints[i] + 0.5)] * position);\n"
            "            position = skinned;\n"
            "        }\n"
            "        position = bModelMatrices[first] * position;\n"
            "    }\n"
            "    gl_Position = uViewProjection * vec4(position.xyz / position.w, 1.0);\n"
            "    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
            "}\n";

        std::string const sShadowFragmentShader =
            "#version 460 core\n"
            "layout (location = 0) in vec2 TexCoord;\n"
            "layout (location = 1) in float Alpha;\n"
            "layout (set = 0, binding = 0) uniform sampler2D uBaseTexture;\n"
            "layout(push_constant) uniform ShadowParameters\n"
            "{\n"
            "    mat4 uViewProjection;\n"
            "    vec4 uTextureTransform;\n"
            "    ivec2 uTextureWrap;\n"
            "    uint uModelTableBase;\n"
            "    int uAlphaTest;\n"
            "    float uAlphaCutoff;\n"
            "};\n"
            "float wrap(float c, int mode)\n"
            "{\n"
            "    if (mode == 1)\n"
            "        return fract(c);\n"
            "    if (mode == 2)\n"
            "    {\n"
            "        float m = mod(c, 2.0);\n"
            "        return m > 1.0 ? 2.0 - m : m;\n"
            "    }\n"
            "    return clamp(c, 0.0, 1.0);\n"
            "}\n"
            "void main()\n"
            "{\n"
            "    if (uAlphaTest != 0)\n"
            "    {\n"
            "        vec2 coord = vec2(wrap(TexCoord.x, uTextureWrap.x), wrap(TexCoord.y, uTextureWrap.y)) * uTextureTransform.xy + uTextureTransform.zw;\n"
            "        vec2 unwrapped = TexCoord * uTextureTransform.xy;\n"
            "        if (Alpha * textureGrad(uBaseTexture, coord, dFdx(unwrapped), dFdy(unwrapped)).a < uAlphaCutoff)\n"
            "            discard;\n"
            "    }\n"
            "}\n";

        struct shadow_parameters
        {
            float viewProjection[16];
            float textureTransform[4];
            std::int32_t textureWrap[2];
            std::uint32_t modelTableBase;
            std::int32_t alphaTest;
            float alphaCutoff;
        };
        static_assert(sizeof(shadow_parameters) == 100u);

        std::string const sBackgroundVertexShader =
            "#version 460 core\n"
            "layout (set = 0, binding = 0, std140) uniform BackgroundParameters\n"
            "{\n"
            "    mat4 uNdcToClip;\n"
            "    mat4 uClipToWorld;\n"
            "    ivec2 uBackgroundExtents;\n"
            "    int uSource;\n"
            "    float uBlur;\n"
            "    float uIntensity;\n"
            "};\n"
            "layout (location = 0) out vec2 Ndc;\n"
            "void main()\n"
            "{\n"
            "    Ndc = vec2(float((gl_VertexIndex & 1) * 2 - 1), float((gl_VertexIndex >> 1) * 2 - 1));\n"
            "    vec4 position = uNdcToClip * vec4(Ndc, 0.0, 1.0);\n"
            "    gl_Position = vec4(position.xy, 0.0, position.w);\n"
            "    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
            "}\n";

        std::string const sBackgroundFragmentShader =
            "#version 460 core\n"
            "layout (location = 0) in vec2 Ndc;\n"
            "layout (location = 0) out vec4 FragColor;\n"
            "layout (set = 0, binding = 0, std140) uniform BackgroundParameters\n"
            "{\n"
            "    mat4 uNdcToClip;\n"
            "    mat4 uClipToWorld;\n"
            "    ivec2 uBackgroundExtents;\n"
            "    int uSource;\n"
            "    float uBlur;\n"
            "    float uIntensity;\n"
            "};\n"
            "layout (set = 0, binding = 1) uniform sampler2D uEnvironment;\n"
            "layout (set = 0, binding = 2) uniform sampler2D uBackground;\n"
            "const float PI = 3.14159265358979;\n"
            "vec3 environment_texel(int band, ivec2 t)\n"
            "{\n"
            "    return texelFetch(uEnvironment, ivec2(((t.x % 256) + 256) % 256, band * 128 + clamp(t.y, 0, 127)), 0).rgb;\n"
            "}\n"
            "vec3 environment_band(int band, vec2 uv)\n"
            "{\n"
            "    vec2 p = uv * vec2(256.0, 128.0) - 0.5;\n"
            "    ivec2 i = ivec2(floor(p));\n"
            "    vec2 f = p - vec2(i);\n"
            "    return mix(mix(environment_texel(band, i), environment_texel(band, i + ivec2(1, 0)), f.x),\n"
            "        mix(environment_texel(band, i + ivec2(0, 1)), environment_texel(band, i + ivec2(1, 1)), f.x), f.y);\n"
            "}\n"
            "int background_top_level()\n"
            "{\n"
            "    int level = 0;\n"
            "    for (ivec2 e = uBackgroundExtents; e.x > 8 && e.y > 4; e /= 2)\n"
            "        ++level;\n"
            "    return level;\n"
            "}\n"
            "ivec4 background_level(int level)\n"
            "{\n"
            "    if (level == 0)\n"
            "        return ivec4(0, 0, uBackgroundExtents);\n"
            "    int x = 0;\n"
            "    for (int i = 1; i < level; ++i)\n"
            "        x += uBackgroundExtents.x >> i;\n"
            "    return ivec4(x, uBackgroundExtents.y, uBackgroundExtents.x >> level, uBackgroundExtents.y >> level);\n"
            "}\n"
            "vec3 background_texel(ivec4 level, ivec2 t)\n"
            "{\n"
            "    vec4 rgbe = floor(texelFetch(uBackground, level.xy + ivec2(((t.x % level.z) + level.z) % level.z, clamp(t.y, 0, level.w - 1)), 0) * 255.0 + 0.5);\n"
            "    return rgbe.a > 0.0 ? (rgbe.rgb + 0.5) * exp2(rgbe.a - 136.0) : vec3(0.0);\n"
            "}\n"
            "vec3 background_bilinear(int level, vec2 uv)\n"
            "{\n"
            "    ivec4 l = background_level(level);\n"
            "    vec2 p = uv * vec2(l.zw) - 0.5;\n"
            "    ivec2 i = ivec2(floor(p));\n"
            "    vec2 f = p - vec2(i);\n"
            "    return mix(mix(background_texel(l, i), background_texel(l, i + ivec2(1, 0)), f.x),\n"
            "        mix(background_texel(l, i + ivec2(0, 1)), background_texel(l, i + ivec2(1, 1)), f.x), f.y);\n"
            "}\n"
            "vec4 bspline(float t)\n"
            "{\n"
            "    float s = 1.0 - t;\n"
            "    return vec4(s * s * s, 4.0 - 6.0 * t * t + 3.0 * t * t * t, 4.0 - 6.0 * s * s + 3.0 * s * s * s, t * t * t) / 6.0;\n"
            "}\n"
            "// cubic B-spline: smooth where the halved versions are magnified\n"
            "vec3 background_bicubic(int level, vec2 uv)\n"
            "{\n"
            "    ivec4 l = background_level(level);\n"
            "    vec2 p = uv * vec2(l.zw) - 0.5;\n"
            "    ivec2 i = ivec2(floor(p));\n"
            "    vec2 f = p - vec2(i);\n"
            "    vec4 wx = bspline(f.x);\n"
            "    vec4 wy = bspline(f.y);\n"
            "    vec3 result = vec3(0.0);\n"
            "    for (int y = 0; y < 4; ++y)\n"
            "    {\n"
            "        vec3 row = vec3(0.0);\n"
            "        for (int x = 0; x < 4; ++x)\n"
            "            row += wx[x] * background_texel(l, i + ivec2(x - 1, y - 1));\n"
            "        result += wy[y] * row;\n"
            "    }\n"
            "    return result;\n"
            "}\n"
            "vec3 from_linear(vec3 c)\n"
            "{\n"
            "    c = clamp(c, 0.0, 1.0);\n"
            "    return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), c));\n"
            "}\n"
            "vec3 tone_map(vec3 color)\n"
            "{\n"
            "    const float startCompression = 0.8 - 0.04;\n"
            "    const float desaturation = 0.15;\n"
            "    float x = min(color.r, min(color.g, color.b));\n"
            "    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;\n"
            "    color -= offset;\n"
            "    float peak = max(color.r, max(color.g, color.b));\n"
            "    if (peak < startCompression)\n"
            "        return color;\n"
            "    const float d = 1.0 - startCompression;\n"
            "    float newPeak = 1.0 - d * d / (peak + d - startCompression);\n"
            "    color *= newPeak / peak;\n"
            "    float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);\n"
            "    return mix(color, vec3(newPeak), g);\n"
            "}\n"
            "void main()\n"
            "{\n"
            "    vec4 nearPoint = uClipToWorld * vec4(Ndc, -1.0, 1.0);\n"
            "    vec4 farPoint = uClipToWorld * vec4(Ndc, 1.0, 1.0);\n"
            "    vec3 direction = normalize(farPoint.xyz * nearPoint.w - nearPoint.xyz * farPoint.w);\n"
            "    vec2 uv = vec2(atan(direction.z, direction.x) / (2.0 * PI) + 0.5, 0.5 - asin(clamp(direction.y, -1.0, 1.0)) / PI);\n"
            "    // the angle (radians) a fragment covers\n"
            "    float footprint = max(length(dFdx(direction)), length(dFdy(direction)));\n"
            "    float blur = clamp(uBlur, 0.0, 1.0);\n"
            "    vec3 radiance;\n"
            "    if (uSource == 1)\n"
            "    {\n"
            "        // the level whose texels match the blur (about the width of a GGX lobe of roughness blur) or the footprint\n"
            "        float texelsPerRadian = float(uBackgroundExtents.x) / (2.0 * PI);\n"
            "        float level = log2(max(max(footprint, 2.0 * blur * blur) * texelsPerRadian, 1.0));\n"
            "        level = min(level, float(background_top_level()));\n"
            "        int level0 = int(floor(level));\n"
            "        float t = level - float(level0);\n"
            "        radiance = level0 == 0 ? background_bilinear(0, uv) : background_bicubic(level0, uv);\n"
            "        if (t > 0.0)\n"
            "            radiance = mix(radiance, background_bicubic(level0 + 1, uv), t);\n"
            "    }\n"
            "    else\n"
            "    {\n"
            "        float band = blur * 5.0;\n"
            "        int band0 = min(int(floor(band)), 4);\n"
            "        radiance = mix(environment_band(band0, uv), environment_band(band0 + 1, uv), band - float(band0));\n"
            "    }\n"
            "    FragColor = vec4(from_linear(tone_map(radiance * uIntensity)), 1.0);\n"
            "}\n";

        struct background_parameters
        {
            float ndcToClip[16];
            float clipToWorld[16];
            std::int32_t backgroundExtents[2];
            std::int32_t source;
            float blur;
            float intensity;
        };
    }

    struct vulkan_graphics_backend::shadow_resources
    {
        bool failed = true;
        std::unique_ptr<vulkan_image> atlas;
        VkShaderModule vertex = VK_NULL_HANDLE;
        VkShaderModule fragment = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        std::uint32_t matricesBinding = 0u;
        std::uint32_t tableBinding = 0u;
    };

    struct vulkan_graphics_backend::background_resources
    {
        bool failed = true;
        VkShaderModule vertex = VK_NULL_HANDLE;
        VkShaderModule fragment = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        std::map<std::tuple<VkFormat, VkFormat, VkSampleCountFlagBits>, VkPipeline> pipelines;
    };

    vulkan_graphics_backend::vulkan_graphics_backend()
    {
        sInstance = this;
    }

    vulkan_graphics_backend::~vulkan_graphics_backend()
    {
        try
        {
            cleanup();
        }
        catch (...)
        {
        }
        if (sInstance == this)
            sInstance = nullptr;
    }

    vulkan_graphics_backend* vulkan_graphics_backend::instance()
    {
        return sInstance;
    }

    neogfx::renderer vulkan_graphics_backend::renderer() const
    {
        return neogfx::renderer::Vulkan;
    }

    void vulkan_graphics_backend::initialize()
    {
        iStatistics.enabled = (std::getenv("NEOGFX_VULKAN_STATS") != nullptr);
        iStatistics.periodStart = std::chrono::steady_clock::now();
        if (iDevice != VK_NULL_HANDLE)
            return;
        create_instance();
        create_device();
        create_defaults();
        service<debug::logger>() << neolib::logger::severity::Debug << "Vulkan device: " << iProperties.deviceName << std::endl;
        service<debug::logger>() << neolib::logger::severity::Debug << "Vulkan API version: " <<
            VK_API_VERSION_MAJOR(iProperties.apiVersion) << "." << VK_API_VERSION_MINOR(iProperties.apiVersion) << "." << VK_API_VERSION_PATCH(iProperties.apiVersion) << std::endl;
        service<debug::logger>() << neolib::logger::severity::Debug << "Vulkan driver version: " << iProperties.driverVersion << std::endl;
    }

    void vulkan_graphics_backend::cleanup()
    {
        if (iDevice == VK_NULL_HANDLE)
            return;
        execute();
        vkDeviceWaitIdle(iDevice);
        for (std::uint32_t f = 0u; f < FramesInFlight; ++f)
            retire_frame(f);
        if (iShadows)
        {
            if (iShadows->atlas)
            {
                vkDestroyImageView(iDevice, iShadows->atlas->view, nullptr);
                vkDestroyImage(iDevice, iShadows->atlas->image, nullptr);
                vkFreeMemory(iDevice, iShadows->atlas->memory, nullptr);
            }
            vkDestroyPipeline(iDevice, iShadows->pipeline, nullptr);
            vkDestroyPipelineLayout(iDevice, iShadows->pipelineLayout, nullptr);
            vkDestroyDescriptorSetLayout(iDevice, iShadows->descriptorSetLayout, nullptr);
            vkDestroyShaderModule(iDevice, iShadows->vertex, nullptr);
            vkDestroyShaderModule(iDevice, iShadows->fragment, nullptr);
            iShadows = nullptr;
        }
        if (iBackground)
        {
            for (auto& p : iBackground->pipelines)
                vkDestroyPipeline(iDevice, p.second, nullptr);
            vkDestroyPipelineLayout(iDevice, iBackground->pipelineLayout, nullptr);
            vkDestroyDescriptorSetLayout(iDevice, iBackground->descriptorSetLayout, nullptr);
            vkDestroyShaderModule(iDevice, iBackground->vertex, nullptr);
            vkDestroyShaderModule(iDevice, iBackground->fragment, nullptr);
            iBackground = nullptr;
        }
        for (auto* image : { &iDummyTexture, &iDummyTextureMS })
            if (*image)
            {
                vkDestroyImageView(iDevice, (**image).view, nullptr);
                vkDestroyImage(iDevice, (**image).image, nullptr);
                vkFreeMemory(iDevice, (**image).memory, nullptr);
                *image = nullptr;
            }
        free_buffer(iZeroBuffer);
        for (auto& f : iFrames)
        {
            for (auto& chunk : f.transient)
                free_buffer(chunk.buffer);
            f.transient.clear();
        }
        for (auto& s : iSamplers)
            vkDestroySampler(iDevice, s.second, nullptr);
        iSamplers.clear();
        for (auto& f : iFrames)
        {
            vkDestroyFence(iDevice, f.fence, nullptr);
            vkDestroyCommandPool(iDevice, f.commandPool, nullptr);
            f = frame{};
        }
        iCommandBuffer = VK_NULL_HANDLE;
        vkDestroyDevice(iDevice, nullptr);
        iDevice = VK_NULL_HANDLE;
        if (iDebugMessenger != VK_NULL_HANDLE)
        {
            auto destroyMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(iInstance, "vkDestroyDebugUtilsMessengerEXT"));
            if (destroyMessenger)
                destroyMessenger(iInstance, iDebugMessenger, nullptr);
            iDebugMessenger = VK_NULL_HANDLE;
        }
        vkDestroyInstance(iInstance, nullptr);
        iInstance = VK_NULL_HANDLE;
        iColor = nullptr;
        iDepthStencil = nullptr;
        iProgram = nullptr;
        iVertexArray = nullptr;
        iTextureUnits = {};
    }

    void vulkan_graphics_backend::finish()
    {
        execute();
        if (iDevice != VK_NULL_HANDLE)
            vkDeviceWaitIdle(iDevice);
    }

    void vulkan_graphics_backend::execute()
    {
        // n.b. the work recorded so far submitted and all the work in flight done (e.g. before the shared code reuses
        // vertex buffer space; see native_renderer::clear_non_cacheable_vertex_buffers)
        if (iRecording)
        {
            ++iStatistics.executes;
            submit();
        }
        wait_for_frames();
    }

    void vulkan_graphics_backend::create_instance()
    {
        std::vector<char const*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
#ifdef _WIN32
        extensions.push_back("VK_KHR_win32_surface");
#endif
        std::vector<char const*> layers;

        bool debug = false;
#ifndef NDEBUG
        debug = true;
#endif
        if (std::getenv("NEOGFX_VULKAN_VALIDATION") != nullptr)
            debug = true;
        if (debug)
        {
            std::uint32_t layerCount = 0u;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            for (auto const& layer : availableLayers)
                if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
                    layers.push_back("VK_LAYER_KHRONOS_validation");
            std::uint32_t extensionCount = 0u;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
            debug = false;
            for (auto const& extension : availableExtensions)
                if (std::strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                    debug = true;
            if (debug)
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        VkApplicationInfo applicationInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        applicationInfo.pApplicationName = "neoGFX";
        applicationInfo.pEngineName = "neoGFX";
        applicationInfo.apiVersion = VK_API_VERSION_1_3;
        VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instanceInfo.pApplicationInfo = &applicationInfo;
        instanceInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
        instanceInfo.ppEnabledExtensionNames = extensions.data();
        instanceInfo.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
        instanceInfo.ppEnabledLayerNames = layers.data();
        auto const result = vkCreateInstance(&instanceInfo, nullptr, &iInstance);
        if (result != VK_SUCCESS)
            throw failed_to_initialize("vkCreateInstance: " + vkErrorString(result));

        if (debug)
        {
            auto createMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(iInstance, "vkCreateDebugUtilsMessengerEXT"));
            if (createMessenger)
            {
                VkDebugUtilsMessengerCreateInfoEXT messengerInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
                messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                messengerInfo.pfnUserCallback = debug_callback;
                createMessenger(iInstance, &messengerInfo, nullptr, &iDebugMessenger);
            }
        }
    }

    void vulkan_graphics_backend::create_device()
    {
        std::uint32_t deviceCount = 0u;
        vkEnumeratePhysicalDevices(iInstance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(iInstance, &deviceCount, devices.data());

        char const* const requiredExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME };

        std::optional<std::pair<int, VkPhysicalDevice>> best;
        std::uint32_t bestQueueFamily = 0u;
        for (auto device : devices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            if (properties.apiVersion < VK_API_VERSION_1_3)
                continue;
            std::uint32_t extensionCount = 0u;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
            bool haveExtensions = true;
            for (auto required : requiredExtensions)
                if (std::none_of(extensions.begin(), extensions.end(), [&](VkExtensionProperties const& e) { return std::strcmp(e.extensionName, required) == 0; }))
                    haveExtensions = false;
            if (!haveExtensions)
                continue;
            VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
            VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
            features.pNext = &features13;
            vkGetPhysicalDeviceFeatures2(device, &features);
            if (!features13.dynamicRendering || !features13.synchronization2)
                continue;
            std::uint32_t familyCount = 0u;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
            std::optional<std::uint32_t> family;
            for (std::uint32_t f = 0u; f < familyCount && !family; ++f)
                if ((families[f].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                    family = f;
            if (!family)
                continue;
            int const score =
                properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 3 :
                properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2 : 1;
            if (!best || score > best->first)
            {
                best.emplace(score, device);
                bestQueueFamily = *family;
            }
        }
        if (!best)
            throw failed_to_initialize("no Vulkan 1.3 device with dynamic rendering, synchronization2, "
                VK_KHR_SWAPCHAIN_EXTENSION_NAME " and " VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

        iPhysicalDevice = best->second;
        iQueueFamily = bestQueueFamily;
        vkGetPhysicalDeviceProperties(iPhysicalDevice, &iProperties);
        vkGetPhysicalDeviceMemoryProperties(iPhysicalDevice, &iMemoryProperties);

        VkPhysicalDeviceFeatures supported;
        vkGetPhysicalDeviceFeatures(iPhysicalDevice, &supported);
        iSampleRateShading = (supported.sampleRateShading == VK_TRUE);

        float const priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        queueInfo.queueFamilyIndex = iQueueFamily;
        queueInfo.queueCount = 1u;
        queueInfo.pQueuePriorities = &priority;

        VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        features13.dynamicRendering = VK_TRUE;
        features13.synchronization2 = VK_TRUE;
        VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        features.pNext = &features13;
        // n.b. gl_SampleID and gl_SamplePosition (see standard-glyph.frag and standard-shape.frag) need sample rate shading
        features.features.sampleRateShading = supported.sampleRateShading;
        features.features.shaderFloat64 = supported.shaderFloat64;

        VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        deviceInfo.pNext = &features;
        deviceInfo.queueCreateInfoCount = 1u;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.enabledExtensionCount = static_cast<std::uint32_t>(std::size(requiredExtensions));
        deviceInfo.ppEnabledExtensionNames = requiredExtensions;
        auto const result = vkCreateDevice(iPhysicalDevice, &deviceInfo, nullptr, &iDevice);
        if (result != VK_SUCCESS)
            throw failed_to_initialize("vkCreateDevice: " + vkErrorString(result));
        vkGetDeviceQueue(iDevice, iQueueFamily, 0u, &iQueue);
        iCmdPushDescriptorSet = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(iDevice, "vkCmdPushDescriptorSetKHR"));
        if (iCmdPushDescriptorSet == nullptr)
            throw failed_to_initialize("vkCmdPushDescriptorSetKHR");

        for (auto format : { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT })
            if (format_supports(format, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
            {
                iDepthStencilFormat = format;
                break;
            }
        if (iDepthStencilFormat == VK_FORMAT_UNDEFINED)
            throw failed_to_initialize("no depth/stencil format");

        for (auto& f : iFrames)
        {
            VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = iQueueFamily;
            vkCheck(vkCreateCommandPool(iDevice, &poolInfo, nullptr, &f.commandPool));
            VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocateInfo.commandPool = f.commandPool;
            allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocateInfo.commandBufferCount = 1u;
            vkCheck(vkAllocateCommandBuffers(iDevice, &allocateInfo, &f.commandBuffer));
            VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
            vkCheck(vkCreateFence(iDevice, &fenceInfo, nullptr, &f.fence));
        }
        iFrame = 0u;
        iCommandBuffer = iFrames[iFrame].commandBuffer;
    }

    void vulkan_graphics_backend::create_defaults()
    {
        // what a texture unit without a texture (or with a texture of the wrong kind) samples
        iDummyTexture = create_image(1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, 4u);
        iDummyTextureMS = create_image(1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, 4u, sample_count(4u));
        // what a vertex input variable without an attribute (and a storage buffer binding without a buffer) reads
        iZeroBuffer = allocate_buffer(256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
        std::memset(iZeroBuffer.mapping, 0, static_cast<std::size_t>(iZeroBuffer.size));
        execute();
    }

    std::unique_ptr<texture_manager> vulkan_graphics_backend::create_texture_manager()
    {
        return std::make_unique<vulkan_texture_manager>();
    }

    ref_ptr<i_shader_program> vulkan_graphics_backend::create_standard_shader_program()
    {
        return neolib::make_ref<vulkan_standard_shader_program>().as<i_shader_program>();
    }

    void* vulkan_graphics_backend::create_shader_program_object()
    {
        return new vulkan_program{};
    }

    void vulkan_graphics_backend::destroy_shader_program_object(void* aShaderProgramObject)
    {
        auto* program = static_cast<vulkan_program*>(aShaderProgramObject);
        if (program == nullptr)
            return;
        if (iProgram == program)
            iProgram = nullptr;
        if (alive())
            release_program(*program);
        delete program;
    }

    void* vulkan_graphics_backend::create_shader_object(shader_type aShaderType)
    {
        return new vulkan_shader_object{ aShaderType };
    }

    void vulkan_graphics_backend::destroy_shader_object(void* aShaderObject)
    {
        auto* shader = static_cast<vulkan_shader_object*>(aShaderObject);
        if (shader == nullptr)
            return;
        if (alive() && shader->module != VK_NULL_HANDLE)
            defer([device = iDevice, module = shader->module]() { vkDestroyShaderModule(device, module, nullptr); });
        delete shader;
    }

    viewport vulkan_graphics_backend::viewport() const
    {
        return rect{ point{ iState.viewport.x, iState.viewport.y }, size{ iState.viewport.width, iState.viewport.height } };
    }

    std::optional<rect> vulkan_graphics_backend::scissor() const
    {
        if (!iState.scissorEnabled)
            return std::nullopt;
        return rect{ point{ static_cast<scalar>(iState.scissor.offset.x), static_cast<scalar>(iState.scissor.offset.y) },
            size{ static_cast<scalar>(iState.scissor.extent.width), static_cast<scalar>(iState.scissor.extent.height) } };
    }

    std::uint32_t vulkan_graphics_backend::memory_type(std::uint32_t aTypeBits, VkMemoryPropertyFlags aRequired, VkMemoryPropertyFlags aPreferred) const
    {
        for (auto const wanted : { aRequired | aPreferred, aRequired })
            for (std::uint32_t i = 0u; i < iMemoryProperties.memoryTypeCount; ++i)
                if ((aTypeBits & (1u << i)) != 0u && (iMemoryProperties.memoryTypes[i].propertyFlags & wanted) == wanted)
                    return i;
        throw vk_error("no suitable memory type");
    }

    vulkan_buffer vulkan_graphics_backend::allocate_buffer(VkDeviceSize aSize, VkBufferUsageFlags aUsage, bool aMapped)
    {
        struct allocation_statistics
        {
            vulkan_graphics_backend& backend;
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            ~allocation_statistics()
            {
                auto const elapsed = std::chrono::steady_clock::now() - start;
                ++backend.iStatistics.allocations;
                backend.iStatistics.allocationTime += elapsed;
                backend.iStatistics.frameBackend += elapsed;
            }
        } allocationStatistics{ *this };
        vulkan_buffer result;
        result.size = std::max<VkDeviceSize>(aSize, 4u);
        result.deviceLocal = !aMapped;
        result.usage = aUsage;
        VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = result.size;
        bufferInfo.usage = aUsage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCheck(vkCreateBuffer(iDevice, &bufferInfo, nullptr, &result.buffer));
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(iDevice, result.buffer, &requirements);
        VkMemoryAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocateInfo.allocationSize = requirements.size;
        if (aMapped)
        {
            // n.b. mapped buffers (which the GPU reads every frame: vertices, SSBOs and uniforms) are preferably in device local
            // memory the host can write (resizable BAR); else (or if that small heap is full) in host memory
            auto const hostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            allocateInfo.memoryTypeIndex = memory_type(requirements.memoryTypeBits, hostVisible, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (vkAllocateMemory(iDevice, &allocateInfo, nullptr, &result.memory) != VK_SUCCESS)
            {
                std::uint32_t hostMemoryTypeBits = 0u;
                for (std::uint32_t i = 0u; i < iMemoryProperties.memoryTypeCount; ++i)
                    if ((iMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == 0u)
                        hostMemoryTypeBits |= (1u << i);
                auto const fallbackTypeBits = requirements.memoryTypeBits & hostMemoryTypeBits;
                allocateInfo.memoryTypeIndex = memory_type(fallbackTypeBits != 0u ? fallbackTypeBits : requirements.memoryTypeBits, hostVisible);
                vkCheck(vkAllocateMemory(iDevice, &allocateInfo, nullptr, &result.memory));
            }
        }
        else
        {
            allocateInfo.memoryTypeIndex = memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vkCheck(vkAllocateMemory(iDevice, &allocateInfo, nullptr, &result.memory));
        }
        vkCheck(vkBindBufferMemory(iDevice, result.buffer, result.memory, 0u));
        result.hostCached = (iMemoryProperties.memoryTypes[allocateInfo.memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0u;
        if (aMapped)
            vkCheck(vkMapMemory(iDevice, result.memory, 0u, VK_WHOLE_SIZE, 0u, &result.mapping));
        return result;
    }

    void vulkan_graphics_backend::free_buffer(vulkan_buffer& aBuffer)
    {
        if (aBuffer.buffer == VK_NULL_HANDLE)
            return;
        if (aBuffer.mapping)
            vkUnmapMemory(iDevice, aBuffer.memory);
        vkDestroyBuffer(iDevice, aBuffer.buffer, nullptr);
        vkFreeMemory(iDevice, aBuffer.memory, nullptr);
        aBuffer = vulkan_buffer{};
    }

    vulkan_graphics_backend::transient_allocation vulkan_graphics_backend::allocate_transient(VkDeviceSize aSize, VkDeviceSize aAlignment)
    {
        iStatistics.transientBytes += aSize;
        auto const align = [&](VkDeviceSize aOffset) { return (aOffset + aAlignment - 1u) / aAlignment * aAlignment; };
        command_buffer(); // n.b. the current frame's transient memory is reusable once it is recording
        auto& f = iFrames[iFrame];
        while (f.transientChunk < f.transient.size())
        {
            auto& chunk = f.transient[f.transientChunk];
            auto const offset = align(chunk.used);
            if (offset + aSize <= chunk.buffer.size)
            {
                chunk.used = offset + aSize;
                return { chunk.buffer.buffer, offset, static_cast<std::uint8_t*>(chunk.buffer.mapping) + offset };
            }
            ++f.transientChunk;
        }
        f.transient.push_back(transient_chunk{ allocate_buffer(std::max(TransientChunkSize, aSize),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true) });
        f.transientChunk = f.transient.size() - 1u;
        auto& chunk = f.transient.back();
        chunk.used = aSize;
        return { chunk.buffer.buffer, 0u, chunk.buffer.mapping };
    }

    void vulkan_graphics_backend::defer(std::function<void()> aDestroy)
    {
        // n.b. with the current frame recording, so that this frame's previous use is done and the destruction follows
        // the frames submitted so far
        command_buffer();
        iFrames[iFrame].deferred.push_back(std::move(aDestroy));
    }

    VkCommandBuffer vulkan_graphics_backend::command_buffer()
    {
        if (!iRecording)
        {
            // this frame's previous use must be done before its command buffer and memory are reused
            auto& f = iFrames[iFrame];
            if (f.inFlight)
            {
                auto const waitStart = std::chrono::steady_clock::now();
                vkCheck(vkWaitForFences(iDevice, 1u, &f.fence, VK_TRUE, UINT64_MAX));
                iStatistics.fenceWait += std::chrono::steady_clock::now() - waitStart;
            iStatistics.frameBackend += std::chrono::steady_clock::now() - waitStart;
                iStatistics.frameBackend += std::chrono::steady_clock::now() - waitStart;
                retire_frame(iFrame);
            }
            iCommandBuffer = f.commandBuffer;
            VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkCheck(vkBeginCommandBuffer(iCommandBuffer, &beginInfo));
            iRecording = true;
            invalidate_bindings();
        }
        return iCommandBuffer;
    }

    void vulkan_graphics_backend::submit(VkSemaphore aWait, VkSemaphore aSignal)
    {
        command_buffer();
        end_rendering();
        vkCheck(vkEndCommandBuffer(iCommandBuffer));
        iRecording = false;

        VkCommandBufferSubmitInfo commandBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        commandBufferInfo.commandBuffer = iCommandBuffer;
        VkSemaphoreSubmitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
        waitInfo.semaphore = aWait;
        waitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        VkSemaphoreSubmitInfo signalInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
        signalInfo.semaphore = aSignal;
        signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.commandBufferInfoCount = 1u;
        submitInfo.pCommandBufferInfos = &commandBufferInfo;
        submitInfo.waitSemaphoreInfoCount = (aWait != VK_NULL_HANDLE ? 1u : 0u);
        submitInfo.pWaitSemaphoreInfos = &waitInfo;
        submitInfo.signalSemaphoreInfoCount = (aSignal != VK_NULL_HANDLE ? 1u : 0u);
        submitInfo.pSignalSemaphoreInfos = &signalInfo;
        auto& f = iFrames[iFrame];
        vkCheck(vkQueueSubmit2(iQueue, 1u, &submitInfo, f.fence));
        ++iStatistics.submits;
        f.inFlight = true;
        f.serial = ++iSubmitted;
        // n.b. not waited for: the next frame is recorded while the GPU executes this one (see command_buffer and
        // wait_for_frames); the uniform blocks uploaded are in this frame's transient memory
        iUploadedUniformBlocks.clear();
        iFrame = (iFrame + 1u) % FramesInFlight;
    }

    void vulkan_graphics_backend::wait_for_frames()
    {
        // all the frames in flight
        for (std::uint32_t i = 1u; i <= FramesInFlight; ++i)
        {
            auto const index = (iFrame + i) % FramesInFlight;
            auto& f = iFrames[index];
            if (!f.inFlight)
                continue;
            auto const waitStart = std::chrono::steady_clock::now();
            vkCheck(vkWaitForFences(iDevice, 1u, &f.fence, VK_TRUE, UINT64_MAX));
            iStatistics.fenceWait += std::chrono::steady_clock::now() - waitStart;
            iStatistics.frameBackend += std::chrono::steady_clock::now() - waitStart;
            retire_frame(index);
        }
    }

    void vulkan_graphics_backend::poll_frames()
    {
        // the frames in flight that the GPU has completed (n.b. without waiting)
        for (std::uint32_t index = 0u; index < FramesInFlight; ++index)
            if (iFrames[index].inFlight && vkGetFenceStatus(iDevice, iFrames[index].fence) == VK_SUCCESS)
                retire_frame(index);
    }

    void vulkan_graphics_backend::retire_frame(std::uint32_t aFrame)
    {
        // the GPU is done with everything recorded for the frame
        auto& f = iFrames[aFrame];
        if (f.inFlight)
        {
            vkCheck(vkResetFences(iDevice, 1u, &f.fence));
            vkCheck(vkResetCommandPool(iDevice, f.commandPool, 0u));
            f.inFlight = false;
            iCompleted = std::max(iCompleted, f.serial);
        }

        auto deferred = std::move(f.deferred);
        f.deferred.clear();
        for (auto& d : deferred)
            d();
        for (auto& chunk : f.transient)
            chunk.used = 0u;
        f.transientChunk = 0u;
    }

    void vulkan_graphics_backend::begin_rendering()
    {
        if (iRendering)
            return;
        if (iColor == nullptr && iDepthStencil == nullptr)
            throw no_target();
        auto const commandBuffer = command_buffer();
        ++iStatistics.passes;
        memory_barrier();
        VkRenderingAttachmentInfo colorAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        VkRenderingAttachmentInfo depthStencilAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        renderingInfo.renderArea = VkRect2D{ { 0, 0 }, target_extent() };
        renderingInfo.layerCount = 1u;
        if (iColor != nullptr)
        {
            colorAttachment.imageView = iColor->view;
            colorAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            renderingInfo.colorAttachmentCount = 1u;
            renderingInfo.pColorAttachments = &colorAttachment;
        }
        if (iDepthStencil != nullptr)
        {
            depthStencilAttachment.imageView = iDepthStencil->view;
            depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            renderingInfo.pDepthAttachment = &depthStencilAttachment;
            if (has_stencil(iDepthStencil->format))
                renderingInfo.pStencilAttachment = &depthStencilAttachment;
        }
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
        iRendering = true;
    }

    void vulkan_graphics_backend::end_rendering()
    {
        if (!iRendering)
            return;
        vkCmdEndRendering(iCommandBuffer);
        iRendering = false;
    }

    void vulkan_graphics_backend::memory_barrier()
    {
        ++iStatistics.barriers;
        VkMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dependencyInfo.memoryBarrierCount = 1u;
        dependencyInfo.pMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(command_buffer(), &dependencyInfo);
    }

    void vulkan_graphics_backend::image_barrier(VkImage aImage, VkImageAspectFlags aAspect, VkImageLayout aOldLayout, VkImageLayout aNewLayout)
    {
        ++iStatistics.barriers;
        VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        barrier.oldLayout = aOldLayout;
        barrier.newLayout = aNewLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = aImage;
        barrier.subresourceRange = VkImageSubresourceRange{ aAspect, 0u, VK_REMAINING_MIP_LEVELS, 0u, 1u };
        VkDependencyInfo dependencyInfo{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        dependencyInfo.imageMemoryBarrierCount = 1u;
        dependencyInfo.pImageMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(command_buffer(), &dependencyInfo);
    }

    VkExtent2D vulkan_graphics_backend::target_extent() const
    {
        auto const* image = (iColor != nullptr ? iColor : iDepthStencil);
        if (image == nullptr)
            return VkExtent2D{ 0u, 0u };
        return VkExtent2D{ image->width, image->height };
    }

    void vulkan_graphics_backend::invalidate_bindings()
    {
        iBoundPipeline = VK_NULL_HANDLE;
        iDynamicStateValid = false;
    }

    gpu_buffer vulkan_graphics_backend::create_buffer(std::size_t aSize, bool aDeviceLocal)
    {
        auto* buffer = new vulkan_buffer{ allocate_buffer(aSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, !aDeviceLocal) };
        return reinterpret_cast<gpu_buffer>(buffer);
    }

    void vulkan_graphics_backend::destroy_buffer(gpu_buffer aBuffer)
    {
        auto* buffer = buffer_of(aBuffer);
        if (buffer == nullptr)
            return;
        if (alive())
        {
            auto toFree = *buffer;
            defer([this, toFree]() mutable
            {
                for (auto const& s : toFree.spares)
                {
                    vulkan_buffer spare{ s.buffer, s.memory, toFree.size, s.mapping };
                    free_buffer(spare);
                }
                free_buffer(toFree);
            });
        }
        delete buffer;
    }

    void* vulkan_graphics_backend::map_buffer(gpu_buffer aBuffer, std::size_t)
    {
        auto* buffer = buffer_of(aBuffer);
        if (buffer->mapping == nullptr)
            throw std::logic_error("neogfx::vulkan_graphics_backend::map_buffer: device local buffer cannot be mapped");
        // n.b. persistently mapped (and coherent)
        return buffer->mapping;
    }

    void vulkan_graphics_backend::flush_buffer(gpu_buffer, std::size_t, std::size_t)
    {
        // nothing to do: mapped buffers are host coherent and host writes are made available when the work is submitted
    }

    void vulkan_graphics_backend::unmap_buffer(gpu_buffer)
    {
        // nothing to do: mapped buffers stay mapped
    }

    bool vulkan_graphics_backend::discard_buffer(gpu_buffer aBuffer)
    {
        auto* buffer = buffer_of(aBuffer);
        if (buffer == nullptr || buffer->mapping == nullptr || !alive())
            return false;
        // the last submission that may read the buffer: the one being recorded (if any) else the last submitted
        poll_frames();
        auto const lastUse = iSubmitted + (iRecording ? 1u : 0u);
        if (lastUse <= iCompleted)
            return false; // n.b. the GPU is done with it: rewritten in place
        // replaced: by a spare the GPU is done with, else by new storage (cf. OpenGL buffer orphaning)
        vulkan_buffer::spare const retiring{ buffer->buffer, buffer->memory, buffer->mapping, buffer->hostCached, lastUse };
        auto reusable = std::find_if(buffer->spares.begin(), buffer->spares.end(),
            [&](vulkan_buffer::spare const& s) { return s.retired <= iCompleted; });
        if (reusable != buffer->spares.end())
        {
            buffer->buffer = reusable->buffer;
            buffer->memory = reusable->memory;
            buffer->mapping = reusable->mapping;
            buffer->hostCached = reusable->hostCached;
            *reusable = retiring;
        }
        else
        {
            auto const fresh = allocate_buffer(buffer->size, buffer->usage, true);
            buffer->buffer = fresh.buffer;
            buffer->memory = fresh.memory;
            buffer->mapping = fresh.mapping;
            buffer->hostCached = fresh.hostCached;
            buffer->spares.push_back(retiring);
        }
        ++iStatistics.discards;
        return true;
    }

    void vulkan_graphics_backend::write_buffer(gpu_buffer aBuffer, std::size_t aOffset, void const* aData, std::size_t aSize)
    {
        auto* buffer = buffer_of(aBuffer);
        if (buffer->mapping != nullptr)
        {
            std::memcpy(static_cast<std::uint8_t*>(buffer->mapping) + aOffset, aData, aSize);
            return;
        }
        auto const staging = allocate_transient(aSize, 16u);
        std::memcpy(staging.mapping, aData, aSize);
        end_rendering();
        memory_barrier();
        VkBufferCopy const region{ staging.offset, aOffset, aSize };
        vkCmdCopyBuffer(command_buffer(), staging.buffer, buffer->buffer, 1u, &region);
    }

    void vulkan_graphics_backend::copy_buffer(gpu_buffer aSource, gpu_buffer aDestination, std::size_t aSize)
    {
        auto* source = buffer_of(aSource);
        auto* destination = buffer_of(aDestination);
        if (aSize == 0u)
            return;
        ++iStatistics.bufferCopies;
        iStatistics.bufferCopyBytes += aSize;
        if (source->mapping != nullptr && destination->mapping != nullptr && source->hostCached)
        {
            // n.b. what was written through the source's mapping (the draws already recorded keep using the source buffer)
            std::memcpy(destination->mapping, source->mapping, aSize);
            return;
        }
        end_rendering();
        memory_barrier();
        VkBufferCopy const region{ 0u, 0u, aSize };
        vkCmdCopyBuffer(command_buffer(), source->buffer, destination->buffer, 1u, &region);
        if (source->mapping != nullptr && destination->mapping != nullptr)
        {
            // n.b. copied by the GPU (reading memory that is not host cached, e.g. device local, is very slow for the CPU) and
            // waited for: the destination is written through its mapping next (e.g. a buffer that has grown; see native_buffer)
            memory_barrier();
            execute();
        }
    }

    gpu_vertex_array vulkan_graphics_backend::create_vertex_array()
    {
        return reinterpret_cast<gpu_vertex_array>(new vertex_array{});
    }

    void vulkan_graphics_backend::destroy_vertex_array(gpu_vertex_array aVertexArray)
    {
        auto* vertexArray = reinterpret_cast<vertex_array*>(aVertexArray);
        if (iVertexArray == vertexArray)
            iVertexArray = nullptr;
        delete vertexArray;
    }

    gpu_vertex_array vulkan_graphics_backend::bound_vertex_array() const
    {
        return reinterpret_cast<gpu_vertex_array>(iVertexArray);
    }

    void vulkan_graphics_backend::bind_vertex_array(gpu_vertex_array aVertexArray)
    {
        iVertexArray = reinterpret_cast<vertex_array*>(aVertexArray);
    }

    void vulkan_graphics_backend::set_vertex_attribute(i_shader_program const& aProgram, std::string const& aName, gpu_buffer aBuffer,
        std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset)
    {
        auto const& program = *static_cast<vulkan_program const*>(aProgram.handle());
        auto const input = program.vertexInputs.find(aName);
        if (input == program.vertexInputs.end())
            return;
        set_vertex_attribute(input->second.location, aBuffer, aArity, aType, aNormalized, aStride, aOffset);
    }

    void vulkan_graphics_backend::set_vertex_attribute(std::uint32_t aLocation, gpu_buffer aBuffer,
        std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset)
    {
        if (iVertexArray == nullptr)
            throw no_vertex_array();
        iVertexArray->attributes[aLocation] = vertex_array::attribute{ aBuffer, to_vk_attribute_format(aArity, aType, aNormalized),
            static_cast<std::uint32_t>(aStride), static_cast<std::uint32_t>(aOffset) };
    }

    void vulkan_graphics_backend::set_index_buffer(gpu_buffer aBuffer)
    {
        if (iVertexArray == nullptr)
            throw no_vertex_array();
        iVertexArray->indexBuffer = aBuffer;
    }

    std::uint32_t vulkan_graphics_backend::blend_key() const
    {
        if (!iState.blending)
            return 0u;
        if (iState.xorBlending)
            return 0x100u;
        return 1u + static_cast<std::uint32_t>(iState.blendingMode);
    }

    VkPipeline vulkan_graphics_backend::pipeline(vulkan_program& aProgram, vulkan_vertex_input_layout const& aVertexInput)
    {
        vulkan_pipeline_key key;
        key.vertexInput = aVertexInput;
        key.colorFormat = (iColor != nullptr ? iColor->format : VK_FORMAT_UNDEFINED);
        key.depthStencilFormat = (iDepthStencil != nullptr ? iDepthStencil->format : VK_FORMAT_UNDEFINED);
        key.samples = (iColor != nullptr ? iColor->samples : iDepthStencil->samples);
        key.blend = blend_key();
        key.colorWrite = iState.colorWrite;
        key.sampleShading = (iState.sampleShading && iSampleRateShading && key.samples != VK_SAMPLE_COUNT_1_BIT ?
            static_cast<float>(*iState.sampleShading) : 0.0f);
        auto existing = aProgram.pipelines.find(key);
        if (existing != aProgram.pipelines.end())
            return existing->second;

        // the sample count, for gl_NumSamples (see vulkan_glsl); n.b. ignored by shaders that don't use it
        std::int32_t const numSamples = static_cast<std::int32_t>(key.samples);
        VkSpecializationMapEntry const numSamplesEntry{ VulkanGlslNumSamplesConstantId, 0u, sizeof(numSamples) };
        VkSpecializationInfo const specializationInfo{ 1u, &numSamplesEntry, sizeof(numSamples), &numSamples };
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        for (std::size_t stage = 0u; stage < aProgram.modules.size(); ++stage)
            if (aProgram.modules[stage] != VK_NULL_HANDLE && static_cast<shader_type>(stage) != shader_type::Compute)
            {
                VkPipelineShaderStageCreateInfo stageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
                stageInfo.stage = to_vk_stage(static_cast<shader_type>(stage));
                stageInfo.module = aProgram.modules[stage];
                stageInfo.pName = "main";
                stageInfo.pSpecializationInfo = &specializationInfo;
                stages.push_back(stageInfo);
            }

        std::vector<VkVertexInputBindingDescription> bindings;
        for (std::uint32_t binding = 0u; binding < aVertexInput.strides.size(); ++binding)
            bindings.push_back(VkVertexInputBindingDescription{ binding, aVertexInput.strides[binding], VK_VERTEX_INPUT_RATE_VERTEX });
        std::vector<VkVertexInputAttributeDescription> attributes;
        for (auto const& a : aVertexInput.attributes)
            attributes.push_back(VkVertexInputAttributeDescription{ a.location, a.binding, a.format, a.offset });
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<std::uint32_t>(bindings.size());
        vertexInputInfo.pVertexBindingDescriptions = bindings.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportInfo.viewportCount = 1u;
        viewportInfo.scissorCount = 1u;

        VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationInfo.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampleInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampleInfo.rasterizationSamples = key.samples;
        multisampleInfo.sampleShadingEnable = (key.sampleShading > 0.0f ? VK_TRUE : VK_FALSE);
        multisampleInfo.minSampleShading = key.sampleShading;

        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilInfo.front = VkStencilOpState{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0xFFu, 0xFFu, 0u };
        depthStencilInfo.back = depthStencilInfo.front;
        depthStencilInfo.maxDepthBounds = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = key.colorWrite ?
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT : 0u;
        auto const set_blend = [&](VkBlendOp aColorOp, VkBlendFactor aSrcColor, VkBlendFactor aDstColor, VkBlendOp aAlphaOp, VkBlendFactor aSrcAlpha, VkBlendFactor aDstAlpha)
            {
                blendAttachment.blendEnable = VK_TRUE;
                blendAttachment.colorBlendOp = aColorOp;
                blendAttachment.srcColorBlendFactor = aSrcColor;
                blendAttachment.dstColorBlendFactor = aDstColor;
                blendAttachment.alphaBlendOp = aAlphaOp;
                blendAttachment.srcAlphaBlendFactor = aSrcAlpha;
                blendAttachment.dstAlphaBlendFactor = aDstAlpha;
            };
        // n.b. as opengl_graphics_backend::set_blending_mode and set_xor_blending
        if (key.blend == 0x100u)
            set_blend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
                VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR);
        else if (key.blend != 0u)
        {
            switch (static_cast<neogfx::blending_mode>(key.blend - 1u))
            {
            case neogfx::blending_mode::None:
                break;
            case neogfx::blending_mode::Default:
                set_blend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Sprite:
            case neogfx::blending_mode::FilterFinish:
                set_blend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Blit:
                set_blend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO,
                    VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO);
                break;
            case neogfx::blending_mode::Lighten:
                set_blend(VK_BLEND_OP_MAX, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE,
                    VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Filter:
                set_blend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Premultiply:
                set_blend(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
                break;
            }
        }
        VkPipelineColorBlendStateCreateInfo blendInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        blendInfo.attachmentCount = (key.colorFormat != VK_FORMAT_UNDEFINED ? 1u : 0u);
        blendInfo.pAttachments = &blendAttachment;

        VkDynamicState const dynamicStates[] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_CULL_MODE,
            VK_DYNAMIC_STATE_FRONT_FACE,
            VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
            VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
            VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
            VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
            VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
            VK_DYNAMIC_STATE_STENCIL_OP,
            VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
            VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE,
            VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
            VK_DYNAMIC_STATE_DEPTH_BIAS
        };
        VkPipelineDynamicStateCreateInfo dynamicInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamicInfo.dynamicStateCount = static_cast<std::uint32_t>(std::size(dynamicStates));
        dynamicInfo.pDynamicStates = dynamicStates;

        VkPipelineRenderingCreateInfo renderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        renderingInfo.colorAttachmentCount = blendInfo.attachmentCount;
        renderingInfo.pColorAttachmentFormats = &key.colorFormat;
        renderingInfo.depthAttachmentFormat = key.depthStencilFormat;
        renderingInfo.stencilAttachmentFormat = has_stencil(key.depthStencilFormat) ? key.depthStencilFormat : VK_FORMAT_UNDEFINED;

        VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.stageCount = static_cast<std::uint32_t>(stages.size());
        pipelineInfo.pStages = stages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &rasterizationInfo;
        pipelineInfo.pMultisampleState = &multisampleInfo;
        pipelineInfo.pDepthStencilState = &depthStencilInfo;
        pipelineInfo.pColorBlendState = &blendInfo;
        pipelineInfo.pDynamicState = &dynamicInfo;
        pipelineInfo.layout = aProgram.pipelineLayout;
        VkPipeline result = VK_NULL_HANDLE;
        vkCheck(vkCreateGraphicsPipelines(iDevice, VK_NULL_HANDLE, 1u, &pipelineInfo, nullptr, &result));
        aProgram.pipelines.emplace(key, result);
        ++iStatistics.pipelinesCreated;
        return result;
    }

    void vulkan_graphics_backend::apply_dynamic_state(bool aForce)
    {
        if (iDynamicStateValid && !aForce)
            return;
        auto const commandBuffer = command_buffer();
        apply_viewport_and_scissor();
        VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
        switch (iState.faceCulling)
        {
        case neogfx::face_culling::None:
            break;
        case neogfx::face_culling::Front:
            cullMode = !iState.faceCullingFlipped ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_BACK_BIT;
            break;
        case neogfx::face_culling::Back:
            cullMode = !iState.faceCullingFlipped ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT;
            break;
        case neogfx::face_culling::FrontAndBack:
            cullMode = VK_CULL_MODE_FRONT_AND_BACK;
            break;
        }
        vkCmdSetCullMode(commandBuffer, cullMode);
        // n.b. the winding of a triangle as OpenGL sees it (window y up) is the opposite of the winding Vulkan sees for the
        // same framebuffer coordinates (y down)
        vkCmdSetFrontFace(commandBuffer, iState.frontFace == neogfx::front_face::CounterClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE);
        vkCmdSetDepthTestEnable(commandBuffer, iState.depthTest && iDepthStencil != nullptr ? VK_TRUE : VK_FALSE);
        vkCmdSetDepthWriteEnable(commandBuffer, iState.depthWrite && iDepthStencil != nullptr ? VK_TRUE : VK_FALSE);
        vkCmdSetDepthCompareOp(commandBuffer, iState.depthCompare);
        vkCmdSetStencilTestEnable(commandBuffer, iState.stencilTest && iDepthStencil != nullptr && has_stencil(iDepthStencil->format) ? VK_TRUE : VK_FALSE);
        vkCmdSetStencilOp(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, iState.stencilPassOp, VK_STENCIL_OP_KEEP, iState.stencilCompare);
        vkCmdSetStencilCompareMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, iState.stencilCompareMask);
        vkCmdSetStencilWriteMask(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, iState.stencilWriteMask);
        vkCmdSetStencilReference(commandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, iState.stencilReference);
        vkCmdSetDepthBiasEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthBias(commandBuffer, 0.0f, 0.0f, 0.0f);
        iDynamicStateValid = true;
    }

    void vulkan_graphics_backend::apply_viewport_and_scissor()
    {
        auto const commandBuffer = command_buffer();
        auto const extent = target_extent();
        vkCmdSetViewport(commandBuffer, 0u, 1u, &iState.viewport);
        VkRect2D scissor = { { 0, 0 }, extent };
        if (iState.scissorEnabled)
        {
            std::int64_t x = iState.scissor.offset.x;
            std::int64_t y = iState.scissor.offset.y;
            std::int64_t cx = static_cast<std::int32_t>(iState.scissor.extent.width);
            std::int64_t cy = static_cast<std::int32_t>(iState.scissor.extent.height);
            if (x < 0) { cx += x; x = 0; }
            if (y < 0) { cy += y; y = 0; }
            cx = std::clamp<std::int64_t>(cx, 0, std::max<std::int64_t>(0, static_cast<std::int64_t>(extent.width) - x));
            cy = std::clamp<std::int64_t>(cy, 0, std::max<std::int64_t>(0, static_cast<std::int64_t>(extent.height) - y));
            scissor = VkRect2D{ { static_cast<std::int32_t>(x), static_cast<std::int32_t>(y) }, { static_cast<std::uint32_t>(cx), static_cast<std::uint32_t>(cy) } };
        }
        vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);
    }

    vulkan_image const& vulkan_graphics_backend::texture_unit_image(std::uint32_t aTextureUnit, bool aMultisample) const
    {
        auto const* image = (aTextureUnit < iTextureUnits.size() ? iTextureUnits[aTextureUnit] : nullptr);
        if (image == nullptr || (image->samples != VK_SAMPLE_COUNT_1_BIT) != aMultisample)
            return aMultisample ? *iDummyTextureMS : *iDummyTexture;
        return *image;
    }

    void vulkan_graphics_backend::push_descriptors(vulkan_program& aProgram)
    {
        ++iStatistics.descriptorPushes;
        thread_local std::vector<VkWriteDescriptorSet> tWrites;
        thread_local std::vector<VkDescriptorBufferInfo> tBufferInfos;
        thread_local std::vector<VkDescriptorImageInfo> tImageInfos;
        tWrites.clear();
        tBufferInfos.clear();
        tImageInfos.clear();
        tBufferInfos.reserve(aProgram.uniformBlocks.size() + aProgram.storageBuffers.size());
        tImageInfos.reserve(aProgram.samplers.size());

        for (auto const& ub : aProgram.uniformBlocks)
        {
            if (ub.block->data.empty())
                continue;
            auto& uploaded = iUploadedUniformBlocks[ub.block];
            if (uploaded.first != ub.block->generation)
            {
                auto const allocation = allocate_transient(ub.block->data.size(), iProperties.limits.minUniformBufferOffsetAlignment);
                std::memcpy(allocation.mapping, ub.block->data.data(), ub.block->data.size());
                uploaded = { ub.block->generation, allocation };
            }
            tBufferInfos.push_back(VkDescriptorBufferInfo{ uploaded.second.buffer, uploaded.second.offset, ub.block->data.size() });
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstBinding = ub.binding;
            write.descriptorCount = 1u;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.pBufferInfo = &tBufferInfos.back();
            tWrites.push_back(write);
        }
        for (auto const& sb : aProgram.storageBuffers)
        {
            auto const* buffer = buffer_of(sb.buffer());
            tBufferInfos.push_back(VkDescriptorBufferInfo{ buffer != nullptr ? buffer->buffer : iZeroBuffer.buffer, 0u, VK_WHOLE_SIZE });
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstBinding = sb.binding;
            write.descriptorCount = 1u;
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo = &tBufferInfos.back();
            tWrites.push_back(write);
        }
        for (auto const& s : aProgram.samplers)
        {
            auto const& image = texture_unit_image(static_cast<std::uint32_t>(std::max(s.textureUnit, 0)), s.multisample);
            tImageInfos.push_back(VkDescriptorImageInfo{ sampler(image.sampler, image.mipLevels), image.view, VK_IMAGE_LAYOUT_GENERAL });
            VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            write.dstBinding = s.binding;
            write.descriptorCount = 1u;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo = &tImageInfos.back();
            tWrites.push_back(write);
        }
        if (!tWrites.empty())
            iCmdPushDescriptorSet(command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, aProgram.pipelineLayout, 0u,
                static_cast<std::uint32_t>(tWrites.size()), tWrites.data());
    }

    void vulkan_graphics_backend::prepare_draw()
    {
        ++iStatistics.draws;
        if (iProgram == nullptr || !iProgram->linked)
            throw no_program();
        if (iColor == nullptr && iDepthStencil == nullptr)
            throw no_target();
        // a texture sampled that is being drawn to (e.g. the glyph shader's render output): what has been drawn to it so far
        // must be visible (as it would be after glTextureBarrier)
        if (iRendering)
            for (auto const& s : iProgram->samplers)
            {
                auto const unit = static_cast<std::uint32_t>(std::max(s.textureUnit, 0));
                auto const* image = (unit < iTextureUnits.size() ? iTextureUnits[unit] : nullptr);
                if (image != nullptr && (image == iColor || image == iDepthStencil))
                {
                    texture_barrier();
                    break;
                }
            }
        begin_rendering();
    }

    void vulkan_graphics_backend::draw_arrays(gpu_primitive aPrimitive, std::size_t aFirst, std::size_t aCount)
    {
        if (aCount == 0u)
            return;
        prepare_draw();
        auto const commandBuffer = command_buffer();

        thread_local vulkan_vertex_input_layout tLayout;
        thread_local std::vector<std::pair<gpu_buffer, std::uint32_t>> tBindings;
        thread_local std::vector<VkBuffer> tBuffers;
        thread_local std::vector<VkDeviceSize> tOffsets;
        tLayout.attributes.clear();
        tLayout.strides.clear();
        tBindings.clear();
        tBuffers.clear();
        tOffsets.clear();
        auto const binding_of = [&](gpu_buffer aBuffer, std::uint32_t aStride) -> std::uint32_t
            {
                auto existing = std::find(tBindings.begin(), tBindings.end(), std::make_pair(aBuffer, aStride));
                if (existing != tBindings.end())
                    return static_cast<std::uint32_t>(std::distance(tBindings.begin(), existing));
                tBindings.emplace_back(aBuffer, aStride);
                tLayout.strides.push_back(aStride);
                auto const* buffer = buffer_of(aBuffer);
                tBuffers.push_back(buffer != nullptr ? buffer->buffer : iZeroBuffer.buffer);
                tOffsets.push_back(0u);
                return static_cast<std::uint32_t>(tBindings.size() - 1u);
            };
        for (auto const& input : iProgram->vertexInputs)
        {
            auto const location = input.second.location;
            if (iVertexArray != nullptr)
            {
                auto existing = iVertexArray->attributes.find(location);
                if (existing != iVertexArray->attributes.end() && existing->second.buffer != no_gpu_buffer)
                {
                    auto const& a = existing->second;
                    tLayout.attributes.push_back({ location, binding_of(a.buffer, a.stride), a.format, a.offset });
                    continue;
                }
            }
            // n.b. read as zero (cf. a disabled OpenGL vertex attribute array)
            tLayout.attributes.push_back({ location, binding_of(no_gpu_buffer, 0u), zero_attribute_format(input.second.type), 0u });
        }
        std::sort(tLayout.attributes.begin(), tLayout.attributes.end());

        auto const p = pipeline(*iProgram, tLayout);
        if (p != iBoundPipeline)
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
            ++iStatistics.pipelineBinds;
            iBoundPipeline = p;
        }
        apply_dynamic_state(false);
        vkCmdSetPrimitiveTopology(commandBuffer, to_vk_topology(aPrimitive));
        if (!tBuffers.empty())
            vkCmdBindVertexBuffers(commandBuffer, 0u, static_cast<std::uint32_t>(tBuffers.size()), tBuffers.data(), tOffsets.data());
        push_descriptors(*iProgram);
        // n.b. the index buffer (if any) is bound by draw_elements
        if (aFirst != std::numeric_limits<std::size_t>::max())
            vkCmdDraw(commandBuffer, static_cast<std::uint32_t>(aCount), 1u, static_cast<std::uint32_t>(aFirst), 0u);
    }

    void vulkan_graphics_backend::draw_elements(gpu_primitive aPrimitive, std::size_t aFirstIndex, std::size_t aCount)
    {
        if (aCount == 0u)
            return;
        if (iVertexArray == nullptr || buffer_of(iVertexArray->indexBuffer) == nullptr)
            throw no_vertex_array();
        // n.b. everything but the draw
        draw_arrays(aPrimitive, std::numeric_limits<std::size_t>::max(), aCount);
        auto const commandBuffer = command_buffer();
        vkCmdBindIndexBuffer(commandBuffer, buffer_of(iVertexArray->indexBuffer)->buffer, 0u, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, static_cast<std::uint32_t>(aCount), 1u, static_cast<std::uint32_t>(aFirstIndex), 0, 0u);
    }

    void vulkan_graphics_backend::texture_barrier()
    {
        // n.b. the next draw begins rendering again, after a barrier
        end_rendering();
    }

    void vulkan_graphics_backend::enable_scissor(bool aEnable)
    {
        if (iState.scissorEnabled != aEnable)
        {
            iState.scissorEnabled = aEnable;
            iDynamicStateValid = false;
        }
    }

    void vulkan_graphics_backend::set_scissor(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight)
    {
        iState.scissor = VkRect2D{ { aX, aY }, { static_cast<std::uint32_t>(std::max(aWidth, 0)), static_cast<std::uint32_t>(std::max(aHeight, 0)) } };
        iDynamicStateValid = false;
    }

    void vulkan_graphics_backend::enable_multisample(bool aEnable)
    {
        // n.b. a multisample target is always rasterized with all its samples (there is no Vulkan equivalent of disabling
        // GL_MULTISAMPLE)
        iState.multisample = aEnable;
    }

    void vulkan_graphics_backend::set_sample_shading(std::optional<double> const& aSampleShadingRate)
    {
        iState.sampleShading = aSampleShadingRate;
    }

    void vulkan_graphics_backend::set_front_face(neogfx::front_face aFrontFace)
    {
        iState.frontFace = aFrontFace;
        iDynamicStateValid = false;
    }

    void vulkan_graphics_backend::set_face_culling(neogfx::face_culling aCulling, bool aFlipped)
    {
        iState.faceCulling = aCulling;
        iState.faceCullingFlipped = aFlipped;
        iDynamicStateValid = false;
    }

    void vulkan_graphics_backend::set_blending_mode(neogfx::blending_mode aBlendingMode)
    {
        // n.b. as glDisable(GL_BLEND): the blend function is kept
        if (aBlendingMode == neogfx::blending_mode::None)
        {
            iState.blending = false;
            return;
        }
        iState.blending = true;
        iState.blendingMode = aBlendingMode;
        iState.xorBlending = false;
    }

    void vulkan_graphics_backend::set_xor_blending()
    {
        iState.blending = true;
        iState.xorBlending = true;
    }

    void vulkan_graphics_backend::enable_smoothing(bool)
    {
        // n.b. no Vulkan equivalent of GL_LINE_SMOOTH and GL_POLYGON_SMOOTH (the shaders anti-alias)
    }

    void vulkan_graphics_backend::clear(color const& aColor)
    {
        if (iColor == nullptr || !iState.colorWrite)
            return;
        begin_rendering();
        VkClearAttachment attachment{ VK_IMAGE_ASPECT_COLOR_BIT, 0u };
        attachment.clearValue.color = VkClearColorValue{ { aColor.red<float>(), aColor.green<float>(), aColor.blue<float>(), aColor.alpha<float>() } };
        auto const extent = target_extent();
        VkClearRect clearRect{ { { 0, 0 }, extent }, 0u, 1u };
        if (iState.scissorEnabled)
        {
            std::int64_t const x0 = std::clamp<std::int64_t>(iState.scissor.offset.x, 0, extent.width);
            std::int64_t const y0 = std::clamp<std::int64_t>(iState.scissor.offset.y, 0, extent.height);
            std::int64_t const x1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor.offset.x) + iState.scissor.extent.width, 0, extent.width);
            std::int64_t const y1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor.offset.y) + iState.scissor.extent.height, 0, extent.height);
            if (x1 <= x0 || y1 <= y0)
                return;
            clearRect.rect = VkRect2D{ { static_cast<std::int32_t>(x0), static_cast<std::int32_t>(y0) },
                { static_cast<std::uint32_t>(x1 - x0), static_cast<std::uint32_t>(y1 - y0) } };
        }
        vkCmdClearAttachments(command_buffer(), 1u, &attachment, 1u, &clearRect);
    }

    void vulkan_graphics_backend::clear_depth_buffer()
    {
        if (iDepthStencil == nullptr)
            return;
        begin_rendering();
        VkClearAttachment attachment{ VK_IMAGE_ASPECT_DEPTH_BIT, 0u };
        attachment.clearValue.depthStencil = VkClearDepthStencilValue{ 1.0f, 0u };
        auto const extent = target_extent();
        VkClearRect clearRect{ { { 0, 0 }, extent }, 0u, 1u };
        if (iState.scissorEnabled)
        {
            std::int64_t const x0 = std::clamp<std::int64_t>(iState.scissor.offset.x, 0, extent.width);
            std::int64_t const y0 = std::clamp<std::int64_t>(iState.scissor.offset.y, 0, extent.height);
            std::int64_t const x1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor.offset.x) + iState.scissor.extent.width, 0, extent.width);
            std::int64_t const y1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor.offset.y) + iState.scissor.extent.height, 0, extent.height);
            if (x1 <= x0 || y1 <= y0)
                return;
            clearRect.rect = VkRect2D{ { static_cast<std::int32_t>(x0), static_cast<std::int32_t>(y0) },
                { static_cast<std::uint32_t>(x1 - x0), static_cast<std::uint32_t>(y1 - y0) } };
        }
        vkCmdClearAttachments(command_buffer(), 1u, &attachment, 1u, &clearRect);
    }

    void vulkan_graphics_backend::clear_stencil_buffer(std::int32_t aValue)
    {
        // n.b. as glStencilMask(0xFF) then glClear(GL_STENCIL_BUFFER_BIT)
        iState.stencilWriteMask = 0xFFu;
        iDynamicStateValid = false;
        if (iDepthStencil == nullptr || !has_stencil(iDepthStencil->format))
            return;
        begin_rendering();
        VkClearAttachment attachment{ VK_IMAGE_ASPECT_STENCIL_BIT, 0u };
        attachment.clearValue.depthStencil = VkClearDepthStencilValue{ 1.0f, static_cast<std::uint32_t>(aValue) };
        auto const extent = target_extent();
        VkClearRect clearRect{ { { 0, 0 }, extent }, 0u, 1u };
        if (iState.scissorEnabled)
        {
            std::int64_t const x0 = std::clamp<std::int64_t>(iState.scissor.offset.x, 0, extent.width);
            std::int64_t const y0 = std::clamp<std::int64_t>(iState.scissor.offset.y, 0, extent.height);
            std::int64_t const x1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor.offset.x) + iState.scissor.extent.width, 0, extent.width);
            std::int64_t const y1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor.offset.y) + iState.scissor.extent.height, 0, extent.height);
            if (x1 <= x0 || y1 <= y0)
                return;
            clearRect.rect = VkRect2D{ { static_cast<std::int32_t>(x0), static_cast<std::int32_t>(y0) },
                { static_cast<std::uint32_t>(x1 - x0), static_cast<std::uint32_t>(y1 - y0) } };
        }
        vkCmdClearAttachments(command_buffer(), 1u, &attachment, 1u, &clearRect);
    }

    void vulkan_graphics_backend::apply_stencil(bool aEnabled, bool aUpdating, std::int32_t aRef)
    {
        // n.b. as opengl_graphics_backend::apply_stencil
        if (aEnabled)
        {
            iState.stencilTest = true;
            iState.stencilReference = static_cast<std::uint32_t>(aRef);
            iState.stencilCompareMask = 0xFFu;
            if (aUpdating)
            {
                iState.colorWrite = false;
                iState.depthWrite = false;
                iState.stencilCompare = VK_COMPARE_OP_ALWAYS;
                iState.stencilPassOp = VK_STENCIL_OP_REPLACE;
                iState.stencilWriteMask = 0xFFu;
            }
            else
            {
                iState.colorWrite = true;
                iState.depthWrite = true;
                iState.stencilCompare = VK_COMPARE_OP_EQUAL;
                iState.stencilPassOp = VK_STENCIL_OP_KEEP;
                iState.stencilWriteMask = 0x00u;
            }
        }
        else
            iState.stencilTest = false;
        iDynamicStateValid = false;
    }

    bool vulkan_graphics_backend::depth_test_enabled() const
    {
        return iState.depthTest;
    }

    void vulkan_graphics_backend::enable_depth_test(bool aEnable)
    {
        iState.depthTest = aEnable;
        iDynamicStateValid = false;
    }

    void vulkan_graphics_backend::set_texture_filter(i_texture const& aTexture, texture_sampling aSampling)
    {
        // n.b. as glTexParameteri(GL_TEXTURE_2D, ...): rectangle (data) and multisample textures are unaffected
        auto const textureSampling = aTexture.sampling();
        if (textureSampling == texture_sampling::Data || textureSampling == texture_sampling::Multisample)
            return;
        auto* image = image_of(aTexture);
        if (image == nullptr)
            return;
        image->sampler.magLinear = (aSampling != texture_sampling::Nearest && aSampling != texture_sampling::Data);
        image->sampler.minLinear = image->sampler.magLinear;
        image->sampler.mipmap = (aSampling == texture_sampling::NormalMipmap);
    }

    void vulkan_graphics_backend::set_texture_linear_clamp(i_texture const& aTexture)
    {
        auto* image = image_of(aTexture);
        if (image == nullptr)
            return;
        image->sampler.magLinear = true;
        image->sampler.minLinear = true;
        image->sampler.mipmap = false;
        image->sampler.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }

    void vulkan_graphics_backend::set_active_texture_unit(std::uint32_t)
    {
        // nothing to do
    }

    void vulkan_graphics_backend::unbind_texture_unit(std::uint32_t aTextureUnit)
    {
        bind_texture(aTextureUnit, nullptr);
    }

    VkSampler vulkan_graphics_backend::sampler(vulkan_sampler_state const& aState, std::uint32_t aMipLevels)
    {
        bool const mipmapped = aState.mipmap && aMipLevels > 1u;
        auto const key = std::make_pair(aState, mipmapped ? 1u : 0u);
        auto existing = iSamplers.find(key);
        if (existing != iSamplers.end())
            return existing->second;
        VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = aState.magLinear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        samplerInfo.minFilter = aState.minLinear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        samplerInfo.mipmapMode = mipmapped ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.addressModeU = aState.addressMode;
        samplerInfo.addressModeV = aState.addressMode;
        samplerInfo.addressModeW = aState.addressMode;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerInfo.minLod = 0.0f;
        // n.b. as GL_LINEAR/GL_NEAREST minification: the base level only
        samplerInfo.maxLod = mipmapped ? VK_LOD_CLAMP_NONE : 0.25f;
        VkSampler result = VK_NULL_HANDLE;
        vkCheck(vkCreateSampler(iDevice, &samplerInfo, nullptr, &result));
        iSamplers.emplace(key, result);
        return result;
    }

    bool vulkan_graphics_backend::alive() const
    {
        return iDevice != VK_NULL_HANDLE;
    }

    VkInstance vulkan_graphics_backend::vk_instance() const
    {
        return iInstance;
    }

    VkPhysicalDevice vulkan_graphics_backend::physical_device() const
    {
        return iPhysicalDevice;
    }

    VkDevice vulkan_graphics_backend::device() const
    {
        return iDevice;
    }

    VkFormat vulkan_graphics_backend::depth_stencil_format() const
    {
        return iDepthStencilFormat;
    }

    VkSampleCountFlagBits vulkan_graphics_backend::sample_count(std::uint32_t aSamples) const
    {
        auto const supported = iProperties.limits.framebufferColorSampleCounts & iProperties.limits.framebufferDepthSampleCounts &
            iProperties.limits.framebufferStencilSampleCounts & iProperties.limits.sampledImageColorSampleCounts;
        for (std::uint32_t samples = std::min(aSamples, 64u); samples > 1u; samples /= 2u)
            if ((supported & samples) != 0u)
                return static_cast<VkSampleCountFlagBits>(samples);
        return VK_SAMPLE_COUNT_1_BIT;
    }

    bool vulkan_graphics_backend::format_supports(VkFormat aFormat, VkFormatFeatureFlags aFeatures) const
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(iPhysicalDevice, aFormat, &properties);
        return (properties.optimalTilingFeatures & aFeatures) == aFeatures;
    }

    std::unique_ptr<vulkan_image> vulkan_graphics_backend::create_image(std::uint32_t aWidth, std::uint32_t aHeight, VkFormat aFormat, std::uint32_t aTexelSize,
        VkSampleCountFlagBits aSamples, std::uint32_t aMipLevels)
    {
        auto result = std::make_unique<vulkan_image>();
        result->format = aFormat;
        result->aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        result->samples = aSamples;
        result->width = std::max(aWidth, 1u);
        result->height = std::max(aHeight, 1u);
        result->mipLevels = (aSamples == VK_SAMPLE_COUNT_1_BIT ? std::max(aMipLevels, 1u) : 1u);
        result->texelSize = aTexelSize;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (format_supports(aFormat, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = aFormat;
        imageInfo.extent = VkExtent3D{ result->width, result->height, 1u };
        imageInfo.mipLevels = result->mipLevels;
        imageInfo.arrayLayers = 1u;
        imageInfo.samples = aSamples;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkCheck(vkCreateImage(iDevice, &imageInfo, nullptr, &result->image));
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(iDevice, result->image, &requirements);
        VkMemoryAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vkCheck(vkAllocateMemory(iDevice, &allocateInfo, nullptr, &result->memory));
        vkCheck(vkBindImageMemory(iDevice, result->image, result->memory, 0u));
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = result->image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = aFormat;
        viewInfo.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, result->mipLevels, 0u, 1u };
        vkCheck(vkCreateImageView(iDevice, &viewInfo, nullptr, &result->view));
        // to the general layout (in which it stays) and cleared (cf. a new OpenGL texture's storage, which is zeroed in practice)
        end_rendering();
        image_barrier(result->image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        VkClearColorValue const clearColor = {};
        VkImageSubresourceRange const range{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, result->mipLevels, 0u, 1u };
        vkCmdClearColorImage(command_buffer(), result->image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1u, &range);
        return result;
    }

    std::unique_ptr<vulkan_image> vulkan_graphics_backend::create_depth_stencil_image(std::uint32_t aWidth, std::uint32_t aHeight, VkSampleCountFlagBits aSamples)
    {
        auto result = std::make_unique<vulkan_image>();
        result->format = iDepthStencilFormat;
        result->aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        result->samples = aSamples;
        result->width = std::max(aWidth, 1u);
        result->height = std::max(aHeight, 1u);
        result->texelSize = 4u;
        VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = iDepthStencilFormat;
        imageInfo.extent = VkExtent3D{ result->width, result->height, 1u };
        imageInfo.mipLevels = 1u;
        imageInfo.arrayLayers = 1u;
        imageInfo.samples = aSamples;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkCheck(vkCreateImage(iDevice, &imageInfo, nullptr, &result->image));
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(iDevice, result->image, &requirements);
        VkMemoryAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vkCheck(vkAllocateMemory(iDevice, &allocateInfo, nullptr, &result->memory));
        vkCheck(vkBindImageMemory(iDevice, result->image, result->memory, 0u));
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = result->image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = iDepthStencilFormat;
        viewInfo.subresourceRange = VkImageSubresourceRange{ result->aspect, 0u, 1u, 0u, 1u };
        vkCheck(vkCreateImageView(iDevice, &viewInfo, nullptr, &result->view));
        end_rendering();
        image_barrier(result->image, result->aspect, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        VkClearDepthStencilValue const clearValue{ 1.0f, 0u };
        VkImageSubresourceRange const range{ result->aspect, 0u, 1u, 0u, 1u };
        vkCmdClearDepthStencilImage(command_buffer(), result->image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1u, &range);
        return result;
    }

    void vulkan_graphics_backend::destroy_image(std::unique_ptr<vulkan_image>& aImage)
    {
        if (!aImage)
            return;
        if (!alive())
        {
            aImage = nullptr;
            return;
        }
        if (iColor == aImage.get() || iDepthStencil == aImage.get())
        {
            end_rendering();
            iColor = nullptr;
            iDepthStencil = nullptr;
        }
        for (auto& unit : iTextureUnits)
            if (unit == aImage.get())
                unit = nullptr;
        defer([device = iDevice, view = aImage->view, image = aImage->image, memory = aImage->memory]()
            {
                vkDestroyImageView(device, view, nullptr);
                vkDestroyImage(device, image, nullptr);
                vkFreeMemory(device, memory, nullptr);
            });
        aImage = nullptr;
    }

    void vulkan_graphics_backend::upload_image(vulkan_image& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight,
        void const* aData, std::size_t aRowPitch)
    {
        if (aWidth == 0u || aHeight == 0u)
            return;
        auto const rowSize = static_cast<std::size_t>(aWidth) * aImage.texelSize;
        ++iStatistics.uploads;
        iStatistics.uploadBytes += rowSize * aHeight;
        auto const staging = allocate_transient(rowSize * aHeight, 16u);
        auto const* source = static_cast<std::uint8_t const*>(aData);
        auto* destination = static_cast<std::uint8_t*>(staging.mapping);
        for (std::uint32_t row = 0u; row < aHeight; ++row)
            std::memcpy(destination + row * rowSize, source + row * aRowPitch, rowSize);
        end_rendering();
        memory_barrier();
        VkBufferImageCopy region{};
        region.bufferOffset = staging.offset;
        region.imageSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u };
        region.imageOffset = VkOffset3D{ static_cast<std::int32_t>(aX), static_cast<std::int32_t>(aY), 0 };
        region.imageExtent = VkExtent3D{ aWidth, aHeight, 1u };
        vkCmdCopyBufferToImage(command_buffer(), staging.buffer, aImage.image, VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
        if (aImage.mipLevels > 1u)
            generate_mipmaps(aImage);
    }

    void vulkan_graphics_backend::clear_image(vulkan_image& aImage, std::array<float, 4> const& aColor)
    {
        end_rendering();
        memory_barrier();
        VkClearColorValue clearColor;
        std::copy(aColor.begin(), aColor.end(), clearColor.float32);
        VkImageSubresourceRange const range{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, aImage.mipLevels, 0u, 1u };
        vkCmdClearColorImage(command_buffer(), aImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1u, &range);
        if (aImage.mipLevels > 1u)
            generate_mipmaps(aImage);
    }

    void vulkan_graphics_backend::generate_mipmaps(vulkan_image& aImage)
    {
        end_rendering();
        auto const filter = format_supports(aImage.format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        auto const commandBuffer = command_buffer();
        std::int32_t width = static_cast<std::int32_t>(aImage.width);
        std::int32_t height = static_cast<std::int32_t>(aImage.height);
        for (std::uint32_t level = 1u; level < aImage.mipLevels; ++level)
        {
            memory_barrier();
            auto const nextWidth = std::max(width / 2, 1);
            auto const nextHeight = std::max(height / 2, 1);
            VkImageBlit blit{};
            blit.srcSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT, level - 1u, 0u, 1u };
            blit.srcOffsets[1] = VkOffset3D{ width, height, 1 };
            blit.dstSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT, level, 0u, 1u };
            blit.dstOffsets[1] = VkOffset3D{ nextWidth, nextHeight, 1 };
            vkCmdBlitImage(commandBuffer, aImage.image, VK_IMAGE_LAYOUT_GENERAL, aImage.image, VK_IMAGE_LAYOUT_GENERAL, 1u, &blit, filter);
            width = nextWidth;
            height = nextHeight;
        }
    }

    void vulkan_graphics_backend::read_image(vulkan_image const& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight, void* aData)
    {
        ++iStatistics.readbacks;
        if (aWidth == 0u || aHeight == 0u)
            return;
        auto const size = static_cast<VkDeviceSize>(aWidth) * aHeight * aImage.texelSize;
        auto readBack = allocate_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);
        end_rendering();
        memory_barrier();
        VkBufferImageCopy region{};
        region.imageSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u };
        region.imageOffset = VkOffset3D{ static_cast<std::int32_t>(aX), static_cast<std::int32_t>(aY), 0 };
        region.imageExtent = VkExtent3D{ aWidth, aHeight, 1u };
        vkCmdCopyImageToBuffer(command_buffer(), aImage.image, VK_IMAGE_LAYOUT_GENERAL, readBack.buffer, 1u, &region);
        memory_barrier();
        submit();
        wait_for_frames();
        std::memcpy(aData, readBack.mapping, static_cast<std::size_t>(size));
        free_buffer(readBack);
    }

    void vulkan_graphics_backend::set_render_target(vulkan_image* aColor, vulkan_image* aDepthStencil)
    {
        if (iColor == aColor && iDepthStencil == aDepthStencil)
            return;
        end_rendering();
        iColor = aColor;
        iDepthStencil = aDepthStencil;
        iDynamicStateValid = false;
    }

    void vulkan_graphics_backend::release_render_target(vulkan_image const* aColor)
    {
        if (iColor != aColor)
            return;
        end_rendering();
        iColor = nullptr;
        iDepthStencil = nullptr;
    }

    void vulkan_graphics_backend::set_viewport(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight)
    {
        iState.viewport = VkViewport{ static_cast<float>(aX), static_cast<float>(aY), static_cast<float>(aWidth), static_cast<float>(aHeight), 0.0f, 1.0f };
        iDynamicStateValid = false;
    }

    void vulkan_graphics_backend::enable_blending(bool aEnable)
    {
        iState.blending = aEnable;
    }

    void vulkan_graphics_backend::set_depth_compare(VkCompareOp aCompareOp)
    {
        iState.depthCompare = aCompareOp;
        iDynamicStateValid = false;
    }

    void vulkan_graphics_backend::bind_texture(std::uint32_t aTextureUnit, vulkan_image const* aImage)
    {
        if (aTextureUnit < iTextureUnits.size())
            iTextureUnits[aTextureUnit] = aImage;
    }

    vulkan_image const* vulkan_graphics_backend::bound_texture(std::uint32_t aTextureUnit) const
    {
        if (aTextureUnit < iTextureUnits.size())
            return iTextureUnits[aTextureUnit];
        return nullptr;
    }

    void vulkan_graphics_backend::use_program(vulkan_program* aProgram)
    {
        iProgram = aProgram;
    }

    std::vector<std::uint32_t> vulkan_graphics_backend::compile_shader(shader_type aType, std::string const& aSource, std::string const& aName, bool aOptimize) const
    {
        shaderc_shader_kind kind;
        switch (aType)
        {
        case shader_type::Compute:
            kind = shaderc_glsl_compute_shader;
            break;
        case shader_type::Vertex:
        default:
            kind = shaderc_glsl_vertex_shader;
            break;
        case shader_type::TessellationControl:
            kind = shaderc_glsl_tess_control_shader;
            break;
        case shader_type::TessellationEvaluation:
            kind = shaderc_glsl_tess_evaluation_shader;
            break;
        case shader_type::Geometry:
            kind = shaderc_glsl_geometry_shader;
            break;
        case shader_type::Fragment:
            kind = shaderc_glsl_fragment_shader;
            break;
        }
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetSourceLanguage(shaderc_source_language_glsl);
        // n.b. optimizing strips the names that reflection needs (see basic_vulkan_shader_program::compile)
        options.SetOptimizationLevel(aOptimize ? shaderc_optimization_level_performance : shaderc_optimization_level_zero);
        auto const result = compiler.CompileGlslToSpv(aSource, kind, aName.c_str(), options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            auto const& errMsg = result.GetErrorMessage();
            std::cerr << "neogfx::vulkan_graphics_backend::compile_shader::error: " << errMsg << std::endl;
            throw failed_to_create_shader_program(errMsg);
        }
        return std::vector<std::uint32_t>{ result.cbegin(), result.cend() };
    }

    VkShaderModule vulkan_graphics_backend::create_shader_module(std::vector<std::uint32_t> const& aSpirv)
    {
        VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        moduleInfo.codeSize = aSpirv.size() * sizeof(std::uint32_t);
        moduleInfo.pCode = aSpirv.data();
        VkShaderModule result = VK_NULL_HANDLE;
        vkCheck(vkCreateShaderModule(iDevice, &moduleInfo, nullptr, &result));
        return result;
    }

    void vulkan_graphics_backend::destroy_shader_module(VkShaderModule aModule)
    {
        if (aModule == VK_NULL_HANDLE || !alive())
            return;
        defer([device = iDevice, aModule]() { vkDestroyShaderModule(device, aModule, nullptr); });
    }

    void vulkan_graphics_backend::release_program(vulkan_program& aProgram)
    {
        if (iBoundPipeline != VK_NULL_HANDLE)
            for (auto const& p : aProgram.pipelines)
                if (p.second == iBoundPipeline)
                    iBoundPipeline = VK_NULL_HANDLE;
        auto pipelines = std::move(aProgram.pipelines);
        aProgram.pipelines.clear();
        defer([device = iDevice, pipelines = std::move(pipelines), pipelineLayout = aProgram.pipelineLayout, descriptorSetLayout = aProgram.descriptorSetLayout]()
            {
                for (auto const& p : pipelines)
                    vkDestroyPipeline(device, p.second, nullptr);
                vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
                vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
            });
        aProgram.pipelineLayout = VK_NULL_HANDLE;
        aProgram.descriptorSetLayout = VK_NULL_HANDLE;
        aProgram.linked = false;
    }

    bool vulkan_graphics_backend::vsync() const
    {
        return iVsync;
    }

    void vulkan_graphics_backend::set_vsync(bool aEnable)
    {
        if (iVsync != aEnable)
        {
            iVsync = aEnable;
            ++iVsyncGeneration;
        }
    }

    void vulkan_graphics_backend::create_surface_swapchain(vulkan_swapchain& aSwapchain, std::uint32_t aWidth, std::uint32_t aHeight)
    {
        ++iStatistics.swapchainsCreated;
        execute();
        vkDeviceWaitIdle(iDevice);

        VkBool32 presentSupported = VK_FALSE;
        vkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(iPhysicalDevice, iQueueFamily, aSwapchain.surface, &presentSupported));
        if (!presentSupported)
            throw vk_error("queue family cannot present to surface");

        VkSurfaceCapabilitiesKHR capabilities;
        vkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(iPhysicalDevice, aSwapchain.surface, &capabilities));
        VkExtent2D extent = capabilities.currentExtent;
        if (extent.width == 0xFFFFFFFFu)
            extent = VkExtent2D{
                std::clamp(aWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp(aHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height) };
        aSwapchain.extent = extent;
        aSwapchain.vsyncGeneration = iVsyncGeneration;
        aSwapchain.outOfDate = false;
        if (extent.width == 0u || extent.height == 0u)
            return; // minimized
        if ((capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0u)
            throw vk_error("swapchain images cannot be copied to");

        std::uint32_t formatCount = 0u;
        vkGetPhysicalDeviceSurfaceFormatsKHR(iPhysicalDevice, aSwapchain.surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(iPhysicalDevice, aSwapchain.surface, &formatCount, formats.data());
        if (formats.empty())
            throw vk_error("no surface formats");
        // n.b. not an sRGB format: as the OpenGL default framebuffer, what is drawn is already sRGB encoded
        VkSurfaceFormatKHR surfaceFormat = formats[0];
        for (auto const& f : formats)
            if ((f.format == VK_FORMAT_B8G8R8A8_UNORM || f.format == VK_FORMAT_R8G8B8A8_UNORM) && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = f;
                break;
            }

        std::uint32_t presentModeCount = 0u;
        vkGetPhysicalDeviceSurfacePresentModesKHR(iPhysicalDevice, aSwapchain.surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(iPhysicalDevice, aSwapchain.surface, &presentModeCount, presentModes.data());
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!iVsync)
        {
            // n.b. as OpenGL with a swap interval of 0: presented immediately (mailbox shows at most one frame per refresh)
            if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end())
                presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            else if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }

        std::uint32_t imageCount = capabilities.minImageCount + 1u;
        if (capabilities.maxImageCount != 0u)
            imageCount = std::min(imageCount, capabilities.maxImageCount);
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        for (auto alpha : { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR })
            if ((capabilities.supportedCompositeAlpha & alpha) != 0u)
            {
                compositeAlpha = alpha;
                break;
            }

        VkSwapchainCreateInfoKHR swapchainInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        swapchainInfo.surface = aSwapchain.surface;
        swapchainInfo.minImageCount = imageCount;
        swapchainInfo.imageFormat = surfaceFormat.format;
        swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainInfo.imageExtent = extent;
        swapchainInfo.imageArrayLayers = 1u;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.preTransform = capabilities.currentTransform;
        swapchainInfo.compositeAlpha = compositeAlpha;
        swapchainInfo.presentMode = presentMode;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.oldSwapchain = aSwapchain.swapchain;
        VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
        vkCheck(vkCreateSwapchainKHR(iDevice, &swapchainInfo, nullptr, &newSwapchain));
        if (aSwapchain.swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(iDevice, aSwapchain.swapchain, nullptr);
        aSwapchain.swapchain = newSwapchain;
        aSwapchain.format = surfaceFormat.format;

        std::uint32_t swapchainImageCount = 0u;
        vkGetSwapchainImagesKHR(iDevice, aSwapchain.swapchain, &swapchainImageCount, nullptr);
        aSwapchain.images.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(iDevice, aSwapchain.swapchain, &swapchainImageCount, aSwapchain.images.data());
        for (auto semaphore : aSwapchain.renderingFinished)
            vkDestroySemaphore(iDevice, semaphore, nullptr);
        aSwapchain.renderingFinished.assign(swapchainImageCount, VK_NULL_HANDLE);
        VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        for (auto& semaphore : aSwapchain.renderingFinished)
            vkCheck(vkCreateSemaphore(iDevice, &semaphoreInfo, nullptr, &semaphore));
        if (aSwapchain.imageAcquired.empty())
        {
            aSwapchain.imageAcquired.assign(FramesInFlight, VK_NULL_HANDLE);
            for (auto& semaphore : aSwapchain.imageAcquired)
                vkCheck(vkCreateSemaphore(iDevice, &semaphoreInfo, nullptr, &semaphore));
        }
    }

    void vulkan_graphics_backend::destroy_swapchain(vulkan_swapchain& aSwapchain, bool aDestroySurface)
    {
        if (!alive())
            return;
        execute();
        vkDeviceWaitIdle(iDevice);
        destroy_image(aSwapchain.resolved);
        execute();
        for (auto semaphore : aSwapchain.renderingFinished)
            vkDestroySemaphore(iDevice, semaphore, nullptr);
        aSwapchain.renderingFinished.clear();
        for (auto semaphore : aSwapchain.imageAcquired)
            vkDestroySemaphore(iDevice, semaphore, nullptr);
        aSwapchain.imageAcquired.clear();
        if (aSwapchain.swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(iDevice, aSwapchain.swapchain, nullptr);
        aSwapchain.swapchain = VK_NULL_HANDLE;
        aSwapchain.images.clear();
        if (aDestroySurface && aSwapchain.surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(iInstance, aSwapchain.surface, nullptr);
            aSwapchain.surface = VK_NULL_HANDLE;
        }
    }

    void vulkan_graphics_backend::present(vulkan_swapchain& aSwapchain, vulkan_image& aSource, std::uint32_t aWidth, std::uint32_t aHeight)
    {
        if (!alive())
            return;
        auto const presentStart = std::chrono::steady_clock::now();
        struct present_statistics
        {
            vulkan_graphics_backend& backend;
            std::chrono::steady_clock::time_point start;
            ~present_statistics()
            {
                auto& stats = backend.iStatistics;
                auto const now = std::chrono::steady_clock::now();
                stats.present += now - start;
                stats.frameBackend += now - start;
                ++stats.frames;
                if (stats.lastPresentEnd != std::chrono::steady_clock::time_point{})
                {
                    auto const frame = now - stats.lastPresentEnd;
                    if (frame > stats.longestFrame)
                    {
                        stats.longestFrame = frame;
                        stats.longestFrameBackend = stats.frameBackend;
                    }
                    if (frame > std::chrono::milliseconds{ 50 })
                        ++stats.longFrames;
                }
                stats.lastPresentEnd = now;
                stats.frameBackend = {};
                backend.report_statistics();
            }
        } presentStatistics{ *this, presentStart };
        aWidth = std::min(aWidth, aSource.width);
        aHeight = std::min(aHeight, aSource.height);
        if (aSwapchain.swapchain == VK_NULL_HANDLE || aSwapchain.outOfDate || aSwapchain.vsyncGeneration != iVsyncGeneration ||
            aSwapchain.extent.width != aWidth || aSwapchain.extent.height != aHeight)
            create_surface_swapchain(aSwapchain, aWidth, aHeight);
        if (aSwapchain.swapchain == VK_NULL_HANDLE || aSwapchain.extent.width == 0u || aSwapchain.extent.height == 0u)
        {
            execute();
            return;
        }

        // n.b. recording (so this frame's previous use, and so its wait on its acquire semaphore, is done)
        command_buffer();
        auto const imageAcquired = aSwapchain.imageAcquired[iFrame];
        std::uint32_t imageIndex = 0u;
        auto const acquireStart = std::chrono::steady_clock::now();
        auto result = vkAcquireNextImageKHR(iDevice, aSwapchain.swapchain, UINT64_MAX, imageAcquired, VK_NULL_HANDLE, &imageIndex);
        iStatistics.acquire += std::chrono::steady_clock::now() - acquireStart;
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // n.b. this frame is dropped; the swapchain is recreated for the next
            aSwapchain.outOfDate = true;
            execute();
            return;
        }
        if (result != VK_SUBOPTIMAL_KHR)
            vkCheck(result);

        auto const width = std::min(aSwapchain.extent.width, aWidth);
        auto const height = std::min(aSwapchain.extent.height, aHeight);
        end_rendering();
        memory_barrier();
        auto const commandBuffer = command_buffer();
        vulkan_image* source = &aSource;
        if (aSource.samples != VK_SAMPLE_COUNT_1_BIT)
        {
            if (!aSwapchain.resolved || aSwapchain.resolved->format != aSource.format ||
                aSwapchain.resolved->width < width || aSwapchain.resolved->height < height)
            {
                destroy_image(aSwapchain.resolved);
                aSwapchain.resolved = create_image(aSource.width, aSource.height, aSource.format, aSource.texelSize);
                memory_barrier();
            }
            VkImageResolve region{};
            region.srcSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u };
            region.dstSubresource = region.srcSubresource;
            region.extent = VkExtent3D{ width, height, 1u };
            vkCmdResolveImage(commandBuffer, aSource.image, VK_IMAGE_LAYOUT_GENERAL, aSwapchain.resolved->image, VK_IMAGE_LAYOUT_GENERAL, 1u, &region);
            memory_barrier();
            source = aSwapchain.resolved.get();
        }
        auto const swapchainImage = aSwapchain.images[imageIndex];
        image_barrier(swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // n.b. flipped: the image is as OpenGL would have it (bottom row first)
        VkImageBlit blit{};
        blit.srcSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u };
        blit.srcOffsets[0] = VkOffset3D{ 0, 0, 0 };
        blit.srcOffsets[1] = VkOffset3D{ static_cast<std::int32_t>(width), static_cast<std::int32_t>(height), 1 };
        blit.dstSubresource = blit.srcSubresource;
        blit.dstOffsets[0] = VkOffset3D{ 0, static_cast<std::int32_t>(height), 0 };
        blit.dstOffsets[1] = VkOffset3D{ static_cast<std::int32_t>(width), 0, 1 };
        vkCmdBlitImage(commandBuffer, source->image, VK_IMAGE_LAYOUT_GENERAL, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_NEAREST);
        image_barrier(swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        submit(imageAcquired, aSwapchain.renderingFinished[imageIndex]);

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1u;
        presentInfo.pWaitSemaphores = &aSwapchain.renderingFinished[imageIndex];
        presentInfo.swapchainCount = 1u;
        presentInfo.pSwapchains = &aSwapchain.swapchain;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(iQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            aSwapchain.outOfDate = true;
        else
            vkCheck(result);
    }

    void vulkan_graphics_backend::report_statistics()
    {
        if (!iStatistics.enabled)
            return;
        auto const now = std::chrono::steady_clock::now();
        auto const period = now - iStatistics.periodStart;
        if (period < std::chrono::seconds{ 1 } || iStatistics.frames == 0u)
            return;
        auto const frames = static_cast<double>(iStatistics.frames);
        auto const ms = [&](std::chrono::steady_clock::duration aDuration)
        {
            return std::chrono::duration<double, std::milli>{ aDuration }.count() / frames;
        };
        auto const perFrame = [&](std::uint64_t aCount) { return static_cast<double>(aCount) / frames; };
        std::cerr << std::fixed << std::setprecision(2) <<
            "neogfx::vulkan_graphics_backend: " << frames / std::chrono::duration<double>{ period }.count() << " fps, per frame: " <<
            ms(period) << " ms total, " <<
            ms(iStatistics.fenceWait) << " ms fence wait, " <<
            ms(iStatistics.present) << " ms present (" << ms(iStatistics.acquire) << " ms acquire), " <<
            perFrame(iStatistics.submits) << " submits (" << perFrame(iStatistics.executes) << " execute, " << perFrame(iStatistics.readbacks) << " readback), " <<
            perFrame(iStatistics.passes) << " passes, " <<
            perFrame(iStatistics.barriers) << " barriers, " <<
            perFrame(iStatistics.draws) << " draws, " <<
            perFrame(iStatistics.pipelineBinds) << " pipeline binds, " <<
            perFrame(iStatistics.descriptorPushes) << " descriptor pushes, " <<
            perFrame(iStatistics.uploads) << " uploads (" << perFrame(iStatistics.uploadBytes) / 1024.0 << " KiB), " <<
            perFrame(iStatistics.transientBytes) / 1024.0 << " KiB transient; " <<
            perFrame(iStatistics.discards) << " buffer discards, " <<
            iStatistics.bufferCopies << " buffer copies (" << static_cast<double>(iStatistics.bufferCopyBytes) / 1024.0 << " KiB), " <<
            iStatistics.allocations << " allocations (" << std::chrono::duration<double, std::milli>{ iStatistics.allocationTime }.count() << " ms); " <<
            "longest frame " << std::chrono::duration<double, std::milli>{ iStatistics.longestFrame }.count() << " ms (" <<
            std::chrono::duration<double, std::milli>{ iStatistics.longestFrameBackend }.count() << " ms in backend), " <<
            iStatistics.longFrames << " frames over 50 ms; " <<
            iStatistics.pipelinesCreated << " pipelines created, " <<
            iStatistics.swapchainsCreated << " swapchains created" << std::endl;
        auto const enabled = iStatistics.enabled;
        auto const lastPresentEnd = iStatistics.lastPresentEnd;
        iStatistics = statistics{};
        iStatistics.enabled = enabled;
        iStatistics.periodStart = now;
        iStatistics.lastPresentEnd = lastPresentEnd;
    }

    vulkan_graphics_backend::shadow_resources& vulkan_graphics_backend::shadows(i_standard_shader_program& aProgram)
    {
        if (iShadows)
            return *iShadows;
        iShadows = std::make_unique<shadow_resources>();
        auto& resources = *iShadows;
        try
        {
            resources.matricesBinding = StorageBindingBase + static_cast<std::uint32_t>(aProgram.model_matrices().id());
            resources.tableBinding = StorageBindingBase + static_cast<std::uint32_t>(aProgram.model_table().id());
            std::string vertexSource = sShadowVertexShader;
            auto const replace = [&](std::string const& aWhat, std::string const& aWith)
                {
                    vertexSource.replace(vertexSource.find(aWhat), aWhat.size(), aWith);
                };
            replace("%MATRICES%", std::to_string(resources.matricesBinding));
            replace("%TABLE%", std::to_string(resources.tableBinding));
            resources.vertex = create_shader_module(compile_shader(shader_type::Vertex, vertexSource, "neogfx::shadow_map.vert", true));
            resources.fragment = create_shader_module(compile_shader(shader_type::Fragment, sShadowFragmentShader, "neogfx::shadow_map.frag", true));

            VkFormat const atlasFormat = VK_FORMAT_D32_SFLOAT;
            auto atlas = std::make_unique<vulkan_image>();
            atlas->format = atlasFormat;
            atlas->aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            atlas->width = static_cast<std::uint32_t>(scene_shadow_atlas::AtlasSize);
            atlas->height = static_cast<std::uint32_t>(scene_shadow_atlas::AtlasSize);
            atlas->sampler = vulkan_sampler_state{ false, false, false, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };
            VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = atlasFormat;
            imageInfo.extent = VkExtent3D{ atlas->width, atlas->height, 1u };
            imageInfo.mipLevels = 1u;
            imageInfo.arrayLayers = 1u;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            vkCheck(vkCreateImage(iDevice, &imageInfo, nullptr, &atlas->image));
            VkMemoryRequirements requirements;
            vkGetImageMemoryRequirements(iDevice, atlas->image, &requirements);
            VkMemoryAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
            allocateInfo.allocationSize = requirements.size;
            allocateInfo.memoryTypeIndex = memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vkCheck(vkAllocateMemory(iDevice, &allocateInfo, nullptr, &atlas->memory));
            vkCheck(vkBindImageMemory(iDevice, atlas->image, atlas->memory, 0u));
            VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            viewInfo.image = atlas->image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = atlasFormat;
            viewInfo.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 1u, 0u, 1u };
            vkCheck(vkCreateImageView(iDevice, &viewInfo, nullptr, &atlas->view));
            end_rendering();
            image_barrier(atlas->image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            VkClearDepthStencilValue const clearValue{ 1.0f, 0u };
            VkImageSubresourceRange const range{ VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 1u, 0u, 1u };
            vkCmdClearDepthStencilImage(command_buffer(), atlas->image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1u, &range);
            resources.atlas = std::move(atlas);

            VkDescriptorSetLayoutBinding const bindings[] =
            {
                { 0u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { resources.matricesBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1u, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
                { resources.tableBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1u, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
            };
            VkDescriptorSetLayoutCreateInfo setLayoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            setLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            setLayoutInfo.bindingCount = static_cast<std::uint32_t>(std::size(bindings));
            setLayoutInfo.pBindings = bindings;
            vkCheck(vkCreateDescriptorSetLayout(iDevice, &setLayoutInfo, nullptr, &resources.descriptorSetLayout));
            VkPushConstantRange const pushConstants{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0u, sizeof(shadow_parameters) };
            VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
            layoutInfo.setLayoutCount = 1u;
            layoutInfo.pSetLayouts = &resources.descriptorSetLayout;
            layoutInfo.pushConstantRangeCount = 1u;
            layoutInfo.pPushConstantRanges = &pushConstants;
            vkCheck(vkCreatePipelineLayout(iDevice, &layoutInfo, nullptr, &resources.pipelineLayout));

            VkPipelineShaderStageCreateInfo stages[2] = { { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO }, { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO } };
            stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            stages[0].module = resources.vertex;
            stages[0].pName = "main";
            stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            stages[1].module = resources.fragment;
            stages[1].pName = "main";
            VkVertexInputBindingDescription const binding{ 0u, sizeof(scene_vertex), VK_VERTEX_INPUT_RATE_VERTEX };
            VkVertexInputAttributeDescription const attributes[] =
            {
                { 0u, 0u, VK_FORMAT_R32G32B32_SFLOAT, static_cast<std::uint32_t>(scene_vertex::offset::xyz) },
                { 1u, 0u, VK_FORMAT_R8G8B8A8_UNORM, static_cast<std::uint32_t>(scene_vertex::offset::rgba) },
                { 2u, 0u, VK_FORMAT_R32G32_SFLOAT, static_cast<std::uint32_t>(scene_vertex::offset::st) },
                { 11u, 0u, VK_FORMAT_R32_SFLOAT, static_cast<std::uint32_t>(scene_vertex::offset::model) },
                { 12u, 0u, VK_FORMAT_R16G16B16A16_USCALED, static_cast<std::uint32_t>(scene_vertex::offset::joints) },
                { 13u, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<std::uint32_t>(scene_vertex::offset::weights) }
            };
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
            vertexInputInfo.vertexBindingDescriptionCount = 1u;
            vertexInputInfo.pVertexBindingDescriptions = &binding;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(std::size(attributes));
            vertexInputInfo.pVertexAttributeDescriptions = attributes;
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
            inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
            viewportInfo.viewportCount = 1u;
            viewportInfo.scissorCount = 1u;
            VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
            rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
            rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
            // slope scaled depth bias against shadow acne (cf. glPolygonOffset(1.5f, 2.0f))
            rasterizationInfo.depthBiasEnable = VK_TRUE;
            rasterizationInfo.depthBiasConstantFactor = 2.0f;
            rasterizationInfo.depthBiasSlopeFactor = 1.5f;
            rasterizationInfo.lineWidth = 1.0f;
            VkPipelineMultisampleStateCreateInfo multisampleInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
            multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
            depthStencilInfo.depthTestEnable = VK_TRUE;
            depthStencilInfo.depthWriteEnable = VK_TRUE;
            depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencilInfo.maxDepthBounds = 1.0f;
            VkPipelineColorBlendStateCreateInfo blendInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
            VkDynamicState const dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamicInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
            dynamicInfo.dynamicStateCount = static_cast<std::uint32_t>(std::size(dynamicStates));
            dynamicInfo.pDynamicStates = dynamicStates;
            VkPipelineRenderingCreateInfo renderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
            renderingInfo.depthAttachmentFormat = atlasFormat;
            VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
            pipelineInfo.pNext = &renderingInfo;
            pipelineInfo.stageCount = 2u;
            pipelineInfo.pStages = stages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
            pipelineInfo.pViewportState = &viewportInfo;
            pipelineInfo.pRasterizationState = &rasterizationInfo;
            pipelineInfo.pMultisampleState = &multisampleInfo;
            pipelineInfo.pDepthStencilState = &depthStencilInfo;
            pipelineInfo.pColorBlendState = &blendInfo;
            pipelineInfo.pDynamicState = &dynamicInfo;
            pipelineInfo.layout = resources.pipelineLayout;
            vkCheck(vkCreateGraphicsPipelines(iDevice, VK_NULL_HANDLE, 1u, &pipelineInfo, nullptr, &resources.pipeline));
            resources.failed = false;
        }
        catch (std::exception const& e)
        {
            service<debug::logger>() << neolib::logger::severity::Debug << "neogfx: shadow map shader: " << e.what() << std::endl;
        }
        return resources;
    }

    bool vulkan_graphics_backend::scene_shadows_available(i_standard_shader_program& aProgram)
    {
        return !shadows(aProgram).failed;
    }

    void vulkan_graphics_backend::draw_scene_shadow_maps(i_standard_shader_program& aProgram, native_scene_buffer& aSceneBuffer,
        std::vector<mat44> const& aViews, std::int32_t aDirectionalView,
        std::vector<std::optional<gpu_mesh_range>> const& aMeshes, std::vector<std::optional<scene_shadow_alpha_test>> const& aAlphaTests,
        std::uint32_t aModelTableBase)
    {
        auto& resources = shadows(aProgram);
        if (resources.failed)
            return;
        auto const* vertices = buffer_of(aSceneBuffer.vertex_buffer());
        auto const* indices = buffer_of(aSceneBuffer.index_buffer());
        if (vertices == nullptr || indices == nullptr)
            return;
        auto const& program = *static_cast<vulkan_program const*>(aProgram.handle());
        auto const storage_buffer = [&](std::uint32_t aBinding) -> VkBuffer
            {
                for (auto const& sb : program.storageBuffers)
                    if (sb.binding == aBinding)
                        if (auto const* buffer = buffer_of(sb.buffer()))
                            return buffer->buffer;
                return iZeroBuffer.buffer;
            };
        VkDescriptorBufferInfo const matrices{ storage_buffer(resources.matricesBinding), 0u, VK_WHOLE_SIZE };
        VkDescriptorBufferInfo const table{ storage_buffer(resources.tableBinding), 0u, VK_WHOLE_SIZE };

        // depth only, into the atlas
        end_rendering();
        memory_barrier();
        auto const commandBuffer = command_buffer();
        VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        depthAttachment.imageView = resources.atlas->view;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        renderingInfo.renderArea = VkRect2D{ { 0, 0 }, { resources.atlas->width, resources.atlas->height } };
        renderingInfo.layerCount = 1u;
        renderingInfo.pDepthAttachment = &depthAttachment;
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resources.pipeline);
        VkDeviceSize const offset = 0u;
        vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, &vertices->buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, indices->buffer, 0u, VK_INDEX_TYPE_UINT32);

        shadow_parameters parameters = {};
        parameters.modelTableBase = aModelTableBase;
        for (std::uint32_t view = 0u; view < aViews.size(); ++view)
        {
            if (view == 0u && aDirectionalView != 0)
                continue;
            auto const tile = scene_shadow_atlas::tile(view);
            VkViewport const viewport{ static_cast<float>(tile[0]), static_cast<float>(tile[1]), static_cast<float>(tile[2]), static_cast<float>(tile[3]), 0.0f, 1.0f };
            VkRect2D const scissor{ { tile[0], tile[1] }, { static_cast<std::uint32_t>(tile[2]), static_cast<std::uint32_t>(tile[3]) } };
            vkCmdSetViewport(commandBuffer, 0u, 1u, &viewport);
            vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);
            VkClearAttachment clearAttachment{ VK_IMAGE_ASPECT_DEPTH_BIT, 0u };
            clearAttachment.clearValue.depthStencil = VkClearDepthStencilValue{ 1.0f, 0u };
            VkClearRect const clearRect{ scissor, 0u, 1u };
            vkCmdClearAttachments(commandBuffer, 1u, &clearAttachment, 1u, &clearRect);
            auto const viewProjection = aViews[view].as<float>();
            std::memcpy(parameters.viewProjection, viewProjection.data(), sizeof(parameters.viewProjection));
            for (std::size_t meshIndex = 0u; meshIndex < aMeshes.size(); ++meshIndex)
            {
                auto const& mesh = aMeshes[meshIndex];
                if (!mesh)
                    continue;
                auto const& alphaTest = aAlphaTests[meshIndex];
                vulkan_image const* baseTexture = iDummyTexture.get();
                parameters.alphaTest = alphaTest ? 1 : 0;
                if (alphaTest)
                {
                    auto const& [texture, transform, cutoff, wrapS, wrapT] = *alphaTest;
                    if (auto const* image = image_of(*texture))
                        baseTexture = image;
                    parameters.textureTransform[0] = transform.x;
                    parameters.textureTransform[1] = transform.y;
                    parameters.textureTransform[2] = transform.z;
                    parameters.textureTransform[3] = transform.w;
                    parameters.alphaCutoff = cutoff;
                    parameters.textureWrap[0] = static_cast<std::int32_t>(wrapS);
                    parameters.textureWrap[1] = static_cast<std::int32_t>(wrapT);
                }
                VkDescriptorImageInfo const baseTextureInfo{ sampler(baseTexture->sampler, baseTexture->mipLevels), baseTexture->view, VK_IMAGE_LAYOUT_GENERAL };
                VkWriteDescriptorSet writes[3] = { { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET }, { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET }, { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET } };
                writes[0].dstBinding = 0u;
                writes[0].descriptorCount = 1u;
                writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writes[0].pImageInfo = &baseTextureInfo;
                writes[1].dstBinding = resources.matricesBinding;
                writes[1].descriptorCount = 1u;
                writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                writes[1].pBufferInfo = &matrices;
                writes[2].dstBinding = resources.tableBinding;
                writes[2].descriptorCount = 1u;
                writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                writes[2].pBufferInfo = &table;
                iCmdPushDescriptorSet(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resources.pipelineLayout, 0u, 3u, writes);
                vkCmdPushConstants(commandBuffer, resources.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0u, sizeof(parameters), &parameters);
                vkCmdDrawIndexed(commandBuffer, mesh->indexEnd - mesh->indexStart, 1u, mesh->indexStart, 0, 0u);
            }
        }
        vkCmdEndRendering(commandBuffer);
        // n.b. the target is rendered to again (after a barrier) by the next draw, with the state rebound
        invalidate_bindings();
        bind_texture(static_cast<std::uint32_t>(reserved_texture_unit::PbrShadow), resources.atlas.get());
    }

    vulkan_graphics_backend::background_resources& vulkan_graphics_backend::background()
    {
        if (iBackground)
            return *iBackground;
        iBackground = std::make_unique<background_resources>();
        auto& resources = *iBackground;
        try
        {
            resources.vertex = create_shader_module(compile_shader(shader_type::Vertex, sBackgroundVertexShader, "neogfx::scene_background.vert", true));
            resources.fragment = create_shader_module(compile_shader(shader_type::Fragment, sBackgroundFragmentShader, "neogfx::scene_background.frag", true));
            VkDescriptorSetLayoutBinding const bindings[] =
            {
                { 0u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1u, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 1u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
                { 2u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
            };
            VkDescriptorSetLayoutCreateInfo setLayoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
            setLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
            setLayoutInfo.bindingCount = static_cast<std::uint32_t>(std::size(bindings));
            setLayoutInfo.pBindings = bindings;
            vkCheck(vkCreateDescriptorSetLayout(iDevice, &setLayoutInfo, nullptr, &resources.descriptorSetLayout));
            VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
            layoutInfo.setLayoutCount = 1u;
            layoutInfo.pSetLayouts = &resources.descriptorSetLayout;
            vkCheck(vkCreatePipelineLayout(iDevice, &layoutInfo, nullptr, &resources.pipelineLayout));
            resources.failed = false;
        }
        catch (std::exception const& e)
        {
            service<debug::logger>() << neolib::logger::severity::Debug << "neogfx: scene background shader: " << e.what() << std::endl;
        }
        return resources;
    }

    void vulkan_graphics_backend::draw_scene_background(i_standard_shader_program&, mat44 const& aNdcToClip, mat44 const& aClipToWorld,
        i_texture const* aBackgroundTexture, i_texture const* aEnvironment, double aBlur, double aIntensity)
    {
        auto& resources = background();
        if (resources.failed || iColor == nullptr)
            return;
        begin_rendering();
        auto const commandBuffer = command_buffer();

        auto const key = std::make_tuple(iColor->format, iDepthStencil != nullptr ? iDepthStencil->format : VK_FORMAT_UNDEFINED, iColor->samples);
        auto existing = resources.pipelines.find(key);
        if (existing == resources.pipelines.end())
        {
            VkPipelineShaderStageCreateInfo stages[2] = { { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO }, { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO } };
            stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            stages[0].module = resources.vertex;
            stages[0].pName = "main";
            stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            stages[1].module = resources.fragment;
            stages[1].pName = "main";
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
            inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
            viewportInfo.viewportCount = 1u;
            viewportInfo.scissorCount = 1u;
            VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
            rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
            rasterizationInfo.lineWidth = 1.0f;
            VkPipelineMultisampleStateCreateInfo multisampleInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
            multisampleInfo.rasterizationSamples = iColor->samples;
            VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
            depthStencilInfo.maxDepthBounds = 1.0f;
            VkPipelineColorBlendAttachmentState blendAttachment{};
            blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            VkPipelineColorBlendStateCreateInfo blendInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
            blendInfo.attachmentCount = 1u;
            blendInfo.pAttachments = &blendAttachment;
            VkDynamicState const dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamicInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
            dynamicInfo.dynamicStateCount = static_cast<std::uint32_t>(std::size(dynamicStates));
            dynamicInfo.pDynamicStates = dynamicStates;
            VkPipelineRenderingCreateInfo renderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
            renderingInfo.colorAttachmentCount = 1u;
            renderingInfo.pColorAttachmentFormats = &iColor->format;
            renderingInfo.depthAttachmentFormat = std::get<1>(key);
            renderingInfo.stencilAttachmentFormat = has_stencil(std::get<1>(key)) ? std::get<1>(key) : VK_FORMAT_UNDEFINED;
            VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
            pipelineInfo.pNext = &renderingInfo;
            pipelineInfo.stageCount = 2u;
            pipelineInfo.pStages = stages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
            pipelineInfo.pViewportState = &viewportInfo;
            pipelineInfo.pRasterizationState = &rasterizationInfo;
            pipelineInfo.pMultisampleState = &multisampleInfo;
            pipelineInfo.pDepthStencilState = &depthStencilInfo;
            pipelineInfo.pColorBlendState = &blendInfo;
            pipelineInfo.pDynamicState = &dynamicInfo;
            pipelineInfo.layout = resources.pipelineLayout;
            VkPipeline newPipeline = VK_NULL_HANDLE;
            vkCheck(vkCreateGraphicsPipelines(iDevice, VK_NULL_HANDLE, 1u, &pipelineInfo, nullptr, &newPipeline));
            existing = resources.pipelines.emplace(key, newPipeline).first;
        }

        background_parameters parameters = {};
        auto const ndcToClip = aNdcToClip.as<float>();
        auto const clipToWorld = aClipToWorld.as<float>();
        std::memcpy(parameters.ndcToClip, ndcToClip.data(), sizeof(parameters.ndcToClip));
        std::memcpy(parameters.clipToWorld, clipToWorld.data(), sizeof(parameters.clipToWorld));
        parameters.source = (aBackgroundTexture != nullptr ? 1 : 0);
        parameters.blur = static_cast<float>(aBlur);
        parameters.intensity = static_cast<float>(aIntensity);
        if (aBackgroundTexture != nullptr)
        {
            // the panorama's extents: the texture is half as tall again (see i_pbr_shader::set_background_texture)
            auto const extents = aBackgroundTexture->storage_extents();
            parameters.backgroundExtents[0] = static_cast<std::int32_t>(extents.cx);
            parameters.backgroundExtents[1] = static_cast<std::int32_t>(extents.cy * 2.0 / 3.0 + 0.5);
        }
        auto const uniforms = allocate_transient(sizeof(parameters), iProperties.limits.minUniformBufferOffsetAlignment);
        std::memcpy(uniforms.mapping, &parameters, sizeof(parameters));

        vulkan_image const* environment = (aEnvironment != nullptr ? image_of(*aEnvironment) : nullptr);
        vulkan_image const* backgroundImage = (aBackgroundTexture != nullptr ? image_of(*aBackgroundTexture) : nullptr);
        if (environment == nullptr)
            environment = iDummyTexture.get();
        if (backgroundImage == nullptr)
            backgroundImage = iDummyTexture.get();
        VkDescriptorBufferInfo const uniformsInfo{ uniforms.buffer, uniforms.offset, sizeof(parameters) };
        VkDescriptorImageInfo const environmentInfo{ sampler(environment->sampler, environment->mipLevels), environment->view, VK_IMAGE_LAYOUT_GENERAL };
        VkDescriptorImageInfo const backgroundInfo{ sampler(backgroundImage->sampler, backgroundImage->mipLevels), backgroundImage->view, VK_IMAGE_LAYOUT_GENERAL };
        VkWriteDescriptorSet writes[3] = { { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET }, { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET }, { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET } };
        writes[0].dstBinding = 0u;
        writes[0].descriptorCount = 1u;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].pBufferInfo = &uniformsInfo;
        writes[1].dstBinding = 1u;
        writes[1].descriptorCount = 1u;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].pImageInfo = &environmentInfo;
        writes[2].dstBinding = 2u;
        writes[2].descriptorCount = 1u;
        writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[2].pImageInfo = &backgroundInfo;

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, existing->second);
        // n.b. viewport and scissor as the current state has them
        apply_viewport_and_scissor();
        iCmdPushDescriptorSet(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, resources.pipelineLayout, 0u, 3u, writes);
        vkCmdDraw(commandBuffer, 4u, 1u, 0u, 0u);
        // n.b. the next draw rebinds its pipeline and sets the dynamic state the background pipeline does not have
        invalidate_bindings();
    }
}
