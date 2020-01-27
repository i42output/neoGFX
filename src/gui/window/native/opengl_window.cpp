// opengl_window.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/thread.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include "opengl_window.hpp"
#include "../../../gfx/native/opengl_helpers.hpp"
#include "../../../gfx/native/opengl_texture.hpp"

namespace neogfx
{
    opengl_window::opengl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow) :
        native_window{ aRenderingEngine, aSurfaceManager },
        iSurfaceWindow{ aWindow },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGui },
        iFrameCounter{ 0 },
        iRendering{ false },
        iPaused{ 0 }
    {
    }

    opengl_window::~opengl_window()
    {
        set_destroyed();
    }

    render_target_type opengl_window::target_type() const
    {
        return render_target_type::Surface;
    }

    const i_texture& opengl_window::target_texture() const
    {
        if (iFrameBufferTexture == std::nullopt || iFrameBufferTexture->extents() != iFrameBufferExtents)
        {
            iFrameBufferTexture = std::nullopt;
            iFrameBufferTexture.emplace(iFrameBufferExtents, 1.0, texture_sampling::Multisample);
            glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(reinterpret_cast<std::intptr_t>(iFrameBufferTexture->native_texture()->handle())), 0));
        }
        return *iFrameBufferTexture;
    }

    size opengl_window::target_extents() const
    {
        return extents();
    }

    void opengl_window::activate_target() const
    {
        bool alreadyActive = target_active();
        if (!alreadyActive)
        {
            TargetActivating.trigger();
            service<i_rendering_engine>().activate_context(*this);
        }
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
            if (queryResult != static_cast<GLint>(reinterpret_cast<std::intptr_t>(iFrameBufferTexture->native_texture()->handle())))
            {
                glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(reinterpret_cast<std::intptr_t>(iFrameBufferTexture->native_texture()->handle())), 0));
            }
            glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(reinterpret_cast<std::intptr_t>(target_texture().native_texture()->handle()))));
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
            glCheck(glViewport(0, 0, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
            GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
            glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
        }
        if (!alreadyActive)
            TargetActivated.trigger();
    }

    bool opengl_window::target_active() const
    {
        return rendering_engine().active_target() == this;
    }

    void opengl_window::deactivate_target() const
    {
        if (target_active())
        {
            TargetDeactivating.trigger();
            rendering_engine().deactivate_context();
            TargetDeactivated.trigger();
            return;
        }
//        throw not_active();
    }

    color opengl_window::read_pixel(const point& aPosition) const
    {
        if (target_texture().sampling() != neogfx::texture_sampling::Multisample)
        {
            scoped_render_target srt{ *this };
            std::array<uint8_t, 4> pixel;
            basic_point<GLint> pos{ aPosition };
            glCheck(glReadPixels(pos.x, pos.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel));
            return color{ pixel[0], pixel[1], pixel[2], pixel[3] };
        }
        else
            throw std::logic_error("opengl_window::read_pixel: not yet implemented for multisample render targets");
    }

    neogfx::logical_coordinate_system opengl_window::logical_coordinate_system() const
    {
        return iLogicalCoordinateSystem;
    }

    void opengl_window::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    logical_coordinates opengl_window::logical_coordinates() const
    {
        if (iLogicalCoordinates != std::nullopt)
            return *iLogicalCoordinates;
        neogfx::logical_coordinates result;
        switch (iLogicalCoordinateSystem)
        {
        case neogfx::logical_coordinate_system::Specified:
            throw logical_coordinates_not_specified();
            break;
        case neogfx::logical_coordinate_system::AutomaticGui:
            result.bottomLeft = vec2{ 0.0, extents().cy };
            result.topRight = vec2{ extents().cx, 0.0 };
            break;
        case neogfx::logical_coordinate_system::AutomaticGame:
            result.bottomLeft = vec2{ 0.0, 0.0 };
            result.topRight = vec2{ extents().cx, extents().cy };
            break;
        }
        return result;
    }

    void opengl_window::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    uint64_t opengl_window::frame_counter() const
    {
        return iFrameCounter;
    }

    double opengl_window::fps() const
    {
        if (iFpsData.size() < 2)
            return 0.0;
        double const totalDuration_s = std::chrono::duration_cast<std::chrono::microseconds>(iFpsData.back().second - iFpsData.front().first).count() / 1000000.0;
        double const averageDuration_s = totalDuration_s / iFpsData.size();
        return 1.0 / averageDuration_s;
    }

    double opengl_window::potential_fps() const
    {
        if (iFpsData.size() < 1)
            return 0.0;
        double const totalDuration_s = std::accumulate(iFpsData.begin(), iFpsData.end(), 0.0, [](double sum, const frame_times& frameTimes) -> double
            { return sum + std::chrono::duration_cast<std::chrono::microseconds>(frameTimes.second - frameTimes.first).count() / 1000000.0; });
        double const averageDuration_s = totalDuration_s / iFpsData.size();
        return 1.0 / averageDuration_s;
    }

    void opengl_window::invalidate(const rect& aInvalidatedRect)
    {
        if (aInvalidatedRect.cx != 0.0 && aInvalidatedRect.cy != 0.0)
        {
            if (!has_invalidated_area())
                iInvalidatedArea = aInvalidatedRect.ceil();
            else
                iInvalidatedArea = invalidated_area().combine(aInvalidatedRect).ceil();
        }
    }

    bool opengl_window::has_invalidated_area() const
    {
        return iInvalidatedArea != std::nullopt;
    }

    const rect& opengl_window::invalidated_area() const
    {
        if (has_invalidated_area())
            return *iInvalidatedArea;
        throw no_invalidated_area();
    }

    rect opengl_window::validate()
    {
        if (has_invalidated_area())
        {
            rect validatedArea = invalidated_area();
            iInvalidatedArea = std::nullopt;
            return validatedArea;
        }
        throw no_invalidated_area();
    }

    bool opengl_window::can_render() const
    {
        return !iPaused;
    }

    void opengl_window::render(bool aOOBRequest)
    {
        if (iRendering || rendering_engine().creating_window() || !can_render())
            return;

        auto const now = std::chrono::high_resolution_clock::now();

        if (!aOOBRequest)
        {
            if (rendering_engine().frame_rate_limited() && iLastFrameTime != std::nullopt &&
                std::chrono::duration_cast<std::chrono::milliseconds>(now - *iLastFrameTime).count() < 1000 / (rendering_engine().frame_rate_limit() * (!rendering_engine().use_rendering_priority() ? 1.0 : rendering_priority())))
                return;

            if (!surface_window().native_window_ready_to_render())
                return;
        }

        if (!has_invalidated_area())
            return;

        if (invalidated_area().cx <= 0.0 || invalidated_area().cy <= 0.0)
        {
            validate();
            return;
        }

        //std::cout << "to render: " << invalidated_area() << std::endl;

        ++iFrameCounter;

        iRendering = true;
        iLastFrameTime = now;

        surface_window().rendering().trigger();

        scoped_render_target srt{ *this };

        glCheck(glEnable(GL_MULTISAMPLE));
        glCheck(glEnable(GL_BLEND));
        glCheck(glEnable(GL_DEPTH_TEST));
        glCheck(glDepthFunc(GL_LEQUAL));

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
            glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(reinterpret_cast<std::intptr_t>(target_texture().native_texture()->handle()))));
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
        }
        glCheck(glClear(GL_DEPTH_BUFFER_BIT));
        GLenum status;
        glCheck(status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
            throw failed_to_create_framebuffer(glErrorString(status));
        glCheck(glViewport(0, 0, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));

        glCheck(surface_window().native_window_render(invalidated_area()));

        rendering_engine().vertex_arrays().execute();

        glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
        glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, iFrameBuffer));
        glCheck(glBlitFramebuffer(0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), 0, 0, static_cast<GLint>(extents().cx), static_cast<GLint>(extents().cy), GL_COLOR_BUFFER_BIT, GL_NEAREST));

        display();

        iRendering = false;
        validate();

        surface_window().rendering_finished().trigger();

        iFpsData.push_back(frame_times{ *iLastFrameTime, std::chrono::high_resolution_clock::now() });
        if (iFpsData.size() > 100)
            iFpsData.pop_front();        
    }

    void opengl_window::pause()
    {
        ++iPaused;
    }

    void opengl_window::resume()
    {
        if (iPaused == 0)
            throw bad_pause_count();
        --iPaused;
    }

    bool opengl_window::is_rendering() const
    {
        return iRendering;
    }

    bool opengl_window::metrics_available() const
    {
        return true;
    }

    size opengl_window::extents() const
    {
        return surface_size();
    }

    i_surface_window& opengl_window::surface_window() const
    {
        return iSurfaceWindow;
    }

    void opengl_window::set_destroying()
    {
        if (!is_alive())
            return;
        native_window::set_destroying();
        if (iFrameBufferExtents != size{})
        {
            scoped_render_target srt{ *this };
            glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
            iFrameBufferTexture = std::nullopt;
            glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
        }
        if (target_active())
            deactivate_target();
    }

    void opengl_window::set_destroyed()
    {
        native_window::set_destroyed();
    }
}