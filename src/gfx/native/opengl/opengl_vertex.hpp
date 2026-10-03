// opengl_vertex.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <vector>
#include <bit>
#include <optional>
#include <unordered_map>

#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_shader_program.hpp>
#include <neogfx/gfx/vertex_buffer.hpp>
#include "opengl_buffer.hpp"
#include "opengl.hpp"

namespace neogfx
{
    class opengl_vertex_array
    {
    public:
        opengl_vertex_array();
        ~opengl_vertex_array();
    public:
        void bind();
    private:
        GLint iPreviousVertexArrayBindingHandle;
        GLuint iHandle;
    };

    template <typename T>
    struct opengl_attrib_data_type {};
    template <>
    struct opengl_attrib_data_type<double> { static constexpr GLenum type = GL_DOUBLE; };
    template <>
    struct opengl_attrib_data_type<float> { static constexpr GLenum type = GL_FLOAT; };
    template <>
    struct opengl_attrib_data_type<std::uint8_t> { static constexpr GLenum type = GL_UNSIGNED_BYTE; };
    template <>
    struct opengl_attrib_data_type<std::uint16_t> { static constexpr GLenum type = GL_UNSIGNED_SHORT; };

    template <typename Vertex, typename Attrib>
    class opengl_vertex_attrib_array
    {
    public:
        struct cannot_get_attrib_location : std::logic_error { cannot_get_attrib_location(std::string const& aName) : std::logic_error("neogfx::opengl_vertex_attrib_array::cannot_get_attrib_location: " + aName) {} };
    public:
        typedef Vertex vertex_type;
        typedef Attrib attribute_type;
        typedef typename attribute_type::value_type value_type;
        static constexpr std::size_t arity = sizeof(attribute_type) / sizeof(value_type);
    public:
        opengl_vertex_attrib_array(bool aNormalized, std::size_t aStride, std::size_t aOffset, const i_shader_program& aShaderProgram, std::string const& aVariableName);
        ~opengl_vertex_attrib_array();
    public:
        void update(opengl_buffer<vertex_type>& aBuffer);
    private:
        bool const iNormalized;
        std::size_t const iStride;
        std::size_t const iOffset;
        i_shader_program const& iShaderProgram;
        std::string const iVariableName;
    };

    inline vec4f color_to_vec4f(const avec4u8& aSource)
    {
        return vec4f{{ aSource[0] / 255.0f, aSource[1] / 255.0f, aSource[2] / 255.0f, aSource[3] / 255.0f }};
    }

    struct standard_vertex
    {
        vec3f xyz;
        vec4f rgba;
        vec2f st;
        vec4f xyzw;
        vec4f abcd;
        vec4f efgh;
        vec4f ijkl;
        vec4f mnop;
        vec4f abcd2;
        vec4f efgh2;
        vec3f debug;
        standard_vertex(const vec3f& xyz = vec3f{}) :
            xyz{ xyz }
        {
        }
        standard_vertex(const vec3f& xyz, const vec4f& rgba, const vec2f& st = {}, const vec4f& xyzw = {}, const vec4f& abcd = {}, const vec4f& efgh = {}, const vec4f& ijkl = {}, const vec4f mnop = {}, const vec4f abcd2 = {}, const vec4f efgh2 = {}, const vec3f debug = {}) :
            xyz{ xyz }, rgba{ rgba }, st{ st }, xyzw{ xyzw }, abcd{ abcd }, efgh{ efgh }, ijkl{ ijkl }, mnop{ mnop }, abcd2{ abcd2 }, efgh2{ efgh2 }, debug{ debug }
        {
        }
        struct offset
        {
            static constexpr std::size_t xyz = 0u;
            static constexpr std::size_t rgba = xyz + sizeof(decltype(standard_vertex::xyz));
            static constexpr std::size_t st = rgba + sizeof(decltype(standard_vertex::rgba));
            static constexpr std::size_t xyzw = st + sizeof(decltype(standard_vertex::st));
            static constexpr std::size_t abcd = xyzw + sizeof(decltype(standard_vertex::xyzw));
            static constexpr std::size_t efgh = abcd + sizeof(decltype(standard_vertex::abcd));
            static constexpr std::size_t ijkl = efgh + sizeof(decltype(standard_vertex::efgh));
            static constexpr std::size_t mnop = ijkl + sizeof(decltype(standard_vertex::ijkl));
            static constexpr std::size_t abcd2 = mnop + sizeof(decltype(standard_vertex::mnop));
            static constexpr std::size_t efgh2 = abcd2 + sizeof(decltype(standard_vertex::abcd2));
            static constexpr std::size_t debug = efgh2 + sizeof(decltype(standard_vertex::efgh2));
        };
    };

    // Compact vertex of a scene mesh (an entity with a game::model_transformation component): cached once in model
    // space, drawn indexed and transformed (and skinned) on the GPU.
    struct scene_vertex
    {
        vec3f xyz;
        avec4u8 rgba;       // normalized
        vec2f st;
        vec1f model;        // entity id (indexes the model table)
        avec4u16 joints;    // skin joint indices
        vec4f weights;      // skin joint weights (all zero if not skinned)
        vec3f normal;       // for lighting (see i_standard_vertex_shader::set_scene_light)
        struct offset
        {
            static constexpr std::size_t xyz = 0u;
            static constexpr std::size_t rgba = xyz + sizeof(decltype(scene_vertex::xyz));
            static constexpr std::size_t st = rgba + sizeof(decltype(scene_vertex::rgba));
            static constexpr std::size_t model = st + sizeof(decltype(scene_vertex::st));
            static constexpr std::size_t joints = model + sizeof(decltype(scene_vertex::model));
            static constexpr std::size_t weights = joints + sizeof(decltype(scene_vertex::joints));
            static constexpr std::size_t normal = weights + sizeof(decltype(scene_vertex::weights));
        };
    };

