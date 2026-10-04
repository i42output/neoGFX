// native_vertex.cpp
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

#include <neogfx/neogfx.hpp>

#include "native_buffer.ipp"
#include "native_vertex.hpp"
#include "native_vertex.ipp"

#include <limits>

namespace neogfx
{
    template class native_buffer<standard_vertex>;
    template class native_vertex_buffer<>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::xyz)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::rgba)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::st)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::xyzw)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::abcd)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::efgh)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::ijkl)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::mnop)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::abcd2)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::efgh2)>;
    template class native_vertex_attrib_array<standard_vertex, decltype(standard_vertex::debug)>;

    native_scene_buffer::native_scene_buffer() :
        iVertices{ *this, true, 0u, true }, iIndices{ *this, true, 0u, true }
    {
    }

    native_scene_buffer::~native_scene_buffer()
    {
    }

    void native_scene_buffer::reserve(std::size_t aExtraVertices, std::size_t aExtraIndices)
    {
        iVertices.reserve(iVertices.size() + aExtraVertices);
        iIndices.reserve(iIndices.size() + aExtraIndices);
    }

    native_scene_buffer::mesh_range native_scene_buffer::allocate(std::uint32_t aVertexCount, std::uint32_t aIndexCount)
    {
        auto const vertexStart = iVertices.find_space_for(aVertexCount);
        if (vertexStart == iVertices.size())
            iVertices.resize(iVertices.size() + aVertexCount);
        auto const indexStart = iIndices.find_space_for(aIndexCount);
        if (indexStart == iIndices.size())
            iIndices.resize(iIndices.size() + aIndexCount);
        if (vertexStart + aVertexCount > std::numeric_limits<std::uint32_t>::max() || 
            indexStart + aIndexCount > std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("neogfx::native_scene_buffer::allocate");
        mesh_range const result{
            static_cast<std::uint32_t>(vertexStart), static_cast<std::uint32_t>(vertexStart + aVertexCount),
            static_cast<std::uint32_t>(indexStart), static_cast<std::uint32_t>(indexStart + aIndexCount) };
        iMeshes[result.vertexStart] = result;
        return result;
    }

    std::optional<native_scene_buffer::mesh_range> native_scene_buffer::find(std::uint32_t aVertexStart, std::uint32_t aVertexEnd) const
    {
        auto existing = iMeshes.find(aVertexStart);
        if (existing == iMeshes.end() || existing->second.vertexEnd != aVertexEnd)
            return {};
        return existing->second;
    }

    void native_scene_buffer::write(mesh_range const& aMesh, scene_vertex const* aVertices, std::uint32_t const* aIndices)
    {
        iVertices.write(aMesh.vertexStart, aVertices, aMesh.vertexEnd - aMesh.vertexStart);
        iIndices.write(aMesh.indexStart, aIndices, aMesh.indexEnd - aMesh.indexStart);
    }

    bool native_scene_buffer::reclaim(std::uint32_t aVertexStart, std::uint32_t aVertexEnd)
    {
        auto existing = iMeshes.find(aVertexStart);
        if (existing == iMeshes.end() || existing->second.vertexEnd != aVertexEnd)
            return false;
        iVertices.reclaim(existing->second.vertexStart, existing->second.vertexEnd);
        iIndices.reclaim(existing->second.indexStart, existing->second.indexEnd);
        iMeshes.erase(existing);
        return true;
    }

    void native_scene_buffer::reclaim()
    {
        iVertices.reclaim();
        iIndices.reclaim();
    }

    void native_scene_buffer::draw_depth(std::uint32_t aIndexStart, std::uint32_t aIndexCount)
    {
        if (aIndexCount == 0u)
            return;
        if (iVao == std::nullopt)
            iVao.emplace();
        else
            iVao->bind();
        // n.b. the standard program's attribute locations (see standard_vertex_shader), so the attributes it set up are unchanged
        auto& backend = graphics_backend();
        backend.set_vertex_attribute(0u, iVertices.handle(), 3, gpu_attribute_type::Float, false, sizeof(scene_vertex), scene_vertex::offset::xyz);
        backend.set_vertex_attribute(1u, iVertices.handle(), 4, gpu_attribute_type::UnsignedByte, true, sizeof(scene_vertex), scene_vertex::offset::rgba);
        backend.set_vertex_attribute(2u, iVertices.handle(), 2, gpu_attribute_type::Float, false, sizeof(scene_vertex), scene_vertex::offset::st);
        backend.set_vertex_attribute(11u, iVertices.handle(), 1, gpu_attribute_type::Float, false, sizeof(scene_vertex), scene_vertex::offset::model);
        backend.set_vertex_attribute(12u, iVertices.handle(), 4, gpu_attribute_type::UnsignedShort, false, sizeof(scene_vertex), scene_vertex::offset::joints);
        backend.set_vertex_attribute(13u, iVertices.handle(), 4, gpu_attribute_type::Float, false, sizeof(scene_vertex), scene_vertex::offset::weights);
        backend.set_index_buffer(iIndices.handle());
        backend.draw_elements(gpu_primitive::Triangles, aIndexStart, aIndexCount);
    }

    void native_scene_buffer::draw(i_rendering_context& aContext, i_shader_program& aShaderProgram, optional_mat44 const& aTransformation, std::uint32_t aIndexStart, std::uint32_t aIndexCount)
    {
        if (aIndexCount == 0u)
            return;
        if (iVao == std::nullopt)
            iVao.emplace();
        else
            iVao->bind();
        // n.b. the vertex attributes this format lacks (e.g. the function attributes) are disabled in this vertex
        // array object so read as their default value (zero)
        iPositionAttribArray.emplace(false, sizeof(scene_vertex), scene_vertex::offset::xyz, aShaderProgram,
            standard_vertex_attribute_name(vertex_buffer_type::Vertices));
        iColorAttribArray.emplace(true, sizeof(scene_vertex), scene_vertex::offset::rgba, aShaderProgram,
            standard_vertex_attribute_name(vertex_buffer_type::Color));
        if (aShaderProgram.supports(vertex_buffer_type::UV))
            iTextureCoordAttribArray.emplace(false, sizeof(scene_vertex), scene_vertex::offset::st, aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::UV));
        if (aShaderProgram.supports(vertex_buffer_type::Model))
            iModelAttribArray.emplace(false, sizeof(scene_vertex), scene_vertex::offset::model, aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Model));
        if (aShaderProgram.supports(vertex_buffer_type::Joints))
            iJointsAttribArray.emplace(false, sizeof(scene_vertex), scene_vertex::offset::joints, aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Joints));
        if (aShaderProgram.supports(vertex_buffer_type::Weights))
            iWeightsAttribArray.emplace(false, sizeof(scene_vertex), scene_vertex::offset::weights, aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Weights));
        if (aShaderProgram.supports(vertex_buffer_type::Normal))
            iNormalAttribArray.emplace(false, sizeof(scene_vertex), scene_vertex::offset::normal, aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Normal));
        iPositionAttribArray->update(iVertices);
        iColorAttribArray->update(iVertices);
        if (iTextureCoordAttribArray)
            iTextureCoordAttribArray->update(iVertices);
        if (iModelAttribArray)
            iModelAttribArray->update(iVertices);
        if (iJointsAttribArray)
            iJointsAttribArray->update(iVertices);
        if (iWeightsAttribArray)
            iWeightsAttribArray->update(iVertices);
        if (iNormalAttribArray)
            iNormalAttribArray->update(iVertices);
        // the element array buffer binding is vertex array object state (and the buffer changes if it grows)
        graphics_backend().set_index_buffer(iIndices.handle());
        if (aShaderProgram.type() == shader_program_type::Standard)
            static_cast<i_standard_vertex_shader&>(aShaderProgram.vertex_shader()).set_transformation_matrix(aTransformation);
        aShaderProgram.instantiate(aContext);
        graphics_backend().draw_elements(gpu_primitive::Triangles, aIndexStart, aIndexCount);
    }

    gpu_buffer native_scene_buffer::vertex_buffer() const
    {
        return iVertices.handle();
    }

    gpu_buffer native_scene_buffer::index_buffer() const
    {
        return iIndices.handle();
    }

    void native_scene_buffer::buffer_grown()
    {
        // nothing to do: draw() binds the (possibly new) buffers every time
    }

    native_vertex_array::native_vertex_array()
    {
        iPreviousVertexArrayBindingHandle = graphics_backend().bound_vertex_array();
        iHandle = graphics_backend().create_vertex_array();
        bind();
    }

    native_vertex_array::~native_vertex_array()
    {
        graphics_backend().bind_vertex_array(iPreviousVertexArrayBindingHandle);
        graphics_backend().destroy_vertex_array(iHandle);
    }

    void native_vertex_array::bind()
    {
        graphics_backend().bind_vertex_array(iHandle);
    }

    use_shader_program::use_shader_program(i_rendering_context& aContext, i_shader_program& aShaderProgram, scalar aOpacity) :
        iRenderingContext{ aContext },
        iCurrentProgram{ aShaderProgram },
        iPreviousProgram{ service<i_rendering_engine>().is_shader_program_active() ? &service<i_rendering_engine>().active_shader_program() : nullptr }
    {
        iCurrentProgram.activate(iRenderingContext);
        if (iCurrentProgram.type() == shader_program_type::Standard)
        {
            iCurrentProgram.as<i_standard_shader_program>().standard_vertex_shader().set_opacity(aOpacity);
            if (iRenderingContext.gradient_set())
                iRenderingContext.apply_gradient(iCurrentProgram.as<i_standard_shader_program>().gradient_shader());
            else
                iCurrentProgram.as<i_standard_shader_program>().gradient_shader().clear_gradient();
        }
    }

    use_shader_program::~use_shader_program()
    {
        if (&iCurrentProgram != iPreviousProgram)
        {
            iCurrentProgram.deactivate();
            if (iPreviousProgram != nullptr)
                iPreviousProgram->activate(iRenderingContext);
        }
        if (iCurrentProgram.type() == shader_program_type::Standard)
        {
            if (!iRenderingContext.gradient_set())
                iCurrentProgram.as<i_standard_shader_program>().gradient_shader().clear_gradient();
            iCurrentProgram.as<i_standard_shader_program>().texture_shader().clear_texture();
            iCurrentProgram.as<i_standard_shader_program>().filter_shader().clear_filter();
            iCurrentProgram.as<i_standard_shader_program>().glyph_shader().clear_glyph();
            iCurrentProgram.as<i_standard_shader_program>().stipple_shader().clear_stipple();
            iCurrentProgram.as<i_standard_shader_program>().shape_shader().clear_shape();
        }
    }
}
