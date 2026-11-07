// linux_renderer.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <GL/glew.h>
#define EGL_NO_X11
#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <neolib/core/scoped.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../app/native/linux_basic_services.hpp"
#include "../../gui/window/native/wayland_window.hpp"
#include "../../gui/window/native/x11_window.hpp"
#include "../../gui/window/native/virtual_window.hpp"
#include "../../gui/window/native/virtual_surface.hpp"
#include "opengl/opengl_surface.hpp"
#include "vulkan/vulkan_graphics_backend.hpp"
#include "vulkan/vulkan_surface.hpp"
#include "linux_renderer.hpp"

#ifndef NDEBUG
#define DEBUG_GL
#endif

namespace neogfx
{
    namespace native::linux
    {
        namespace
        {
            std::string egl_error_text()
            {
                switch (::eglGetError())
                {
                case EGL_SUCCESS: return "EGL_SUCCESS";
                case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
                case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
                case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
                case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
                case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
                case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
                case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
                case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
                case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
                case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
                case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
                case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
                case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
                case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
                default: return "unknown EGL error";
                }
            }

#ifdef DEBUG_GL
            void GLAPIENTRY gl_debug_callback(GLenum, GLenum, GLuint, GLenum aSeverity, GLsizei, const GLchar* aMessage, const void*)
            {
                if (aSeverity != GL_DEBUG_SEVERITY_NOTIFICATION)
                    std::cerr << "GL: " << aMessage << std::endl;
            }
#endif
        }

        renderer::renderer(neogfx::renderer aRenderer) :
            native_renderer{ aRenderer },
            iInitialized{ false },
            iVsyncEnabled{ true },
            iContext{ nullptr },
            iCreatingWindow{ 0 },
            iActiveTarget{ nullptr }
        {
            if (aRenderer != neogfx::renderer::None)
            {
                switch (aRenderer)
                {
                case neogfx::renderer::Software:
                case neogfx::renderer::DirectX:
                    throw unsupported_renderer();
                    break;
                case neogfx::renderer::OpenGL:
                case neogfx::renderer::Vulkan:
                    break;
                default:
                    break;
                }
            }
        }

        renderer::~renderer()
        {
            cleanup();
        }

        void renderer::initialize()
        {
            if (!iInitialized)
            {
                if (!vulkan())
                    init_egl();
                native_renderer::initialize();
                iInitialized = true;
            }
        }

        void renderer::cleanup()
        {
            if (iInitialized && native_renderer::renderer() != neogfx::renderer::None)
            {
                if (iContext != nullptr)
                    make_current(iOffscreenSurface);
                native_renderer::cleanup();
                if (iContext != nullptr)
                    destroy_context(iContext);
                for (auto& s : iWindowSurfaces)
                    ::eglDestroySurface(iEglDisplay, s.second);
                iWindowSurfaces.clear();
                if (iOffscreenSurface != EGL_NO_SURFACE && iOffscreenSurface != nullptr)
                    ::eglDestroySurface(iEglDisplay, iOffscreenSurface);
                iOffscreenSurface = nullptr;
                if (iEglDisplay != nullptr)
                    ::eglTerminate(iEglDisplay);
                iEglDisplay = nullptr;
                iInitialized = false;
            }
        }

        bool renderer::vsync_enabled() const
        {
            return iVsyncEnabled;
        }

        void renderer::enable_vsync()
        {
            if (!iVsyncEnabled)
            {
                iVsyncEnabled = true;
                if (vulkan())
                    static_cast<vulkan_graphics_backend&>(backend()).set_vsync(true);
                else
                {
                    iSwapIntervalApplied.clear();
                    apply_swap_interval(::eglGetCurrentSurface(EGL_DRAW));
                }
            }
        }

        void renderer::disable_vsync()
        {
            if (iVsyncEnabled)
            {
                iVsyncEnabled = false;
                if (vulkan())
                    static_cast<vulkan_graphics_backend&>(backend()).set_vsync(false);
                else
                {
                    iSwapIntervalApplied.clear();
                    apply_swap_interval(::eglGetCurrentSurface(EGL_DRAW));
                }
            }
        }

