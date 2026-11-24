// d3d12_graphics_backend.cpp
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
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <shaderc/shaderc.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include "../i_native_texture.hpp"
#include "../native_vertex.hpp"
#include "d3d12_texture_manager.hpp"
#include "d3d12_shader_program.hpp"
#include "d3d12_hlsl.hpp"
#include "d3d12_graphics_backend.hpp"

namespace neogfx
{
    namespace
    {
        d3d12_graphics_backend* sInstance;

        constexpr std::uint64_t TransientChunkSize = 8u * 1024u * 1024u;
        // the shader visible CBV/SRV/UAV heaps (a frame has as many as it needs) and the shader visible sampler heap (the
        // largest a sampler heap may be)
        constexpr std::uint32_t VisibleHeapSize = 16384u;
        constexpr std::uint32_t SamplerHeapSize = 2048u;
        constexpr std::uint32_t ScratchRtvCount = 64u;
        // what a vertex input variable without an attribute, and an SSBO without a buffer, reads (n.b. root SRVs are not bounds
        // checked, so this is large enough for the reads a shader guards with a count or a flag of zero)
        constexpr std::uint64_t ZeroBufferSize = 64u * 1024u;

        D3D12_RESOURCE_STATES const ShaderResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        // a device local buffer's state when it is read (see transition)
        D3D12_RESOURCE_STATES const BufferReadState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;

        // the on-disk cache (see d3d12_graphics_backend::cache_folder): compiled SPIR-V and DXBC, one file per compilation (named by
        // a hash of its key, which is stored in it and compared, so a hash collision is a cache miss); the data in each is hashed, so
        // a corrupt file is a cache miss (n.b. not protection against tampering). A file's last write time is when it was last used;
        // unused for CacheMaxAge they are deleted (see prune_cache_folder).
        constexpr char SpirvCacheMagic[8] = { 'N', 'G', 'D', 'X', 'S', '0', '0', '1' };
        constexpr char DxbcCacheMagic[8] = { 'N', 'G', 'D', 'X', 'B', '0', '0', '1' };
        constexpr std::chrono::hours CacheMaxAge{ 30 * 24 };

        std::uint64_t fnv1a(void const* aData, std::size_t aSize)
        {
            std::uint64_t result = 0xCBF29CE484222325ull;
            for (auto byte = static_cast<std::uint8_t const*>(aData); byte != static_cast<std::uint8_t const*>(aData) + aSize; ++byte)
            {
                result ^= *byte;
                result *= 0x100000001B3ull;
            }
            return result;
        }

        std::uint64_t fnv1a(std::string const& aText)
        {
            return fnv1a(aText.data(), aText.size());
        }

        std::filesystem::path cache_file(std::filesystem::path const& aFolder, std::string const& aKey, char const* aExtension)
        {
            std::ostringstream fileName;
            fileName << std::hex << std::setw(16) << std::setfill('0') << fnv1a(aKey) << aExtension;
            return aFolder / fileName.str();
        }

        // n.b. written to a temporary file and then renamed, so a cache file is never partially written
        void write_cached_blob(std::filesystem::path const& aPath, char const (&aMagic)[8], std::string const& aKey, void const* aData, std::size_t aSize)
        {
            try
            {
                std::string contents{ std::begin(aMagic), std::end(aMagic) };
                std::uint64_t const keySize = aKey.size();
                contents.append(reinterpret_cast<char const*>(&keySize), sizeof(keySize));
                contents += aKey;
                std::uint64_t const dataSize = aSize;
                contents.append(reinterpret_cast<char const*>(&dataSize), sizeof(dataSize));
                std::uint64_t const hash = fnv1a(aData, aSize);
                contents.append(reinterpret_cast<char const*>(&hash), sizeof(hash));
                contents.append(static_cast<char const*>(aData), aSize);
                auto temporary = aPath;
                temporary += ".tmp";
                {
                    std::ofstream output{ temporary, std::ios::binary | std::ios::trunc };
                    if (!output.write(contents.data(), static_cast<std::streamsize>(contents.size())))
                        return;
                }
                std::filesystem::rename(temporary, aPath);
            }
            catch (...)
            {
                // n.b. the cache is an optimization: failing to write it is not an error
            }
        }

