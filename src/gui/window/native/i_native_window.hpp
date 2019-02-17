// i_native_window.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/event.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gui/window/window_events.hpp>
#include "../../../hid/native/i_native_surface.hpp"

namespace neogfx
{
	class i_surface_window;

	class i_native_window : public i_native_surface
	{
	public:
		typedef neolib::variant<window_event, mouse_event, non_client_mouse_event, keyboard_event> native_event;
	public:
		event<native_event&> filter_event;
	public:
		struct no_current_event : std::logic_error { no_current_event() : std::logic_error("neogfx::i_native_window::no_current_event") {} };
	public:
		virtual ~i_native_window() {}
	public:
		virtual const i_native_window& parent() const = 0;
		virtual i_native_window& parent() = 0;
	public:
		virtual void display_error_message(const std::string& aTitle, const std::string& aMessage) const = 0;
		virtual bool events_queued() const = 0;
		virtual void push_event(const native_event& aEvent) = 0;
		virtual bool pump_event() = 0;
		virtual void handle_event(const native_event& aNativeEvent) = 0;
		virtual bool has_current_event() const = 0;
		virtual const native_event& current_event() const = 0;
		virtual void handle_event() = 0;
		virtual bool processing_event() const = 0;
		virtual i_surface_window& surface_window() const = 0;
		virtual void close(bool aForce = false) = 0;
		virtual bool visible() const = 0;
		virtual void show(bool aActivate = false) = 0;
		virtual void hide() = 0;
		virtual double opacity() const = 0;
		virtual void set_opacity(double aOpacity) = 0;
		virtual double transparency() const = 0;
		virtual void set_transparency(double aTransparency) = 0;
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual bool is_iconic() const = 0;
		virtual void iconize() = 0;
		virtual bool is_maximized() const = 0;
		virtual void maximize() = 0;
		virtual bool is_restored() const = 0;
		virtual void restore() = 0;
		virtual bool is_fullscreen() const = 0;
		virtual bool enabled() const = 0;
		virtual void enable(bool aEnable) = 0;
		virtual bool is_capturing() const = 0;
		virtual void set_capture() = 0;
		virtual void release_capture() = 0;
		virtual void non_client_set_capture() = 0;
		virtual void non_client_release_capture() = 0;
		virtual double rendering_priority() const = 0;
		virtual const std::string& title_text() const = 0;
		virtual void set_title_text(const std::string& aTitleText) = 0;
	};
}