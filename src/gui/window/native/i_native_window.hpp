// i_native_window.hpp
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
#include <neogfx/core/geometry.hpp>
#include "../../../hid/native/i_native_surface.hpp"
#include "native_window_events.hpp"

namespace neogfx
{
	class i_window;

	class i_native_window : public i_native_surface, public i_device_metrics
	{
	public:
		typedef neolib::variant<native_window_event, native_mouse_event, native_keyboard_event> native_event;
	public:
		virtual ~i_native_window() {}
	public:
		virtual void display_error_message(const std::string& aTitle, const std::string& aMessage) const = 0;
		virtual void push_event(const native_event& aEvent) = 0;
		virtual bool pump_event() = 0;
		virtual void handle_event(const native_event& aNativeEvent) = 0;
		virtual bool processing_event() const = 0;
		virtual i_window& window() const = 0;
		virtual void close() = 0;
		virtual void show(bool aActivate = false) = 0;
		virtual void hide() = 0;
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual bool is_enabled() const = 0;
		virtual void enable(bool aEnable) = 0;
		virtual bool is_capturing() const = 0;
		virtual void set_capture() = 0;
		virtual void release_capture() = 0;
		virtual bool is_destroyed() const = 0;
		virtual bool has_rendering_priority() const = 0;
	};
}