        pixel_format_t renderer::set_pixel_format(const i_render_target&)
        {
            // n.b. EGL has one configuration for all of the surfaces (see init_egl)
            return 0;
        }

        void renderer::push_target(const i_render_target& aTarget)
        {
            auto const currentlyActive = active_target();
            if (currentlyActive && currentlyActive != &aTarget)
                currentlyActive->deactivate_target();
            aTarget.target_add_ref();
            aTarget.activate_target();
            iTargetStack.push_back(&aTarget);
        }

        void renderer::pop_target()
        {
            if (iTargetStack.empty())
                return;
            auto const popped = iTargetStack.back();
            bool const wasActive = (active_target() == popped);
            popped->target_release();
            if (wasActive)
                popped->deactivate_target();
            iTargetStack.pop_back();
            if (wasActive && !iTargetStack.empty())
                iTargetStack.back()->activate_target();
        }

        const i_render_target* renderer::active_target() const
        {
            if (iTargetStack.empty() || iTargetStack.back() != iActiveTarget)
                return nullptr;
            return iActiveTarget;
        }

        void renderer::activate_context(const i_render_target& aTarget)
        {
            (void)create_context(aTarget);

            if (!iInitialized)
                initialize();

            activate_target(aTarget);
        }

        void renderer::deactivate_context()
        {
            if (!active_target())
                throw no_target_active();

            deactivate_target(*active_target());
        }

        renderer::handle renderer::create_context(const i_render_target& aTarget)
        {
            if (vulkan())
                return nullptr;
            if (aTarget.target_type() == render_target_type::Surface)
                aTarget.pixel_format();
            init_egl();
            return iContext;
        }

