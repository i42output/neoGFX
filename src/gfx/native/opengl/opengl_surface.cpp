// opengl_surface.cpp
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

#ifdef _WIN32
#include <D2d1.h>
#endif
#include <numeric>
#include <neolib/task/thread.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include "opengl_surface.hpp"
#include "opengl_helpers.hpp"
#include "opengl_texture.hpp"
#include "opengl_rendering_context.hpp"

namespace neogfx
{
    opengl_surface::opengl_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow) :
        native_surface{ aRenderingEngine, aWindow }, iIdealGraphicsOperationQueueCapacity{ 0u }
    {
    }

    opengl_surface::~opengl_surface()
    {
        set_destroyed();
    }

    const i_texture& opengl_surface::target_texture() const
    {
        if (iFrameBufferTexture == std::nullopt || iFrameBufferTexture->extents() != iFrameBufferExtents)
        {
            iFrameBufferTexture = std::nullopt;
            iFrameBufferTexture.emplace(iFrameBufferExtents, 1.0, texture_sampling::Multisample);
            glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(iFrameBufferTexture->native_texture().native_handle()), 0));
        }
        return *iFrameBufferTexture;
    }

    void opengl_surface::do_activate_target() const
    {
        glCheck(glEnable(GL_MULTISAMPLE));
        glCheck(glEnable(GL_BLEND));
        glCheck(glEnable(GL_DEPTH_TEST));
        glCheck(glDepthFunc(GL_LEQUAL));
        if (iFrameBufferTexture != std::nullopt)
        {
            GLint currentFramebuffer;
            glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFramebuffer));
            if (static_cast<GLuint>(currentFramebuffer) != iFrameBuffer)
            {
                glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
            }
            GLint queryResult = 0;
            glCheck(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &queryResult));
            if (queryResult == GL_TEXTURE)
            {
                glCheck(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &queryResult));
            }
            else
                queryResult = 0;
            if (queryResult != static_cast<GLint>(iFrameBufferTexture->native_texture().native_handle()))
            {
                glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(iFrameBufferTexture->native_texture().native_handle()), 0));
            }
            glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(target_texture().native_texture().native_handle())));
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
            set_viewport(rect_i32{ point_i32{ 0, 0 }, extents().as<std::int32_t>() });
            GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
            glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
        }
    }

    color opengl_surface::read_pixel(const point& aPosition) const
    {
        if (target_texture().sampling() != neogfx::texture_sampling::Multisample)
        {
            scoped_render_target srt{ *this };
            avec4u8 pixel;
            basic_point<GLint> pos{ aPosition };
            glCheck(glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel));
            return color{ pixel[0], pixel[1], pixel[2], pixel[3] };
        }
        else
            throw std::logic_error("opengl_surface::read_pixel: not yet implemented for multisample render targets");
    }

    rect_i32 opengl_surface::viewport() const
    {
        GLint currentViewport[4];
        glCheck(glGetIntegerv(GL_VIEWPORT, currentViewport));
        return rect_i32{ point_i32{ currentViewport[0], currentViewport[1] }, size_i32{ currentViewport[2], currentViewport[3] } };
    }

    rect_i32 opengl_surface::set_viewport(const rect_i32& aViewport) const
    {
        auto const oldViewport = viewport();
        glCheck(glViewport(aViewport.x, aViewport.y, static_cast<GLsizei>(aViewport.cx), static_cast<GLsizei>(aViewport.cy)));
        return oldViewport;
    }
    
    void opengl_surface::do_render()
    {
        if (iFrameBufferExtents.cx < static_cast<double>(extents().cx) || iFrameBufferExtents.cy < static_cast<double>(extents().cy))
        {
            if (iFrameBufferExtents != size{})
            {
                glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
                iFrameBufferTexture = std::nullopt;
                glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
            }
            iFrameBufferExtents = size{
                iFrameBufferExtents.cx < extents().cx ? extents().cx * 1.5f : iFrameBufferExtents.cx,
                iFrameBufferExtents.cy < extents().cy ? extents().cy * 1.5f : iFrameBufferExtents.cy }.ceil();
            glCheck(glGenFramebuffers(1, &iFrameBuffer));
            glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, iFrameBuffer));
            target_texture();
            glCheck(glGenRenderbuffers(1, &iDepthStencilBuffer));
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
            glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(iFrameBufferExtents.cx), static_cast<GLsizei>(iFrameBufferExtents.cy)));
            glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
            glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
        }
        else
        {
            glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, iFrameBuffer));
            glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(target_texture().native_texture().native_handle())));
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
        }

        glCheck(glClear(GL_DEPTH_BUFFER_BIT));
        
        GLenum status;
        glCheck(status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
            throw failed_to_create_framebuffer(glErrorString(status));
        
        set_viewport(rect_i32{ point_i32{ 0, 0 }, extents().as<std::int32_t>() });
        
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));

        glCheck(surface_window().native_window_render(invalidated_area()));

        rendering_engine().execute_vertex_buffers();

        glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
        glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, iFrameBuffer));
        glCheck(glBlitFramebuffer(0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), 0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), GL_COLOR_BUFFER_BIT, GL_NEAREST));
    }

    std::unique_ptr<i_rendering_context> opengl_surface::create_graphics_context(blending_mode aBlendingMode) const
    {
        return std::make_unique<opengl_rendering_context>(*this, aBlendingMode);
    }

    std::unique_ptr<i_rendering_context> opengl_surface::create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode) const
    {
        return std::make_unique<opengl_rendering_context>(*this, aWidget, aBlendingMode);
    }

    std::size_t opengl_surface::ideal_graphics_operation_queue_capacity() const
    {
        return iIdealGraphicsOperationQueueCapacity;
    }

    void opengl_surface::new_graphics_operation_queue_capacity(std::size_t aCapacity) const
    {
        iIdealGraphicsOperationQueueCapacity = aCapacity;
    }

    void opengl_surface::set_destroying()
    {
        if (!is_alive())
            return;
        if (iFrameBufferExtents != size{})
        {
            scoped_render_target srt{ *this };
            glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
            iFrameBufferTexture = std::nullopt;
            glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
        }
        native_surface::set_destroying();
    }

    void opengl_surface::set_destroyed()
    {
        native_surface::set_destroyed();
    }
}