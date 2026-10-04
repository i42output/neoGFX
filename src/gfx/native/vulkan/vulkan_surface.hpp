// vulkan_surface.hpp
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

#include <neogfx/gfx/texture.hpp>
#include <neogfx/hid/i_native_surface.hpp>

#include "vulkan.hpp"
#include "vulkan_graphics_backend.hpp"

#include "../../../gui/window/native/native_surface.hpp"

namespace neogfx
{
    class i_surface_window;

    // n.b. as opengl_surface: rendered to a multisample frame buffer texture which is presented by copying it to the
    // window's swapchain
    class vulkan_surface : public native_surface
    {
    public:
        vulkan_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow);
        ~vulkan_surface();
    public:
        const i_texture& target_texture() const override;
    public:
        neogfx::viewport apply_viewport() const override;
    public:
        void do_activate_target() const override;
    public:
        color read_pixel(const point& aPosition, bool aCreateCache = true) const override;
        void do_render() override;
    public:
        std::unique_ptr<i_rendering_context> create_rendering_context(blending_mode aBlendingMode) const override;
        std::unique_ptr<i_rendering_context> create_rendering_context(const i_widget& aWidget, blending_mode aBlendingMode) const override;
    protected:
        void set_destroying() override;
        void set_destroyed() override;
    private:
        vulkan_graphics_backend& backend() const;
        vulkan_image& frame_buffer_image() const;
        bool need_to_create_frame_buffer() const;
        void create_frame_buffer() const;
        void create_window_surface();
    private:
        mutable optional_texture iFrameBufferTexture;
        mutable std::unique_ptr<vulkan_image> iDepthStencilBuffer;
        mutable size iFrameBufferExtents;
        vulkan_swapchain iSwapchain;
    };
}
