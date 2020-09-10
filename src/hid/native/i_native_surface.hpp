// i_native_surface.hpp
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
#include <neogfx/hid/mouse.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/i_property.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gfx/i_render_target.hpp>

namespace neogfx
{
    class i_rendering_engine;
    class i_rendering_context;
    class i_widget;

    class i_native_surface : public i_render_target, public i_property_owner
    {
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error("neogfx::i_native_surface::no_parent") {} };
        struct context_mismatch : std::logic_error { context_mismatch() : std::logic_error("neogfx::i_native_surface::context_mismatch") {} };
        struct no_invalidated_area : std::logic_error { no_invalidated_area() : std::logic_error("neogfx::i_native_surface::no_invalidated_area") {} };
    public:
        virtual ~i_native_surface() = default;
    public:
        virtual bool has_parent() const = 0;
        virtual const i_native_surface& parent() const = 0;
        virtual i_native_surface& parent() = 0;
    public:
        virtual bool pump_event() = 0;
        virtual void close(bool aForce = false) = 0;
    public:
        virtual void handle_dpi_changed() = 0;
    public:
        virtual bool initialising() const = 0;
        virtual void initialisation_complete() = 0;
        virtual void* handle() const = 0;
        virtual void* native_handle() const = 0;
        virtual point surface_position() const = 0;
        virtual void move_surface(const point& aPosition) = 0;
        virtual size surface_size() const = 0;
        virtual void resize_surface(const size& aSize) = 0;
    public:
        virtual uint64_t frame_counter() const = 0;
        virtual double fps() const = 0;
        virtual double potential_fps() const = 0;
    public:
        virtual void invalidate(const rect& aInvalidatedRect) = 0;
        virtual bool has_invalidated_area() const = 0;
        virtual const rect& invalidated_area() const = 0;
        virtual rect validate() = 0;
        virtual bool can_render() const = 0;
        virtual void render(bool aOOBRequest = false) = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
        virtual bool is_rendering() const = 0;
        using i_render_target::create_graphics_context;
        virtual std::unique_ptr<i_rendering_context> create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode = blending_mode::Default) const = 0;
    public:
        virtual void debug(bool aEnableDebug) = 0;
    };
}