        void renderer::destroy_context(handle aContext)
        {
            ::eglMakeCurrent(iEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (!::eglDestroyContext(iEglDisplay, static_cast<EGLContext>(aContext)))
                throw failed_to_destroy_opengl_context(egl_error_text());
            if (iContext == aContext)
                iContext = nullptr;
        }

        void renderer::remove_target(const i_render_target& aTarget)
        {
            if (iActiveTarget == &aTarget)
                iActiveTarget = nullptr;
            iTargetStack.erase(std::remove(iTargetStack.begin(), iTargetStack.end(), &aTarget), iTargetStack.end());
        }

        std::uint64_t renderer::target_activation_counter(render_target_type aTargetType) const
        {
            return iTargetActivationCounter[static_cast<std::size_t>(aTargetType)];
        }

        viewport renderer::viewport() const
        {
            return backend().viewport();
        }

        std::optional<rect> renderer::scissor() const
        {
            return backend().scissor();
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if ((aWindow.style() & window_style::Nested) != window_style::Nested)
            {
                if (wayland())
                    aResult = make_ref<wayland::window>(*this, aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle);
                else
                    aResult = make_ref<x11::window>(*this, aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle);
                attach_surface(aWindow, *aResult);
            }
            else
                throw virtual_surface_must_have_parent();
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if ((aWindow.style() & window_style::Nested) != window_style::Nested)
            {
                if (wayland())
                    aResult = make_ref<wayland::window>(*this, aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle);
                else
                    aResult = make_ref<x11::window>(*this, aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle);
                attach_surface(aWindow, *aResult);
            }
            else
                throw virtual_surface_must_have_parent();
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const point& aPosition, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if ((aWindow.style() & window_style::Nested) != window_style::Nested)
            {
                if (wayland())
                    aResult = make_ref<wayland::window>(*this, aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle);
                else
                    aResult = make_ref<x11::window>(*this, aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle);
                attach_surface(aWindow, *aResult);
            }
            else
                throw virtual_surface_must_have_parent();
        }

        // n.b. a parent that is not nested is one of this renderer's windows (Wayland's or X11's, as the display
        // server is); a nested (virtual) window cannot be the parent of a native window

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const video_mode& aVideoMode, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if (!aParent.is_nested())
            {
                if ((aWindow.style() & window_style::Nested) != window_style::Nested)
                {
                    if (wayland())
                        aResult = make_ref<wayland::window>(*this, aSurfaceManager, aWindow, static_cast<wayland::window&>(aParent), aVideoMode, aWindowTitle, aStyle);
                    else
                        aResult = make_ref<x11::window>(*this, aSurfaceManager, aWindow, static_cast<x11::window&>(aParent), aVideoMode, aWindowTitle, aStyle);
                    attach_surface(aWindow, *aResult);
                }
                else
                    throw virtual_surface_cannot_be_fullscreen();
            }
            else
                create_window(aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle, aResult);
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if (!aParent.is_nested())
            {
                if ((aWindow.style() & window_style::Nested) != window_style::Nested)
                {
                    if (wayland())
                        aResult = make_ref<wayland::window>(*this, aSurfaceManager, aWindow, static_cast<wayland::window&>(aParent), aDimensions, aWindowTitle, aStyle);
                    else
                        aResult = make_ref<x11::window>(*this, aSurfaceManager, aWindow, static_cast<x11::window&>(aParent), aDimensions, aWindowTitle, aStyle);
                    attach_surface(aWindow, *aResult);
                }
                else
                {
                    aResult = make_ref<virtual_window>(*this, aSurfaceManager, aWindow, aParent, aDimensions, aWindowTitle, aStyle);
                    auto newSurface = make_ref<virtual_surface>(*this, aWindow);
                    aResult->attach(*newSurface);
                }
            }
            else
                create_window(aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle, aResult);
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const point& aPosition, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if (!aParent.is_nested())
            {
                if ((aWindow.style() & window_style::Nested) != window_style::Nested)
                {
                    if (wayland())
                        aResult = make_ref<wayland::window>(*this, aSurfaceManager, aWindow, static_cast<wayland::window&>(aParent), aPosition, aDimensions, aWindowTitle, aStyle);
                    else
                        aResult = make_ref<x11::window>(*this, aSurfaceManager, aWindow, static_cast<x11::window&>(aParent), aPosition, aDimensions, aWindowTitle, aStyle);
                    attach_surface(aWindow, *aResult);
                }
                else
                {
                    aResult = make_ref<virtual_window>(*this, aSurfaceManager, aWindow, aParent, aPosition, aDimensions, aWindowTitle, aStyle);
                    auto newSurface = make_ref<virtual_surface>(*this, aWindow);
                    aResult->attach(*newSurface);
                }
            }
            else
                create_window(aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle, aResult);
        }

        bool renderer::creating_window() const
        {
            return iCreatingWindow != 0;
        }

        void renderer::sync()
        {
            backend().finish();
        }

        void renderer::render_now()
        {
            service<i_surface_manager>().render_surfaces();
        }

        bool renderer::use_rendering_priority() const
        {
            // todo
            return false;
        }

        bool renderer::process_events()
        {
            // (the display server's events become the native windows' events, which are then handled as on Windows)
            bool didSome = display_server().dispatch_events();
            bool eventsAlreadyQueued = false;
            for (std::size_t s = 0; !eventsAlreadyQueued && s < service<i_surface_manager>().surface_count(); ++s)
            {
                auto& surface = service<i_surface_manager>().surface(s);
                if (!surface.has_native_surface())
                    continue;
                if (surface.surface_type() == surface_type::Window && surface.as_surface_window().as_window().native_window().events_queued())
                    eventsAlreadyQueued = true;
            }
            if (eventsAlreadyQueued)
                didSome = native_renderer::process_events() || didSome;
            display_server().flush();
            return didSome;
        }

        bool renderer::vulkan() const
        {
            return native_renderer::renderer() == neogfx::renderer::Vulkan;
        }

        void renderer::create_window_surface(void* aTargetHandle, void* aPlatformWindow)
        {
            init_egl();
            auto const surface = ::eglCreatePlatformWindowSurface(iEglDisplay, iEglConfig, aPlatformWindow, nullptr);
            if (surface == EGL_NO_SURFACE)
                throw failed_to_create_window_surface(egl_error_text());
            iWindowSurfaces[aTargetHandle] = surface;
        }

