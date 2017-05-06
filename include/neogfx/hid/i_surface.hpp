// i_surface.hpp
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
#include <neolib/variant.hpp>
#include <neogfx/core/geometry.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include "mouse.hpp"

namespace neogfx
{
	class i_widget;

	enum class surface_type
	{
		Window,
		Touchscreen,
		Paper		// Printing support
	};

	class i_native_surface;

	class i_surface : public i_device_metrics, public i_units_context
	{
	public:
		event<const i_widget*> dismissing_children;
		event<> closed;
	public:
		enum dismissal_type_e
		{
			CannotDismiss,
			CloseOnDismissal,
			HideOnDismissal
		};
	public:
		struct no_native_surface : std::logic_error { no_native_surface() : std::logic_error("neogfx::i_surface::no_native_surface") {} };
	public:
		virtual ~i_surface() {}
	public:
		virtual bool is_weak() const = 0;
		virtual bool is_closed() const = 0;
		virtual void close() = 0;
		virtual bool has_parent_surface() const = 0;
		virtual const i_surface& parent_surface() const = 0;
		virtual i_surface& parent_surface() = 0;
		virtual bool is_owner_of(const i_surface& aChildSurface) const = 0;
		virtual bool is_dismissing_children() const = 0;
		virtual bool can_dismiss(const i_widget* aClickedWidget) const = 0;
		virtual dismissal_type_e dismissal_type() const = 0;
		virtual bool dismissed() const = 0;
		virtual void dismiss() = 0;
	public:
		virtual neogfx::surface_type surface_type() const = 0;
		virtual uint32_t style() const = 0;
		virtual void set_style(uint32_t aStyle) = 0;
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) = 0;
		virtual const vector4& logical_coordinates() const = 0;
		virtual void set_logical_coordinates(const vector4& aCoordinates) = 0;
		virtual void layout_surface() = 0;
		virtual void invalidate_surface(const rect& aInvalidatedRect, bool aInternal = true) = 0;
		virtual bool has_rendering_priority() const = 0;
		virtual void render_surface() = 0;
		virtual graphics_context create_graphics_context() const = 0;
		virtual graphics_context create_graphics_context(const i_widget& aWidget) const = 0;
		virtual const i_native_surface& native_surface() const = 0;
		virtual i_native_surface& native_surface() = 0;
		virtual bool destroyed() const = 0;
	public:
		virtual point surface_position() const = 0;
		virtual void move_surface(const point& aPosition) = 0;
		virtual size surface_size() const = 0;
		virtual void resize_surface(const size& aSize) = 0;
	public:
		virtual point mouse_position() const = 0;
		virtual bool is_mouse_button_pressed(mouse_button aButton) const = 0;
	public:
		virtual void save_mouse_cursor() = 0;
		virtual void set_mouse_cursor(mouse_system_cursor aSystemCursor) = 0;
		virtual void restore_mouse_cursor() = 0;
		virtual void update_mouse_cursor() = 0;
	public:
		virtual void widget_added(i_widget& aWidget) = 0;
		virtual void widget_removed(i_widget& aWidget) = 0;
	public:
		virtual bool requires_owner_focus() const = 0;
		virtual bool has_entered_widget() const = 0;
		virtual i_widget& entered_widget() const = 0;
		virtual bool has_capturing_widget() const = 0;
		virtual i_widget& capturing_widget() const = 0;
		virtual void set_capture(i_widget& aWidget) = 0;
		virtual void release_capture(i_widget& aWidget) = 0;
		virtual bool has_focused_widget() const = 0;
		virtual i_widget& focused_widget() const = 0;
		virtual void set_focused_widget(i_widget& aWidget) = 0;
		virtual void release_focused_widget(i_widget& aWidget) = 0;
	};
}