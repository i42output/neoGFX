// windows_renderer.cpp
/*
  neogfx C++ GUI Library
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
#include <strsafe.h>
#include <GL/glew.h>
#include <GL/wglew.h>

#include <neolib/scoped.hpp>

#include <neogfx/hid/surface_manager.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../gui/window/native/windows_window.hpp"
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
                message = "Unknown error, code: " + boost::lexical_cast<std::string>(errorMessageID);

            return message;
        }

        extern const std::wstring sWindowClassName;

        bool init_opengl(bool doubleBuffering)
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
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | (doubleBuffering ? PFD_DOUBLEBUFFER : 0u)
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

        HGLRC create_opengl_context(HDC hDC)
        {
            if (hDC == NULL)
                throw renderer::failed_to_create_opengl_context("No device context!");

            int contextAttributes[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                0, 0
            };

            HGLRC hRC = wglCreateContextAttribsARB(hDC, 0, contextAttributes);
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

        renderer::renderer(neogfx::renderer aRenderer, bool aDoubleBufferedWindows) :
            opengl_renderer{ aRenderer },
            iInitialized{ false },
            iDoubleBuffering{ aDoubleBufferedWindows },
            iContext{ nullptr },
            iCreatingWindow{ 0 }
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
                wc.style = CS_OWNDC | CS_DBLCLKS;
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
                init_opengl(iDoubleBuffering);
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
                    wglMakeCurrent(static_cast<HDC>(iDefaultOffscreenWindow.lock()->device_handle()), static_cast<HGLRC>(iContext));
                opengl_renderer::cleanup();
                if (iContext != nullptr)
                    destroy_context(iContext);
                iOffscreenWindows.clear();
                iOffscreenWindowPool.clear();
                ::UnregisterClass(sWindowClassName.c_str(), GetModuleHandle(NULL));
            }
        }

        bool renderer::double_buffering() const
        {
            return iDoubleBuffering;
        }

        renderer::pixel_format_t renderer::set_pixel_format(const i_render_target& aTarget)
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
            //    std::cerr << "renderer: activating context..." << std::endl;

            if (iContext == nullptr)
                iContext = static_cast<HGLRC>(create_context(aTarget));
            else
                aTarget.pixel_format();

            iTargetStack.push_back(&aTarget);

            if (!iInitialized)
                initialize();

            activate_current_target();

            //if constexpr (!ndebug)
            //    std::cerr << "renderer: context activated" << std::endl;
        }

        void renderer::deactivate_context()
        {
            //if constexpr (!ndebug)
            //    std::cerr << "renderer: deactivating context..." << std::endl;

            if (active_target() != nullptr)
                deallocate_offscreen_window(active_target());

            if (iTargetStack.empty())
                throw no_target_active();
            iTargetStack.pop_back();

            auto activeTarget = active_target();
            if (activeTarget != nullptr)
            {
                iTargetStack.pop_back();
                activeTarget->activate_target();
            }
            else
                activate_current_target();

            //if constexpr (!ndebug)
            //    std::cerr << "renderer: context deactivated" << std::endl;
        }

        renderer::opengl_context renderer::create_context(const i_render_target& aTarget)
        {
            if (aTarget.target_type() == render_target_type::Surface)
            {
                aTarget.pixel_format();
                return create_opengl_context(static_cast<HDC>(aTarget.target_device_handle()));
            }
            else
                return create_opengl_context(static_cast<HDC>(allocate_offscreen_window(&aTarget)->device_handle()));
        }

        void renderer::destroy_context(opengl_context aContext)
        {
            if (!::wglDeleteContext(static_cast<HGLRC>(aContext)))
                throw renderer::failed_to_destroy_opengl_context(GetLastErrorText());
            if (iContext == aContext)
                iContext = nullptr;
        }

        std::unique_ptr<i_native_window> renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle)
        {
            neolib::scoped_counter<uint32_t> sc(iCreatingWindow);
            return std::unique_ptr<i_native_window>(new window{ *this, aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle });
        }

        std::unique_ptr<i_native_window> renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
        {
            neolib::scoped_counter<uint32_t> sc(iCreatingWindow);
            return std::unique_ptr<i_native_window>(new window{ *this, aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle });
        }

        std::unique_ptr<i_native_window> renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
        {
            neolib::scoped_counter<uint32_t> sc(iCreatingWindow);
            return std::unique_ptr<i_native_window>(new window{ *this, aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle });
        }

        std::unique_ptr<i_native_window> renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle)
        {
            neolib::scoped_counter<uint32_t> sc(iCreatingWindow);
            window* parent = dynamic_cast<window*>(&aParent);
            if (parent != nullptr)
                return std::unique_ptr<i_native_window>(new window{ *this, aSurfaceManager, aWindow, *parent, aVideoMode, aWindowTitle, aStyle });
            else
                return create_window(aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle);
        }

        std::unique_ptr<i_native_window> renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
        {
            neolib::scoped_counter<uint32_t> sc(iCreatingWindow);
            window* parent = dynamic_cast<window*>(&aParent);
            if (parent != nullptr)
                return std::unique_ptr<i_native_window>(new window{ *this, aSurfaceManager, aWindow, *parent, aDimensions, aWindowTitle, aStyle });
            else
                return create_window(aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle);
        }

        std::unique_ptr<i_native_window> renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
        {
            neolib::scoped_counter<uint32_t> sc(iCreatingWindow);
            window* parent = dynamic_cast<window*>(&aParent);
            if (parent != nullptr)
                return std::unique_ptr<i_native_window>(new window{ *this, aSurfaceManager, aWindow, *parent, aPosition, aDimensions, aWindowTitle, aStyle });
            else
                return create_window(aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle);
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
                if (surface.surface_type() == surface_type::Window && static_cast<i_native_window&>(surface.native_surface()).events_queued())
                    eventsAlreadyQueued = true;
            }
            if (eventsAlreadyQueued)
                return opengl_renderer::process_events();
            else
                return false;
        }

        renderer::pixel_format_t renderer::set_pixel_format(void* aNativeSurfaceDevinceHandle)
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
            BOOL result = FALSE;
            if (active_target() != nullptr && active_target()->target_type() == render_target_type::Surface)
                result = ::wglMakeCurrent(static_cast<HDC>(active_target()->target_device_handle()), static_cast<HGLRC>(iContext));
            else
                result = ::wglMakeCurrent(static_cast<HDC>(allocate_offscreen_window(active_target())->device_handle()), static_cast<HGLRC>(iContext));
            if (!result)
                throw failed_to_activate_opengl_context(GetLastErrorText());

            typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
            static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
            if (wglSwapIntervalEXT == NULL)
                throw failed_to_get_opengl_function(GetLastErrorText());
            wglSwapIntervalEXT(0);
        }
    }
}