        void renderer::destroy_window_surface(void* aTargetHandle)
        {
            auto const existing = iWindowSurfaces.find(aTargetHandle);
            if (existing == iWindowSurfaces.end())
                return;
            if (::eglGetCurrentSurface(EGL_DRAW) == existing->second)
                make_current(iOffscreenSurface);
            iSwapIntervalApplied.erase(existing->second);
            ::eglDestroySurface(iEglDisplay, existing->second);
            iWindowSurfaces.erase(existing);
        }

        void renderer::swap_buffers(void* aTargetHandle)
        {
            auto const existing = iWindowSurfaces.find(aTargetHandle);
            if (existing != iWindowSurfaces.end())
                ::eglSwapBuffers(iEglDisplay, existing->second);
        }

        std::uint32_t renderer::native_visual_id()
        {
            if (vulkan())
                return 0u;
            init_egl();
            EGLint visualId = 0;
            ::eglGetConfigAttrib(iEglDisplay, iEglConfig, EGL_NATIVE_VISUAL_ID, &visualId);
            return static_cast<std::uint32_t>(visualId);
        }

        linux::display_server& renderer::display_server() const
        {
            return static_cast<linux::basic_services&>(service<i_basic_services>()).display_server();
        }

        bool renderer::wayland() const
        {
            return display_server().windowing_system() == neogfx::windowing_system::Wayland;
        }

        const i_render_target* renderer::current_target() const
        {
            if (!iTargetStack.empty())
                return iTargetStack.back();
            return nullptr;
        }

        void renderer::attach_surface(i_surface_window& aWindow, i_native_window& aNativeWindow)
        {
            if (vulkan())
            {
                auto newSurface = make_ref<vulkan_surface>(*this, aWindow);
                aNativeWindow.attach(*newSurface);
            }
            else
            {
                auto newSurface = make_ref<opengl_surface>(*this, aWindow);
                aNativeWindow.attach(*newSurface);
            }
        }

