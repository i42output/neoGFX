// device_metrics.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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

    inline dimension default_dpi_scale_factor(dimension aPpi)
    {
        return static_cast<dimension>(static_cast<int32_t>(aPpi / 150.0) + 1);
    }

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
