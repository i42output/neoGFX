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

#include "neogfx.hpp"
#include "geometry.hpp"
#include "i_native_surface.hpp"

namespace neogfx
{
	class i_native_window_event_handler;

	class i_native_window : public i_native_surface, public i_device_metrics
	{
	public:
		virtual ~i_native_window() {}
	public:
		virtual void display_error_message(const std::string& aTitle, const std::string& aMessage) const = 0;
		virtual void close() = 0;
		virtual void show(bool aActivate = false) = 0;
		virtual void hide() = 0;
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual void enable(bool aEnable) = 0;
		virtual void set_capture() = 0;
		virtual void release_capture() = 0;
		virtual bool is_destroyed() const = 0;
	};
}