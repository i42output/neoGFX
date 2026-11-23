// d3d12_hlsl.hpp
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
#include <string>
#include <vector>

namespace neogfx
{
    // The shaders' path to Direct3D 12: the GLSL generated for OpenGL is made Vulkan GLSL (see vulkan_glsl), which these
    // functions adapt further; it is compiled to SPIR-V by shaderc, translated to HLSL (shader model 5.1) by SPIRV-Cross and
    // compiled to DXBC by D3DCompile (see d3d12_graphics_backend::shader_bytecode). SPIRV-Cross gives each resource the register
    // of its Vulkan binding (b for uniform blocks, t for SSBOs and textures, s for the samplers of combined image samplers) in
    // space 0, each vertex input the semantic TEXCOORD<location> and each varying TEXCOORD<location>, in location order.

    // Vulkan GLSL made Direct3D 12 GLSL: the SSBOs are readonly (so that they are SRVs (ByteAddressBuffer) rather than UAVs)
    std::string d3d12_glsl(std::string const& aVulkanGlsl);

    // the vertex inputs at the locations in aScaledLocations (a bit per location) given unsigned integer attributes that are to be
    // converted to floating point (as OpenGL converts a non-normalized integer attribute (glVertexAttribPointer) and Vulkan a
    // USCALED one; Direct3D 12 has no such formats): each such input is declared as the unsigned integer vector its attribute
    // is and converted at the start of main()
    std::string d3d12_glsl_scaled_inputs(std::string const& aSource, std::uint32_t aScaledLocations);

    // SPIRV-Cross (spirv-cross-c-shared.dll, from the Vulkan SDK) is loaded when first needed; throws if it can't be
    void load_spirv_cross();
    // SPIR-V to HLSL (shader model 5.1); throws (i_rendering_engine::shader_program_error) on failure
    std::string spirv_to_hlsl(std::vector<std::uint32_t> const& aSpirv);
}
