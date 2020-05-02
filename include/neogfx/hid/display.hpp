// display.hpp
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
#include <neogfx/core/device_metrics.hpp>
#include <neogfx/core/color.hpp>
#include <neogfx/gui/window/window_bits.hpp>
#include <neogfx/hid/i_display.hpp>

namespace neogfx
{
    class display : public i_display, public i_device_metrics
    {
    public:
        display(uint32_t aIndex, const neogfx::rect& aRect, const neogfx::rect& aDesktopRect);
        ~display();
    public:
        bool high_dpi() const override;
        dimension dpi_scale_factor() const override;
    public:
        bool device_metrics_available() const override;
        const i_device_metrics& device_metrics() const override;
    public:
        uint32_t index() const override;
    public:
        const i_device_metrics& metrics() const override;
    public:
        neogfx::rect rect() const override;
        neogfx::rect desktop_rect() const override;
        window_placement default_window_placement() const override;
    public:
        neogfx::subpixel_format subpixel_format() const override;
    public:
        bool metrics_available() const override;
        size extents() const override;
        dimension horizontal_dpi() const override;
        dimension vertical_dpi() const override;
        dimension ppi() const override;
        dimension em_size() const override;
    private:
        uint32_t iIndex;
        mutable neogfx::rect iRect;
        mutable neogfx::rect iDesktopRect;
    protected:
        neogfx::size iPixelDensityDpi;
        neogfx::subpixel_format iSubpixelFormat;
    };
}