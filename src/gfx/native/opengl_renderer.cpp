// opengl_renderer.cpp
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
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <ShellScalingApi.h>
#include <D2d1.h>
#pragma comment(lib, "Shcore.lib")
#endif

#include <neolib/thread.hpp>

#include <neogfx/core/numerical.hpp>

#include <neogfx/hid/i_display.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include "opengl_renderer.hpp"
#include "i_native_texture.hpp"
#include "../../gui/window/native/opengl_window.hpp"
#include "opengl_shader_program.hpp"
#include <neogfx/gfx/vertex_shader.hpp>
#include <neogfx/gfx/fragment_shader.hpp>

namespace neogfx
{
    template <>
    i_font_manager& service<i_font_manager>()
    {
        return service<i_rendering_engine>().font_manager();
    }

    template <>
    i_texture_manager& service<i_texture_manager>()
    {
        return service<i_rendering_engine>().texture_manager();
    }
}

namespace neogfx
{
    frame_counter::frame_counter(uint32_t aDuration) : iTimer{ service<neolib::async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            ++iCounter;
            for (auto w : iWidgets)
                w->update();
        }, aDuration }, iCounter{ 0 }
    {
    }

    uint32_t frame_counter::counter() const
    {
        return iCounter;
    }

    void frame_counter::add(i_widget& aWidget)
    {
        auto iterWidget = std::find(iWidgets.begin(), iWidgets.end(), &aWidget);
        if (iterWidget == iWidgets.end())
            iWidgets.push_back(&aWidget);
    }

    void frame_counter::remove(i_widget& aWidget)
    {
        auto iterWidget = std::find(iWidgets.begin(), iWidgets.end(), &aWidget);
        if (iterWidget != iWidgets.end())
            iWidgets.erase(iterWidget);
    }

    opengl_renderer::opengl_renderer(neogfx::renderer aRenderer) :
        iRenderer{ aRenderer },
        iLimitFrameRate{ true },
        iFrameRateLimit{ 60u },
        iSubpixelRendering{ true },
        iLastGameRenderTime{ 0ull }
    {
#ifdef _WIN32
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
    }

    opengl_renderer::~opengl_renderer()
    {
    }

    const i_device_metrics& opengl_renderer::default_screen_metrics() const
    {
        return service<i_basic_services>().display().metrics();
    }

    renderer opengl_renderer::renderer() const
    {
        return iRenderer;
    }

    void opengl_renderer::initialize()
    {
        std::cout << "OpenGL vendor: " << reinterpret_cast<const char*>(glGetString(GL_VENDOR)) << std::endl;
        std::cout << "OpenGL renderer: " << reinterpret_cast<const char*>(glGetString(GL_RENDERER)) << std::endl;
        std::cout << "OpenGL version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << std::endl;
        std::cout << "OpenGL shading language version: " << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)) << std::endl;

        iDefaultShaderProgram = add_shader_program(neolib::make_ref<opengl_shader_program>().as<i_shader_program>()).as<i_standard_shader_program>();
    }

    void opengl_renderer::cleanup()
    {
        // We explictly destroy these OpenGL objects here when context should still exist
        iVertexArrays = std::nullopt;
        iFontManager = std::nullopt;
        iTextureManager = std::nullopt;
        iShaderPrograms.clear();
        opengl_buffer_cleanup();
    }

    const opengl_renderer::shader_program_list& opengl_renderer::shader_programs() const
    {
        return iShaderPrograms;
    }

    const i_shader_program& opengl_renderer::shader_program(const neolib::i_string& aName) const
    {
        for (auto const& s : shader_programs())
            if (s->name() == aName)
                return *s;
        throw shader_program_not_found();
    }

    i_shader_program& opengl_renderer::shader_program(const neolib::i_string& aName)
    {
        return const_cast<i_shader_program&>(to_const(*this).shader_program(aName));
    }

    i_shader_program& opengl_renderer::add_shader_program(const neolib::i_ref_ptr<i_shader_program>& aShaderProgram)
    {
        iShaderPrograms.push_back(aShaderProgram);
        return *aShaderProgram;
    }

    bool opengl_renderer::is_shader_program_active() const
    {
        for (auto const& shaderProgram : shader_programs())
            if (shaderProgram->active())
                return true;
        return false;
    }

    i_shader_program& opengl_renderer::active_shader_program()
    {
        for (auto const& shaderProgram : shader_programs())
            if (shaderProgram->active())
                return *shaderProgram;
        throw no_shader_program_active();
    }

    const i_standard_shader_program& opengl_renderer::default_shader_program() const
    {
        return *iDefaultShaderProgram;
    }

    i_standard_shader_program& opengl_renderer::default_shader_program()
    {
        return *iDefaultShaderProgram;
    }

    opengl_renderer::handle opengl_renderer::create_shader_program_object()
    {
        GLuint programHandle = 0;;
        glCheck(programHandle = glCreateProgram());
        if (0 == programHandle)
             throw failed_to_create_shader_program("Failed to create shader program object");
        return to_opaque_handle(programHandle);
    }

    void opengl_renderer::destroy_shader_program_object(handle aShaderProgramObject)
    {
        glCheck(glDeleteProgram(to_gl_handle<GLuint>(aShaderProgramObject)));
    }

    opengl_renderer::handle opengl_renderer::create_shader_object(shader_type aShaderType)
    {
        GLenum shaderType;
        switch (aShaderType)
        {
        case shader_type::Compute:
            shaderType = GL_COMPUTE_SHADER;
            break;
        case shader_type::Vertex:
            shaderType = GL_VERTEX_SHADER;
            break;
        case shader_type::TessellationControl:
            shaderType = GL_TESS_CONTROL_SHADER;
            break;
        case shader_type::TessellationEvaluation:
            shaderType = GL_TESS_EVALUATION_SHADER;
            break;
        case shader_type::Geometry:
            shaderType = GL_GEOMETRY_SHADER;
            break;
        case shader_type::Fragment:
            shaderType = GL_FRAGMENT_SHADER;
            break;
        default:
            throw std::logic_error("neogfx: invalid shader type");
        }
        GLuint shaderHandle = 0;
        glCheck(shaderHandle = glCreateShader(shaderType));
        if (0 == shaderHandle)
            throw failed_to_create_shader();
        return to_opaque_handle(shaderHandle);
    }

    void opengl_renderer::destroy_shader_object(handle aShaderObject)
    {
        glCheck(glDeleteShader(to_gl_handle<GLuint>(aShaderObject)));
    }
      
    i_font_manager& opengl_renderer::font_manager()
    {
        if (iFontManager == std::nullopt)
            iFontManager.emplace();
        return *iFontManager;
    }

    i_texture_manager& opengl_renderer::texture_manager()
    {
        if (iTextureManager == std::nullopt)
            iTextureManager.emplace();
        return *iTextureManager;
    }

    const opengl_standard_vertex_arrays& opengl_renderer::vertex_arrays() const
    {
        if (iVertexArrays == std::nullopt)
            iVertexArrays.emplace();
        return *iVertexArrays;
    }

    opengl_standard_vertex_arrays& opengl_renderer::vertex_arrays()
    {
        return const_cast<opengl_standard_vertex_arrays&>(to_const(*this).vertex_arrays());
    }

    i_texture& opengl_renderer::ping_pong_buffer1(const size& aExtents, texture_sampling aSampling)
    {
        auto& bufferTexture = create_ping_pong_buffer(iPingPongBuffer1s, aExtents, aSampling);
        return bufferTexture;
    }

    i_texture& opengl_renderer::ping_pong_buffer2(const size& aExtents, texture_sampling aSampling)
    {
        auto& bufferTexture = create_ping_pong_buffer(iPingPongBuffer2s, aExtents, aSampling);
        return bufferTexture;
    }

    bool opengl_renderer::is_subpixel_rendering_on() const
    {
        return iSubpixelRendering;
    }
    
    void opengl_renderer::subpixel_rendering_on()
    {
        if (!iSubpixelRendering)
        {
            iSubpixelRendering = true;
            SubpixelRenderingChanged.trigger();
        }
    }

    void opengl_renderer::subpixel_rendering_off()
    {
        if (iSubpixelRendering)
        {
            iSubpixelRendering = false;
            SubpixelRenderingChanged.trigger();
        }
    }

    bool opengl_renderer::frame_rate_limited() const
    {
        return iLimitFrameRate; 
    }

    void opengl_renderer::enable_frame_rate_limiter(bool aEnable)
    {
        iLimitFrameRate = aEnable;
    }

    uint32_t opengl_renderer::frame_rate_limit() const
    {
        return iFrameRateLimit;
    }

    void opengl_renderer::set_frame_rate_limit(uint32_t aFps)
    {
        iFrameRateLimit = aFps;
    }

    bool opengl_renderer::process_events()
    {
        bool didSome = false;
        auto lastRenderTime = neolib::thread::program_elapsed_ms();
        bool finished = false;
        while (!finished)
        {    
            finished = true;
            for (std::size_t s = 0; s < service<i_surface_manager>().surface_count(); ++s)
            {
                auto& surface = service<i_surface_manager>().surface(s);
                if (surface.has_native_surface() && surface.native_surface().pump_event())
                {
                    didSome = true;
                    finished = false;
                }
            }
            if (neolib::thread::program_elapsed_ms() - lastRenderTime > 10)
            {
                lastRenderTime = neolib::thread::program_elapsed_ms();
                render_now();
            }
        }
        return didSome;
    }

    void opengl_renderer::want_game_mode()
    {
        iLastGameRenderTime = neolib::thread::program_elapsed_ms();
    }

    bool opengl_renderer::game_mode() const
    {
        return neolib::thread::program_elapsed_ms() - iLastGameRenderTime < 5000u;
    }

    void opengl_renderer::register_frame_counter(i_widget& aWidget, uint32_t aDuration)
    {
        auto iterFrameCounter = iFrameCounters.find(aDuration);
        if (iterFrameCounter == iFrameCounters.end())
            iterFrameCounter = iFrameCounters.emplace(aDuration, aDuration).first;
        iterFrameCounter->second.add(aWidget);
    }

    void opengl_renderer::unregister_frame_counter(i_widget& aWidget, uint32_t aDuration)
    {
        auto iterFrameCounter = iFrameCounters.find(aDuration);
        if (iterFrameCounter != iFrameCounters.end())
            iterFrameCounter->second.remove(aWidget);
    }

    uint32_t opengl_renderer::frame_counter(uint32_t aDuration) const
    {
        auto iterFrameCounter = iFrameCounters.find(aDuration);
        if (iterFrameCounter != iFrameCounters.end())
            return iterFrameCounter->second.counter();
        return 0;
    }    
    
    i_texture& opengl_renderer::create_ping_pong_buffer(ping_pong_buffers_t& aBufferList, const size& aExtents, texture_sampling aSampling)
    {
        auto existing = aBufferList.lower_bound(std::make_pair(aSampling, aExtents));
        if (existing != aBufferList.end() && existing->first.first == aSampling && existing->first.second >= aExtents)
            return existing->second;
        auto const sizeMultiple = 1024;
        basic_size<int32_t> idealSize{ (((static_cast<int32_t>(aExtents.cx) - 1) / sizeMultiple) + 1) * sizeMultiple, (((static_cast<int32_t>(aExtents.cy) - 1) / sizeMultiple) + 1) * sizeMultiple };
        return aBufferList.emplace(std::make_pair(aSampling, idealSize), texture{ idealSize, 1.0, aSampling }).first->second;
    }
}