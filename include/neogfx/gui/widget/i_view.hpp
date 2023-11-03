// i_view.hpp
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
    class i_view
    {
    public:
        using abstract_type = i_view;
    public:
        enum class geometry_policy : std::uint32_t
        {
            Automatic   = 0x00000001, ///< Parent widget moves/resizes the view port and/or view according to the widget's client rect.
            Specified   = 0x00000002  ///< View port and/or view's position/size is specified (i.e. set explicitly).
        };
    public:
        virtual geometry_policy view_port_geometry_policy() const = 0;
        virtual void set_view_port_geometry_policy(geometry_policy aGeometryPolicy) = 0;
        virtual geometry_policy view_geometry_policy() const = 0;
        virtual void set_view_geometry_policy(geometry_policy aGeometryPolicy) = 0;
        virtual std::optional<quad> const& view_port() const = 0;
        virtual void set_view_port(std::optional<quad> const& aViewPort) = 0;
        virtual vec2 const& center() const = 0;
        virtual void set_center(vec2 const& aCenter) = 0;
        virtual void move(vec2 const& aOffset) = 0;
        virtual vec2 const& size() const = 0;
        virtual void set_size(vec2 const& aSize) = 0;
        virtual void zoom(scalar aFactor) = 0;
        virtual scalar rotation() const = 0;
        virtual void set_rotation(scalar aAngle) = 0;
        virtual void rotate(scalar aAngle) = 0;
        virtual mat33 const& transformation() const = 0;
    public:
        optional_rect view_port_as_rect() const
        {
            auto const& viewPort = view_port();
            if (viewPort != std::nullopt)
                return rect{ point{ viewPort.value()[0] }, neogfx::size{ viewPort.value()[2] - viewPort.value()[0] } };
            return {};
        }
        void set_view_port(optional_rect const& aViewPort)
        {
            if (aViewPort != std::nullopt)
                set_view_port(quad{ aViewPort.value().top_left().to_vec3(), aViewPort.value().bottom_right().to_vec3() });
            else
                set_view_port(std::optional<quad>{});
        }
    };
}