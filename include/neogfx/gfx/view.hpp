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

#include <neogfx/gfx/i_view.hpp>

namespace neogfx
{
    class view : public i_view
    {
    public:
        view();
        view(i_view const& aOther);
        view(rect const& aViewport);
    public:
        bool viewport_set() const final;
        rect const& viewport() const final;
        void set_viewport(rect const& aViewport) final;
        void reset_viewport() final;
        vec2 const& center() const final;
        void set_center(vec2 const& aCenter) final;
        void move(vec2 const& aOffset) final;
        vec2 const& size() const final;
        void set_size(vec2 const& aSize) final;
        void zoom(scalar aFactor) final;
        scalar rotation() const final;
        void set_rotation(scalar aAngle) final;
        void rotate(scalar aAngle) final;
        mat33 const& transformation() const final;
    private:
        optional_rect iViewport;
        vec2 iCenter;
        vec2 iSize;
        scalar iRotation = 0.0;
        mutable cache<mat33> iTransformation;
    };

    using optional_view = optional<view>;
}