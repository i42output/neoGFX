// use_vertex_arrays.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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
    namespace
    {
        struct with_textures_t {} with_textures;

        class use_vertex_arrays
        {
        public:
            struct not_enough_room : std::invalid_argument { not_enough_room() : std::invalid_argument("neogfx::use_vertex_arrays::not_enough_room") {} };
            struct invalid_draw_count : std::invalid_argument { invalid_draw_count() : std::invalid_argument("neogfx::use_vertex_arrays::invalid_draw_count") {} };
            struct cannot_use_barrier : std::invalid_argument { cannot_use_barrier() : std::invalid_argument("neogfx::use_vertex_arrays::cannot_use_barrier") {} };
        public:
            typedef opengl_vertex_buffer<>::vertex_array::value_type value_type;
            typedef opengl_vertex_buffer<>::vertex_array::const_iterator const_iterator;
            typedef opengl_vertex_buffer<>::vertex_array::iterator iterator;
        public:
            use_vertex_arrays(i_vertex_provider& aProvider, i_rendering_context& aParent, GLenum aMode, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iProvider{ aProvider },
                iParent{ aParent }, 
                iUse{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
                iMode{ aMode }, 
                iWithTextures{ false }, 
                iStart{ static_cast<GLint>(iUse.vertices().size()) }, 
                iUseBarrier{ aUseBarrier },
                iDrawOnExit{ true }
            {
                if (!room_for(aNeed) || aUseBarrier)
                    execute();
                set_transformation(optional_mat44{});
                if (!room_for(aNeed) && !need(aNeed))
                    throw not_enough_room();
            }
            use_vertex_arrays(i_vertex_provider& aProvider, i_rendering_context& aParent, GLenum aMode, const optional_mat44& aTransformation, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iProvider{ aProvider },
                iParent{ aParent },
                iUse{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
                iMode{ aMode },
                iWithTextures{ false }, 
                iStart{ static_cast<GLint>(iUse.vertices().size()) },
                iUseBarrier{ aUseBarrier },
                iDrawOnExit{ true }
            {
                if (!room_for(aNeed) || aUseBarrier)
                    execute();
                set_transformation(aTransformation);
                if (!room_for(aNeed) && !need(aNeed))
                    throw not_enough_room();
            }
            use_vertex_arrays(i_vertex_provider& aProvider, i_rendering_context& aParent, GLenum aMode, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iProvider{ aProvider },
                iParent{ aParent },
                iUse{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
                iMode{ aMode },
                iWithTextures{ true }, 
                iStart{ static_cast<GLint>(iUse.vertices().size()) },
                iUseBarrier{ aUseBarrier },
                iDrawOnExit{ true }
            {
                if (!room_for(aNeed) || aUseBarrier)
                    execute();
                set_transformation(optional_mat44{});
                if (!room_for(aNeed) && !need(aNeed))
                    throw not_enough_room();
            }
            use_vertex_arrays(i_vertex_provider& aProvider, i_rendering_context& aParent, GLenum aMode, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iProvider{ aProvider },
                iParent{ aParent },
                iUse{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
                iMode{ aMode },
                iWithTextures{ true }, 
                iStart{ static_cast<GLint>(iUse.vertices().size()) },
                iUseBarrier{ aUseBarrier },
                iDrawOnExit{ true }
            {
                if (!room_for(aNeed) || aUseBarrier)
                    execute();
                set_transformation(aTransformation);
                if (!room_for(aNeed) && !need(aNeed))
                    throw not_enough_room();
            }
            ~use_vertex_arrays()
            {
                if (iDrawOnExit)
                    draw();
            }
        public:
            i_rendering_context& parent()
            {
                return iParent;
            }
            std::size_t primitive_vertex_count() const
            {
                switch (mode())
                {
                case GL_TRIANGLES:
                    return 3;
                case GL_QUADS: // two triangles
                    return 6;
                case GL_LINES:
                    return 2;
                case GL_POINTS:
                    return 1;
                case GL_TRIANGLE_FAN:
                case GL_LINE_LOOP:
                case GL_LINE_STRIP:
                case GL_TRIANGLE_STRIP:
                default:
                    return 0;
                }
            }
            bool with_textures() const
            {
                return iWithTextures;
            }
        public:
            const_iterator begin() const
            {
                return vertices().begin() + static_cast<std::size_t>(iStart);
            }
            iterator begin()
            {
                return vertices().begin() + static_cast<std::size_t>(iStart);
            }
            const_iterator end() const
            {
                return vertices().end();
            }
            iterator end()
            {
                return vertices().end();
            }
            bool empty() const
            {
                return vertices().size() == static_cast<std::size_t>(iStart);
            }
            std::size_t size() const
            {
                return end() - begin();
            }
            const value_type& operator[](std::size_t aOffset) const
            {
                return *(begin() + aOffset);
            }
            value_type& operator[](std::size_t aOffset)
            {
                return *(begin() + aOffset);
            }
        public:
            void push_back(const value_type& aVertex)
            {
                if (!room_for(1))
                    execute();
                vertices().push_back(aVertex);
            }
            template <typename... Args>
            void emplace_back(Args&&... args)
            {
                if (!room_for(1))
                    execute();
                vertices().emplace_back(std::forward<Args>(args)...);
            }
            template <typename Iter>
            iterator insert(const_iterator aPos, Iter aFirst, Iter aLast)
            {
                if (room_for(std::distance(aFirst, aLast)))
                    return vertices().insert(aPos, aFirst, aLast);
                else
                {
                    execute();
                    if (!room_for(std::distance(aFirst, aLast)))
                        vertices().reserve(std::distance(aFirst, aLast));
                    return vertices().insert(vertices().begin(), aFirst, aLast);
                }
            }
        public:
            std::size_t room() const
            {
                return vertices().room();
            }
            bool room_for(std::size_t aAmount) const
            {
                return vertices().room_for(aAmount);
            }
            bool need(std::size_t aAmount)
            {
                try
                {
                    vertices().need(aAmount);
                    return true;
                }
                catch (...)
                {
                    return false;
                }
            }
            void execute()
            {
                draw();
                iUse.execute();
                iUse.vertices().clear();
                iStart = 0;
            }
            struct skip
            {
                std::optional<std::size_t> skipCount;
            };
            void draw(const skip& aSkip = {})
            {
                draw(vertices().size() - static_cast<std::size_t>(iStart), aSkip);
            }
            void draw(std::size_t aStart, std::size_t aCount, const skip& aSkip = {})
            {
                iStart = static_cast<GLint>(aStart);
                draw(aCount, aSkip);
            }
            void draw(std::size_t aCount, const skip& aSkip = {})
            {
                iDrawOnExit = false;
                if (aCount == 0u)
                    return;
                auto skipCount = aSkip.skipCount ? std::max<std::size_t>(*aSkip.skipCount, 1u) : 1u;
                if (static_cast<std::size_t>(iStart) + aCount > vertices().size())
                    throw invalid_draw_count();
                if (static_cast<std::size_t>(iStart) == vertices().size())
                    return;
                iParent.rendering_engine().vertex_buffer(iProvider).attach_shader(iParent, iParent.rendering_engine().active_shader_program());
                if (!iUseBarrier && mode() == translated_mode())
                {
                    glCheck(glDrawArrays(translated_mode(), iStart, static_cast<GLsizei>(aCount)));
                    iStart += static_cast<GLint>(aCount);
                }
                else
                {
                    if (iUseBarrier)
                    {
                        glCheck(glTextureBarrier());
                    }
                    auto const pvc = primitive_vertex_count();
                    auto chunk = pvc * skipCount;
                    while (aCount > 0)
                    {
                        auto amount = std::min(chunk, aCount);
                        glCheck(glDrawArrays(translated_mode(), iStart, static_cast<GLsizei>(amount)));
                        iStart += static_cast<GLint>(amount);
                        aCount -= amount;
                        if (iUseBarrier)
                        {
                            glCheck(glTextureBarrier());
                        }
                    } 
                }
            }
        private:
            bool is_new_transformation(const optional_mat44& aTransformation) const
            {
                return iUse.transformation() != aTransformation;
            }
            const optional_mat44& transformation() const
            {
                return iUse.transformation();
            }
            void set_transformation(const optional_mat44& aTransformation)
            {
                iUse.set_transformation(aTransformation);
            }
            const opengl_vertex_buffer<>::vertex_array& vertices() const
            {
                return iUse.vertices();
            }
            opengl_vertex_buffer<>::vertex_array& vertices()
            {
                return iUse.vertices();
            }
            GLenum translated_mode() const
            {
                switch (iMode)
                {
                case GL_QUADS:
                    return GL_TRIANGLES;
                default:
                    return iMode;
                }
            }
            GLenum mode() const
            {
                return iMode;
            }
        private:
            i_vertex_provider& iProvider;
            i_rendering_context& iParent;
            opengl_vertex_buffer<>::use iUse;
            GLenum iMode;
            bool iWithTextures;
            GLint iStart;
            bool iUseBarrier;
            bool iDrawOnExit;
        };
    }
}