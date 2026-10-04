// vulkan_glsl.hpp
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

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace neogfx
{
    // a sampler uniform of a program's generated GLSL (see shader<>::generate_code) and the descriptor binding it is given
    struct vulkan_glsl_sampler
    {
        std::uint32_t binding;
        bool multisample;
        bool rectangle;
        VkShaderStageFlags stages;
    };
    using vulkan_glsl_samplers = std::map<std::string, vulkan_glsl_sampler>;

    // the specialization constant (an int) that replaces gl_NumSamples, which Vulkan GLSL lacks: the pipeline's sample count
    constexpr std::uint32_t VulkanGlslNumSamplesConstantId = 0u;

    // The GLSL generated for OpenGL made Vulkan GLSL: the uniform blocks (whose binding is their stage) are std140 in
    // descriptor set 0, the SSBOs' bindings are offset by aStorageBindingBase, the sampler uniforms are given bindings
    // (from aSamplerBindingBase; a sampler keeps its binding across compiles and stages) and the rectangle samplers
    // (which Vulkan GLSL lacks) become 2D samplers with their texelFetch, texture and textureSize calls adapted.
    // gl_NumSamples (also lacking) becomes a specialization constant (see VulkanGlslNumSamplesConstantId).
    std::string vulkan_glsl(std::string const& aSource, VkShaderStageFlagBits aStage, vulkan_glsl_samplers& aSamplers,
        std::uint32_t aStorageBindingBase, std::uint32_t aSamplerBindingBase);

    // the layout (std140) of a uniform block, reflected from SPIR-V
    struct spirv_block_member
    {
        std::uint32_t offset;
        std::uint32_t size;
        std::uint32_t arrayStride;
    };
    struct spirv_block
    {
        std::uint32_t size = 0u;
        std::map<std::string, spirv_block_member> members;
    };
    std::optional<spirv_block> spirv_uniform_block(std::vector<std::uint32_t> const& aSpirv, std::string const& aBlockName);
}
