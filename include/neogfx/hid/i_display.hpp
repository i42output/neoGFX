// i_display.hpp
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

namespace neogfx
{
	enum class subpixel_format
	{
		SubpixelFormatNone,
		SubpixelFormatRGBHorizontal,
		SubpixelFormatBGRHorizontal,
		SubpixelFormatRGBVertical,
		SubpixelFormatBGRVertical
	};

	class i_display
	{
	public:
		struct failed_to_get_monitor_dpi : std::runtime_error { failed_to_get_monitor_dpi() : std::runtime_error("neogfx::i_display::failed_to_get_monitor_dpi") {} };
	public:
		virtual ~i_display() {};
	public:
		virtual uint32_t index() const = 0;
	public:
		virtual const i_device_metrics& metrics() const = 0;
		virtual void update_dpi() = 0;
	public:
		virtual neogfx::rect rect() const = 0;
		virtual neogfx::rect desktop_rect() const = 0;
		virtual colour read_pixel(const point& aPosition) const = 0;
	public:
		virtual neogfx::subpixel_format subpixel_format() const = 0;
	};
}