        void renderer::init_egl()
        {
            if (iContext != nullptr)
                return;

            if (iEglDisplay == nullptr)
            {
                auto const platform = wayland() ? EGL_PLATFORM_WAYLAND_KHR : EGL_PLATFORM_X11_KHR;
                iEglDisplay = ::eglGetPlatformDisplay(platform, display_server().native_display(), nullptr);
                if (iEglDisplay == EGL_NO_DISPLAY)
                    throw failed_to_initialize_renderer("eglGetPlatformDisplay: " + egl_error_text());
                EGLint major = 0;
                EGLint minor = 0;
                if (!::eglInitialize(iEglDisplay, &major, &minor))
                    throw failed_to_initialize_renderer("eglInitialize: " + egl_error_text());
                if (!::eglBindAPI(EGL_OPENGL_API))
                    throw failed_to_initialize_renderer("eglBindAPI: " + egl_error_text());
            }

            std::string const extensions = ::eglQueryString(iEglDisplay, EGL_EXTENSIONS);
            bool const surfaceless = extensions.find("EGL_KHR_surfaceless_context") != std::string::npos;

            // (opaque: neoGFX renders to its own (multisample) frame buffer, which is then blitted to the window's)
            EGLint const configAttributes[] =
            {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT | (surfaceless ? 0 : EGL_PBUFFER_BIT),
                EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_NONE
            };
            EGLint configCount = 0;
            ::eglChooseConfig(iEglDisplay, configAttributes, nullptr, 0, &configCount);
            if (configCount == 0)
                throw failed_to_create_opengl_context("no EGL configuration for OpenGL with 24-bit colour");
            std::vector<EGLConfig> configs(static_cast<std::size_t>(configCount));
            ::eglChooseConfig(iEglDisplay, configAttributes, configs.data(), configCount, &configCount);
            // (in preference, one without alpha: a window's is then opaque, and its X visual the usual one)
            iEglConfig = configs[0];
            for (auto config : configs)
            {
                EGLint alpha = 0;
                EGLint red = 0;
                ::eglGetConfigAttrib(iEglDisplay, config, EGL_ALPHA_SIZE, &alpha);
                ::eglGetConfigAttrib(iEglDisplay, config, EGL_RED_SIZE, &red);
                if (alpha == 0 && red == 8)
                {
                    iEglConfig = config;
                    break;
                }
            }

            EGLint const contextAttributes[] =
            {
                EGL_CONTEXT_MAJOR_VERSION, 4,
                EGL_CONTEXT_MINOR_VERSION, 6,
                EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
#ifdef DEBUG_GL
                EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
#endif
                EGL_NONE
            };
            iContext = ::eglCreateContext(iEglDisplay, iEglConfig, EGL_NO_CONTEXT, contextAttributes);
            if (iContext == EGL_NO_CONTEXT)
            {
                iContext = nullptr;
                throw failed_to_create_opengl_context("eglCreateContext (OpenGL 4.6 core): " + egl_error_text());
            }

            if (surfaceless)
                iOffscreenSurface = EGL_NO_SURFACE;
            else
            {
                EGLint const pbufferAttributes[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
                iOffscreenSurface = ::eglCreatePbufferSurface(iEglDisplay, iEglConfig, pbufferAttributes);
                if (iOffscreenSurface == EGL_NO_SURFACE)
                    throw failed_to_create_opengl_context("eglCreatePbufferSurface: " + egl_error_text());
            }
            make_current(iOffscreenSurface);

            // n.b. GLEW loads the entry points by glXGetProcAddress, which (as libglvnd dispatches) are those of the
            // current context whatever made it; it then wants a GLX display for GLX's extensions, which there isn't
            glewExperimental = GL_TRUE;
            auto const errGlew = glewInit();
            if (errGlew != GLEW_OK && errGlew != GLEW_ERROR_NO_GLX_DISPLAY)
                throw failed_to_initialize_renderer("GLEW error: "s + reinterpret_cast<const char*>(glewGetErrorString(errGlew)));
            // (glewInit can leave a GL error behind)
            while (glGetError() != GL_NO_ERROR);

#ifdef DEBUG_GL
            if (glDebugMessageCallback != nullptr)
            {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(gl_debug_callback, nullptr);
            }
#endif
        }

        void renderer::make_current(void* aSurface)
        {
            if (::eglGetCurrentContext() == iContext && ::eglGetCurrentSurface(EGL_DRAW) == aSurface)
                return;
            if (!::eglMakeCurrent(iEglDisplay, aSurface, aSurface, static_cast<EGLContext>(iContext)))
                throw failed_to_activate_opengl_context(egl_error_text());
        }

        void renderer::apply_swap_interval(void* aSurface)
        {
            if (aSurface == EGL_NO_SURFACE || aSurface == iOffscreenSurface || iSwapIntervalApplied.contains(aSurface))
                return;
            ::eglSwapInterval(iEglDisplay, iVsyncEnabled ? 1 : 0);
            iSwapIntervalApplied.insert(aSurface);
        }

        void renderer::activate_target(const i_render_target& aTarget)
        {
            if (iActiveTarget == &aTarget)
                return;

            iActiveTarget = &aTarget;

            ++iTargetActivationCounter[static_cast<std::size_t>(aTarget.target_type())];

            if (vulkan())
            {
                disable_vsync();
                return;
            }

            void* surface = iOffscreenSurface;
            if (aTarget.target_type() == render_target_type::Surface)
            {
                auto const existing = iWindowSurfaces.find(aTarget.target_handle());
                if (existing != iWindowSurfaces.end())
                    surface = existing->second;
            }
            make_current(surface);
            apply_swap_interval(surface);

            disable_vsync();
        }

        void renderer::deactivate_target(const i_render_target& aTarget)
        {
            if (iActiveTarget != &aTarget)
                return;

            iActiveTarget = nullptr;
        }
    }
}
