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

#include "neogfx.hpp"
#include "i_widget.hpp"
#include "i_surface.hpp"
#include "mouse.hpp"

namespace neogfx
{
	class i_native_graphics_context;

	class i_native_surface
	{
	public:
		event<> rendering_check;
		event<> rendering;
	public:
		virtual ~i_native_surface() {}
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) = 0;
		virtual const vector4& logical_coordinates() const = 0;
		virtual void set_logical_coordinates(const vector4& aCoordinates) = 0;
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
	public:
		virtual bool using_frame_buffer() const = 0;
		virtual void limit_frame_rate(uint32_t aFps) = 0;
	public:
		virtual void invalidate(const rect& aInvalidatedRect) = 0;
		virtual void render() = 0;
		virtual bool is_rendering() const = 0;
		virtual std::unique_ptr<i_native_graphics_context> create_graphics_context() const = 0;
		virtual std::unique_ptr<i_native_graphics_context> create_graphics_context(const i_widget& aWidget) const = 0;
	};
}