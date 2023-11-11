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
        virtual optional_rect const& viewport() const = 0;
        virtual void set_viewport(optional_rect const& aViewPort = {}) = 0;
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
        void set_view(rect const& aView, scalar aRotation = 0.0)
        {
            set_center(aView.center().to_vec2());
            set_size(aView.extents().to_vec2());
            set_rotation(aRotation);
        }
    };
}