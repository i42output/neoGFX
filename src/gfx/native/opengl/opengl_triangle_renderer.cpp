// opengl_triangle_renderer.cpp
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

#include "opengl_triangle_renderer.hpp"

namespace neogfx
{
    opengl_triangle_renderer::opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ false },
        iStart{ static_cast<GLint>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        if (!room_for(aNeed))
            draw_and_execute();
        else if (aUseBarrier)
            execute();
        set_transformation(optional_mat44{});
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    opengl_triangle_renderer::opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, const optional_mat44& aTransformation, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ false },
        iStart{ static_cast<GLint>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        if (!room_for(aNeed))
            draw_and_execute();
        else if (aUseBarrier)
            execute();
        set_transformation(aTransformation);
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    opengl_triangle_renderer::opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, GLenum aMode, with_textures_t, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ true },
        iStart{ static_cast<GLint>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        if (!room_for(aNeed))
            draw_and_execute();
        else if (aUseBarrier)
            execute();
        set_transformation(optional_mat44{});
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    opengl_triangle_renderer::opengl_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<opengl_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ true },
        iStart{ static_cast<GLint>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        if (!room_for(aNeed))
            draw_and_execute();
        else if (aUseBarrier)
            execute();
        set_transformation(aTransformation);
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    opengl_triangle_renderer::~opengl_triangle_renderer()
    {
        if (iDrawOnExit)
            draw();
    }

    i_rendering_context& opengl_triangle_renderer::parent()
    {
        return iParent;
    }

    std::size_t opengl_triangle_renderer::primitive_vertex_count() const
    {
        return 3; // triangle
    }

    bool opengl_triangle_renderer::with_textures() const
    {
        return iWithTextures;
    }

    opengl_triangle_renderer::const_iterator opengl_triangle_renderer::begin() const
    {
        return vertices().begin() + static_cast<std::size_t>(iStart);
    }
    
    opengl_triangle_renderer::iterator opengl_triangle_renderer::begin()
    {
        return vertices().begin() + static_cast<std::size_t>(iStart);
    }
    
    opengl_triangle_renderer::const_iterator opengl_triangle_renderer::end() const
    {
        return vertices().end();
    }
    
    opengl_triangle_renderer::iterator opengl_triangle_renderer::end()
    {
        return vertices().end();
    }
    
    bool opengl_triangle_renderer::empty() const
    {
        return vertices().size() == static_cast<std::size_t>(iStart);
    }

    std::size_t opengl_triangle_renderer::size() const
    {
        return end() - begin();
    }

    opengl_triangle_renderer::value_type const& opengl_triangle_renderer::operator[](std::size_t aOffset) const
    {
        return *(begin() + aOffset);
    }

    opengl_triangle_renderer::value_type& opengl_triangle_renderer::operator[](std::size_t aOffset)
    {
        return *(begin() + aOffset);
    }

    void opengl_triangle_renderer::push_back(value_type const& aVertex)
    {
        if (!room_for(1))
            draw_and_execute();
        vertices().push_back(aVertex);
    }

    std::size_t opengl_triangle_renderer::room() const
    {
        return vertices().room();
    }

    bool opengl_triangle_renderer::room_for(std::size_t aAmount) const
    {
        return vertices().room_for(aAmount);
    }

    bool opengl_triangle_renderer::need(std::size_t aAmount)
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
    
    void opengl_triangle_renderer::draw_and_execute()
    {
        draw();
        execute();
        vertices().clear();
        iStart = 0;
    }
    
    void opengl_triangle_renderer::sync()
    {
        iVertexBuffer.sync();
    }
    
    void opengl_triangle_renderer::execute()
    {
        iVertexBuffer.execute();
    }

    void opengl_triangle_renderer::draw(const skip& aSkip)
    {
        draw(vertices().size() - static_cast<std::size_t>(iStart), aSkip);
    }

    void opengl_triangle_renderer::draw(std::size_t aStart, std::size_t aCount, const skip& aSkip)
    {
        iStart = static_cast<GLint>(aStart);
        draw(aCount, aSkip);
    }

    void opengl_triangle_renderer::draw(std::size_t aCount, const skip& aSkip)
    {
        if (aCount == 0u)
            return;

        iDrawOnExit = false;

        auto const skipCount = aSkip.skipCount ? std::max<std::size_t>(*aSkip.skipCount, 1u) : 1u;
        auto const vertexCount = vertices().size();
        if (static_cast<std::size_t>(iStart) + aCount > vertexCount)
            throw invalid_draw_count();
        if (static_cast<std::size_t>(iStart) == vertexCount)
            return;

        iParent.rendering_engine().vertex_buffer(iProvider).attach_shader(iParent, iParent.rendering_engine().active_shader_program());

        vertices().flush(iStart, aCount);

        sync();

        if (!iUseBarrier)
        {
            glCheck(glDrawArrays(GL_TRIANGLES, iStart, static_cast<GLsizei>(aCount)));
            iStart += static_cast<GLint>(aCount);
        }
        else
        {
            glCheck(glTextureBarrier());
            auto const pvc = primitive_vertex_count();
            auto chunk = pvc * skipCount;
            while (aCount > 0)
            {
                auto amount = std::min(chunk, aCount);
                glCheck(glDrawArrays(GL_TRIANGLES, iStart, static_cast<GLsizei>(amount)));
                iStart += static_cast<GLint>(amount);
                aCount -= amount;
                glCheck(glTextureBarrier());
            }
        }
    }

    bool opengl_triangle_renderer::is_new_transformation(const optional_mat44& aTransformation) const
    {
        return iVertexBuffer.transformation() != aTransformation;
    }

    const optional_mat44& opengl_triangle_renderer::transformation() const
    {
        return iVertexBuffer.transformation();
    }

    void opengl_triangle_renderer::set_transformation(const optional_mat44& aTransformation)
    {
        iVertexBuffer.set_transformation(aTransformation);
    }

    const opengl_vertex_buffer<>::vertex_array& opengl_triangle_renderer::vertices() const
    {
        return iVertexBuffer.vertices();
    }

    opengl_vertex_buffer<>::vertex_array& opengl_triangle_renderer::vertices()
    {
        return iVertexBuffer.vertices();
    }
}