// linux_renderer.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <set>
#include <map>

#include "native_renderer.hpp"

namespace neogfx
{
    class i_native_surface;

    namespace native::linux
    {
        class display_server;

        // OpenGL is by EGL (on Wayland and on X11 alike), Vulkan by the Wayland or Xlib surface extension.
        class renderer : public native_renderer
        {
        public:
            struct unsupported_renderer : std::runtime_error { unsupported_renderer() : std::runtime_error("neogfx::native::linux::renderer::unsupported_renderer") {} };
            struct failed_to_initialize_renderer : std::runtime_error { failed_to_initialize_renderer(std::string const& aReason) : std::runtime_error("neogfx::native::linux::renderer::failed_to_initialize_renderer: " + aReason) {} };
            struct failed_to_create_opengl_context : std::runtime_error { failed_to_create_opengl_context(std::string const& aReason) : std::runtime_error("neogfx::native::linux::renderer::failed_to_create_opengl_context: " + aReason) {} };
            struct failed_to_activate_opengl_context : std::runtime_error { failed_to_activate_opengl_context(std::string const& aReason) : std::runtime_error("neogfx::native::linux::renderer::failed_to_activate_opengl_context: " + aReason) {} };
            struct failed_to_destroy_opengl_context : std::runtime_error { failed_to_destroy_opengl_context(std::string const& aReason) : std::runtime_error("neogfx::native::linux::renderer::failed_to_destroy_opengl_context: " + aReason) {} };
            struct failed_to_create_window_surface : std::runtime_error { failed_to_create_window_surface(std::string const& aReason) : std::runtime_error("neogfx::native::linux::renderer::failed_to_create_window_surface: " + aReason) {} };
            struct virtual_surface_must_have_parent : std::logic_error { virtual_surface_must_have_parent() : std::logic_error("neogfx::native::linux::renderer::virtual_surface_must_have_parent") {} };
            struct virtual_surface_cannot_be_fullscreen : std::logic_error { virtual_surface_cannot_be_fullscreen() : std::logic_error("neogfx::native::linux::renderer::virtual_surface_cannot_be_fullscreen") {} };
            struct no_target_active : std::logic_error { no_target_active() : std::logic_error("neogfx::native::linux::renderer::no_target_active") {} };
        public:
            renderer(neogfx::renderer aRenderer);
            ~renderer();
        public:
            void initialize() final;
            void cleanup() final;
        public:
            bool vsync_enabled() const final;
            void enable_vsync() final;
            void disable_vsync() final;
            pixel_format_t set_pixel_format(const i_render_target& aTarget) final;
            void push_target(const i_render_target& aTarget) final;
            void pop_target() final;
            const i_render_target* active_target() const final;
            void activate_context(const i_render_target& aTarget) final;
            void deactivate_context() final;
            handle create_context(const i_render_target& aTarget) final;
            void destroy_context(handle aContext) final;
            void remove_target(const i_render_target& aTarget) final;
            std::uint64_t target_activation_counter(render_target_type aTargetType) const final;
        public:
            neogfx::viewport viewport() const final;
            std::optional<rect> scissor() const final;
        public:
            void create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult) final;
            void create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult) final;
            void create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const point& aPosition, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult) final;
            void create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const video_mode& aVideoMode, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult) final;
            void create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult) final;
            void create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const point& aPosition, const size& aDimensions, i_string const& aWindowTitle, window_style aStyle, i_ref_ptr<i_native_window>& aResult) final;
            bool creating_window() const final;
        public:
            void sync() final;
            void render_now() final;
            bool use_rendering_priority() const final;
        public:
            bool process_events() override;
        public:
            bool vulkan() const;
            // (for the windows: the EGL window surface of a window's native surface (wl_surface* or Window), made
            // from the platform's window (wl_egl_window* or Window*))
            void create_window_surface(void* aTargetHandle, void* aPlatformWindow);
            void destroy_window_surface(void* aTargetHandle);
            void swap_buffers(void* aTargetHandle);
            // the X visual an X11 window must have to have an EGL window surface
            std::uint32_t native_visual_id();
        private:
            linux::display_server& display_server() const;
            bool wayland() const;
            const i_render_target* current_target() const;
            void attach_surface(i_surface_window& aWindow, i_native_window& aNativeWindow);
            void init_egl();
            void make_current(void* aSurface);
            void apply_swap_interval(void* aSurface);
            void activate_target(const i_render_target& aTarget);
            void deactivate_target(const i_render_target& aTarget);
        private:
            bool iInitialized;
            bool iVsyncEnabled;
            handle iContext;
            void* iEglDisplay = nullptr;
            void* iEglConfig = nullptr;
            void* iOffscreenSurface = nullptr;
            std::unordered_map<void*, void*> iWindowSurfaces;
            std::set<void*> iSwapIntervalApplied;
            std::uint32_t iCreatingWindow;
            const i_render_target* iActiveTarget;
            std::vector<const i_render_target*> iTargetStack;
            std::uint64_t iTargetActivationCounter[static_cast<std::size_t>(render_target_type::COUNT)] = {};
        };
    }
}
