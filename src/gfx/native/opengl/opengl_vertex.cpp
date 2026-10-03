// opengl_vertex.cpp
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

#include "opengl_buffer.ipp"
#include "opengl_vertex.hpp"
#include "opengl_vertex.ipp"

#include <limits>

namespace neogfx
{
    template class opengl_buffer<standard_vertex>;
    template class opengl_vertex_buffer<>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::xyz)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::rgba)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::st)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::xyzw)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::abcd)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::efgh)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::ijkl)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::mnop)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::abcd2)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::efgh2)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::debug)>;

    opengl_scene_buffer::opengl_scene_buffer() :
        iVertices{ *this, true, 0u, true }, iIndices{ *this, true, 0u, true }
    {
    }

    opengl_scene_buffer::~opengl_scene_buffer()
    {
    }

    void opengl_scene_buffer::reserve(std::size_t aExtraVertices, std::size_t aExtraIndices)
    {
        iVertices.reserve(iVertices.size() + aExtraVertices);
        iIndices.reserve(iIndices.size() + aExtraIndices);
    }

    opengl_scene_buffer::mesh_range opengl_scene_buffer::allocate(std::uint32_t aVertexCount, std::uint32_t aIndexCount)
    {
        auto const vertexStart = iVertices.find_space_for(aVertexCount);
        if (vertexStart == iVertices.size())
            iVertices.resize(iVertices.size() + aVertexCount);
        auto const indexStart = iIndices.find_space_for(aIndexCount);
        if (indexStart == iIndices.size())
            iIndices.resize(iIndices.size() + aIndexCount);
        if (vertexStart + aVertexCount > std::numeric_limits<std::uint32_t>::max() || 
            indexStart + aIndexCount > std::numeric_limits<std::uint32_t>::max())
            throw std::overflow_error("neogfx::opengl_scene_buffer::allocate");
        mesh_range const result{
            static_cast<std::uint32_t>(vertexStart), static_cast<std::uint32_t>(vertexStart + aVertexCount),
            static_cast<std::uint32_t>(indexStart), static_cast<std::uint32_t>(indexStart + aIndexCount) };
        iMeshes[result.vertexStart] = result;
        return result;
    }

    std::optional<opengl_scene_buffer::mesh_range> opengl_scene_buffer::find(std::uint32_t aVertexStart, std::uint32_t aVertexEnd) const
    {
        auto existing = iMeshes.find(aVertexStart);
        if (existing == iMeshes.end() || existing->second.vertexEnd != aVertexEnd)
            return {};
        return existing->second;
    }

    void opengl_scene_buffer::write(mesh_range const& aMesh, scene_vertex const* aVertices, std::uint32_t const* aIndices)
    {
        iVertices.write(aMesh.vertexStart, aVertices, aMesh.vertexEnd - aMesh.vertexStart);
        iIndices.write(aMesh.indexStart, aIndices, aMesh.indexEnd - aMesh.indexStart);
    }

    bool opengl_scene_buffer::reclaim(std::uint32_t aVertexStart, std::uint32_t aVertexEnd)
    {
        auto existing = iMeshes.find(aVertexStart);
        if (existing == iMeshes.end() || existing->second.vertexEnd != aVertexEnd)
            return false;
        iVertices.reclaim(existing->second.vertexStart, existing->second.vertexEnd);
        iIndices.reclaim(existing->second.indexStart, existing->second.indexEnd);
        iMeshes.erase(existing);
        return true;
    }

    void opengl_scene_buffer::reclaim()
    {
        iVertices.reclaim();
        iIndices.reclaim();
    }

    void opengl_scene_buffer::draw(i_rendering_context& aContext, i_shader_program& aShaderProgram, optional_mat44 const& aTransformation, std::uint32_t aIndexStart, std::uint32_t aIndexCount)
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
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIndices.handle()));
        if (aShaderProgram.type() == shader_program_type::Standard)
            static_cast<i_standard_vertex_shader&>(aShaderProgram.vertex_shader()).set_transformation_matrix(aTransformation);
        aShaderProgram.instantiate(aContext);
        glCheck(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(aIndexCount), GL_UNSIGNED_INT,
            reinterpret_cast<const void*>(static_cast<std::uintptr_t>(aIndexStart) * sizeof(std::uint32_t))));
    }

    void opengl_scene_buffer::buffer_grown()
    {
        // nothing to do: draw() binds the (possibly new) buffers every time
    }

    opengl_vertex_array::opengl_vertex_array()
    {
        glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &iPreviousVertexArrayBindingHandle));
        glCheck(glGenVertexArrays(1, &iHandle));
        bind();
    }

    opengl_vertex_array::~opengl_vertex_array()
    {
        glCheck(glBindVertexArray(iPreviousVertexArrayBindingHandle));
        glCheck(glDeleteVertexArrays(1, &iHandle));
    }

    void opengl_vertex_array::bind()
    {
        glCheck(glBindVertexArray(iHandle));
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
