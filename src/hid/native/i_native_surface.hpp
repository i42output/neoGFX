// i_native_surface.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <neogfx/gfx/graphics_context.hpp>

namespace neogfx
{
	class i_rendering_engine;
	class i_native_graphics_context;
	class i_widget;

	class i_native_surface
	{
	public:
		event<> rendering_check;
		event<> rendering;
		event<> rendering_finished;
	public:
		struct context_mismatch : std::logic_error { context_mismatch() : std::logic_error("neogfx::i_native_surface::context_mismatch") {} };
	public:
		virtual ~i_native_surface() {}
	public:
		virtual bool pump_event() = 0;
		virtual void close() = 0;
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) = 0;
		virtual const std::pair<vec2, vec2>& logical_coordinates() const = 0;
		virtual void set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates) = 0;
	public:
		virtual void* handle() const = 0;
		virtual void* native_handle() const = 0;
		virtual point surface_position() const = 0;
		virtual void move_surface(const point& aPosition) = 0;
		virtual size surface_size() const = 0;
		virtual void resize_surface(const size& aSize) = 0;
		virtual point mouse_position() const = 0;
		virtual bool is_mouse_button_pressed(mouse_button aButton) const = 0;
	public:
		virtual void save_mouse_cursor() = 0;
		virtual void set_mouse_cursor(mouse_system_cursor aSystemCursor) = 0;
		virtual void restore_mouse_cursor() = 0;
		virtual void update_mouse_cursor() = 0;
	public:
		virtual uint64_t frame_counter() const = 0;
		virtual void limit_frame_rate(uint32_t aFps) = 0;
		virtual double fps() const = 0;
	public:
		virtual void invalidate(const rect& aInvalidatedRect) = 0;
		virtual void render(bool aOOBRequest = false) = 0;
		virtual bool is_rendering() const = 0;
		virtual void* rendering_target_texture_handle() const = 0;
		virtual size rendering_target_texture_extents() const = 0;
		virtual std::unique_ptr<i_native_graphics_context> create_graphics_context() const = 0;
		virtual std::unique_ptr<i_native_graphics_context> create_graphics_context(const i_widget& aWidget) const = 0;
	};
}