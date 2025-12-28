// windows_renderer.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#pragma comment(lib, "opengl32.lib")

#include <neogfx/neogfx.hpp>
#include <boost/lexical_cast.hpp>
#include <strsafe.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <neolib/core/scoped.hpp>

#include <neogfx/hid/surface_manager.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../gui/window/native/windows_window.hpp"
#include "../../gui/window/native/virtual_window.hpp"
#include "../../gui/window/native/virtual_surface.hpp"
#include "opengl/opengl_surface.hpp"
#include "windows_renderer.hpp"

namespace neogfx
{
    namespace native::windows
    {
        std::string GetLastErrorText()
        {
            DWORD errorMessageID = ::GetLastError();
            if (errorMessageID == 0)
                return std::string{};

            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            std::string message{ messageBuffer, size };
            LocalFree(messageBuffer);

            if (message.empty())
                message = "Unknown error, code: " + boost::lexical_cast<string>(errorMessageID);

            return message;
        }

        extern const std::wstring sWindowClassName;

        bool init_opengl()
        {
            HWND hTempWND = nullptr;
            HDC hTempDC = nullptr;
            HGLRC hTempRC = nullptr;

            hTempWND = ::CreateWindow(
                sWindowClassName.c_str(),
                L"neogfx::temp_window",
                WS_POPUP,
                0,
                0,
                0,
                0,
                NULL,
                NULL,
                ::GetModuleHandle(NULL),
                0);
            if (hTempWND == nullptr)
                throw renderer::failed_to_create_opengl_context(GetLastErrorText());

            hTempDC = ::GetDC(hTempWND);
            if (hTempDC == nullptr)
                throw renderer::failed_to_create_opengl_context(GetLastErrorText());

            PIXELFORMATDESCRIPTOR pfd =
            {
                sizeof(pfd),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER
            };
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 24;
            pfd.cStencilBits = 8;
            pfd.iLayerType = PFD_MAIN_PLANE;

            if (!::SetPixelFormat(hTempDC, ::ChoosePixelFormat(hTempDC, &pfd), &pfd))
                throw renderer::failed_to_create_opengl_context(GetLastErrorText());

            hTempRC = ::wglCreateContext(hTempDC);
            if (hTempRC == NULL)
                throw renderer::failed_to_create_opengl_context(GetLastErrorText());
            if (!::wglMakeCurrent(hTempDC, hTempRC))
                throw renderer::failed_to_activate_opengl_context(GetLastErrorText());

            auto errGlew = glewInit();
            if (GLEW_OK != errGlew)
                throw renderer::failed_to_initialize_renderer("GLEW error: "s + reinterpret_cast<const char*>(glewGetErrorString(errGlew)));

            if (!::wglDeleteContext(hTempRC))
                throw renderer::failed_to_destroy_opengl_context(GetLastErrorText());
            ::ReleaseDC(hTempWND, hTempDC);
            ::DestroyWindow(hTempWND);

            return true;
        }

        HGLRC create_opengl_context(HDC hDC, HGLRC hShareContext = nullptr)
        {
            if (hDC == NULL)
                throw renderer::failed_to_create_opengl_context("No device context!");

            int contextAttributes[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                0, 0
            };

            HGLRC hRC = wglCreateContextAttribsARB(hDC, hShareContext, contextAttributes);
            if (hRC == NULL)
                throw renderer::failed_to_create_opengl_context(GetLastErrorText());

            if (!::wglMakeCurrent(hDC, hRC))
                throw renderer::failed_to_activate_opengl_context(GetLastErrorText());

            return hRC;
        }

