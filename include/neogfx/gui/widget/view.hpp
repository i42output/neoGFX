// view.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    class view
    {
    public:
        enum class geometry_policy : std::uint32_t
        {
            Automatic   = 0x00000001, ///< Parent widget moves/resizes the view port and/or view according to the widget's client rect.
            Specified   = 0x00000002  ///< View port and/or view's position/size is specified (i.e. set explicitly).
        };
    public:
        view();
    public:
        geometry_policy view_port_geometry_policy() const;
        void set_view_port_geometry_policy(geometry_policy aGeometryPolicy);
        geometry_policy view_geometry_policy() const;
        void set_view_geometry_policy(geometry_policy aGeometryPolicy);
        optional_rect view_port() const;
        void set_view_port(optional_rect const& aViewPort);
        vec2 const& center() const;
        void set_center(vec2 const& aCenter);
        void move(vec2 const& aOffset);
        vec2 const& size() const;
        void set_size(vec2 const& aSize);
        void zoom(scalar aFactor);
        scalar rotation() const;
        void set_rotation(scalar aAngle);
        void rotate(scalar aAngle);
        mat33 const& transformation() const;
    private:
        geometry_policy iViewPortGeometryPolicy = geometry_policy::Automatic;
        geometry_policy iViewGeometryPolicy = geometry_policy::Automatic;
        std::optional<quad> iViewPort;
        vec2 iCenter;
        vec2 iSize;
        scalar iRotation = 0.0;
        mutable cache<mat33> iTransformation;
    };
}