        std::optional<std::vector<std::uint8_t>> read_cached_blob(std::filesystem::path const& aPath, char const (&aMagic)[8], std::string const& aKey)
        {
            std::ifstream input{ aPath, std::ios::binary };
            if (!input)
                return std::nullopt;
            char magic[sizeof(aMagic)];
            if (!input.read(magic, sizeof(magic)) || std::memcmp(magic, aMagic, sizeof(magic)) != 0)
                return std::nullopt;
            std::uint64_t keySize = 0u;
            if (!input.read(reinterpret_cast<char*>(&keySize), sizeof(keySize)) || keySize != aKey.size())
                return std::nullopt;
            std::string key(static_cast<std::size_t>(keySize), '\0');
            if (!input.read(key.data(), static_cast<std::streamsize>(key.size())) || key != aKey)
                return std::nullopt;
            std::uint64_t dataSize = 0u;
            if (!input.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize)) || dataSize == 0u || dataSize > (1ull << 28u))
                return std::nullopt;
            std::uint64_t hash = 0u;
            if (!input.read(reinterpret_cast<char*>(&hash), sizeof(hash)))
                return std::nullopt;
            std::vector<std::uint8_t> result(static_cast<std::size_t>(dataSize));
            if (!input.read(reinterpret_cast<char*>(result.data()), static_cast<std::streamsize>(result.size())))
                return std::nullopt;
            if (hash != fnv1a(result.data(), result.size()))
                return std::nullopt;
            // n.b. its last write time is when it was last used (see prune_cache_folder)
            std::error_code ec;
            std::filesystem::last_write_time(aPath, std::filesystem::file_time_type::clock::now(), ec);
            return result;
        }

        // delete the cache files not used for CacheMaxAge (and any temporary files left by write_cached_blob)
        void prune_cache_folder(std::filesystem::path const& aFolder)
        {
            try
            {
                auto const now = std::filesystem::file_time_type::clock::now();
                std::error_code ec;
                for (auto const& entry : std::filesystem::directory_iterator{ aFolder, ec })
                {
                    auto const extension = entry.path().extension();
                    if (extension != ".spv" && extension != ".dxbc" && extension != ".tmp")
                        continue;
                    auto const lastUsed = entry.last_write_time(ec);
                    if (!ec && now - lastUsed > CacheMaxAge)
                        std::filesystem::remove(entry.path(), ec);
                }
            }
            catch (...)
            {
                // n.b. the cache is an optimization: failing to prune it is not an error
            }
        }

        template <typename T>
        T align_up(T aValue, T aAlignment)
        {
            return (aValue + aAlignment - 1u) / aAlignment * aAlignment;
        }

        D3D_PRIMITIVE_TOPOLOGY to_d3d12_topology(gpu_primitive aPrimitive)
        {
            switch (aPrimitive)
            {
            case gpu_primitive::Triangles:
            default:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case gpu_primitive::TriangleStrip:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            }
        }

        // the format of an attribute and whether it is an unsigned integer to be converted to floating point (n.b. Direct3D 12 has
        // neither three component 8 and 16 bit formats (the four component format is used: the fourth component, within the
        // vertex, is ignored by the three component input) nor scaled formats (see d3d12_glsl_scaled_inputs))
        std::pair<DXGI_FORMAT, bool> to_d3d12_attribute_format(std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized)
        {
            static constexpr DXGI_FORMAT sFloat[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };
            static constexpr DXGI_FORMAT sUnorm8[] = { DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };
            static constexpr DXGI_FORMAT sUint8[] = { DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT };
            static constexpr DXGI_FORMAT sUnorm16[] = { DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM };
            static constexpr DXGI_FORMAT sUint16[] = { DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UINT };
            auto const index = std::clamp<std::uint32_t>(aArity, 1u, 4u) - 1u;
            switch (aType)
            {
            case gpu_attribute_type::Float:
            default:
                return { sFloat[index], false };
            case gpu_attribute_type::Double:
                throw std::logic_error("neogfx::d3d12_graphics_backend: double precision vertex attributes are not supported");
            case gpu_attribute_type::UnsignedByte:
                return aNormalized ? std::make_pair(sUnorm8[index], false) : std::make_pair(sUint8[index], true);
            case gpu_attribute_type::UnsignedShort:
                return aNormalized ? std::make_pair(sUnorm16[index], false) : std::make_pair(sUint16[index], true);
            }
        }

        bool integer_input(shader_data_type aType)
        {
            switch (aType)
            {
            case shader_data_type::Int:
            case shader_data_type::IVec2:
            case shader_data_type::IVec3:
            case shader_data_type::IVec4:
            case shader_data_type::Uint:
            case shader_data_type::UVec2:
            case shader_data_type::UVec3:
            case shader_data_type::UVec4:
                return true;
            default:
                return false;
            }
        }

        // the format of the zero attribute a vertex input variable without an attribute reads (OpenGL: a disabled attribute)
        DXGI_FORMAT zero_attribute_format(shader_data_type aType)
        {
            switch (aType)
            {
            case shader_data_type::Int:
            case shader_data_type::IVec2:
            case shader_data_type::IVec3:
            case shader_data_type::IVec4:
                return DXGI_FORMAT_R32G32B32A32_SINT;
            case shader_data_type::Uint:
            case shader_data_type::UVec2:
            case shader_data_type::UVec3:
            case shader_data_type::UVec4:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            default:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }

        d3d12_image* image_of(i_texture const& aTexture)
        {
            return reinterpret_cast<d3d12_image*>(aTexture.native_texture().native_handle());
        }

        d3d12_buffer* buffer_of(gpu_buffer aBuffer)
        {
            return reinterpret_cast<d3d12_buffer*>(aBuffer);
        }

        D3D12_HEAP_PROPERTIES heap_properties(D3D12_HEAP_TYPE aType)
        {
            D3D12_HEAP_PROPERTIES result = {};
            result.Type = aType;
            result.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            result.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            result.CreationNodeMask = 1u;
            result.VisibleNodeMask = 1u;
            return result;
        }

        D3D12_RESOURCE_DESC buffer_desc(std::uint64_t aSize)
        {
            D3D12_RESOURCE_DESC result = {};
            result.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            result.Width = aSize;
            result.Height = 1u;
            result.DepthOrArraySize = 1u;
            result.MipLevels = 1u;
            result.Format = DXGI_FORMAT_UNKNOWN;
            result.SampleDesc.Count = 1u;
            result.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            return result;
        }

        D3D12_RESOURCE_DESC texture_desc(std::uint32_t aWidth, std::uint32_t aHeight, DXGI_FORMAT aFormat, std::uint32_t aSamples,
            std::uint32_t aMipLevels, D3D12_RESOURCE_FLAGS aFlags)
        {
            D3D12_RESOURCE_DESC result = {};
            result.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            result.Width = aWidth;
            result.Height = aHeight;
            result.DepthOrArraySize = 1u;
            result.MipLevels = static_cast<UINT16>(aMipLevels);
            result.Format = aFormat;
            result.SampleDesc.Count = aSamples;
            result.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            result.Flags = aFlags;
            return result;
        }

        D3D12_SHADER_BYTECODE to_bytecode(d3d12_bytecode const& aBytecode)
        {
            if (!aBytecode)
                return D3D12_SHADER_BYTECODE{ nullptr, 0u };
            return D3D12_SHADER_BYTECODE{ aBytecode->data(), aBytecode->size() };
        }

        D3D12_ROOT_PARAMETER root_descriptor(D3D12_ROOT_PARAMETER_TYPE aType, std::uint32_t aRegister)
        {
            D3D12_ROOT_PARAMETER result = {};
            result.ParameterType = aType;
            result.Descriptor.ShaderRegister = aRegister;
            result.Descriptor.RegisterSpace = 0u;
            result.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            return result;
        }

        D3D12_ROOT_PARAMETER root_table(D3D12_DESCRIPTOR_RANGE const& aRange)
        {
            D3D12_ROOT_PARAMETER result = {};
            result.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            result.DescriptorTable.NumDescriptorRanges = 1u;
            result.DescriptorTable.pDescriptorRanges = &aRange;
            result.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            return result;
        }

        D3D12_DESCRIPTOR_RANGE range_of(D3D12_DESCRIPTOR_RANGE_TYPE aType, std::uint32_t aBaseRegister, std::uint32_t aCount)
        {
            D3D12_DESCRIPTOR_RANGE result = {};
            result.RangeType = aType;
            result.NumDescriptors = aCount;
            result.BaseShaderRegister = aBaseRegister;
            result.RegisterSpace = 0u;
            result.OffsetInDescriptorsFromTableStart = 0u;
            return result;
        }

        D3D12_RASTERIZER_DESC rasterizer_desc(D3D12_CULL_MODE aCullMode, bool aFrontCounterClockwise, std::uint32_t aSamples)
        {
            D3D12_RASTERIZER_DESC result = {};
            result.FillMode = D3D12_FILL_MODE_SOLID;
            result.CullMode = aCullMode;
            result.FrontCounterClockwise = aFrontCounterClockwise ? TRUE : FALSE;
            result.DepthBias = 0;
            result.DepthBiasClamp = 0.0f;
            result.SlopeScaledDepthBias = 0.0f;
            result.DepthClipEnable = TRUE;
            result.MultisampleEnable = aSamples > 1u ? TRUE : FALSE;
            result.AntialiasedLineEnable = FALSE;
            result.ForcedSampleCount = 0u;
            result.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            return result;
        }

        D3D12_BLEND_DESC opaque_blend_desc()
        {
            D3D12_BLEND_DESC result = {};
            for (auto& target : result.RenderTarget)
            {
                target.BlendEnable = FALSE;
                target.SrcBlend = D3D12_BLEND_ONE;
                target.DestBlend = D3D12_BLEND_ZERO;
                target.BlendOp = D3D12_BLEND_OP_ADD;
                target.SrcBlendAlpha = D3D12_BLEND_ONE;
                target.DestBlendAlpha = D3D12_BLEND_ZERO;
                target.BlendOpAlpha = D3D12_BLEND_OP_ADD;
                target.LogicOp = D3D12_LOGIC_OP_NOOP;
                target.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            }
            return result;
        }

        D3D12_DEPTH_STENCIL_DESC no_depth_stencil_desc()
        {
            D3D12_DEPTH_STENCIL_DESC result = {};
            result.DepthEnable = FALSE;
            result.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            result.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            result.StencilEnable = FALSE;
            result.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
            result.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
            result.FrontFace = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
            result.BackFace = result.FrontFace;
            return result;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE offset(D3D12_CPU_DESCRIPTOR_HANDLE aHandle, std::uint32_t aIndex, std::uint32_t aIncrement)
        {
            aHandle.ptr += static_cast<SIZE_T>(aIndex) * aIncrement;
            return aHandle;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE offset(D3D12_GPU_DESCRIPTOR_HANDLE aHandle, std::uint32_t aIndex, std::uint32_t aIncrement)
        {
            aHandle.ptr += static_cast<UINT64>(aIndex) * aIncrement;
            return aHandle;
        }

        // GLSL (Direct3D 12) for the shadow maps and the scene background: as vulkan_graphics_backend's, except that the shadow
        // parameters are a uniform block rather than push constants, the joint indices are unsigned integers (see
        // d3d12_glsl_scaled_inputs), the SSBOs are readonly (see d3d12_glsl) and clip space y is negated (see the header)
        std::string const sShadowVertexShader =
            "#version 460 core\n"
            "layout (location = 0) in vec3 VertexPosition;\n"
            "layout (location = 1) in vec4 VertexColor;\n"
            "layout (location = 2) in vec2 VertexTextureCoord;\n"
            "layout (location = 11) in float VertexModel;\n"
            "layout (location = 12) in uvec4 VertexJoints;\n"
            "layout (location = 13) in vec4 VertexWeights;\n"
            "layout(std430, set = 0, binding = %MATRICES%) readonly buffer SSBO_bModelMatrices { mat4 bModelMatrices[]; };\n"
            "layout(std430, set = 0, binding = %TABLE%) readonly buffer SSBO_bModelTable { uint bModelTable[]; };\n"
            "layout(std140, set = 0, binding = 1) uniform ShadowParameters\n"
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
            "                    skinned += VertexWeights[i] * (bModelMatrices[first + 1u + VertexJoints[i]] * position);\n"
            "            position = skinned;\n"
            "        }\n"
            "        position = bModelMatrices[first] * position;\n"
            "    }\n"
            "    gl_Position = uViewProjection * vec4(position.xyz / position.w, 1.0);\n"
            "    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
            "    gl_Position.y = -gl_Position.y;\n"
            "}\n";

        std::string const sShadowFragmentShader =
            "#version 460 core\n"
            "layout (location = 0) in vec2 TexCoord;\n"
            "layout (location = 1) in float Alpha;\n"
            "layout (set = 0, binding = 0) uniform sampler2D uBaseTexture;\n"
            "layout(std140, set = 0, binding = 1) uniform ShadowParameters\n"
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
            "    gl_Position.y = -gl_Position.y;\n"
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

        // HLSL for drawing an image to a swapchain's back buffer (flipped: the image is as OpenGL would have it, bottom row
        // first) and for generating a mipmap level from the one above
        char const sBlitShaders[] =
            "cbuffer BlitParameters : register(b0)\n"
            "{\n"
            "    int uSourceHeight;\n"
            "};\n"
            "Texture2D<float4> uSource : register(t0);\n"
            "SamplerState uSampler : register(s0);\n"
            "struct blit_vertex\n"
            "{\n"
            "    float4 position : SV_Position;\n"
            "    float2 uv : TEXCOORD0;\n"
            "};\n"
            "blit_vertex vs(uint id : SV_VertexID)\n"
            "{\n"
            "    blit_vertex result;\n"
            "    result.uv = float2((id << 1) & 2, id & 2);\n"
            "    result.position = float4(result.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);\n"
            "    return result;\n"
            "}\n"
            "float4 present_ps(blit_vertex input) : SV_Target\n"
            "{\n"
            "    int2 p = int2(input.position.xy);\n"
            "    return uSource.Load(int3(p.x, uSourceHeight - 1 - p.y, 0));\n"
            "}\n"
            "float4 mipmap_ps(blit_vertex input) : SV_Target\n"
            "{\n"
            "    return uSource.SampleLevel(uSampler, input.uv, 0.0);\n"
            "}\n";
    }

    struct d3d12_graphics_backend::shadow_resources
    {
        bool failed = true;
        std::unique_ptr<d3d12_image> atlas;
        com_ptr<ID3D12RootSignature> rootSignature;
        com_ptr<ID3D12PipelineState> pipeline;
        std::uint32_t matricesBinding = 0u;
        std::uint32_t tableBinding = 0u;
    };

    struct d3d12_graphics_backend::background_resources
    {
        bool failed = true;
        d3d12_bytecode vertex;
        d3d12_bytecode fragment;
        com_ptr<ID3D12RootSignature> rootSignature;
        std::map<std::tuple<DXGI_FORMAT, DXGI_FORMAT, std::uint32_t>, com_ptr<ID3D12PipelineState>> pipelines;
    };

    struct d3d12_graphics_backend::blit_resources
    {
        d3d12_bytecode vertex;
        d3d12_bytecode present;
        d3d12_bytecode mipmap;
        com_ptr<ID3D12RootSignature> presentRootSignature;
        com_ptr<ID3D12RootSignature> mipmapRootSignature;
        std::map<DXGI_FORMAT, com_ptr<ID3D12PipelineState>> presentPipelines;
        std::map<DXGI_FORMAT, com_ptr<ID3D12PipelineState>> mipmapPipelines;
    };

    void d3d12_descriptor_allocator::initialize(ID3D12Device* aDevice, D3D12_DESCRIPTOR_HEAP_TYPE aType, std::uint32_t aHeapSize)
    {
        clear();
        iDevice = aDevice;
        iType = aType;
        iHeapSize = aHeapSize;
        iIncrement = aDevice->GetDescriptorHandleIncrementSize(aType);
    }

    void d3d12_descriptor_allocator::clear()
    {
        iHeaps.clear();
        iFree.clear();
        iUsed = 0u;
    }

    d3d12_descriptor d3d12_descriptor_allocator::allocate()
    {
        if (!iFree.empty())
        {
            auto const result = iFree.back();
            iFree.pop_back();
            return result;
        }
        if (iHeaps.empty() || iUsed == iHeapSize)
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = iType;
            heapDesc.NumDescriptors = iHeapSize;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            com_ptr<ID3D12DescriptorHeap> heap;
            dxCheck(iDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));
            iHeaps.push_back(heap);
            iUsed = 0u;
        }
        d3d12_descriptor result;
        result.heap = static_cast<std::uint32_t>(iHeaps.size() - 1u);
        result.index = iUsed++;
        result.cpu = offset(iHeaps.back()->GetCPUDescriptorHandleForHeapStart(), result.index, iIncrement);
        return result;
    }

    void d3d12_descriptor_allocator::free(d3d12_descriptor& aDescriptor)
    {
        if (!aDescriptor.valid())
            return;
        if (aDescriptor.heap < iHeaps.size())
            iFree.push_back(aDescriptor);
        aDescriptor = d3d12_descriptor{};
    }

    d3d12_graphics_backend::d3d12_graphics_backend()
    {
        sInstance = this;
    }

    d3d12_graphics_backend::~d3d12_graphics_backend()
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

    d3d12_graphics_backend* d3d12_graphics_backend::instance()
    {
        return sInstance;
    }

    neogfx::renderer d3d12_graphics_backend::renderer() const
    {
        return neogfx::renderer::DirectX;
    }

    void d3d12_graphics_backend::initialize()
    {
        iStatistics.enabled = (std::getenv("NEOGFX_D3D12_STATS") != nullptr);
        iStatistics.periodStart = std::chrono::steady_clock::now();
        if (iDevice)
            return;
        load_libraries();
        create_device();
        create_defaults();
        DXGI_ADAPTER_DESC1 adapterDesc = {};
        iAdapter->GetDesc1(&adapterDesc);
        std::string adapterName;
        for (auto const* c = adapterDesc.Description; *c != L'\0'; ++c)
            adapterName += (*c < 0x80 ? static_cast<char>(*c) : '?');
        service<debug::logger>() << neolib::logger::severity::Debug << "Direct3D 12 adapter: " << adapterName << std::endl;
        service<debug::logger>() << neolib::logger::severity::Debug << "Direct3D 12 dedicated video memory: " <<
            adapterDesc.DedicatedVideoMemory / (1024u * 1024u) << " MiB" << std::endl;
    }

    void d3d12_graphics_backend::cleanup()
    {
        if (iDevice)
        {
            if (iQueue && iFence)
            {
                execute();
                for (std::uint32_t f = 0u; f < FramesInFlight; ++f)
                    retire_frame(f);
            }
            iShadows = nullptr;
            iBackground = nullptr;
            iBlits = nullptr;
            iDummyTexture = nullptr;
            iDummyTextureMS = nullptr;
            iZeroBuffer = d3d12_buffer{};
            for (auto& f : iFrames)
                f = frame{};
            iCommandList = nullptr;
            iSamplerTables.clear();
            iSamplerIds.clear();
            iSamplers.clear();
            iSamplerHeap = nullptr;
            iScratchRtvs.clear();
            iRtvDescriptors.clear();
            iDsvDescriptors.clear();
            iSrvDescriptors.clear();
            iSamplerDescriptors.clear();
            if (iCacheFolder)
                prune_cache_folder(*iCacheFolder);
        }
        if (iFenceEvent != nullptr)
        {
            ::CloseHandle(iFenceEvent);
            iFenceEvent = nullptr;
        }
        iFence = nullptr;
        iQueue = nullptr;
        iDevice = nullptr;
        iAdapter = nullptr;
        iFactory = nullptr;
        iRecording = false;
        iColor = nullptr;
        iDepthStencil = nullptr;
        iProgram = nullptr;
        iVertexArray = nullptr;
        iTextureUnits = {};
        iBarriers.clear();
        iUploadedUniformBlocks.clear();
    }

    void d3d12_graphics_backend::finish()
    {
        execute();
    }

    void d3d12_graphics_backend::execute()
    {
        // n.b. the work recorded so far submitted and all the work in flight done
        if (!iDevice)
            return;
        if (iRecording)
        {
            ++iStatistics.executes;
            submit();
        }
        wait_for_frames();
    }

    void d3d12_graphics_backend::load_libraries()
    {
        // n.b. shaderc_shared.dll is delay loaded (see CMakeLists.txt), and a delay load failure is a structured exception rather
        // than a C++ one, so its absence is reported here (once loaded it stays loaded); the Direct3D 12, DXGI and shader compiler
        // DLLs are loaded rather than linked, so that a program runs (with OpenGL) without them
        if (::LoadLibraryA("shaderc_shared.dll") == nullptr)
            throw failed_to_initialize("shaderc_shared.dll not found");
        try
        {
            load_spirv_cross();
        }
        catch (std::exception const& e)
        {
            throw failed_to_initialize(e.what());
        }
        iD3d12 = ::LoadLibraryA("d3d12.dll");
        if (iD3d12 == nullptr)
            throw failed_to_initialize("d3d12.dll not found");
        iDxgi = ::LoadLibraryA("dxgi.dll");
        if (iDxgi == nullptr)
            throw failed_to_initialize("dxgi.dll not found");
        iD3dCompiler = ::LoadLibraryA("d3dcompiler_47.dll");
        if (iD3dCompiler == nullptr)
            throw failed_to_initialize("d3dcompiler_47.dll not found");
        iD3DCompile = reinterpret_cast<pD3DCompile>(reinterpret_cast<void*>(::GetProcAddress(iD3dCompiler, "D3DCompile")));
        if (iD3DCompile == nullptr)
            throw failed_to_initialize("D3DCompile not found");
    }

    void d3d12_graphics_backend::create_device()
    {
        auto const createFactory = reinterpret_cast<decltype(&CreateDXGIFactory2)>(reinterpret_cast<void*>(::GetProcAddress(iDxgi, "CreateDXGIFactory2")));
        auto const getDebugInterface = reinterpret_cast<PFN_D3D12_GET_DEBUG_INTERFACE>(reinterpret_cast<void*>(::GetProcAddress(iD3d12, "D3D12GetDebugInterface")));
        auto const createDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(reinterpret_cast<void*>(::GetProcAddress(iD3d12, "D3D12CreateDevice")));
        if (createFactory == nullptr || createDevice == nullptr)
            throw failed_to_initialize("Direct3D 12 / DXGI 1.3 entry points not found");

        bool debug = false;
#ifndef NDEBUG
        debug = true;
#endif
        if (std::getenv("NEOGFX_D3D12_VALIDATION") != nullptr)
            debug = true;
        UINT factoryFlags = 0u;
        if (debug && getDebugInterface != nullptr)
        {
            com_ptr<ID3D12Debug> debugController;
            if (SUCCEEDED(getDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
        // n.b. the DXGI debug layer may not be installed
        if (FAILED(createFactory(factoryFlags, IID_PPV_ARGS(&iFactory))))
            dxCheck(createFactory(0u, IID_PPV_ARGS(&iFactory)));

        com_ptr<IDXGIFactory5> factory5;
        if (SUCCEEDED(iFactory.As(&factory5)))
        {
            BOOL allowTearing = FALSE;
            if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
                iTearing = (allowTearing != FALSE);
        }

        // the high performance (e.g. discrete) adapter first (n.b. none if software rendering (--software) is asked for: WARP)
        auto const& options = service<i_app>().program_options().options();
        bool const software = (options.count("software") != 0);
        com_ptr<IDXGIFactory6> factory6;
        iFactory.As(&factory6);
        for (UINT index = 0u; !software; ++index)
        {
            com_ptr<IDXGIAdapter1> adapter;
            auto const result = factory6 ?
                factory6->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) :
                iFactory->EnumAdapters1(index, &adapter);
            if (result == DXGI_ERROR_NOT_FOUND)
                break;
            if (FAILED(result))
                continue;
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);
            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0u)
                continue;
            if (SUCCEEDED(createDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&iDevice))))
            {
                iAdapter = adapter;
                break;
            }
        }
        if (!iDevice)
        {
            // (software rendering (WARP) only if DirectX (--dx) or software rendering (--software) is asked for: OpenGL, which may
            // be hardware accelerated, otherwise)
            if (options.count("dx") == 0 && !software)
                throw failed_to_initialize("no hardware Direct3D 12 (feature level 11.0) adapter");
            com_ptr<IDXGIAdapter1> warp;
            if (FAILED(iFactory->EnumWarpAdapter(IID_PPV_ARGS(&warp))) || FAILED(createDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&iDevice))))
                throw failed_to_initialize("no Direct3D 12 (feature level 11.0) adapter");
            iAdapter = warp;
            iWarp = true;
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        dxCheck(iDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&iQueue)));
        dxCheck(iDevice->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&iFence)));
        iFenceEvent = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (iFenceEvent == nullptr)
            throw failed_to_initialize("CreateEvent");
        for (auto& f : iFrames)
            dxCheck(iDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&f.allocator)));
        dxCheck(iDevice->CreateCommandList(0u, D3D12_COMMAND_LIST_TYPE_DIRECT, iFrames[0].allocator.Get(), nullptr, IID_PPV_ARGS(&iCommandList)));
        dxCheck(iCommandList->Close());
        iFrame = 0u;
        iRecording = false;

        iRtvDescriptors.initialize(iDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        iDsvDescriptors.initialize(iDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        iSrvDescriptors.initialize(iDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        iSamplerDescriptors.initialize(iDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 256u);
        iSrvIncrement = iDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        iSamplerIncrement = iDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        iRtvIncrement = iDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.NumDescriptors = SamplerHeapSize;
        samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        dxCheck(iDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&iSamplerHeap)));
        iSamplerHeapUsed = 0u;
        for (std::uint32_t i = 0u; i < ScratchRtvCount; ++i)
            iScratchRtvs.push_back(iRtvDescriptors.allocate());
    }

    void d3d12_graphics_backend::create_defaults()
    {
        // what a texture unit without a texture (or with a texture of the wrong kind) samples
        iDummyTexture = create_image(1u, 1u, DXGI_FORMAT_R8G8B8A8_UNORM, 4u);
        iDummyTextureMS = create_image(1u, 1u, DXGI_FORMAT_R8G8B8A8_UNORM, 4u, sample_count(4u));
        // what a vertex input variable without an attribute (and an SSBO without a buffer) reads
        iZeroBuffer = allocate_buffer(ZeroBufferSize, true);
        std::memset(iZeroBuffer.mapping, 0, static_cast<std::size_t>(iZeroBuffer.size));
        execute();
    }

    std::unique_ptr<texture_manager> d3d12_graphics_backend::create_texture_manager()
    {
        return std::make_unique<d3d12_texture_manager>();
    }

    ref_ptr<i_shader_program> d3d12_graphics_backend::create_standard_shader_program()
    {
        return neolib::make_ref<d3d12_standard_shader_program>().as<i_shader_program>();
    }

    void* d3d12_graphics_backend::create_shader_program_object()
    {
        return new d3d12_program{};
    }

    void d3d12_graphics_backend::destroy_shader_program_object(void* aShaderProgramObject)
    {
        auto* program = static_cast<d3d12_program*>(aShaderProgramObject);
        if (program == nullptr)
            return;
        if (iProgram == program)
            iProgram = nullptr;
        if (alive())
            release_program(*program);
        delete program;
    }

    void* d3d12_graphics_backend::create_shader_object(shader_type aShaderType)
    {
        return new d3d12_shader_object{ aShaderType };
    }

    void d3d12_graphics_backend::destroy_shader_object(void* aShaderObject)
    {
        // n.b. a shader object has no Direct3D objects (a program's pipelines are released with the program)
        delete static_cast<d3d12_shader_object*>(aShaderObject);
    }

    viewport d3d12_graphics_backend::viewport() const
    {
        return rect{ point{ static_cast<scalar>(iState.viewport[0]), static_cast<scalar>(iState.viewport[1]) },
            size{ static_cast<scalar>(iState.viewport[2]), static_cast<scalar>(iState.viewport[3]) } };
    }

    std::optional<rect> d3d12_graphics_backend::scissor() const
    {
        if (!iState.scissorEnabled)
            return std::nullopt;
        return rect{ point{ static_cast<scalar>(iState.scissor[0]), static_cast<scalar>(iState.scissor[1]) },
            size{ static_cast<scalar>(iState.scissor[2]), static_cast<scalar>(iState.scissor[3]) } };
    }

    d3d12_buffer d3d12_graphics_backend::allocate_buffer(std::uint64_t aSize, bool aMapped)
    {
        d3d12_buffer result;
        result.size = std::max<std::uint64_t>(aSize, 16u);
        result.deviceLocal = !aMapped;
        // n.b. mapped buffers (vertices, SSBOs and the transient memory) are in an upload heap (host memory the GPU reads), in the
        // generic read state in which an upload heap's resources must stay; device local buffers are in the default heap
        auto const heap = heap_properties(aMapped ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
        auto const desc = buffer_desc(result.size);
        dxCheck(iDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
            aMapped ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&result.resource)));
        if (aMapped)
            dxCheck(result.resource->Map(0u, nullptr, &result.mapping));
        return result;
    }

    d3d12_graphics_backend::transient_allocation d3d12_graphics_backend::allocate_transient(std::uint64_t aSize, std::uint64_t aAlignment)
    {
        iStatistics.transientBytes += aSize;
        command_list(); // n.b. the current frame's transient memory is reusable once it is recording
        auto& f = iFrames[iFrame];
        while (f.transientChunk < f.transient.size())
        {
            auto& chunk = f.transient[f.transientChunk];
            auto const offset = align_up(chunk.used, aAlignment);
            if (offset + aSize <= chunk.buffer.size)
            {
                chunk.used = offset + aSize;
                return { chunk.buffer.resource.Get(), offset, static_cast<std::uint8_t*>(chunk.buffer.mapping) + offset,
                    chunk.buffer.resource->GetGPUVirtualAddress() + offset };
            }
            ++f.transientChunk;
        }
        f.transient.push_back(transient_chunk{ allocate_buffer(std::max(TransientChunkSize, align_up<std::uint64_t>(aSize, 65536u)), true) });
        f.transientChunk = f.transient.size() - 1u;
        auto& chunk = f.transient.back();
        chunk.used = aSize;
        return { chunk.buffer.resource.Get(), 0u, chunk.buffer.mapping, chunk.buffer.resource->GetGPUVirtualAddress() };
    }

    d3d12_graphics_backend::descriptor_range d3d12_graphics_backend::allocate_descriptors(std::uint32_t aCount)
    {
        iStatistics.descriptors += aCount;
        command_list(); // n.b. as allocate_transient
        auto& f = iFrames[iFrame];
        while (f.descriptorHeap < f.descriptorHeaps.size() && f.descriptorHeaps[f.descriptorHeap].used + aCount > VisibleHeapSize)
        {
            ++f.descriptorHeap;
            // n.b. the heap changes
            iHeapsBound = false;
        }
        if (f.descriptorHeap == f.descriptorHeaps.size())
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.NumDescriptors = VisibleHeapSize;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            visible_heap heap;
            dxCheck(iDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap.heap)));
            f.descriptorHeaps.push_back(heap);
            iHeapsBound = false;
        }
        auto& heap = f.descriptorHeaps[f.descriptorHeap];
        descriptor_range const result{
            offset(heap.heap->GetCPUDescriptorHandleForHeapStart(), heap.used, iSrvIncrement),
            offset(heap.heap->GetGPUDescriptorHandleForHeapStart(), heap.used, iSrvIncrement) };
        heap.used += aCount;
        return result;
    }

    void d3d12_graphics_backend::defer(std::function<void()> aDestroy)
    {
        // n.b. with the current frame recording, so that this frame's previous use is done and the destruction follows the frames
        // submitted so far
        command_list();
        iFrames[iFrame].deferred.push_back(std::move(aDestroy));
    }

    ID3D12GraphicsCommandList* d3d12_graphics_backend::command_list()
    {
        if (!iRecording)
        {
            // this frame's previous use must be done before its command allocator and memory are reused
            auto& f = iFrames[iFrame];
            if (f.inFlight)
            {
                wait_for(f.serial);
                retire_frame(iFrame);
            }
            dxCheck(iCommandList->Reset(f.allocator.Get(), nullptr));
            iRecording = true;
            ++iRecordings;
            invalidate_bindings();
        }
        return iCommandList.Get();
    }

    void d3d12_graphics_backend::submit()
    {
        command_list();
        flush_barriers();
        dxCheck(iCommandList->Close());
        iRecording = false;
        ID3D12CommandList* const lists[] = { iCommandList.Get() };
        iQueue->ExecuteCommandLists(1u, lists);
        auto& f = iFrames[iFrame];
        f.serial = ++iSubmitted;
        dxCheck(iQueue->Signal(iFence.Get(), f.serial));
        f.inFlight = true;
        ++iStatistics.submits;
        // n.b. not waited for: the next frame is recorded while the GPU executes this one (see command_list and wait_for_frames);
        // the uniform blocks uploaded are in this frame's transient memory
        iUploadedUniformBlocks.clear();
        iFrame = (iFrame + 1u) % FramesInFlight;
    }

    void d3d12_graphics_backend::wait_for(std::uint64_t aSerial)
    {
        if (iFence->GetCompletedValue() < aSerial)
        {
            auto const waitStart = std::chrono::steady_clock::now();
            dxCheck(iFence->SetEventOnCompletion(aSerial, iFenceEvent));
            ::WaitForSingleObject(iFenceEvent, INFINITE);
            iStatistics.fenceWait += std::chrono::steady_clock::now() - waitStart;
        }
        iCompleted = std::max(iCompleted, aSerial);
    }

    void d3d12_graphics_backend::wait_for_frames()
    {
        // all the frames in flight
        for (std::uint32_t i = 1u; i <= FramesInFlight; ++i)
        {
            auto const index = (iFrame + i) % FramesInFlight;
            auto& f = iFrames[index];
            if (!f.inFlight)
                continue;
            wait_for(f.serial);
            retire_frame(index);
        }
    }

    void d3d12_graphics_backend::poll_frames()
    {
        // the frames in flight that the GPU has completed (n.b. without waiting)
        auto const completed = iFence->GetCompletedValue();
        for (std::uint32_t index = 0u; index < FramesInFlight; ++index)
            if (iFrames[index].inFlight && iFrames[index].serial <= completed)
                retire_frame(index);
    }

    void d3d12_graphics_backend::retire_frame(std::uint32_t aFrame)
    {
        // the GPU is done with everything recorded for the frame
        auto& f = iFrames[aFrame];
        if (f.inFlight)
        {
            dxCheck(f.allocator->Reset());
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
        for (auto& heap : f.descriptorHeaps)
            heap.used = 0u;
        f.descriptorHeap = 0u;
    }

    void d3d12_graphics_backend::transition(ID3D12Resource* aResource, D3D12_RESOURCE_STATES aBefore, D3D12_RESOURCE_STATES aAfter, std::uint32_t aSubresource)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = aResource;
        barrier.Transition.Subresource = aSubresource;
        barrier.Transition.StateBefore = aBefore;
        barrier.Transition.StateAfter = aAfter;
        iBarriers.push_back(barrier);
    }

    void d3d12_graphics_backend::transition(d3d12_image& aImage, D3D12_RESOURCE_STATES aState)
    {
        if (aImage.state == aState)
            return;
        command_list();
        transition(aImage.resource.Get(), aImage.state, aState);
        aImage.state = aState;
    }

    void d3d12_graphics_backend::transition(d3d12_buffer& aBuffer, D3D12_RESOURCE_STATES aState)
    {
        if (!aBuffer.deviceLocal)
            return; // n.b. an upload heap buffer stays in the generic read state
        command_list();
        auto const current = (aBuffer.stateRecording == iRecordings ? aBuffer.state : D3D12_RESOURCE_STATE_COMMON);
        aBuffer.stateRecording = iRecordings;
        if (current == aState)
            return;
        transition(aBuffer.resource.Get(), current, aState);
        aBuffer.state = aState;
    }

    void d3d12_graphics_backend::flush_barriers()
    {
        if (iBarriers.empty())
            return;
        iStatistics.barriers += iBarriers.size();
        iCommandList->ResourceBarrier(static_cast<UINT>(iBarriers.size()), iBarriers.data());
        iBarriers.clear();
    }

    void d3d12_graphics_backend::bind_descriptor_heaps()
    {
        if (iHeapsBound)
            return;
        auto& f = iFrames[iFrame];
        if (f.descriptorHeap == f.descriptorHeaps.size())
            (void)allocate_descriptors(0u);
        ID3D12DescriptorHeap* const heaps[] = { f.descriptorHeaps[f.descriptorHeap].heap.Get(), iSamplerHeap.Get() };
        command_list()->SetDescriptorHeaps(2u, heaps);
        iHeapsBound = true;
    }

    void d3d12_graphics_backend::bind_render_target()
    {
        if (iTargetBound)
            return;
        auto* const commandList = command_list();
        D3D12_CPU_DESCRIPTOR_HANDLE const* const color = (iColor != nullptr ? &iColor->rtv.cpu : nullptr);
        D3D12_CPU_DESCRIPTOR_HANDLE const* const depthStencil = (iDepthStencil != nullptr ? &iDepthStencil->dsv.cpu : nullptr);
        commandList->OMSetRenderTargets(color != nullptr ? 1u : 0u, color, FALSE, depthStencil);
        iTargetBound = true;
    }

    void d3d12_graphics_backend::invalidate_bindings()
    {
        iTargetBound = false;
        iHeapsBound = false;
        iViewportValid = false;
        iBoundRootSignature = nullptr;
        iBoundPipeline = nullptr;
        iBoundStencilReference = std::nullopt;
    }

    std::array<std::uint32_t, 2> d3d12_graphics_backend::target_extent() const
    {
        auto const* image = (iColor != nullptr ? iColor : iDepthStencil);
        if (image == nullptr)
            return { 0u, 0u };
        return { image->width, image->height };
    }

    gpu_buffer d3d12_graphics_backend::create_buffer(std::size_t aSize, bool aDeviceLocal)
    {
        auto* buffer = new d3d12_buffer{ allocate_buffer(aSize, !aDeviceLocal) };
        return reinterpret_cast<gpu_buffer>(buffer);
    }

    void d3d12_graphics_backend::destroy_buffer(gpu_buffer aBuffer)
    {
        auto* buffer = buffer_of(aBuffer);
        if (buffer == nullptr)
            return;
        if (alive())
        {
            // n.b. released once the GPU is done with it (and its spares)
            std::vector<com_ptr<ID3D12Resource>> resources{ buffer->resource };
            for (auto const& s : buffer->spares)
                resources.push_back(s.resource);
            defer([resources]() {});
        }
        delete buffer;
    }

    void* d3d12_graphics_backend::map_buffer(gpu_buffer aBuffer, std::size_t)
    {
        auto* buffer = buffer_of(aBuffer);
        if (buffer->mapping == nullptr)
            throw std::logic_error("neogfx::d3d12_graphics_backend::map_buffer: device local buffer cannot be mapped");
        // n.b. persistently mapped
        return buffer->mapping;
    }

    void d3d12_graphics_backend::flush_buffer(gpu_buffer, std::size_t, std::size_t)
    {
        // nothing to do: an upload heap is coherent and host writes are visible to the work submitted after them
    }

    void d3d12_graphics_backend::unmap_buffer(gpu_buffer)
    {
        // nothing to do: mapped buffers stay mapped
    }

    bool d3d12_graphics_backend::discard_buffer(gpu_buffer aBuffer)
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
        d3d12_buffer::spare retiring{ buffer->resource, buffer->mapping, lastUse };
        auto reusable = std::find_if(buffer->spares.begin(), buffer->spares.end(),
            [&](d3d12_buffer::spare const& s) { return s.retired <= iCompleted; });
        if (reusable != buffer->spares.end())
        {
            buffer->resource = reusable->resource;
            buffer->mapping = reusable->mapping;
            *reusable = retiring;
        }
        else
        {
            auto const fresh = allocate_buffer(buffer->size, true);
            buffer->resource = fresh.resource;
            buffer->mapping = fresh.mapping;
            buffer->spares.push_back(retiring);
        }
        ++iStatistics.discards;
        return true;
    }

    void d3d12_graphics_backend::write_buffer(gpu_buffer aBuffer, std::size_t aOffset, void const* aData, std::size_t aSize)
    {
        auto* buffer = buffer_of(aBuffer);
        if (buffer->mapping != nullptr)
        {
            std::memcpy(static_cast<std::uint8_t*>(buffer->mapping) + aOffset, aData, aSize);
            return;
        }
        if (aSize == 0u)
            return;
        auto const staging = allocate_transient(aSize, 16u);
        std::memcpy(staging.mapping, aData, aSize);
        transition(*buffer, D3D12_RESOURCE_STATE_COPY_DEST);
        flush_barriers();
        command_list()->CopyBufferRegion(buffer->resource.Get(), aOffset, staging.resource, staging.offset, aSize);
    }

    void d3d12_graphics_backend::copy_buffer(gpu_buffer aSource, gpu_buffer aDestination, std::size_t aSize)
    {
        auto* source = buffer_of(aSource);
        auto* destination = buffer_of(aDestination);
        if (aSize == 0u)
            return;
        if (destination->mapping != nullptr)
        {
            if (source->mapping != nullptr)
            {
                // n.b. what was written through the source's mapping (the draws already recorded keep using the source buffer);
                // the GPU cannot write to an upload heap
                std::memcpy(destination->mapping, source->mapping, aSize);
                return;
            }
            // device local to mapped: by way of a readback buffer
            auto readbackHeap = heap_properties(D3D12_HEAP_TYPE_READBACK);
            auto const desc = buffer_desc(aSize);
            com_ptr<ID3D12Resource> readback;
            dxCheck(iDevice->CreateCommittedResource(&readbackHeap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)));
            transition(*source, D3D12_RESOURCE_STATE_COPY_SOURCE);
            flush_barriers();
            command_list()->CopyBufferRegion(readback.Get(), 0u, source->resource.Get(), 0u, aSize);
            execute();
            void* mapping = nullptr;
            D3D12_RANGE const readRange{ 0u, aSize };
            dxCheck(readback->Map(0u, &readRange, &mapping));
            std::memcpy(destination->mapping, mapping, aSize);
            D3D12_RANGE const noWrite{ 0u, 0u };
            readback->Unmap(0u, &noWrite);
            return;
        }
        transition(*source, D3D12_RESOURCE_STATE_COPY_SOURCE);
        transition(*destination, D3D12_RESOURCE_STATE_COPY_DEST);
        flush_barriers();
        command_list()->CopyBufferRegion(destination->resource.Get(), 0u, source->resource.Get(), 0u, aSize);
    }

    gpu_vertex_array d3d12_graphics_backend::create_vertex_array()
    {
        return reinterpret_cast<gpu_vertex_array>(new vertex_array{});
    }

    void d3d12_graphics_backend::destroy_vertex_array(gpu_vertex_array aVertexArray)
    {
        auto* vertexArray = reinterpret_cast<vertex_array*>(aVertexArray);
        if (iVertexArray == vertexArray)
            iVertexArray = nullptr;
        delete vertexArray;
    }

    gpu_vertex_array d3d12_graphics_backend::bound_vertex_array() const
    {
        return reinterpret_cast<gpu_vertex_array>(iVertexArray);
    }

    void d3d12_graphics_backend::bind_vertex_array(gpu_vertex_array aVertexArray)
    {
        iVertexArray = reinterpret_cast<vertex_array*>(aVertexArray);
    }

    void d3d12_graphics_backend::set_vertex_attribute(i_shader_program const& aProgram, std::string const& aName, gpu_buffer aBuffer,
        std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset)
    {
        auto const& program = *static_cast<d3d12_program const*>(aProgram.handle());
        auto const input = program.vertexInputs.find(aName);
        if (input == program.vertexInputs.end())
            return;
        set_vertex_attribute(input->second.location, aBuffer, aArity, aType, aNormalized, aStride, aOffset);
    }

    void d3d12_graphics_backend::set_vertex_attribute(std::uint32_t aLocation, gpu_buffer aBuffer,
        std::uint32_t aArity, gpu_attribute_type aType, bool aNormalized, std::size_t aStride, std::size_t aOffset)
    {
        if (iVertexArray == nullptr)
            throw no_vertex_array();
        auto const format = to_d3d12_attribute_format(aArity, aType, aNormalized);
        iVertexArray->attributes[aLocation] = vertex_array::attribute{ aBuffer, format.first, format.second,
            static_cast<std::uint32_t>(aStride), static_cast<std::uint32_t>(aOffset) };
    }

    void d3d12_graphics_backend::set_index_buffer(gpu_buffer aBuffer)
    {
        if (iVertexArray == nullptr)
            throw no_vertex_array();
        iVertexArray->indexBuffer = aBuffer;
    }

    std::uint32_t d3d12_graphics_backend::blend_key() const
    {
        if (!iState.blending)
            return 0u;
        if (iState.xorBlending)
            return 0x100u;
        return 1u + static_cast<std::uint32_t>(iState.blendingMode);
    }

    D3D12_BLEND_DESC d3d12_graphics_backend::blend_desc(std::uint32_t aBlend, bool aColorWrite) const
    {
        auto result = opaque_blend_desc();
        auto& target = result.RenderTarget[0];
        target.RenderTargetWriteMask = aColorWrite ? D3D12_COLOR_WRITE_ENABLE_ALL : 0u;
        auto const set_blend = [&](D3D12_BLEND_OP aColorOp, D3D12_BLEND aSrcColor, D3D12_BLEND aDstColor, D3D12_BLEND_OP aAlphaOp, D3D12_BLEND aSrcAlpha, D3D12_BLEND aDstAlpha)
            {
                target.BlendEnable = TRUE;
                target.BlendOp = aColorOp;
                target.SrcBlend = aSrcColor;
                target.DestBlend = aDstColor;
                target.BlendOpAlpha = aAlphaOp;
                target.SrcBlendAlpha = aSrcAlpha;
                target.DestBlendAlpha = aDstAlpha;
            };
        // n.b. as opengl_graphics_backend::set_blending_mode and set_xor_blending (a colour factor for alpha is its alpha factor)
        if (aBlend == 0x100u)
            set_blend(D3D12_BLEND_OP_ADD, D3D12_BLEND_INV_DEST_COLOR, D3D12_BLEND_INV_SRC_COLOR,
                D3D12_BLEND_OP_ADD, D3D12_BLEND_INV_DEST_ALPHA, D3D12_BLEND_INV_SRC_ALPHA);
        else if (aBlend != 0u)
        {
            switch (static_cast<neogfx::blending_mode>(aBlend - 1u))
            {
            case neogfx::blending_mode::Unspecified:
            case neogfx::blending_mode::None:
                break;
            case neogfx::blending_mode::Default:
                set_blend(D3D12_BLEND_OP_ADD, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA,
                    D3D12_BLEND_OP_ADD, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Sprite:
            case neogfx::blending_mode::FilterFinish:
                set_blend(D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA,
                    D3D12_BLEND_OP_ADD, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Blit:
                set_blend(D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_ZERO,
                    D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_ZERO);
                break;
            case neogfx::blending_mode::Lighten:
                set_blend(D3D12_BLEND_OP_MAX, D3D12_BLEND_ONE, D3D12_BLEND_ONE,
                    D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Filter:
                set_blend(D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA,
                    D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA);
                break;
            case neogfx::blending_mode::Premultiply:
                set_blend(D3D12_BLEND_OP_ADD, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA,
                    D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA);
                break;
            }
        }
        return result;
    }

    ID3D12PipelineState* d3d12_graphics_backend::pipeline(d3d12_program& aProgram, d3d12_input_layout const& aInputLayout)
    {
        d3d12_pipeline_key key;
        key.inputLayout = aInputLayout;
        key.colorFormat = (iColor != nullptr ? iColor->format : DXGI_FORMAT_UNKNOWN);
        key.depthStencilFormat = (iDepthStencil != nullptr ? iDepthStencil->format : DXGI_FORMAT_UNKNOWN);
        key.samples = (iColor != nullptr ? iColor->samples : iDepthStencil->samples);
        key.blend = blend_key();
        key.colorWrite = iState.colorWrite;
        key.depthTest = iState.depthTest && iDepthStencil != nullptr;
        key.depthWrite = key.depthTest && iState.depthWrite;
        key.depthCompare = key.depthTest ? iState.depthCompare : D3D12_COMPARISON_FUNC_LESS;
        key.stencilTest = iState.stencilTest && iDepthStencil != nullptr;
        if (key.stencilTest)
        {
            key.stencilCompare = iState.stencilCompare;
            key.stencilPassOp = iState.stencilPassOp;
            key.stencilReadMask = iState.stencilCompareMask;
            key.stencilWriteMask = iState.stencilWriteMask;
        }
        switch (iState.faceCulling)
        {
        case neogfx::face_culling::None:
        case neogfx::face_culling::FrontAndBack: // (not drawn: see prepare_draw)
            key.cullMode = D3D12_CULL_MODE_NONE;
            break;
        case neogfx::face_culling::Front:
            key.cullMode = !iState.faceCullingFlipped ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK;
            break;
        case neogfx::face_culling::Back:
            key.cullMode = !iState.faceCullingFlipped ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_FRONT;
            break;
        }
        // n.b. the winding of a triangle as OpenGL sees it (window y up) is the opposite of the winding Direct3D sees for the same
        // render target coordinates (y down)
        key.frontCounterClockwise = (iState.frontFace == neogfx::front_face::Clockwise);
        auto existing = aProgram.pipelines.find(key);
        if (existing != aProgram.pipelines.end())
            return existing->second.Get();

        auto const vertexShader = aProgram.shaders[static_cast<std::size_t>(shader_type::Vertex)];
        auto const fragmentShader = aProgram.shaders[static_cast<std::size_t>(shader_type::Fragment)];
        auto const geometryShader = aProgram.shaders[static_cast<std::size_t>(shader_type::Geometry)];
        if (vertexShader == nullptr)
            throw no_program();
        auto const vs = shader_bytecode(*vertexShader, key.samples, aInputLayout.scaledLocations);
        auto const ps = (fragmentShader != nullptr ? shader_bytecode(*fragmentShader, key.samples, 0u) : d3d12_bytecode{});
        auto const gs = (geometryShader != nullptr ? shader_bytecode(*geometryShader, key.samples, 0u) : d3d12_bytecode{});

        std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
        for (auto const& e : aInputLayout.elements)
            elements.push_back(D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", e.location, e.format, e.slot, e.offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u });

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
        desc.pRootSignature = aProgram.rootSignature.Get();
        desc.VS = to_bytecode(vs);
        desc.PS = to_bytecode(ps);
        desc.GS = to_bytecode(gs);
        desc.BlendState = blend_desc(key.blend, key.colorWrite);
        desc.SampleMask = UINT_MAX;
        desc.RasterizerState = rasterizer_desc(key.cullMode, key.frontCounterClockwise, key.samples);
        desc.DepthStencilState = no_depth_stencil_desc();
        desc.DepthStencilState.DepthEnable = key.depthTest ? TRUE : FALSE;
        desc.DepthStencilState.DepthWriteMask = key.depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        desc.DepthStencilState.DepthFunc = key.depthCompare;
        desc.DepthStencilState.StencilEnable = key.stencilTest ? TRUE : FALSE;
        desc.DepthStencilState.StencilReadMask = key.stencilReadMask;
        desc.DepthStencilState.StencilWriteMask = key.stencilWriteMask;
        desc.DepthStencilState.FrontFace = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, key.stencilPassOp, key.stencilCompare };
        desc.DepthStencilState.BackFace = desc.DepthStencilState.FrontFace;
        desc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ elements.data(), static_cast<UINT>(elements.size()) };
        desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = (key.colorFormat != DXGI_FORMAT_UNKNOWN ? 1u : 0u);
        desc.RTVFormats[0] = key.colorFormat;
        desc.DSVFormat = key.depthStencilFormat;
        desc.SampleDesc.Count = key.samples;
        desc.SampleDesc.Quality = 0u;
        com_ptr<ID3D12PipelineState> result;
        dxCheck(iDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&result)));
        ++iStatistics.pipelinesCreated;
        return aProgram.pipelines.emplace(key, result).first->second.Get();
    }

    void d3d12_graphics_backend::apply_viewport_and_scissor()
    {
        auto* const commandList = command_list();
        auto const extent = target_extent();
        D3D12_VIEWPORT const viewport{ static_cast<float>(iState.viewport[0]), static_cast<float>(iState.viewport[1]),
            static_cast<float>(iState.viewport[2]), static_cast<float>(iState.viewport[3]), 0.0f, 1.0f };
        commandList->RSSetViewports(1u, &viewport);
        D3D12_RECT scissor{ 0, 0, static_cast<LONG>(extent[0]), static_cast<LONG>(extent[1]) };
        if (iState.scissorEnabled)
        {
            std::int64_t const x0 = std::clamp<std::int64_t>(iState.scissor[0], 0, extent[0]);
            std::int64_t const y0 = std::clamp<std::int64_t>(iState.scissor[1], 0, extent[1]);
            std::int64_t const x1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor[0]) + iState.scissor[2], x0, extent[0]);
            std::int64_t const y1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(iState.scissor[1]) + iState.scissor[3], y0, extent[1]);
            scissor = D3D12_RECT{ static_cast<LONG>(x0), static_cast<LONG>(y0), static_cast<LONG>(x1), static_cast<LONG>(y1) };
        }
        commandList->RSSetScissorRects(1u, &scissor);
        iViewportValid = true;
    }

    d3d12_image& d3d12_graphics_backend::texture_unit_image(std::uint32_t aTextureUnit, bool aMultisample)
    {
        auto* image = (aTextureUnit < iTextureUnits.size() ? iTextureUnits[aTextureUnit] : nullptr);
        if (image == nullptr || (image->samples != 1u) != aMultisample || (image->depthStencil && !image->srv.valid()))
            return aMultisample ? *iDummyTextureMS : *iDummyTexture;
        // n.b. a texture can't be sampled while it is being drawn to (see has_texture_barrier)
        if (image == iColor || image == iDepthStencil)
            return aMultisample ? *iDummyTextureMS : *iDummyTexture;
        return *image;
    }

    std::uint32_t d3d12_graphics_backend::sampler(d3d12_sampler_state const& aState, std::uint32_t aMipLevels)
    {
        bool const mipmapped = aState.mipmap && aMipLevels > 1u;
        auto const key = std::make_pair(aState, mipmapped);
        auto existing = iSamplerIds.find(key);
        if (existing != iSamplerIds.end())
            return existing->second;
        D3D12_SAMPLER_DESC desc = {};
        // (see D3D12_ENCODE_BASIC_FILTER: minification, magnification and mip filters of the standard reduction)
        desc.Filter = static_cast<D3D12_FILTER>(
            ((aState.minLinear ? 1u : 0u) << 4u) | ((aState.magLinear ? 1u : 0u) << 2u) | (mipmapped ? 1u : 0u));
        desc.AddressU = aState.addressMode;
        desc.AddressV = aState.addressMode;
        desc.AddressW = aState.addressMode;
        desc.MipLODBias = 0.0f;
        desc.MaxAnisotropy = 1u;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        // n.b. transparent black (as opengl_texture)
        desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 0.0f;
        desc.MinLOD = 0.0f;
        // n.b. as GL_LINEAR/GL_NEAREST minification: the base level only
        desc.MaxLOD = mipmapped ? D3D12_FLOAT32_MAX : 0.0f;
        auto descriptor = iSamplerDescriptors.allocate();
        iDevice->CreateSampler(&desc, descriptor.cpu);
        auto const id = static_cast<std::uint32_t>(iSamplers.size());
        iSamplers.push_back(descriptor);
        iSamplerIds.emplace(key, id);
        return id;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE d3d12_graphics_backend::sampler_table(std::vector<std::uint32_t> const& aSamplers)
    {
        auto existing = iSamplerTables.find(aSamplers);
        if (existing == iSamplerTables.end())
        {
            if (iSamplerHeapUsed + aSamplers.size() > SamplerHeapSize)
            {
                // the heap is full: emptied once the GPU is done with the tables in it (n.b. so this must be done before anything
                // that is to be drawn with the table is recorded)
                execute();
                iSamplerTables.clear();
                iSamplerHeapUsed = 0u;
                ++iSamplerHeapGeneration;
            }
            auto const start = iSamplerHeapUsed;
            for (std::size_t i = 0u; i < aSamplers.size(); ++i)
                iDevice->CopyDescriptorsSimple(1u, offset(iSamplerHeap->GetCPUDescriptorHandleForHeapStart(), start + static_cast<std::uint32_t>(i), iSamplerIncrement),
                    iSamplers[aSamplers[i]].cpu, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            iSamplerHeapUsed += static_cast<std::uint32_t>(aSamplers.size());
            existing = iSamplerTables.emplace(aSamplers, start).first;
        }
        return offset(iSamplerHeap->GetGPUDescriptorHandleForHeapStart(), existing->second, iSamplerIncrement);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE d3d12_graphics_backend::texture_table(std::vector<d3d12_image*> const& aImages)
    {
        auto const range = allocate_descriptors(static_cast<std::uint32_t>(aImages.size()));
        for (std::size_t i = 0u; i < aImages.size(); ++i)
            iDevice->CopyDescriptorsSimple(1u, offset(range.cpu, static_cast<std::uint32_t>(i), iSrvIncrement), aImages[i]->srv.cpu,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        return range.gpu;
    }

    bool d3d12_graphics_backend::prepare_draw(gpu_primitive aPrimitive, bool aIndexed)
    {
        ++iStatistics.draws;
        if (iProgram == nullptr || !iProgram->linked)
            throw no_program();
        if (iColor == nullptr && iDepthStencil == nullptr)
            throw no_target();
        if (iState.faceCulling == neogfx::face_culling::FrontAndBack)
            return false; // n.b. every triangle is culled (Direct3D 12 has no such cull mode)
        auto& program = *iProgram;
        command_list();

        // the textures and their samplers (n.b. the sampler table first, before anything is recorded or transitioned: getting
        // it may wait for the GPU, so submitting what has been recorded so far)
        thread_local std::vector<d3d12_image*> tImages;
        thread_local std::vector<std::uint32_t> tSamplers;
        tImages.assign(program.samplerSlots, nullptr);
        tSamplers.assign(program.samplerSlots, 0u);
        for (auto const& s : program.samplers)
        {
            auto const slot = s.binding - SamplerBindingBase;
            if (slot >= program.samplerSlots)
                continue;
            auto& image = texture_unit_image(static_cast<std::uint32_t>(std::max(s.textureUnit, 0)), s.multisample);
            tImages[slot] = &image;
            tSamplers[slot] = sampler(image.sampler, image.mipLevels);
        }
        for (std::size_t slot = 0u; slot < tImages.size(); ++slot)
            if (tImages[slot] == nullptr)
            {
                tImages[slot] = iDummyTexture.get();
                tSamplers[slot] = sampler(iDummyTexture->sampler, 1u);
            }
        D3D12_GPU_DESCRIPTOR_HANDLE samplerTable = {};
        if (program.samplerSlots != 0u)
            samplerTable = sampler_table(tSamplers);

        // the input layout: a slot per buffer and stride
        thread_local d3d12_input_layout tLayout;
        thread_local std::vector<std::pair<gpu_buffer, std::uint32_t>> tSlots;
        thread_local std::vector<D3D12_VERTEX_BUFFER_VIEW> tViews;
        tLayout.elements.clear();
        tLayout.scaledLocations = 0u;
        tSlots.clear();
        tViews.clear();
        auto const slot_of = [&](gpu_buffer aBuffer, std::uint32_t aStride) -> std::uint32_t
            {
                auto existing = std::find(tSlots.begin(), tSlots.end(), std::make_pair(aBuffer, aStride));
                if (existing != tSlots.end())
                    return static_cast<std::uint32_t>(std::distance(tSlots.begin(), existing));
                tSlots.emplace_back(aBuffer, aStride);
                auto* buffer = buffer_of(aBuffer);
                if (buffer != nullptr)
                {
                    transition(*buffer, BufferReadState);
                    tViews.push_back(D3D12_VERTEX_BUFFER_VIEW{ buffer->resource->GetGPUVirtualAddress(), static_cast<UINT>(buffer->size), aStride });
                }
                else
                    tViews.push_back(D3D12_VERTEX_BUFFER_VIEW{ iZeroBuffer.resource->GetGPUVirtualAddress(), static_cast<UINT>(iZeroBuffer.size), 0u });
                return static_cast<std::uint32_t>(tSlots.size() - 1u);
            };
        for (auto const& input : program.vertexInputs)
        {
            auto const location = input.second.location;
            if (iVertexArray != nullptr)
            {
                auto existing = iVertexArray->attributes.find(location);
                if (existing != iVertexArray->attributes.end() && existing->second.buffer != no_gpu_buffer)
                {
                    auto const& a = existing->second;
                    tLayout.elements.push_back({ location, slot_of(a.buffer, a.stride), a.format, a.offset });
                    if (a.scaled && !integer_input(input.second.type) && location < 32u)
                        tLayout.scaledLocations |= (1u << location);
                    continue;
                }
            }
            // n.b. read as zero (cf. a disabled OpenGL vertex attribute array)
            tLayout.elements.push_back({ location, slot_of(no_gpu_buffer, 0u), zero_attribute_format(input.second.type), 0u });
        }
        std::sort(tLayout.elements.begin(), tLayout.elements.end());
        d3d12_buffer* indices = nullptr;
        if (aIndexed)
        {
            if (iVertexArray == nullptr || (indices = buffer_of(iVertexArray->indexBuffer)) == nullptr)
                throw no_vertex_array();
            transition(*indices, BufferReadState);
        }
        thread_local std::vector<d3d12_buffer*> tStorageBuffers;
        tStorageBuffers.clear();
        for (auto const& sb : program.storageBuffers)
        {
            auto* buffer = buffer_of(sb.buffer());
            if (buffer == nullptr)
                buffer = &iZeroBuffer;
            transition(*buffer, BufferReadState);
            tStorageBuffers.push_back(buffer);
        }

        auto* const pipelineState = pipeline(program, tLayout);

        D3D12_GPU_DESCRIPTOR_HANDLE textureTable = {};
        if (program.samplerSlots != 0u)
        {
            for (auto* image : tImages)
                transition(*image, ShaderResourceState);
            textureTable = texture_table(tImages);
        }

        // the uniform blocks
        thread_local std::vector<D3D12_GPU_VIRTUAL_ADDRESS> tUniformBlocks;
        tUniformBlocks.clear();
        for (auto const& ub : program.uniformBlocks)
        {
            auto& uploaded = iUploadedUniformBlocks[ub.block];
            if (uploaded.first != ub.block->generation || uploaded.second == 0u)
            {
                auto const allocation = allocate_transient(align_up<std::uint64_t>(ub.block->data.size(), 256u), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
                std::memcpy(allocation.mapping, ub.block->data.data(), ub.block->data.size());
                uploaded = { ub.block->generation, allocation.address };
            }
            tUniformBlocks.push_back(uploaded.second);
        }

        // the target
        if (iColor != nullptr)
            transition(*iColor, D3D12_RESOURCE_STATE_RENDER_TARGET);
        if (iDepthStencil != nullptr)
            transition(*iDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        flush_barriers();

        auto* const commandList = command_list();
        bind_descriptor_heaps();
        bind_render_target();
        if (!iViewportValid)
            apply_viewport_and_scissor();
        if (iBoundRootSignature != program.rootSignature.Get())
        {
            commandList->SetGraphicsRootSignature(program.rootSignature.Get());
            iBoundRootSignature = program.rootSignature.Get();
        }
        if (iBoundPipeline != pipelineState)
        {
            commandList->SetPipelineState(pipelineState);
            iBoundPipeline = pipelineState;
            ++iStatistics.pipelineBinds;
        }
        if (iBoundStencilReference != iState.stencilReference)
        {
            commandList->OMSetStencilRef(iState.stencilReference);
            iBoundStencilReference = iState.stencilReference;
        }
        UINT parameter = 0u;
        for (auto const address : tUniformBlocks)
            commandList->SetGraphicsRootConstantBufferView(parameter++, address);
        for (auto* buffer : tStorageBuffers)
            commandList->SetGraphicsRootShaderResourceView(parameter++, buffer->resource->GetGPUVirtualAddress());
        if (program.samplerSlots != 0u)
        {
            commandList->SetGraphicsRootDescriptorTable(parameter++, textureTable);
            commandList->SetGraphicsRootDescriptorTable(parameter++, samplerTable);
        }
        commandList->IASetPrimitiveTopology(to_d3d12_topology(aPrimitive));
        if (!tViews.empty())
            commandList->IASetVertexBuffers(0u, static_cast<UINT>(tViews.size()), tViews.data());
        if (indices != nullptr)
        {
            D3D12_INDEX_BUFFER_VIEW const indexView{ indices->resource->GetGPUVirtualAddress(), static_cast<UINT>(indices->size), DXGI_FORMAT_R32_UINT };
            commandList->IASetIndexBuffer(&indexView);
        }
        return true;
    }

    void d3d12_graphics_backend::draw_arrays(gpu_primitive aPrimitive, std::size_t aFirst, std::size_t aCount)
    {
        if (aCount == 0u)
            return;
        if (prepare_draw(aPrimitive, false))
            command_list()->DrawInstanced(static_cast<UINT>(aCount), 1u, static_cast<UINT>(aFirst), 0u);
    }

    void d3d12_graphics_backend::draw_elements(gpu_primitive aPrimitive, std::size_t aFirstIndex, std::size_t aCount)
    {
        if (aCount == 0u)
            return;
        if (prepare_draw(aPrimitive, true))
            command_list()->DrawIndexedInstanced(static_cast<UINT>(aCount), 1u, static_cast<UINT>(aFirstIndex), 0, 0u);
    }

    void d3d12_graphics_backend::texture_barrier()
    {
        // nothing to do: draws are ordered, and what is drawn to is not sampled (see has_texture_barrier)
    }

    void d3d12_graphics_backend::enable_scissor(bool aEnable)
    {
        if (iState.scissorEnabled != aEnable)
        {
            iState.scissorEnabled = aEnable;
            iViewportValid = false;
        }
    }

    void d3d12_graphics_backend::set_scissor(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight)
    {
        iState.scissor = { aX, aY, std::max(aWidth, 0), std::max(aHeight, 0) };
        iViewportValid = false;
    }

    void d3d12_graphics_backend::enable_multisample(bool aEnable)
    {
        // n.b. a multisample target is always rasterized with all its samples (as with Vulkan)
        iState.multisample = aEnable;
    }

    void d3d12_graphics_backend::set_sample_shading(std::optional<double> const& aSampleShadingRate)
    {
        // n.b. Direct3D 12 has no minimum sample shading rate: a pixel shader runs per sample if it reads the sample index
        // (gl_SampleID: SV_SampleIndex), as the glyph and shape shaders do
        iState.sampleShading = aSampleShadingRate;
    }

    void d3d12_graphics_backend::set_front_face(neogfx::front_face aFrontFace)
    {
        iState.frontFace = aFrontFace;
    }

    void d3d12_graphics_backend::set_face_culling(neogfx::face_culling aCulling, bool aFlipped)
    {
        iState.faceCulling = aCulling;
        iState.faceCullingFlipped = aFlipped;
    }

    void d3d12_graphics_backend::set_blending_mode(neogfx::blending_mode aBlendingMode)
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

    void d3d12_graphics_backend::set_xor_blending()
    {
        iState.blending = true;
        iState.xorBlending = true;
    }

    void d3d12_graphics_backend::enable_smoothing(bool)
    {
        // n.b. no Direct3D 12 equivalent of GL_LINE_SMOOTH and GL_POLYGON_SMOOTH (the shaders anti-alias)
    }

    namespace
    {
        // the clear's rectangle (the target, scissored); std::nullopt if empty
        std::optional<D3D12_RECT> clear_rect(std::array<std::uint32_t, 2> const& aExtent, bool aScissorEnabled, std::array<std::int32_t, 4> const& aScissor)
        {
            D3D12_RECT result{ 0, 0, static_cast<LONG>(aExtent[0]), static_cast<LONG>(aExtent[1]) };
            if (aScissorEnabled)
            {
                std::int64_t const x0 = std::clamp<std::int64_t>(aScissor[0], 0, aExtent[0]);
                std::int64_t const y0 = std::clamp<std::int64_t>(aScissor[1], 0, aExtent[1]);
                std::int64_t const x1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(aScissor[0]) + aScissor[2], 0, aExtent[0]);
                std::int64_t const y1 = std::clamp<std::int64_t>(static_cast<std::int64_t>(aScissor[1]) + aScissor[3], 0, aExtent[1]);
                if (x1 <= x0 || y1 <= y0)
                    return std::nullopt;
                result = D3D12_RECT{ static_cast<LONG>(x0), static_cast<LONG>(y0), static_cast<LONG>(x1), static_cast<LONG>(y1) };
            }
            return result;
        }
    }

    void d3d12_graphics_backend::clear(color const& aColor)
    {
        if (iColor == nullptr || !iState.colorWrite)
            return;
        auto const rect = clear_rect(target_extent(), iState.scissorEnabled, iState.scissor);
        if (!rect)
            return;
        transition(*iColor, D3D12_RESOURCE_STATE_RENDER_TARGET);
        flush_barriers();
        float const clearColor[4] = { aColor.red<float>(), aColor.green<float>(), aColor.blue<float>(), aColor.alpha<float>() };
        command_list()->ClearRenderTargetView(iColor->rtv.cpu, clearColor, 1u, &*rect);
    }

    void d3d12_graphics_backend::clear_depth_buffer()
    {
        if (iDepthStencil == nullptr)
            return;
        auto const rect = clear_rect(target_extent(), iState.scissorEnabled, iState.scissor);
        if (!rect)
            return;
        transition(*iDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        flush_barriers();
        command_list()->ClearDepthStencilView(iDepthStencil->dsv.cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0u, 1u, &*rect);
    }

    void d3d12_graphics_backend::clear_stencil_buffer(std::int32_t aValue)
    {
        // n.b. as glStencilMask(0xFF) then glClear(GL_STENCIL_BUFFER_BIT)
        iState.stencilWriteMask = 0xFFu;
        if (iDepthStencil == nullptr)
            return;
        auto const rect = clear_rect(target_extent(), iState.scissorEnabled, iState.scissor);
        if (!rect)
            return;
        transition(*iDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        flush_barriers();
        command_list()->ClearDepthStencilView(iDepthStencil->dsv.cpu, D3D12_CLEAR_FLAG_STENCIL, 1.0f, static_cast<UINT8>(aValue), 1u, &*rect);
    }

    void d3d12_graphics_backend::apply_stencil(bool aEnabled, bool aUpdating, std::int32_t aRef)
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
                iState.stencilCompare = D3D12_COMPARISON_FUNC_ALWAYS;
                iState.stencilPassOp = D3D12_STENCIL_OP_REPLACE;
                iState.stencilWriteMask = 0xFFu;
            }
            else
            {
                iState.colorWrite = true;
                iState.depthWrite = true;
                iState.stencilCompare = D3D12_COMPARISON_FUNC_EQUAL;
                iState.stencilPassOp = D3D12_STENCIL_OP_KEEP;
                iState.stencilWriteMask = 0x00u;
            }
        }
        else
            iState.stencilTest = false;
    }

    bool d3d12_graphics_backend::depth_test_enabled() const
    {
        return iState.depthTest;
    }

    void d3d12_graphics_backend::enable_depth_test(bool aEnable)
    {
        iState.depthTest = aEnable;
    }

    void d3d12_graphics_backend::set_texture_filter(i_texture const& aTexture, texture_sampling aSampling)
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

    void d3d12_graphics_backend::set_texture_linear_clamp(i_texture const& aTexture)
    {
        auto* image = image_of(aTexture);
        if (image == nullptr)
            return;
        image->sampler.magLinear = true;
        image->sampler.minLinear = true;
        image->sampler.mipmap = false;
        image->sampler.addressMode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    }

    void d3d12_graphics_backend::set_active_texture_unit(std::uint32_t)
    {
        // nothing to do
    }

    void d3d12_graphics_backend::unbind_texture_unit(std::uint32_t aTextureUnit)
    {
        bind_texture(aTextureUnit, nullptr);
    }

    bool d3d12_graphics_backend::alive() const
    {
        return iDevice != nullptr;
    }

    ID3D12Device* d3d12_graphics_backend::device() const
    {
        return iDevice.Get();
    }

    std::uint32_t d3d12_graphics_backend::sample_count(std::uint32_t aSamples) const
    {
        // n.b. no multisampling when software rendering (WARP): the pixel shaders (which run per sample) on the CPU four times
        // over otherwise; multisample textures are single sample textures then (see basic_d3d12_shader_program::compile)
        if (iWarp)
            return 1u;
        // the largest count up to that asked for that the colour and depth/stencil formats of the render targets support
        for (std::uint32_t samples = std::min(aSamples, 32u); samples > 1u; samples /= 2u)
        {
            bool supported = true;
            for (auto const format : { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT })
            {
                D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels = {};
                levels.Format = format;
                levels.SampleCount = samples;
                if (FAILED(iDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels))) || levels.NumQualityLevels == 0u)
                    supported = false;
            }
            if (supported)
                return samples;
        }
        return 1u;
    }

    std::unique_ptr<d3d12_image> d3d12_graphics_backend::create_image(std::uint32_t aWidth, std::uint32_t aHeight, DXGI_FORMAT aFormat, std::uint32_t aTexelSize,
        std::uint32_t aSamples, std::uint32_t aMipLevels)
    {
        auto result = std::make_unique<d3d12_image>();
        result->format = aFormat;
        result->srvFormat = aFormat;
        result->width = std::max(aWidth, 1u);
        result->height = std::max(aHeight, 1u);
        result->texelSize = aTexelSize;
        // n.b. the format may support fewer samples than the render targets' formats (see sample_count)
        result->samples = 1u;
        for (std::uint32_t samples = std::max(aSamples, 1u); samples > 1u; samples /= 2u)
        {
            D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels = {};
            levels.Format = aFormat;
            levels.SampleCount = samples;
            if (SUCCEEDED(iDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &levels, sizeof(levels))) && levels.NumQualityLevels != 0u)
            {
                result->samples = samples;
                break;
            }
        }
        result->mipLevels = (result->samples == 1u ? std::max(aMipLevels, 1u) : 1u);
        auto const heap = heap_properties(D3D12_HEAP_TYPE_DEFAULT);
        auto const desc = texture_desc(result->width, result->height, aFormat, result->samples, result->mipLevels, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = aFormat;
        dxCheck(iDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&result->resource)));
        result->state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = aFormat;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        if (result->samples == 1u)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = result->mipLevels;
        }
        else
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        result->srv = iSrvDescriptors.allocate();
        iDevice->CreateShaderResourceView(result->resource.Get(), &srvDesc, result->srv.cpu);
        result->rtv = iRtvDescriptors.allocate();
        iDevice->CreateRenderTargetView(result->resource.Get(), nullptr, result->rtv.cpu);
        // cleared (cf. a new OpenGL texture's storage, which is zeroed in practice)
        float const clearColor[4] = {};
        auto* const commandList = command_list();
        flush_barriers();
        commandList->ClearRenderTargetView(result->rtv.cpu, clearColor, 0u, nullptr);
        for (std::uint32_t level = 1u; level < result->mipLevels; ++level)
        {
            auto const& rtv = iScratchRtvs[iNextScratchRtv++ % ScratchRtvCount];
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = aFormat;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = level;
            iDevice->CreateRenderTargetView(result->resource.Get(), &rtvDesc, rtv.cpu);
            commandList->ClearRenderTargetView(rtv.cpu, clearColor, 0u, nullptr);
        }
        return result;
    }

    std::unique_ptr<d3d12_image> d3d12_graphics_backend::create_depth_stencil_image(std::uint32_t aWidth, std::uint32_t aHeight, std::uint32_t aSamples)
    {
        auto result = std::make_unique<d3d12_image>();
        result->format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        result->srvFormat = DXGI_FORMAT_UNKNOWN;
        result->depthStencil = true;
        result->samples = std::max(aSamples, 1u);
        result->width = std::max(aWidth, 1u);
        result->height = std::max(aHeight, 1u);
        result->texelSize = 4u;
        auto const heap = heap_properties(D3D12_HEAP_TYPE_DEFAULT);
        auto const desc = texture_desc(result->width, result->height, result->format, result->samples, 1u,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = result->format;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0u;
        dxCheck(iDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&result->resource)));
        result->state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        result->dsv = iDsvDescriptors.allocate();
        iDevice->CreateDepthStencilView(result->resource.Get(), nullptr, result->dsv.cpu);
        auto* const commandList = command_list();
        flush_barriers();
        commandList->ClearDepthStencilView(result->dsv.cpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0u, 0u, nullptr);
        return result;
    }

    void d3d12_graphics_backend::destroy_image(std::unique_ptr<d3d12_image>& aImage)
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
            iColor = nullptr;
            iDepthStencil = nullptr;
            iTargetBound = false;
        }
        for (auto& unit : iTextureUnits)
            if (unit == aImage.get())
                unit = nullptr;
        // n.b. a descriptor is read when it is recorded (render target and depth stencil views) or copied (shader resource views),
        // so is free now; the resource once the GPU is done with it
        iSrvDescriptors.free(aImage->srv);
        iRtvDescriptors.free(aImage->rtv);
        iDsvDescriptors.free(aImage->dsv);
        defer([resource = aImage->resource]() {});
        aImage = nullptr;
    }

    void d3d12_graphics_backend::upload_image(d3d12_image& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight,
        void const* aData, std::size_t aRowPitch)
    {
        if (aWidth == 0u || aHeight == 0u)
            return;
        auto const rowSize = static_cast<std::size_t>(aWidth) * aImage.texelSize;
        auto const pitch = align_up<std::size_t>(rowSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        ++iStatistics.uploads;
        iStatistics.uploadBytes += rowSize * aHeight;
        auto const staging = allocate_transient(pitch * aHeight, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
        auto const* source = static_cast<std::uint8_t const*>(aData);
        auto* destination = static_cast<std::uint8_t*>(staging.mapping);
        for (std::uint32_t row = 0u; row < aHeight; ++row)
            std::memcpy(destination + row * pitch, source + row * aRowPitch, rowSize);
        transition(aImage, D3D12_RESOURCE_STATE_COPY_DEST);
        flush_barriers();
        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource = aImage.resource.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0u;
        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = staging.resource;
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint.Offset = staging.offset;
        src.PlacedFootprint.Footprint.Format = aImage.format;
        src.PlacedFootprint.Footprint.Width = aWidth;
        src.PlacedFootprint.Footprint.Height = aHeight;
        src.PlacedFootprint.Footprint.Depth = 1u;
        src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(pitch);
        command_list()->CopyTextureRegion(&dst, aX, aY, 0u, &src, nullptr);
        if (aImage.mipLevels > 1u)
            generate_mipmaps(aImage);
    }

    void d3d12_graphics_backend::clear_image(d3d12_image& aImage, std::array<float, 4> const& aColor)
    {
        transition(aImage, D3D12_RESOURCE_STATE_RENDER_TARGET);
        flush_barriers();
        command_list()->ClearRenderTargetView(aImage.rtv.cpu, aColor.data(), 0u, nullptr);
        if (aImage.mipLevels > 1u)
            generate_mipmaps(aImage);
    }

    void d3d12_graphics_backend::generate_mipmaps(d3d12_image& aImage)
    {
        // each level drawn from the one above (Direct3D 12 has no blit), which is a shader resource while it is drawn to
        auto& resources = blits();
        auto existing = resources.mipmapPipelines.find(aImage.format);
        if (existing == resources.mipmapPipelines.end())
        {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
            desc.pRootSignature = resources.mipmapRootSignature.Get();
            desc.VS = to_bytecode(resources.vertex);
            desc.PS = to_bytecode(resources.mipmap);
            desc.BlendState = opaque_blend_desc();
            desc.SampleMask = UINT_MAX;
            desc.RasterizerState = rasterizer_desc(D3D12_CULL_MODE_NONE, false, 1u);
            desc.DepthStencilState = no_depth_stencil_desc();
            desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            desc.NumRenderTargets = 1u;
            desc.RTVFormats[0] = aImage.format;
            desc.SampleDesc.Count = 1u;
            com_ptr<ID3D12PipelineState> pipelineState;
            dxCheck(iDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
            existing = resources.mipmapPipelines.emplace(aImage.format, pipelineState).first;
        }
        transition(aImage, D3D12_RESOURCE_STATE_RENDER_TARGET);
        flush_barriers();
        auto* const commandList = command_list();
        bind_descriptor_heaps();
        commandList->SetGraphicsRootSignature(resources.mipmapRootSignature.Get());
        commandList->SetPipelineState(existing->second.Get());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        std::uint32_t width = aImage.width;
        std::uint32_t height = aImage.height;
        for (std::uint32_t level = 1u; level < aImage.mipLevels; ++level)
        {
            transition(aImage.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, ShaderResourceState, level - 1u);
            flush_barriers();
            width = std::max(width / 2u, 1u);
            height = std::max(height / 2u, 1u);
            auto const source = allocate_descriptors(1u);
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = aImage.srvFormat;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = level - 1u;
            srvDesc.Texture2D.MipLevels = 1u;
            iDevice->CreateShaderResourceView(aImage.resource.Get(), &srvDesc, source.cpu);
            auto const& rtv = iScratchRtvs[iNextScratchRtv++ % ScratchRtvCount];
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = aImage.format;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = level;
            iDevice->CreateRenderTargetView(aImage.resource.Get(), &rtvDesc, rtv.cpu);
            // n.b. the descriptor heap may have changed (see allocate_descriptors)
            bind_descriptor_heaps();
            commandList->OMSetRenderTargets(1u, &rtv.cpu, FALSE, nullptr);
            D3D12_VIEWPORT const viewport{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
            D3D12_RECT const scissor{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
            commandList->RSSetViewports(1u, &viewport);
            commandList->RSSetScissorRects(1u, &scissor);
            commandList->SetGraphicsRootDescriptorTable(0u, source.gpu);
            commandList->DrawInstanced(3u, 1u, 0u, 0u);
        }
        // the last level drawn to: all the levels are now shader resources
        transition(aImage.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, ShaderResourceState, aImage.mipLevels - 1u);
        aImage.state = ShaderResourceState;
        // n.b. the next draw binds its target, viewport, root signature and pipeline again
        invalidate_bindings();
    }

    void d3d12_graphics_backend::read_image(d3d12_image& aImage, std::uint32_t aX, std::uint32_t aY, std::uint32_t aWidth, std::uint32_t aHeight, void* aData)
    {
        ++iStatistics.readbacks;
        if (aWidth == 0u || aHeight == 0u)
            return;
        auto const rowSize = static_cast<std::size_t>(aWidth) * aImage.texelSize;
        auto const pitch = align_up<std::size_t>(rowSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        auto const heap = heap_properties(D3D12_HEAP_TYPE_READBACK);
        auto const desc = buffer_desc(pitch * aHeight);
        com_ptr<ID3D12Resource> readback;
        dxCheck(iDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback)));
        transition(aImage, D3D12_RESOURCE_STATE_COPY_SOURCE);
        flush_barriers();
        D3D12_TEXTURE_COPY_LOCATION dst = {};
        dst.pResource = readback.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dst.PlacedFootprint.Offset = 0u;
        dst.PlacedFootprint.Footprint.Format = aImage.format;
        dst.PlacedFootprint.Footprint.Width = aWidth;
        dst.PlacedFootprint.Footprint.Height = aHeight;
        dst.PlacedFootprint.Footprint.Depth = 1u;
        dst.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(pitch);
        D3D12_TEXTURE_COPY_LOCATION src = {};
        src.pResource = aImage.resource.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = 0u;
        D3D12_BOX const box{ aX, aY, 0u, aX + aWidth, aY + aHeight, 1u };
        command_list()->CopyTextureRegion(&dst, 0u, 0u, 0u, &src, &box);
        submit();
        wait_for_frames();
        void* mapping = nullptr;
        D3D12_RANGE const readRange{ 0u, pitch * aHeight };
        dxCheck(readback->Map(0u, &readRange, &mapping));
        for (std::uint32_t row = 0u; row < aHeight; ++row)
            std::memcpy(static_cast<std::uint8_t*>(aData) + row * rowSize, static_cast<std::uint8_t const*>(mapping) + row * pitch, rowSize);
        D3D12_RANGE const noWrite{ 0u, 0u };
        readback->Unmap(0u, &noWrite);
    }

    void d3d12_graphics_backend::set_render_target(d3d12_image* aColor, d3d12_image* aDepthStencil)
    {
        if (iColor == aColor && iDepthStencil == aDepthStencil)
            return;
        iColor = aColor;
        iDepthStencil = aDepthStencil;
        iTargetBound = false;
        iViewportValid = false;
    }

    void d3d12_graphics_backend::release_render_target(d3d12_image const* aColor)
    {
        if (iColor != aColor)
            return;
        iColor = nullptr;
        iDepthStencil = nullptr;
        iTargetBound = false;
    }

    void d3d12_graphics_backend::set_viewport(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight)
    {
        iState.viewport = { aX, aY, aWidth, aHeight };
        iViewportValid = false;
    }

    void d3d12_graphics_backend::enable_blending(bool aEnable)
    {
        iState.blending = aEnable;
    }

    void d3d12_graphics_backend::set_depth_compare(D3D12_COMPARISON_FUNC aCompare)
    {
        iState.depthCompare = aCompare;
    }

    void d3d12_graphics_backend::bind_texture(std::uint32_t aTextureUnit, d3d12_image* aImage)
    {
        if (aTextureUnit < iTextureUnits.size())
            iTextureUnits[aTextureUnit] = aImage;
    }

    d3d12_image* d3d12_graphics_backend::bound_texture(std::uint32_t aTextureUnit) const
    {
        if (aTextureUnit < iTextureUnits.size())
            return iTextureUnits[aTextureUnit];
        return nullptr;
    }

    void d3d12_graphics_backend::use_program(d3d12_program* aProgram)
    {
        iProgram = aProgram;
    }

    std::vector<std::uint32_t> d3d12_graphics_backend::compile_spirv(shader_type aType, std::string const& aSource, std::string const& aName) const
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
        // the on-disk cache: keyed by everything that determines the compilation's result
        std::string const cacheKey = "neogfx::d3d12_graphics_backend::compile_spirv;kind=" + std::to_string(static_cast<int>(kind)) +
            ";env=vulkan_1_3\n" + aSource;
        std::optional<std::filesystem::path> cacheFile;
        if (auto const& folder = cache_folder())
        {
            cacheFile = cache_file(*folder, cacheKey, ".spv");
            if (auto cached = read_cached_blob(*cacheFile, SpirvCacheMagic, cacheKey))
                if (cached->size() % sizeof(std::uint32_t) == 0u)
                {
                    std::vector<std::uint32_t> spirv(cached->size() / sizeof(std::uint32_t));
                    std::memcpy(spirv.data(), cached->data(), cached->size());
                    if (!spirv.empty() && spirv[0] == 0x07230203u) // the SPIR-V magic number
                        return spirv;
                }
        }
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetSourceLanguage(shaderc_source_language_glsl);
        // n.b. unoptimized: the reflection needs the uniform blocks' member names (see basic_d3d12_shader_program::link) and
        // the translation to HLSL all the declared varyings (see basic_d3d12_shader_program::compile); D3DCompile optimizes
        options.SetOptimizationLevel(shaderc_optimization_level_zero);
        auto const result = compiler.CompileGlslToSpv(aSource, kind, aName.c_str(), options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            auto const& errMsg = result.GetErrorMessage();
            std::cerr << "neogfx::d3d12_graphics_backend::compile_spirv::error: " << errMsg << std::endl;
            throw failed_to_create_shader_program(errMsg);
        }
        std::vector<std::uint32_t> spirv{ result.cbegin(), result.cend() };
        if (cacheFile)
            write_cached_blob(*cacheFile, SpirvCacheMagic, cacheKey, spirv.data(), spirv.size() * sizeof(std::uint32_t));
        return spirv;
    }

    d3d12_bytecode d3d12_graphics_backend::compile_hlsl(shader_type aType, std::string const& aHlsl, std::string const& aName, std::uint32_t aNumSamples,
        char const* aEntryPoint) const
    {
        char const* target = nullptr;
        switch (aType)
        {
        case shader_type::Vertex:
            target = "vs_5_1";
            break;
        case shader_type::Fragment:
            target = "ps_5_1";
            break;
        case shader_type::Geometry:
            target = "gs_5_1";
            break;
        default:
            throw failed_to_create_shader_program("Direct3D 12: unsupported shader stage (" + aName + ")");
        }
        UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#ifndef NDEBUG
        flags = D3DCOMPILE_OPTIMIZATION_LEVEL1 | D3DCOMPILE_DEBUG;
#endif
        std::string const numSamples = std::to_string(aNumSamples);
        std::string const cacheKey = "neogfx::d3d12_graphics_backend::compile_hlsl;target=" + std::string{ target } + ";entry=" + aEntryPoint +
            ";samples=" + numSamples + ";flags=" + std::to_string(flags) + "\n" + aHlsl;
        std::optional<std::filesystem::path> cacheFile;
        if (auto const& folder = cache_folder())
        {
            cacheFile = cache_file(*folder, cacheKey, ".dxbc");
            if (auto cached = read_cached_blob(*cacheFile, DxbcCacheMagic, cacheKey))
                return std::make_shared<std::vector<std::uint8_t> const>(std::move(*cached));
        }
        // gl_NumSamples (see vulkan_glsl): SPIRV-Cross's specialization constant macro
        D3D_SHADER_MACRO const macros[] = { { "SPIRV_CROSS_CONSTANT_ID_0", numSamples.c_str() }, { nullptr, nullptr } };
        com_ptr<ID3DBlob> code;
        com_ptr<ID3DBlob> errors;
        auto const result = iD3DCompile(aHlsl.data(), aHlsl.size(), aName.c_str(), aNumSamples != 0u ? macros : nullptr, nullptr,
            aEntryPoint, target, flags, 0u, &code, &errors);
        if (FAILED(result))
        {
            std::string const errMsg = (errors ? std::string{ static_cast<char const*>(errors->GetBufferPointer()), errors->GetBufferSize() } : dxErrorString(result));
            std::cerr << "neogfx::d3d12_graphics_backend::compile_hlsl::error: " << errMsg << std::endl;
            service<debug::logger>() << neolib::logger::severity::Debug << "HLSL compilation error (" << aName << "); HLSL dump:-" << std::endl;
            std::int32_t lineNumber = 1;
            std::istringstream iss{ aHlsl };
            std::string line;
            while (std::getline(iss, line))
                service<debug::logger>() << neolib::logger::severity::Debug << lineNumber++ << ": " << line << std::endl;
            throw failed_to_create_shader_program(errMsg);
        }
        auto const* begin = static_cast<std::uint8_t const*>(code->GetBufferPointer());
        auto bytecode = std::make_shared<std::vector<std::uint8_t> const>(begin, begin + code->GetBufferSize());
        if (cacheFile)
            write_cached_blob(*cacheFile, DxbcCacheMagic, cacheKey, bytecode->data(), bytecode->size());
        return bytecode;
    }

    d3d12_bytecode d3d12_graphics_backend::shader_bytecode(d3d12_shader_object& aShader, std::uint32_t aSamples, std::uint32_t aScaledLocations)
    {
        auto const key = std::make_pair(aShader.usesNumSamples ? aSamples : 0u, aShader.type == shader_type::Vertex ? aScaledLocations : 0u);
        auto existing = aShader.bytecode.find(key);
        if (existing != aShader.bytecode.end())
            return existing->second;
        std::string hlsl;
        if (key.second == 0u)
            hlsl = spirv_to_hlsl(aShader.spirv);
        else
            hlsl = spirv_to_hlsl(compile_spirv(aShader.type, d3d12_glsl_scaled_inputs(aShader.glsl, key.second), aShader.name));
        auto const bytecode = compile_hlsl(aShader.type, hlsl, aShader.name, key.first);
        aShader.bytecode.emplace(key, bytecode);
        return bytecode;
    }

    com_ptr<ID3D12RootSignature> d3d12_graphics_backend::create_root_signature(D3D12_ROOT_SIGNATURE_DESC const& aDesc)
    {
        // (D3D12SerializeRootSignature: PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)
        using serialize_root_signature = HRESULT (WINAPI*)(D3D12_ROOT_SIGNATURE_DESC const*, D3D_ROOT_SIGNATURE_VERSION, ID3DBlob**, ID3DBlob**);
        auto const serialize = reinterpret_cast<serialize_root_signature>(reinterpret_cast<void*>(::GetProcAddress(iD3d12, "D3D12SerializeRootSignature")));
        if (serialize == nullptr)
            throw d3d12_error("D3D12SerializeRootSignature not found");
        com_ptr<ID3DBlob> blob;
        com_ptr<ID3DBlob> errors;
        if (FAILED(serialize(&aDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errors)))
            throw d3d12_error("D3D12SerializeRootSignature: " +
                (errors ? std::string{ static_cast<char const*>(errors->GetBufferPointer()), errors->GetBufferSize() } : std::string{}));
        com_ptr<ID3D12RootSignature> result;
        dxCheck(iDevice->CreateRootSignature(0u, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&result)));
        return result;
    }

    void d3d12_graphics_backend::create_root_signature(d3d12_program& aProgram)
    {
        // (see d3d12_program)
        std::vector<D3D12_ROOT_PARAMETER> parameters;
        for (auto const& ub : aProgram.uniformBlocks)
            parameters.push_back(root_descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, ub.binding));
        for (auto const& sb : aProgram.storageBuffers)
            parameters.push_back(root_descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, sb.binding));
        auto const textures = range_of(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SamplerBindingBase, aProgram.samplerSlots);
        auto const samplers = range_of(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, SamplerBindingBase, aProgram.samplerSlots);
        if (aProgram.samplerSlots != 0u)
        {
            parameters.push_back(root_table(textures));
            parameters.push_back(root_table(samplers));
        }
        D3D12_ROOT_SIGNATURE_DESC desc = {};
        desc.NumParameters = static_cast<UINT>(parameters.size());
        desc.pParameters = parameters.data();
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        aProgram.rootSignature = create_root_signature(desc);
    }

    std::optional<std::filesystem::path> const& d3d12_graphics_backend::cache_folder() const
    {
        // n.b. resolved when first needed (and retried until it can be: there may be no application yet)
        if (!iCacheFolder)
        {
            try
            {
                auto const settingsFolder = service<i_app>().info().settings_folder().to_std_string();
                std::filesystem::path folder{ std::u8string{ settingsFolder.begin(), settingsFolder.end() } };
                folder /= "dx_cache";
                std::filesystem::create_directories(folder);
                iCacheFolder = folder;
            }
            catch (...)
            {
            }
        }
        return iCacheFolder;
    }

    void d3d12_graphics_backend::release_program(d3d12_program& aProgram)
    {
        for (auto const& p : aProgram.pipelines)
            if (p.second.Get() == iBoundPipeline)
                iBoundPipeline = nullptr;
        if (aProgram.rootSignature.Get() == iBoundRootSignature)
            iBoundRootSignature = nullptr;
        // n.b. released once the GPU is done with them
        defer([pipelines = std::move(aProgram.pipelines), rootSignature = aProgram.rootSignature]() {});
        aProgram.pipelines.clear();
        aProgram.rootSignature = nullptr;
        aProgram.linked = false;
    }

    bool d3d12_graphics_backend::vsync() const
    {
        return iVsync;
    }

    void d3d12_graphics_backend::set_vsync(bool aEnable)
    {
        iVsync = aEnable;
    }

    void d3d12_graphics_backend::destroy_swapchain(d3d12_swapchain& aSwapchain)
    {
        if (!alive())
            return;
        execute();
        destroy_image(aSwapchain.resolved);
        for (auto& rtv : aSwapchain.rtvs)
            iRtvDescriptors.free(rtv);
        aSwapchain.rtvs.clear();
        aSwapchain.buffers.clear();
        aSwapchain.swapchain = nullptr;
        aSwapchain.width = 0u;
        aSwapchain.height = 0u;
        execute();
    }

    void d3d12_graphics_backend::present(d3d12_swapchain& aSwapchain, d3d12_image& aSource, std::uint32_t aWidth, std::uint32_t aHeight)
    {
        if (!alive())
            return;
        auto const presentStart = std::chrono::steady_clock::now();
        struct present_statistics
        {
            d3d12_graphics_backend& backend;
            std::chrono::steady_clock::time_point start;
            ~present_statistics()
            {
                backend.iStatistics.present += std::chrono::steady_clock::now() - start;
                ++backend.iStatistics.frames;
                backend.report_statistics();
            }
        } presentStatistics{ *this, presentStart };
        aWidth = std::min(aWidth, aSource.width);
        aHeight = std::min(aHeight, aSource.height);
        if (aSwapchain.window == nullptr || aWidth == 0u || aHeight == 0u)
        {
            execute();
            return; // minimized
        }
        if (!aSwapchain.swapchain || aSwapchain.width != aWidth || aSwapchain.height != aHeight)
        {
            ++iStatistics.swapchainsCreated;
            // n.b. the back buffers must be released, and the GPU done with them, before they are resized
            execute();
            for (auto& rtv : aSwapchain.rtvs)
                iRtvDescriptors.free(rtv);
            aSwapchain.rtvs.clear();
            aSwapchain.buffers.clear();
            UINT const flags = iTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;
            constexpr UINT bufferCount = 3u;
            if (!aSwapchain.swapchain)
            {
                DXGI_SWAP_CHAIN_DESC1 desc = {};
                desc.Width = aWidth;
                desc.Height = aHeight;
                // n.b. not an sRGB format: as the OpenGL default framebuffer, what is drawn is already sRGB encoded
                desc.Format = aSwapchain.format;
                desc.SampleDesc.Count = 1u;
                desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc.BufferCount = bufferCount;
                desc.Scaling = DXGI_SCALING_NONE;
                desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                desc.Flags = flags;
                com_ptr<IDXGISwapChain1> swapchain;
                if (FAILED(iFactory->CreateSwapChainForHwnd(iQueue.Get(), aSwapchain.window, &desc, nullptr, nullptr, &swapchain)))
                {
                    // (n.b. no scaling isn't supported everywhere)
                    desc.Scaling = DXGI_SCALING_STRETCH;
                    dxCheck(iFactory->CreateSwapChainForHwnd(iQueue.Get(), aSwapchain.window, &desc, nullptr, nullptr, &swapchain));
                }
                dxCheck(swapchain.As(&aSwapchain.swapchain));
                // (full screen is the window's business, not DXGI's)
                iFactory->MakeWindowAssociation(aSwapchain.window, DXGI_MWA_NO_ALT_ENTER);
                aSwapchain.tearing = iTearing;
            }
            else
                dxCheck(aSwapchain.swapchain->ResizeBuffers(bufferCount, aWidth, aHeight, aSwapchain.format, flags));
            aSwapchain.width = aWidth;
            aSwapchain.height = aHeight;
            for (UINT i = 0u; i < bufferCount; ++i)
            {
                com_ptr<ID3D12Resource> buffer;
                dxCheck(aSwapchain.swapchain->GetBuffer(i, IID_PPV_ARGS(&buffer)));
                auto rtv = iRtvDescriptors.allocate();
                iDevice->CreateRenderTargetView(buffer.Get(), nullptr, rtv.cpu);
                aSwapchain.buffers.push_back(buffer);
                aSwapchain.rtvs.push_back(rtv);
            }
        }

        auto& resources = blits();
        auto existing = resources.presentPipelines.find(aSwapchain.format);
        if (existing == resources.presentPipelines.end())
        {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
            desc.pRootSignature = resources.presentRootSignature.Get();
            desc.VS = to_bytecode(resources.vertex);
            desc.PS = to_bytecode(resources.present);
            desc.BlendState = opaque_blend_desc();
            desc.SampleMask = UINT_MAX;
            desc.RasterizerState = rasterizer_desc(D3D12_CULL_MODE_NONE, false, 1u);
            desc.DepthStencilState = no_depth_stencil_desc();
            desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            desc.NumRenderTargets = 1u;
            desc.RTVFormats[0] = aSwapchain.format;
            desc.SampleDesc.Count = 1u;
            com_ptr<ID3D12PipelineState> pipelineState;
            dxCheck(iDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
            existing = resources.presentPipelines.emplace(aSwapchain.format, pipelineState).first;
        }

        auto* const commandList = command_list();
        d3d12_image* source = &aSource;
        if (aSource.samples != 1u)
        {
            if (!aSwapchain.resolved || aSwapchain.resolved->format != aSource.format ||
                aSwapchain.resolved->width != aSource.width || aSwapchain.resolved->height != aSource.height)
            {
                destroy_image(aSwapchain.resolved);
                aSwapchain.resolved = create_image(aSource.width, aSource.height, aSource.format, aSource.texelSize);
            }
            transition(aSource, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
            transition(*aSwapchain.resolved, D3D12_RESOURCE_STATE_RESOLVE_DEST);
            flush_barriers();
            commandList->ResolveSubresource(aSwapchain.resolved->resource.Get(), 0u, aSource.resource.Get(), 0u, aSource.format);
            source = aSwapchain.resolved.get();
        }
        transition(*source, ShaderResourceState);
        auto const backBufferIndex = aSwapchain.swapchain->GetCurrentBackBufferIndex();
        auto* const backBuffer = aSwapchain.buffers[backBufferIndex].Get();
        transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        flush_barriers();
        auto const sourceView = allocate_descriptors(1u);
        iDevice->CopyDescriptorsSimple(1u, sourceView.cpu, source->srv.cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        bind_descriptor_heaps();
        commandList->OMSetRenderTargets(1u, &aSwapchain.rtvs[backBufferIndex].cpu, FALSE, nullptr);
        D3D12_VIEWPORT const viewport{ 0.0f, 0.0f, static_cast<float>(aWidth), static_cast<float>(aHeight), 0.0f, 1.0f };
        D3D12_RECT const scissor{ 0, 0, static_cast<LONG>(aWidth), static_cast<LONG>(aHeight) };
        commandList->RSSetViewports(1u, &viewport);
        commandList->RSSetScissorRects(1u, &scissor);
        commandList->SetGraphicsRootSignature(resources.presentRootSignature.Get());
        commandList->SetPipelineState(existing->second.Get());
        std::int32_t const sourceHeight = static_cast<std::int32_t>(aHeight);
        commandList->SetGraphicsRoot32BitConstants(0u, 1u, &sourceHeight, 0u);
        commandList->SetGraphicsRootDescriptorTable(1u, sourceView.gpu);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawInstanced(3u, 1u, 0u, 0u);
        transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        submit();

        // n.b. as OpenGL with a swap interval of 0 when vsync is off: presented immediately (tearing, if supported)
        auto const syncInterval = iVsync ? 1u : 0u;
        auto const presentFlags = (!iVsync && aSwapchain.tearing) ? DXGI_PRESENT_ALLOW_TEARING : 0u;
        auto const result = aSwapchain.swapchain->Present(syncInterval, presentFlags);
        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
            dxCheck(iDevice->GetDeviceRemovedReason());
        else if (FAILED(result))
            dxCheck(result);
    }

    void d3d12_graphics_backend::report_statistics()
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
            "neogfx::d3d12_graphics_backend: " << frames / std::chrono::duration<double>{ period }.count() << " fps, per frame: " <<
            ms(period) << " ms total, " <<
            ms(iStatistics.fenceWait) << " ms fence wait, " <<
            ms(iStatistics.present) << " ms present, " <<
            perFrame(iStatistics.submits) << " submits (" << perFrame(iStatistics.executes) << " execute, " << perFrame(iStatistics.readbacks) << " readback), " <<
            perFrame(iStatistics.barriers) << " barriers, " <<
            perFrame(iStatistics.draws) << " draws, " <<
            perFrame(iStatistics.pipelineBinds) << " pipeline binds, " <<
            perFrame(iStatistics.descriptors) << " descriptors, " <<
            perFrame(iStatistics.uploads) << " uploads (" << perFrame(iStatistics.uploadBytes) / 1024.0 << " KiB), " <<
            perFrame(iStatistics.transientBytes) / 1024.0 << " KiB transient; " <<
            perFrame(iStatistics.discards) << " buffer discards; " <<
            iStatistics.pipelinesCreated << " pipelines created, " <<
            iStatistics.swapchainsCreated << " swapchains created" << std::endl;
        auto const enabled = iStatistics.enabled;
        iStatistics = statistics{};
        iStatistics.enabled = enabled;
        iStatistics.periodStart = now;
    }

    d3d12_graphics_backend::blit_resources& d3d12_graphics_backend::blits()
    {
        if (iBlits)
            return *iBlits;
        auto resources = std::make_unique<blit_resources>();
        std::string const blitShaders{ sBlitShaders };
        resources->vertex = compile_hlsl(shader_type::Vertex, blitShaders, "neogfx::blit.vs", 0u, "vs");
        resources->present = compile_hlsl(shader_type::Fragment, blitShaders, "neogfx::blit.present_ps", 0u, "present_ps");
        resources->mipmap = compile_hlsl(shader_type::Fragment, blitShaders, "neogfx::blit.mipmap_ps", 0u, "mipmap_ps");
        auto const source = range_of(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0u, 1u);
        {
            D3D12_ROOT_PARAMETER parameters[2] = {};
            parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            parameters[0].Constants.ShaderRegister = 0u;
            parameters[0].Constants.RegisterSpace = 0u;
            parameters[0].Constants.Num32BitValues = 1u;
            parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            parameters[1] = root_table(source);
            D3D12_ROOT_SIGNATURE_DESC desc = {};
            desc.NumParameters = 2u;
            desc.pParameters = parameters;
            resources->presentRootSignature = create_root_signature(desc);
        }
        {
            D3D12_ROOT_PARAMETER parameters[1] = { root_table(source) };
            D3D12_STATIC_SAMPLER_DESC sampler = {};
            sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            sampler.MaxAnisotropy = 1u;
            sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            sampler.MaxLOD = D3D12_FLOAT32_MAX;
            sampler.ShaderRegister = 0u;
            sampler.RegisterSpace = 0u;
            sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            D3D12_ROOT_SIGNATURE_DESC desc = {};
            desc.NumParameters = 1u;
            desc.pParameters = parameters;
            desc.NumStaticSamplers = 1u;
            desc.pStaticSamplers = &sampler;
            resources->mipmapRootSignature = create_root_signature(desc);
        }
        iBlits = std::move(resources);
        return *iBlits;
    }

    d3d12_graphics_backend::shadow_resources& d3d12_graphics_backend::shadows(i_standard_shader_program& aProgram)
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
            auto const vertex = compile_hlsl(shader_type::Vertex,
                spirv_to_hlsl(compile_spirv(shader_type::Vertex, vertexSource, "neogfx::shadow_map.vert")), "neogfx::shadow_map.vert");
            auto const fragment = compile_hlsl(shader_type::Fragment,
                spirv_to_hlsl(compile_spirv(shader_type::Fragment, sShadowFragmentShader, "neogfx::shadow_map.frag")), "neogfx::shadow_map.frag");

            // the atlas: a depth buffer that is also sampled (its resource typeless)
            auto atlas = std::make_unique<d3d12_image>();
            atlas->format = DXGI_FORMAT_D32_FLOAT;
            atlas->srvFormat = DXGI_FORMAT_R32_FLOAT;
            atlas->depthStencil = true;
            atlas->width = static_cast<std::uint32_t>(scene_shadow_atlas::AtlasSize);
            atlas->height = static_cast<std::uint32_t>(scene_shadow_atlas::AtlasSize);
            atlas->sampler = d3d12_sampler_state{ false, false, false, D3D12_TEXTURE_ADDRESS_MODE_CLAMP };
            auto const heap = heap_properties(D3D12_HEAP_TYPE_DEFAULT);
            auto const desc = texture_desc(atlas->width, atlas->height, DXGI_FORMAT_R32_TYPELESS, 1u, 1u, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil.Depth = 1.0f;
            dxCheck(iDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&atlas->resource)));
            atlas->state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            atlas->dsv = iDsvDescriptors.allocate();
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            iDevice->CreateDepthStencilView(atlas->resource.Get(), &dsvDesc, atlas->dsv.cpu);
            atlas->srv = iSrvDescriptors.allocate();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1u;
            iDevice->CreateShaderResourceView(atlas->resource.Get(), &srvDesc, atlas->srv.cpu);
            command_list()->ClearDepthStencilView(atlas->dsv.cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0u, 0u, nullptr);
            resources.atlas = std::move(atlas);

            // the shadow parameters (b1), the model matrices and table (SSBOs) and the base texture (t0, s0)
            auto const baseTexture = range_of(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0u, 1u);
            auto const baseSampler = range_of(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0u, 1u);
            D3D12_ROOT_PARAMETER const parameters[] =
            {
                root_descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 1u),
                root_descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, resources.matricesBinding),
                root_descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, resources.tableBinding),
                root_table(baseTexture),
                root_table(baseSampler)
            };
            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
            rootSignatureDesc.NumParameters = static_cast<UINT>(std::size(parameters));
            rootSignatureDesc.pParameters = parameters;
            rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            resources.rootSignature = create_root_signature(rootSignatureDesc);

            D3D12_INPUT_ELEMENT_DESC const elements[] =
            {
                { "TEXCOORD", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, static_cast<UINT>(scene_vertex::offset::xyz), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u },
                { "TEXCOORD", 1u, DXGI_FORMAT_R8G8B8A8_UNORM, 0u, static_cast<UINT>(scene_vertex::offset::rgba), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u },
                { "TEXCOORD", 2u, DXGI_FORMAT_R32G32_FLOAT, 0u, static_cast<UINT>(scene_vertex::offset::st), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u },
                { "TEXCOORD", 11u, DXGI_FORMAT_R32_FLOAT, 0u, static_cast<UINT>(scene_vertex::offset::model), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u },
                { "TEXCOORD", 12u, DXGI_FORMAT_R16G16B16A16_UINT, 0u, static_cast<UINT>(scene_vertex::offset::joints), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u },
                { "TEXCOORD", 13u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, static_cast<UINT>(scene_vertex::offset::weights), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0u }
            };
            D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
            pipelineDesc.pRootSignature = resources.rootSignature.Get();
            pipelineDesc.VS = to_bytecode(vertex);
            pipelineDesc.PS = to_bytecode(fragment);
            pipelineDesc.BlendState = opaque_blend_desc();
            pipelineDesc.SampleMask = UINT_MAX;
            pipelineDesc.RasterizerState = rasterizer_desc(D3D12_CULL_MODE_NONE, false, 1u);
            // slope scaled depth bias against shadow acne (cf. glPolygonOffset(1.5f, 2.0f))
            pipelineDesc.RasterizerState.DepthBias = 2;
            pipelineDesc.RasterizerState.SlopeScaledDepthBias = 1.5f;
            pipelineDesc.DepthStencilState = no_depth_stencil_desc();
            pipelineDesc.DepthStencilState.DepthEnable = TRUE;
            pipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            pipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
            pipelineDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ elements, static_cast<UINT>(std::size(elements)) };
            pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pipelineDesc.NumRenderTargets = 0u;
            pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            pipelineDesc.SampleDesc.Count = 1u;
            dxCheck(iDevice->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&resources.pipeline)));
            resources.failed = false;
        }
        catch (std::exception const& e)
        {
            service<debug::logger>() << neolib::logger::severity::Debug << "neogfx: shadow map shader: " << e.what() << std::endl;
        }
        return resources;
    }

    bool d3d12_graphics_backend::scene_shadows_available(i_standard_shader_program& aProgram)
    {
        return !shadows(aProgram).failed;
    }

    void d3d12_graphics_backend::draw_scene_shadow_maps(i_standard_shader_program& aProgram, native_scene_buffer& aSceneBuffer,
        std::vector<mat44> const& aViews, std::int32_t aDirectionalView,
        std::vector<std::optional<gpu_mesh_range>> const& aMeshes, std::vector<std::optional<scene_shadow_alpha_test>> const& aAlphaTests,
        std::uint32_t aModelTableBase)
    {
        auto& resources = shadows(aProgram);
        if (resources.failed)
            return;
        auto* const vertices = buffer_of(aSceneBuffer.vertex_buffer());
        auto* const indices = buffer_of(aSceneBuffer.index_buffer());
        if (vertices == nullptr || indices == nullptr)
            return;
        auto const& program = *static_cast<d3d12_program const*>(aProgram.handle());
        auto const storage_buffer = [&](std::uint32_t aBinding) -> d3d12_buffer*
            {
                for (auto const& sb : program.storageBuffers)
                    if (sb.binding == aBinding)
                        if (auto* buffer = buffer_of(sb.buffer()))
                            return buffer;
                return &iZeroBuffer;
            };
        auto* const matrices = storage_buffer(resources.matricesBinding);
        auto* const table = storage_buffer(resources.tableBinding);

        // the base textures (for the alpha tests) and their sampler tables first (a sampler table may wait for the GPU, and so
        // is got before anything is recorded with it; if that empties the sampler heap the tables are got again)
        std::vector<d3d12_image*> baseTextures(aMeshes.size(), iDummyTexture.get());
        std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> samplerTables(aMeshes.size());
        for (int attempt = 0; attempt < 2; ++attempt)
        {
            auto const generation = iSamplerHeapGeneration;
            for (std::size_t meshIndex = 0u; meshIndex < aMeshes.size(); ++meshIndex)
            {
                if (!aMeshes[meshIndex])
                    continue;
                auto const& alphaTest = aAlphaTests[meshIndex];
                if (alphaTest)
                    if (auto* image = image_of(*std::get<0>(*alphaTest)))
                        if (image->samples == 1u && !image->depthStencil)
                            baseTextures[meshIndex] = image;
                samplerTables[meshIndex] = sampler_table({ sampler(baseTextures[meshIndex]->sampler, baseTextures[meshIndex]->mipLevels) });
            }
            if (generation == iSamplerHeapGeneration)
                break;
        }

        // depth only, into the atlas
        transition(*resources.atlas, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        transition(*vertices, BufferReadState);
        transition(*indices, BufferReadState);
        transition(*matrices, BufferReadState);
        transition(*table, BufferReadState);
        for (auto* image : baseTextures)
            transition(*image, ShaderResourceState);
        flush_barriers();
        auto* const commandList = command_list();
        bind_descriptor_heaps();
        commandList->SetGraphicsRootSignature(resources.rootSignature.Get());
        commandList->SetPipelineState(resources.pipeline.Get());
        commandList->OMSetRenderTargets(0u, nullptr, FALSE, &resources.atlas->dsv.cpu);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D12_VERTEX_BUFFER_VIEW const vertexView{ vertices->resource->GetGPUVirtualAddress(), static_cast<UINT>(vertices->size), static_cast<UINT>(sizeof(scene_vertex)) };
        commandList->IASetVertexBuffers(0u, 1u, &vertexView);
        D3D12_INDEX_BUFFER_VIEW const indexView{ indices->resource->GetGPUVirtualAddress(), static_cast<UINT>(indices->size), DXGI_FORMAT_R32_UINT };
        commandList->IASetIndexBuffer(&indexView);
        commandList->SetGraphicsRootShaderResourceView(1u, matrices->resource->GetGPUVirtualAddress());
        commandList->SetGraphicsRootShaderResourceView(2u, table->resource->GetGPUVirtualAddress());

        shadow_parameters parameters = {};
        parameters.modelTableBase = aModelTableBase;
        for (std::uint32_t view = 0u; view < aViews.size(); ++view)
        {
            if (view == 0u && aDirectionalView != 0)
                continue;
            auto const tile = scene_shadow_atlas::tile(view);
            D3D12_VIEWPORT const viewport{ static_cast<float>(tile[0]), static_cast<float>(tile[1]), static_cast<float>(tile[2]), static_cast<float>(tile[3]), 0.0f, 1.0f };
            D3D12_RECT const scissor{ tile[0], tile[1], tile[0] + tile[2], tile[1] + tile[3] };
            commandList->RSSetViewports(1u, &viewport);
            commandList->RSSetScissorRects(1u, &scissor);
            commandList->ClearDepthStencilView(resources.atlas->dsv.cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0u, 1u, &scissor);
            auto const viewProjection = aViews[view].as<float>();
            std::memcpy(parameters.viewProjection, viewProjection.data(), sizeof(parameters.viewProjection));
            for (std::size_t meshIndex = 0u; meshIndex < aMeshes.size(); ++meshIndex)
            {
                auto const& mesh = aMeshes[meshIndex];
                if (!mesh)
                    continue;
                auto const& alphaTest = aAlphaTests[meshIndex];
                parameters.alphaTest = alphaTest ? 1 : 0;
                if (alphaTest)
                {
                    auto const& [texture, transform, cutoff, wrapS, wrapT] = *alphaTest;
                    parameters.textureTransform[0] = transform.x;
                    parameters.textureTransform[1] = transform.y;
                    parameters.textureTransform[2] = transform.z;
                    parameters.textureTransform[3] = transform.w;
                    parameters.alphaCutoff = cutoff;
                    parameters.textureWrap[0] = static_cast<std::int32_t>(wrapS);
                    parameters.textureWrap[1] = static_cast<std::int32_t>(wrapT);
                }
                auto const uniforms = allocate_transient(256u, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
                std::memcpy(uniforms.mapping, &parameters, sizeof(parameters));
                auto const textureTable = texture_table({ baseTextures[meshIndex] });
                // n.b. the descriptor heap may have changed (see allocate_descriptors)
                bind_descriptor_heaps();
                commandList->SetGraphicsRootConstantBufferView(0u, uniforms.address);
                commandList->SetGraphicsRootDescriptorTable(3u, textureTable);
                commandList->SetGraphicsRootDescriptorTable(4u, samplerTables[meshIndex]);
                commandList->DrawIndexedInstanced(mesh->indexEnd - mesh->indexStart, 1u, mesh->indexStart, 0, 0u);
            }
        }
        // n.b. the next draw binds its target and state again
        invalidate_bindings();
        bind_texture(static_cast<std::uint32_t>(reserved_texture_unit::PbrShadow), resources.atlas.get());
    }

    d3d12_graphics_backend::background_resources& d3d12_graphics_backend::background()
    {
        if (iBackground)
            return *iBackground;
        iBackground = std::make_unique<background_resources>();
        auto& resources = *iBackground;
        try
        {
            resources.vertex = compile_hlsl(shader_type::Vertex,
                spirv_to_hlsl(compile_spirv(shader_type::Vertex, sBackgroundVertexShader, "neogfx::scene_background.vert")), "neogfx::scene_background.vert");
            resources.fragment = compile_hlsl(shader_type::Fragment,
                spirv_to_hlsl(compile_spirv(shader_type::Fragment, sBackgroundFragmentShader, "neogfx::scene_background.frag")), "neogfx::scene_background.frag");
            // the parameters (b0) and the environment and background textures (t1 and t2, s1 and s2)
            auto const textures = range_of(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, 2u);
            auto const samplers = range_of(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1u, 2u);
            D3D12_ROOT_PARAMETER const parameters[] =
            {
                root_descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 0u),
                root_table(textures),
                root_table(samplers)
            };
            D3D12_ROOT_SIGNATURE_DESC desc = {};
            desc.NumParameters = static_cast<UINT>(std::size(parameters));
            desc.pParameters = parameters;
            resources.rootSignature = create_root_signature(desc);
            resources.failed = false;
        }
        catch (std::exception const& e)
        {
            service<debug::logger>() << neolib::logger::severity::Debug << "neogfx: scene background shader: " << e.what() << std::endl;
        }
        return resources;
    }

    void d3d12_graphics_backend::draw_scene_background(i_standard_shader_program&, mat44 const& aNdcToClip, mat44 const& aClipToWorld,
        i_texture const* aBackgroundTexture, i_texture const* aEnvironment, double aBlur, double aIntensity)
    {
        auto& resources = background();
        if (resources.failed || iColor == nullptr)
            return;

        auto const key = std::make_tuple(iColor->format, iDepthStencil != nullptr ? iDepthStencil->format : DXGI_FORMAT_UNKNOWN, iColor->samples);
        auto existing = resources.pipelines.find(key);
        if (existing == resources.pipelines.end())
        {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
            desc.pRootSignature = resources.rootSignature.Get();
            desc.VS = to_bytecode(resources.vertex);
            desc.PS = to_bytecode(resources.fragment);
            desc.BlendState = opaque_blend_desc();
            desc.SampleMask = UINT_MAX;
            desc.RasterizerState = rasterizer_desc(D3D12_CULL_MODE_NONE, false, iColor->samples);
            desc.DepthStencilState = no_depth_stencil_desc();
            desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            desc.NumRenderTargets = 1u;
            desc.RTVFormats[0] = iColor->format;
            desc.DSVFormat = std::get<1>(key);
            desc.SampleDesc.Count = iColor->samples;
            com_ptr<ID3D12PipelineState> pipelineState;
            dxCheck(iDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
            existing = resources.pipelines.emplace(key, pipelineState).first;
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

        d3d12_image* environment = (aEnvironment != nullptr ? image_of(*aEnvironment) : nullptr);
        d3d12_image* backgroundImage = (aBackgroundTexture != nullptr ? image_of(*aBackgroundTexture) : nullptr);
        for (auto* image : { &environment, &backgroundImage })
            if (*image == nullptr || (**image).samples != 1u || (**image).depthStencil || *image == iColor)
                *image = iDummyTexture.get();
        // n.b. the sampler table first (see sampler_table)
        auto const samplerTable = sampler_table({ sampler(environment->sampler, environment->mipLevels), sampler(backgroundImage->sampler, backgroundImage->mipLevels) });
        auto const uniforms = allocate_transient(align_up<std::uint64_t>(sizeof(parameters), 256u), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        std::memcpy(uniforms.mapping, &parameters, sizeof(parameters));
        transition(*environment, ShaderResourceState);
        transition(*backgroundImage, ShaderResourceState);
        transition(*iColor, D3D12_RESOURCE_STATE_RENDER_TARGET);
        if (iDepthStencil != nullptr)
            transition(*iDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        flush_barriers();
        auto const textureTable = texture_table({ environment, backgroundImage });

        auto* const commandList = command_list();
        bind_descriptor_heaps();
        bind_render_target();
        // n.b. viewport and scissor as the current state has them
        apply_viewport_and_scissor();
        commandList->SetGraphicsRootSignature(resources.rootSignature.Get());
        commandList->SetPipelineState(existing->second.Get());
        commandList->SetGraphicsRootConstantBufferView(0u, uniforms.address);
        commandList->SetGraphicsRootDescriptorTable(1u, textureTable);
        commandList->SetGraphicsRootDescriptorTable(2u, samplerTable);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        commandList->DrawInstanced(4u, 1u, 0u, 0u);
        // n.b. the next draw binds its root signature and pipeline again
        iBoundRootSignature = nullptr;
        iBoundPipeline = nullptr;
    }
}