        class offscreen_window : public neogfx::offscreen_window
        {
        public:
            offscreen_window() : iHandle{ nullptr }, iDeviceHandle{ nullptr }
            {
                iHandle = ::CreateWindow(
                    sWindowClassName.c_str(),
                    L"neogfx::offscreen_window",
                    WS_POPUP,
                    0,
                    0,
                    0,
                    0,
                    NULL,
                    NULL,
                    ::GetModuleHandle(NULL),
                    0);
                if (iHandle == nullptr)
                    throw renderer::failed_to_create_offscreen_window(GetLastErrorText());
                iDeviceHandle = ::GetDC(iHandle);
                if (iDeviceHandle == nullptr)
                {
                    ::DestroyWindow(iHandle);
                    throw renderer::failed_to_create_offscreen_window(GetLastErrorText());
                }
                renderer::set_pixel_format(iDeviceHandle);
            }
            ~offscreen_window()
            {
                ::ReleaseDC(iHandle, iDeviceHandle);
                ::DestroyWindow(iHandle);
            }
        public:
            void* handle() const override
            {
                return iHandle;
            }
            void* device_handle() const override
            {
                return iDeviceHandle;
            }
        private:
            HWND iHandle;
            HDC iDeviceHandle;
        };

        renderer::renderer(neogfx::renderer aRenderer) :
            opengl_renderer{ aRenderer },
            iInitialized{ false },
            iVsyncEnabled{ true },
            iContext{ nullptr },
            iCreatingWindow{ 0 },
            iPreviousActiveTarget{ nullptr },
            iActiveTarget{ nullptr }
        {
            if (aRenderer != neogfx::renderer::None)
            {
                switch (aRenderer)
                {
                case neogfx::renderer::Vulkan:
                case neogfx::renderer::Software:
                case neogfx::renderer::DirectX: // ANGLE
                    throw unsupported_renderer();
                    break;
                case neogfx::renderer::OpenGL:
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
                WNDCLASS wc;
                wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
                wc.lpfnWndProc = window::WindowProc;
                wc.cbClsExtra = 0;
                wc.cbWndExtra = 0;
                wc.hInstance = GetModuleHandle(NULL);
                wc.hIcon = NULL;
                wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
                wc.lpszMenuName = NULL;
                wc.lpszClassName = sWindowClassName.c_str();
                ::RegisterClass(&wc);
                init_opengl();
                auto dow = allocate_offscreen_window(nullptr);
                iDefaultOffscreenWindow = dow;
                iContext = create_opengl_context(static_cast<HDC>(dow->device_handle()));
                opengl_renderer::initialize();
                iInitialized = true;
            }
        }

        void renderer::cleanup()
        {
            if (iInitialized && opengl_renderer::renderer() != neogfx::renderer::None)
            {
                if (iContext != nullptr)
                    wglMakeCurrent(static_cast<HDC>(iDefaultOffscreenWindow.lock()->device_handle()), 
                        static_cast<HGLRC>(iContext));
                opengl_renderer::cleanup();
                if (iContext != nullptr)
                    destroy_context(iContext);
                iOffscreenWindows.clear();
                iOffscreenWindowPool.clear();
                ::UnregisterClass(sWindowClassName.c_str(), GetModuleHandle(NULL));
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
                wglSwapIntervalEXT(1);
                iVsyncEnabled = true;
            }
        }

        void renderer::disable_vsync()
        {
            if (iVsyncEnabled)
            {
                wglSwapIntervalEXT(0);
                iVsyncEnabled = false;
            }
        }

        pixel_format_t renderer::set_pixel_format(const i_render_target& aTarget)
        {
            return set_pixel_format(aTarget.target_device_handle());
        }

        const i_render_target* renderer::active_target() const
        {
            if (iTargetStack.empty())
                return nullptr;
            return iTargetStack.back();
        }

        void renderer::activate_context(const i_render_target& aTarget)
        {
            //if constexpr (!ndebug)
                //service<debug::logger>() << neolib::logger::severity::Debug << "renderer: activating context..." << std::endl;

            (void)create_context(aTarget);

            iTargetStack.push_back(&aTarget);

            if (!iInitialized)
                initialize();

            activate_current_target();

            //if constexpr (!ndebug)
                //service<debug::logger>() << neolib::logger::severity::Debug << "renderer: context activated" << std::endl;
        }

        void renderer::deactivate_context()
        {
            //if constexpr (!ndebug)
                //service<debug::logger>() << neolib::logger::severity::Debug << "renderer: deactivating context..." << std::endl;

            if (iTargetStack.empty())
                throw no_target_active();

            iTargetStack.pop_back();

            iPreviousActiveTarget = iActiveTarget;

            deallocate_offscreen_window(iPreviousActiveTarget);

            activate_current_target();

            //if constexpr (!ndebug)
                //service<debug::logger>() << neolib::logger::severity::Debug << "renderer: context deactivated" << std::endl;
        }

        renderer::handle renderer::create_context(const i_render_target& aTarget)
        {
            if (aTarget.target_type() == render_target_type::Surface)
            {
                aTarget.pixel_format();

                if (iContext == nullptr)
                    iContext = create_opengl_context(static_cast<HDC>(aTarget.target_device_handle()));
            }
            else
            {
                if (iContext == nullptr)
                    iContext = create_opengl_context(static_cast<HDC>(allocate_offscreen_window(&aTarget)->device_handle()));
            }
            return iContext;
        }

        void renderer::destroy_context(handle aContext)
        {
            if (!::wglDeleteContext(static_cast<HGLRC>(aContext)))
                throw renderer::failed_to_destroy_opengl_context(GetLastErrorText());
            if (iContext == aContext)
                iContext = nullptr;
        }

        void renderer::remove_target(const i_render_target& aTarget)
        {
            if (iPreviousActiveTarget == &aTarget)
                iPreviousActiveTarget = nullptr;
            if (iActiveTarget == &aTarget)
                iActiveTarget = nullptr;
            iTargetStack.erase(std::remove(iTargetStack.begin(), iTargetStack.end(), &aTarget), iTargetStack.end());
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if ((aWindow.style() & window_style::Nested) != window_style::Nested)
            {
                aResult = make_ref<window>(*this, aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle);
                auto newSurface = make_ref<opengl_surface>(*this, aWindow);
                aResult->attach(*newSurface);
            }
            else
                throw virtual_surface_must_have_parent();
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if ((aWindow.style() & window_style::Nested) != window_style::Nested)
            {
                aResult = make_ref<window>(*this, aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle);
                auto newSurface = make_ref<opengl_surface>(*this, aWindow);
                aResult->attach(*newSurface);
            }
            else
                throw virtual_surface_must_have_parent();
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const point& aPosition, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            if ((aWindow.style() & window_style::Nested) != window_style::Nested)
            {
                aResult = make_ref<window>(*this, aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle);
                auto newSurface = make_ref<opengl_surface>(*this, aWindow);
                aResult->attach(*newSurface);
            }
            else
                throw virtual_surface_must_have_parent();
        }

        void renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const video_mode& aVideoMode, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult)
        {
            neolib::scoped_counter<std::uint32_t> sc(iCreatingWindow);
            window* parent = dynamic_cast<window*>(&aParent);
            if (parent != nullptr)
            {
                if ((aWindow.style() & window_style::Nested) != window_style::Nested)
                {
                    aResult = make_ref<window>(*this, aSurfaceManager, aWindow, *parent, aVideoMode, aWindowTitle, aStyle);
                    auto newSurface = make_ref<opengl_surface>(*this, aWindow);
                    aResult->attach(*newSurface);
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
            window* parent = dynamic_cast<window*>(&aParent);
            if (parent != nullptr)
            {
                if ((aWindow.style() & window_style::Nested) != window_style::Nested)
                {
                    aResult = make_ref<window>(*this, aSurfaceManager, aWindow, *parent, aDimensions, aWindowTitle, aStyle);
                    auto newSurface = make_ref<opengl_surface>(*this, aWindow);
                    aResult->attach(*newSurface);
                }
                else
                {
                    aResult = make_ref<virtual_window>(*this, aSurfaceManager, aWindow, *parent, aDimensions, aWindowTitle, aStyle);
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
            window* parent = dynamic_cast<window*>(&aParent);
            if (parent != nullptr)
            {
                if ((aWindow.style() & window_style::Nested) != window_style::Nested)
                {
                    aResult = make_ref<window>(*this, aSurfaceManager, aWindow, *parent, aPosition, aDimensions, aWindowTitle, aStyle);
                    auto newSurface = make_ref<opengl_surface>(*this, aWindow);
                    aResult->attach(*newSurface);
                }
                else
                {
                    aResult = make_ref<virtual_window>(*this, aSurfaceManager, aWindow, *parent, aPosition, aDimensions, aWindowTitle, aStyle);
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
                return opengl_renderer::process_events();
            else
                return false;
        }

        pixel_format_t renderer::set_pixel_format(void* aNativeSurfaceDevinceHandle)
        {
            int attributes[] =
            {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB, 32,
                WGL_DEPTH_BITS_ARB, 24,
                WGL_STENCIL_BITS_ARB, 8,
                WGL_SAMPLE_BUFFERS_ARB, 1,
                WGL_SAMPLES_ARB, 4,
                0
            };

            pixel_format_t pixelFormat = 0;
            unsigned int matching;
            if (!wglChoosePixelFormatARB(static_cast<HDC>(aNativeSurfaceDevinceHandle), attributes, NULL, 1, &pixelFormat, &matching))
                throw failed_to_set_pixel_format(GetLastErrorText());

            PIXELFORMATDESCRIPTOR pfd = {};
            ::DescribePixelFormat(static_cast<HDC>(aNativeSurfaceDevinceHandle), pixelFormat, sizeof(pfd), &pfd);
            if (!::SetPixelFormat(static_cast<HDC>(aNativeSurfaceDevinceHandle), pixelFormat, &pfd))
                throw failed_to_set_pixel_format(GetLastErrorText());
    
            return pixelFormat;
        }

        const i_render_target* renderer::current_target() const
        {
            if (!iTargetStack.empty())
                return iTargetStack.back();
            return iPreviousActiveTarget;
        }

        std::shared_ptr<neogfx::offscreen_window> renderer::allocate_offscreen_window(const i_render_target* aRenderTarget)
        {
            auto existingWindow = iOffscreenWindows.find(aRenderTarget);
            if (existingWindow != iOffscreenWindows.end())
                return existingWindow->second;
            for (auto& ow : iOffscreenWindowPool)
            {
                if (ow.use_count() == 1)
                {
                    iOffscreenWindows[aRenderTarget] = ow;
                    return ow;
                }
            }
            auto newOffscreenWindow = std::make_shared<offscreen_window>();
            iOffscreenWindowPool.push_back(newOffscreenWindow);
            iOffscreenWindows[aRenderTarget] = newOffscreenWindow;
            return newOffscreenWindow;
        }

        void renderer::deallocate_offscreen_window(const i_render_target* aRenderTarget)
        {
            auto iterRemove = iOffscreenWindows.find(aRenderTarget);
            if (iterRemove != iOffscreenWindows.end())
                iOffscreenWindows.erase(iterRemove);
        }

        void renderer::activate_current_target()
        {
            if (iActiveTarget == current_target())
                return;

            auto previouslyActive = iActiveTarget;
            iActiveTarget = current_target();

            BOOL result = FALSE;
            if (iActiveTarget->target_type() == render_target_type::Surface)
                result = ::wglMakeCurrent(static_cast<HDC>(iActiveTarget->target_device_handle()),
                    static_cast<HGLRC>(iContext));
            else
                result = ::wglMakeCurrent(static_cast<HDC>(allocate_offscreen_window(iActiveTarget)->device_handle()),
                    static_cast<HGLRC>(iContext));

            if (!result)
            {
                iActiveTarget = previouslyActive;
                throw failed_to_activate_opengl_context(GetLastErrorText());
            }

            typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
            static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
            if (wglSwapIntervalEXT == NULL)
                throw failed_to_get_opengl_function(GetLastErrorText());

            disable_vsync();
        }
    }
}