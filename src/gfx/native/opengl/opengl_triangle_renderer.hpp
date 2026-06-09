// opengl_triangle_renderer.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018-2026 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_vertex_buffer.hpp>
#include "opengl_helpers.hpp"

namespace neogfx
{
    inline constexpr struct with_textures_t {} with_textures;

    class opengl_triangle_renderer
    {
    public:
        struct not_enough_room : std::invalid_argument { not_enough_room() : std::invalid_argument("neogfx::opengl_triangle_renderer::not_enough_room") {} };
        struct invalid_draw_count : std::invalid_argument { invalid_draw_count() : std::invalid_argument("neogfx::opengl_triangle_renderer::invalid_draw_count") {} };
        struct cannot_use_barrier : std::invalid_argument { cannot_use_barrier() : std::invalid_argument("neogfx::opengl_triangle_renderer::cannot_use_barrier") {} };
    public:
        typedef opengl_vertex_buffer<>::vertex_array::value_type value_type;
        typedef opengl_vertex_buffer<>::vertex_array::const_iterator const_iterator;
        typedef opengl_vertex_buffer<>::vertex_array::iterator iterator;
    public:
        opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, std::size_t aNeed = 0u, bool aUseBarrier = false);
        opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, const optional_mat44& aTransformation, std::size_t aNeed = 0u, bool aUseBarrier = false);
        opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, GLenum aMode, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false);
        opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false);
        ~opengl_triangle_renderer();
    public:
        i_rendering_context& parent();
        std::size_t primitive_vertex_count() const;
        bool with_textures() const;
    public:
        const_iterator begin() const;
        iterator begin();
        const_iterator end() const;
        iterator end();
        bool empty() const;
        std::size_t size() const;
        value_type const& operator[](std::size_t aOffset) const;
        value_type& operator[](std::size_t aOffset);
    public:
        void push_back(value_type const& aVertex);
        template <typename... Args>
        void emplace_back(Args&&... args)
        {
            if (!room_for(1))
                draw_and_execute();
            vertices().emplace_back(std::forward<Args>(args)...);
        }
        template <typename Iter>
        iterator insert(const_iterator aPos, Iter aFirst, Iter aLast)
        {
            if (room_for(std::distance(aFirst, aLast)))
                return vertices().insert(aPos, aFirst, aLast);
            else
            {
                draw_and_execute();
                if (!room_for(std::distance(aFirst, aLast)))
                    vertices().reserve(std::distance(aFirst, aLast));
                return vertices().insert(vertices().begin(), aFirst, aLast);
            }
        }
    public:
        std::size_t room() const;
        bool room_for(std::size_t aAmount) const;
        bool need(std::size_t aAmount);
        void draw_and_execute();
        struct skip
        {
            std::optional<std::size_t> skipCount;
        };
        void draw(const skip& aSkip = {});
        void draw(std::size_t aStart, std::size_t aCount, const skip& aSkip = {});
        void draw(std::size_t aCount, const skip& aSkip = {});
    private:
        bool is_new_transformation(const optional_mat44& aTransformation) const;
        const optional_mat44& transformation() const;
        void set_transformation(const optional_mat44& aTransformation);
        const opengl_vertex_buffer<>::vertex_array& vertices() const;
        opengl_vertex_buffer<>::vertex_array& vertices();
    private:
        i_vertex_provider& iProvider;
        i_rendering_context& iParent;
        opengl_vertex_buffer<>& iVertexBuffer;
        bool iWithTextures;
        GLint iStart;
        bool iUseBarrier;
        bool iDrawOnExit;
    };
}