    // The scene meshes of a vertex provider: device local vertex and index buffers.
    class opengl_scene_buffer : private opengl_buffer_owner
    {
    public:
        struct mesh_range
        {
            std::uint32_t vertexStart;
            std::uint32_t vertexEnd;
            std::uint32_t indexStart;
            std::uint32_t indexEnd;
        };
    public:
        opengl_scene_buffer();
        ~opengl_scene_buffer();
        opengl_scene_buffer(opengl_scene_buffer const&) = delete;
        opengl_scene_buffer& operator=(opengl_scene_buffer const&) = delete;
    public:
        // make room for meshes about to be allocated (so that a model's first upload allocates exactly what it needs)
        void reserve(std::size_t aExtraVertices, std::size_t aExtraIndices);
        mesh_range allocate(std::uint32_t aVertexCount, std::uint32_t aIndexCount);
        std::optional<mesh_range> find(std::uint32_t aVertexStart, std::uint32_t aVertexEnd) const;
        // the mesh's indices are absolute (i.e. include its vertexStart)
        void write(mesh_range const& aMesh, scene_vertex const* aVertices, std::uint32_t const* aIndices);
        bool reclaim(std::uint32_t aVertexStart, std::uint32_t aVertexEnd);
        void reclaim();
        void draw(i_rendering_context& aContext, i_shader_program& aShaderProgram, optional_mat44 const& aTransformation, std::uint32_t aIndexStart, std::uint32_t aIndexCount);
        // with the current (depth only) program, which takes position (location 0), colour (1), texture coordinates (2), model (11),
        // joints (12) and weights (13)
        void draw_depth(std::uint32_t aIndexStart, std::uint32_t aIndexCount);
    private:
        void buffer_grown() final;
    private:
        opengl_buffer<scene_vertex> iVertices;
        opengl_buffer<std::uint32_t> iIndices;
        std::unordered_map<std::uint32_t, mesh_range> iMeshes;
        std::optional<opengl_vertex_array> iVao;
        std::optional<opengl_vertex_attrib_array<scene_vertex, decltype(scene_vertex::xyz)>> iPositionAttribArray;
        std::optional<opengl_vertex_attrib_array<scene_vertex, decltype(scene_vertex::rgba)>> iColorAttribArray;
        std::optional<opengl_vertex_attrib_array<scene_vertex, decltype(scene_vertex::st)>> iTextureCoordAttribArray;
        std::optional<opengl_vertex_attrib_array<scene_vertex, decltype(scene_vertex::model)>> iModelAttribArray;
        std::optional<opengl_vertex_attrib_array<scene_vertex, decltype(scene_vertex::joints)>> iJointsAttribArray;
        std::optional<opengl_vertex_attrib_array<scene_vertex, decltype(scene_vertex::weights)>> iWeightsAttribArray;
        std::optional<opengl_vertex_attrib_array<scene_vertex, decltype(scene_vertex::normal)>> iNormalAttribArray;
    };

    template <typename V = standard_vertex>
    class opengl_vertex_buffer : public vertex_buffer, private opengl_buffer_owner
    {
    public:
        typedef V vertex_type;
    public:
        typedef opengl_buffer<vertex_type> vertex_array;
    public:
        opengl_vertex_buffer(i_vertex_provider& aProvider, vertex_buffer_type aType);
    public:
        void attach_shader(i_rendering_context& aContext, i_shader_program& aShaderProgram) override;
        void detach_shader() override;
    public:
        const optional_mat44& transformation() const;
        void set_transformation(const optional_mat44& aTransformation);
    public:
        void reclaim(std::size_t aStartIndex, std::size_t aEndIndex) override;
        void reclaim() override;
    public:
        void flush();
        void flush(std::size_t aCount);
        void flush(std::size_t aOffset, std::size_t aCount);
        vertex_array& vertices();
        std::size_t capacity() const;
        // scene meshes (see opengl_rendering_context::draw_scene_meshes)
        opengl_scene_buffer& scene_buffer();
    private:
        void buffer_grown() override;
        void update_attrib_arrays();
    private:
        opengl_buffer<vertex_type> iBuffer;
        optional_mat44 iTransformation;
        std::optional<opengl_vertex_array> iVao;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::xyz)>> iVertexPositionAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::rgba)>> iVertexColorAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::st)>> iVertexTextureCoordAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::xyzw)>> iVertexFunction0AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::abcd)>> iVertexFunction1AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::efgh)>> iVertexFunction2AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::ijkl)>> iVertexFunction3AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::mnop)>> iVertexFunction4AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::abcd2)>> iVertexFunction5AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::efgh2)>> iVertexFunction6AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::debug)>> iVertexDebugAttribArray;
        std::optional<opengl_scene_buffer> iSceneBuffer;
    };

    class use_shader_program
    {
    public:
        use_shader_program(i_rendering_context& aContext, i_shader_program& aShaderProgram, scalar aOpacity = 1.0);
        ~use_shader_program();
    private:
        i_rendering_context& iRenderingContext;
        i_shader_program& iCurrentProgram;
        i_shader_program* iPreviousProgram;
    };
}
