// opengl_surface.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <unordered_set>

#include <neolib/core/string_utils.hpp>

#include <neogfx/neogfx.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/hid/i_native_surface.hpp>

#include "opengl.hpp"

#include "../../../gui/window/native/native_surface.hpp"

namespace neogfx
{
    class i_surface_window;

    class opengl_surface : public native_surface
    {
    public:
        opengl_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow);
        ~opengl_surface();
    public:
        const i_texture& target_texture() const override;
    public:
        rect_i32 viewport() const override;
        rect_i32 set_viewport(const rect_i32& aViewport) const override;
    public:
        void do_activate_target() const override;
    public:
        color read_pixel(const point& aPosition) const override;
        void do_render() override;
    public:
        std::unique_ptr<i_rendering_context> create_graphics_context(blending_mode aBlendingMode) const override;
        std::unique_ptr<i_rendering_context> create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode) const override;
        std::size_t ideal_graphics_operation_queue_capacity() const final;
        void new_graphics_operation_queue_capacity(std::size_t aCapacity) const final;
    protected:
        void set_destroying() override;
        void set_destroyed() override;
    private:
        GLuint iFrameBuffer;
        mutable optional_texture iFrameBufferTexture;
        GLuint iDepthStencilBuffer;
        size iFrameBufferExtents;
        mutable std::size_t iIdealGraphicsOperationQueueCapacity;
    };
}