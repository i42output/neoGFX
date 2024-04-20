// device_metrics.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/core/geometrical.hpp>

namespace neogfx
{ 
    constexpr dimension STANDARD_DPI_PPI = 96.0;
    constexpr dimension HIGH_DPI_PPI = 144.0;
    constexpr dimension DPI_DIVISOR = 48.0;

    inline dimension x2_dpi_scale_factor(dimension aPpi)
    {
        return static_cast<dimension>(static_cast<std::int32_t>(aPpi / HIGH_DPI_PPI) + 1.0);
    }

    inline dimension xn_dpi_scale_factor(dimension aPpi)
    {
        return static_cast<dimension>(static_cast<std::int32_t>(aPpi / (DPI_DIVISOR + 1.0)) + 1) * (DPI_DIVISOR / STANDARD_DPI_PPI);
    }

    enum class dpi_scale_type
    {
        X2,
        XN
    };

    inline dpi_scale_type& dpi_scale_type_for_thread()
    {
        thread_local dpi_scale_type tDs = dpi_scale_type::X2;
        return tDs;
    }

    class scoped_dpi_scale_type
    {
    public:
        scoped_dpi_scale_type(dpi_scale_type aDpiScale = dpi_scale_type::X2) :
            iPrevious{ dpi_scale_type_for_thread() }
        {
            dpi_scale_type_for_thread() = aDpiScale;
        }
        ~scoped_dpi_scale_type()
        {
            dpi_scale_type_for_thread() = iPrevious;
        }
    private:
        dpi_scale_type iPrevious;
    };

    class i_device_resolution
    {
    public:
        virtual dimension horizontal_dpi() const = 0;
        virtual dimension vertical_dpi() const = 0;
        virtual dimension ppi() const = 0;
    };

    class i_device_metrics : public i_device_resolution
    {
    public:
        virtual bool metrics_available() const = 0;
        virtual size extents() const = 0;
        virtual dimension em_size() const = 0;
    };
}
