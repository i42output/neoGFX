// opengl_vertex_arrays.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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

namespace neogfx
{
    namespace
    {
        struct with_textures_t {} with_textures;

        class use_vertex_arrays_instance
        {
        public:
            struct not_enough_room : std::invalid_argument { not_enough_room() : std::invalid_argument("neogfx::use_vertex_arrays_instance::not_enough_room") {} };
            struct invalid_draw_count : std::invalid_argument { invalid_draw_count() : std::invalid_argument("neogfx::use_vertex_arrays_instance::invalid_draw_count") {} };
            struct cannot_use_barrier : std::invalid_argument { cannot_use_barrier() : std::invalid_argument("neogfx::use_vertex_arrays_instance::cannot_use_barrier") {} };
        public:
            typedef opengl_standard_vertex_arrays::vertex_array::value_type value_type;
            typedef opengl_standard_vertex_arrays::vertex_array::const_iterator const_iterator;
            typedef opengl_standard_vertex_arrays::vertex_array::iterator iterator;
        public:
            use_vertex_arrays_instance(opengl_rendering_context& aParent, GLenum aMode, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iParent{ aParent }, 
                iUse{ aParent.rendering_engine().vertex_arrays() }, 
                iMode{ aMode }, 
                iWithTextures{ false }, 
                iStart{ static_cast<GLint>(vertices().size()) }, 
                iUseBarrier{ aUseBarrier }
            {
                if (!room_for(aNeed) || aUseBarrier || is_new_transformation(optional_mat44{}))
                    execute();
                set_transformation(optional_mat44{});
                if (!room_for(aNeed))
                    throw not_enough_room();
            }
            use_vertex_arrays_instance(opengl_rendering_context& aParent, GLenum aMode, const optional_mat44& aTransformation, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iParent{ aParent }, 
                iUse{ aParent.rendering_engine().vertex_arrays() }, 
                iMode{ aMode }, 
                iWithTextures{ false }, 
                iStart{ static_cast<GLint>(vertices().size()) }, 
                iUseBarrier{ aUseBarrier }
            {
                if (!room_for(aNeed) || aUseBarrier || is_new_transformation(aTransformation))
                    execute();
                set_transformation(aTransformation);
                if (!room_for(aNeed))
                    throw not_enough_room();
            }
            use_vertex_arrays_instance(opengl_rendering_context& aParent, GLenum aMode, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iParent{ aParent }, 
                iUse{ aParent.rendering_engine().vertex_arrays() }, 
                iMode{ aMode }, 
                iWithTextures{ true }, 
                iStart{ static_cast<GLint>(vertices().size()) }, 
                iUseBarrier{ aUseBarrier }
            {
                if (!room_for(aNeed) || aUseBarrier || is_new_transformation(optional_mat44{}))
                    execute();
                set_transformation(optional_mat44{});
                if (!room_for(aNeed))
                    throw not_enough_room();
            }
            use_vertex_arrays_instance(opengl_rendering_context& aParent, GLenum aMode, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false) :
                iParent{ aParent }, 
                iUse{ aParent.rendering_engine().vertex_arrays() }, 
                iMode{ aMode }, 
                iWithTextures{ true }, 
                iStart{ static_cast<GLint>(vertices().size()) }, 
                iUseBarrier{ aUseBarrier }
            {
                if (!room_for(aNeed) || aUseBarrier || is_new_transformation(aTransformation))
                    execute();
                set_transformation(aTransformation);
                if (!room_for(aNeed))
                    throw not_enough_room();
            }
            ~use_vertex_arrays_instance()
            {
                draw();
            }
        public:
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
                return vertices().capacity() - vertices().size();
            }
            bool room_for(std::size_t aAmount) const
            {
                return room() >= aAmount;
            }
            void execute()
            {
                draw();
                iUse.execute();
                iUse.vertices().clear();
                iStart = 0;
            }
            void draw()
            {
                draw(vertices().size() - static_cast<std::size_t>(iStart));
            }
            void draw(std::size_t aCount, std::size_t aSkipCount = 1u)
            {
                if (aCount == 0u)
                    return;
                aSkipCount = std::max<std::size_t>(aSkipCount, 1u);
                if (static_cast<std::size_t>(iStart) + aCount > vertices().size())
                    throw invalid_draw_count();
                if (static_cast<std::size_t>(iStart) == vertices().size())
                    return;
                iParent.rendering_engine().active_shader_program().update_uniforms(iParent);
                if (!iWithTextures)
                    iParent.rendering_engine().vertex_arrays().instantiate(iParent, iParent.rendering_engine().active_shader_program());
                else
                    iParent.rendering_engine().vertex_arrays().instantiate_with_texture_coords(iParent, iParent.rendering_engine().active_shader_program());
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
                    auto chunk = pvc * aSkipCount;
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
            const opengl_standard_vertex_arrays::vertex_array& vertices() const
            {
                return iUse.vertices();
            }
            opengl_standard_vertex_arrays::vertex_array& vertices()
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
            opengl_rendering_context& iParent;
            opengl_standard_vertex_arrays::use iUse;
            GLenum iMode;
            bool iWithTextures;
            GLint iStart;
            bool iUseBarrier;
        };

        class use_vertex_arrays
        {
        public:
            use_vertex_arrays(opengl_rendering_context& aParent, GLenum aMode, std::size_t aNeed = 0u, bool aUseBarrier = false)
            {
                instantiate(aParent, aMode, aNeed, aUseBarrier);
            }
            use_vertex_arrays(opengl_rendering_context& aParent, GLenum aMode, const optional_mat44& aTransformation, std::size_t aNeed = 0u, bool aUseBarrier = false)
            {
                instantiate(aParent, aMode, aTransformation, aNeed, aUseBarrier);
            }
            use_vertex_arrays(opengl_rendering_context& aParent, GLenum aMode, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false)
            {
                instantiate(aParent, aMode, with_textures, aNeed, aUseBarrier);
            }
            use_vertex_arrays(opengl_rendering_context& aParent, GLenum aMode, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed = 0u, bool aUseBarrier = false)
            {
                instantiate(aParent, aMode, aTransformation, with_textures, aNeed, aUseBarrier);
            }
            ~use_vertex_arrays()
            {
            }
        public:
            use_vertex_arrays_instance& instance()
            {
                return *iInstance;
            }
            template <typename... Args>
            void instantiate(Args&&... args)
            {
                thread_local std::weak_ptr<use_vertex_arrays_instance> tInstance;
                if (tInstance.expired())
                {
                    iInstance = std::make_shared<use_vertex_arrays_instance>(std::forward<Args>(args)...);
                    tInstance = iInstance;
                }
                else
                    iInstance = tInstance.lock();
            }
        private:
            std::shared_ptr<use_vertex_arrays_instance> iInstance;
        };
    }
}