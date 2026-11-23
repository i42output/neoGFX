// d3d12_surface.cpp
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

#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include "../native_rendering_context.hpp"
#include "d3d12_surface.hpp"

namespace neogfx
{
    d3d12_surface::d3d12_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow) :
        native_surface{ aRenderingEngine, aWindow }
    {
    }

    d3d12_surface::~d3d12_surface()
    {
        set_destroyed();
    }

    const i_texture& d3d12_surface::target_texture() const
    {
        if (iFrameBufferTexture == std::nullopt || iFrameBufferTexture->extents() != iFrameBufferExtents)
        {
            iFrameBufferTexture = std::nullopt;
            iFrameBufferTexture.emplace(iFrameBufferExtents, 1.0, texture_sampling::Multisample);
        }
        return *iFrameBufferTexture;
    }

    void d3d12_surface::do_activate_target() const
    {
        // n.b. as opengl_surface
        backend().enable_multisample(true);
        backend().enable_blending(true);
        backend().enable_depth_test(true);
        backend().set_depth_compare(D3D12_COMPARISON_FUNC_LESS_EQUAL);

        if (need_to_create_frame_buffer())
            create_frame_buffer();

        backend().set_render_target(&frame_buffer_image(), iDepthStencilBuffer.get());

        apply_viewport();
    }

    color d3d12_surface::read_pixel(const point& aPosition, bool aCreateCache) const
    {
        if (target_texture().sampling() != neogfx::texture_sampling::Multisample)
        {
            scoped_render_target srt{ *this };
            avec4u8 pixel;
            basic_point<std::int32_t> pos{ aPosition };
            backend().read_image(frame_buffer_image(), static_cast<std::uint32_t>(pos.x), static_cast<std::uint32_t>(pos.y), 1u, 1u, &pixel);
            return color{ pixel[0], pixel[1], pixel[2], pixel[3] };
        }
        else
            throw std::logic_error("d3d12_surface::read_pixel: not yet implemented for multisample render targets");
    }

    viewport d3d12_surface::apply_viewport() const
    {
        auto const currentViewport = backend().viewport();
        auto previousViewport = to_gui_rect(game_rect{ currentViewport.position(), currentViewport.extents() }, extents().cy);

        auto const ourViewport = to_game_rect(viewport(), extents().cy).as<std::int32_t>();
        backend().set_viewport(ourViewport.x, ourViewport.y, ourViewport.cx, ourViewport.cy);

        return neogfx::viewport{ previousViewport };
    }

    void d3d12_surface::do_render()
    {
        if (iSwapchain.window == nullptr)
            iSwapchain.window = static_cast<HWND>(target_handle());

        rendering_engine().clear_non_cacheable_vertex_buffers();

        if (need_to_create_frame_buffer())
            create_frame_buffer();

        backend().set_render_target(&frame_buffer_image(), iDepthStencilBuffer.get());

        backend().clear_depth_buffer();

        apply_viewport();

        surface_window().native_window_render(invalidated_areas());

        rendering_engine().execute_vertex_buffers();

        backend().enable_scissor(false);

        backend().present(iSwapchain, frame_buffer_image(), static_cast<std::uint32_t>(extents().cx), static_cast<std::uint32_t>(extents().cy));
    }

    std::unique_ptr<i_rendering_context> d3d12_surface::create_rendering_context(blending_mode aBlendingMode) const
    {
        // n.b. the rendering context shared by the native backends (see i_graphics_backend)
        return std::make_unique<native_rendering_context>(*this, aBlendingMode);
    }

    std::unique_ptr<i_rendering_context> d3d12_surface::create_rendering_context(const i_widget& aWidget, blending_mode aBlendingMode) const
    {
        return std::make_unique<native_rendering_context>(*this, aWidget, aBlendingMode);
    }

    void d3d12_surface::set_destroying()
    {
        if (!is_alive())
            return;
        if (d3d12_graphics_backend::instance() != nullptr)
        {
            if (iFrameBufferTexture != std::nullopt)
                backend().release_render_target(&frame_buffer_image());
            backend().destroy_image(iDepthStencilBuffer);
            iFrameBufferTexture = std::nullopt;
            backend().destroy_swapchain(iSwapchain);
        }
        native_surface::set_destroying();
    }

    void d3d12_surface::set_destroyed()
    {
        native_surface::set_destroyed();
    }

    d3d12_graphics_backend& d3d12_surface::backend() const
    {
        return *d3d12_graphics_backend::instance();
    }

    d3d12_image& d3d12_surface::frame_buffer_image() const
    {
        return *reinterpret_cast<d3d12_image*>(target_texture().native_texture().native_handle());
    }

    bool d3d12_surface::need_to_create_frame_buffer() const
    {
        return iFrameBufferTexture == std::nullopt ||
            iFrameBufferExtents.cx < static_cast<double>(extents().cx) || iFrameBufferExtents.cy < static_cast<double>(extents().cy);
    }

    void d3d12_surface::create_frame_buffer() const
    {
        if (!need_to_create_frame_buffer())
            return;

        if (iFrameBufferTexture != std::nullopt)
        {
            backend().release_render_target(&frame_buffer_image());
            backend().destroy_image(iDepthStencilBuffer);
            iFrameBufferTexture = std::nullopt;
        }

        // n.b. as opengl_surface: grown by half as much again
        iFrameBufferExtents = size{
            iFrameBufferExtents.cx < extents().cx ? extents().cx * 1.5f : iFrameBufferExtents.cx,
            iFrameBufferExtents.cy < extents().cy ? extents().cy * 1.5f : iFrameBufferExtents.cy }.max(size{ 32, 32 }).ceil();

        auto const& image = frame_buffer_image();

        iDepthStencilBuffer = backend().create_depth_stencil_image(image.width, image.height, image.samples);
    }
}
