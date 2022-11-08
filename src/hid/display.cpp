// display.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/display.hpp>

namespace neogfx
{
    window_placement window_placement::default_placement()
    {
        return window_placement{ service<i_basic_services>().display().rect().extents() };
    }

    display::display(uint32_t aIndex, const neogfx::rect& aRect, const neogfx::rect& aDesktopRect) :
        iIndex{ aIndex },
        iRect{ aRect },
        iDesktopRect{ aDesktopRect },
        iPixelDensityDpi{ 96.0, 96.0 },
        iSubpixelFormat{ subpixel_format::None }
    {
    }

    display::~display()
    {
    }

    bool display::high_dpi() const
    {
        return device_metrics().ppi() >= HIGH_DPI_PPI;
    }

    dimension display::dpi_scale_factor() const
    {
        return default_dpi_scale_factor(device_metrics().ppi());
    }

    bool display::device_metrics_available() const
    {
        return true;
    }

    const i_device_metrics& display::device_metrics() const
    {
        return *this;
    }

    uint32_t display::index() const
    {
        return iIndex;
    }

    const i_device_metrics& display::metrics() const
    {
        return *this;
    }

    rect display::rect() const
    {
        return iRect;
    }

    rect display::desktop_rect() const
    {
        return iDesktopRect;
    }

    window_placement display::default_window_placement() const
    {
        auto fullscreenResolution = service<i_app>().program_options().full_screen();
        if (fullscreenResolution != std::nullopt)
        {
            if (fullscreenResolution->cx == 0)
                return window_placement{ video_mode{ rect().extents() } };
            else
                return window_placement{ video_mode{ fullscreenResolution->cx, fullscreenResolution->cy } };
        }
        return desktop_rect().deflate(desktop_rect().extents() * 0.167);
    }

    subpixel_format display::subpixel_format() const
    {
        return iSubpixelFormat;
    }

    bool display::metrics_available() const
    {
        return true;
    }

    size display::extents() const
    {
        return desktop_rect().extents();
    }

    dimension display::horizontal_dpi() const
    {
        return iPixelDensityDpi.cx;
    }

    dimension display::vertical_dpi() const
    {
        return iPixelDensityDpi.cy;
    }

    dimension display::ppi() const
    {
        return iPixelDensityDpi.magnitude() / std::sqrt(2.0);
    }

    dimension display::em_size() const
    {
        return 0;
    }
}