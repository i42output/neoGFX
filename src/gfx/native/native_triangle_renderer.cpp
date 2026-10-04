// native_triangle_renderer.cpp
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

#include "native_triangle_renderer.hpp"

namespace neogfx
{
    native_triangle_renderer::native_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<native_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ false },
        iStart{ static_cast<std::int32_t>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        set_transformation(optional_mat44{});
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    native_triangle_renderer::native_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, const optional_mat44& aTransformation, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<native_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ false },
        iStart{ static_cast<std::int32_t>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        set_transformation(aTransformation);
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    native_triangle_renderer::native_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, std::uint32_t aMode, with_textures_t, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<native_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ true },
        iStart{ static_cast<std::int32_t>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        set_transformation(optional_mat44{});
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    native_triangle_renderer::native_triangle_renderer(i_vertex_provider& aProvider, i_rendering_context& aParent, const optional_mat44& aTransformation, with_textures_t, std::size_t aNeed, bool aUseBarrier) :
        iProvider{ aProvider },
        iParent{ aParent },
        iVertexBuffer{ static_cast<native_vertex_buffer<>&>(aParent.rendering_engine().vertex_buffer(aProvider)) },
        iWithTextures{ true },
        iStart{ static_cast<std::int32_t>(vertices().size()) },
        iUseBarrier{ aUseBarrier },
        iDrawOnExit{ true }
    {
        set_transformation(aTransformation);
        if (!room_for(aNeed) && !need(aNeed))
            throw not_enough_room();
    }
    
    native_triangle_renderer::~native_triangle_renderer()
    {
        if (iDrawOnExit)
            draw();
    }

    i_rendering_context& native_triangle_renderer::parent()
    {
        return iParent;
    }

    std::size_t native_triangle_renderer::primitive_vertex_count() const
    {
        return 3; // triangle
    }

    bool native_triangle_renderer::with_textures() const
    {
        return iWithTextures;
    }

    native_triangle_renderer::const_iterator native_triangle_renderer::begin() const
    {
        return vertices().begin() + static_cast<std::size_t>(iStart);
    }
    
    native_triangle_renderer::iterator native_triangle_renderer::begin()
    {
        return vertices().begin() + static_cast<std::size_t>(iStart);
    }
    
    native_triangle_renderer::const_iterator native_triangle_renderer::end() const
    {
        return vertices().end();
    }
    
    native_triangle_renderer::iterator native_triangle_renderer::end()
    {
        return vertices().end();
    }
    
    bool native_triangle_renderer::empty() const
    {
        return vertices().size() == static_cast<std::size_t>(iStart);
    }

    std::size_t native_triangle_renderer::size() const
    {
        return end() - begin();
    }

    native_triangle_renderer::value_type const& native_triangle_renderer::operator[](std::size_t aOffset) const
    {
        return *(begin() + aOffset);
    }

    native_triangle_renderer::value_type& native_triangle_renderer::operator[](std::size_t aOffset)
    {
        return *(begin() + aOffset);
    }

    void native_triangle_renderer::push_back(value_type const& aVertex)
    {
        vertices().push_back(aVertex);
    }

    std::size_t native_triangle_renderer::room() const
    {
        return vertices().room();
    }

    bool native_triangle_renderer::room_for(std::size_t aAmount) const
    {
        return vertices().room_for(aAmount);
    }

    bool native_triangle_renderer::need(std::size_t aAmount)
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
    
    void native_triangle_renderer::draw_and_execute()
    {
        draw();
        graphics_backend().execute();
        vertices().clear();
        iStart = 0;
    }
    
    void native_triangle_renderer::draw(const skip& aSkip)
    {
        draw(vertices().size() - static_cast<std::size_t>(iStart), aSkip);
    }

    void native_triangle_renderer::draw(std::size_t aStart, std::size_t aCount, const skip& aSkip)
    {
        iStart = static_cast<std::int32_t>(aStart);
        draw(aCount, aSkip);
    }

    void native_triangle_renderer::draw(std::size_t aCount, const skip& aSkip)
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

        if (!iUseBarrier)
        {
            graphics_backend().draw_arrays(gpu_primitive::Triangles, static_cast<std::size_t>(iStart), aCount);
            iStart += static_cast<std::int32_t>(aCount);
        }
        else
        {
            graphics_backend().texture_barrier();
            auto const pvc = primitive_vertex_count();
            auto chunk = pvc * skipCount;
            while (aCount > 0)
            {
                auto amount = std::min(chunk, aCount);
                graphics_backend().draw_arrays(gpu_primitive::Triangles, static_cast<std::size_t>(iStart), amount);
                iStart += static_cast<std::int32_t>(amount);
                aCount -= amount;
                graphics_backend().texture_barrier();
            }
        }
    }

    bool native_triangle_renderer::is_new_transformation(const optional_mat44& aTransformation) const
    {
        return iVertexBuffer.transformation() != aTransformation;
    }

    const optional_mat44& native_triangle_renderer::transformation() const
    {
        return iVertexBuffer.transformation();
    }

    void native_triangle_renderer::set_transformation(const optional_mat44& aTransformation)
    {
        auto const& contextTransform = iParent.transform();

        if (!contextTransform)
        {
            iVertexBuffer.set_transformation(aTransformation);
            return;
        }

        // the context transform is expressed relative to the context origin but vertices reach the
        // shader with that origin already added, so conjugate the transform by the origin
        auto const translation = [](scalar x, scalar y)
            {
                return mat44{
                    { 1.0, 0.0, 0.0, 0.0 },
                    { 0.0, 1.0, 0.0, 0.0 },
                    { 0.0, 0.0, 1.0, 0.0 },
                    { x, y, 0.0, 1.0 } };
            };

        auto const origin = iParent.origin();
        auto const transform = translation(origin.x, origin.y) * *contextTransform * translation(-origin.x, -origin.y);

        iVertexBuffer.set_transformation(aTransformation ? transform * *aTransformation : transform);
    }

    const native_vertex_buffer<>::vertex_array& native_triangle_renderer::vertices() const
    {
        return iVertexBuffer.vertices();
    }

    native_vertex_buffer<>::vertex_array& native_triangle_renderer::vertices()
    {
        return iVertexBuffer.vertices();
    }
}