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

// Parts of the code in this source file are based on SFML's View.cpp

////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#include <neogfx/neogfx.hpp>

#include <neogfx/gfx/view.hpp>

namespace neogfx
{
    view::view()
    {
    }

    view::view(i_view const& aOther) :
        iViewport{ aOther.viewport_set() ? aOther.viewport() : decltype(iViewport){} },
        iCenter{ aOther.center() },
        iSize{ aOther.size() },
        iRotation{ aOther.rotation() }
    {
    }

    view::view(rect const& aViewport) :
        view{}
    {
        set_viewport(aViewport);
        set_view(aViewport);
    }

    bool view::viewport_set() const
    {
        return iViewport.has_value();
    }

    rect const& view::viewport() const
    {
        if (viewport_set())
            return iViewport.value();
        throw viewport_not_set();
    }

    void view::set_viewport(rect const& aViewport)
    {
        iViewport = aViewport;
    }

    void view::reset_viewport()
    {
        iViewport = std::nullopt;
    }

    vec2 const& view::center() const
    {
        return iCenter;
    }

    void view::set_center(vec2 const& aCenter)
    {
        iCenter = aCenter;
        iTransformation = invalid;
    }

    void view::move(vec2 const& aOffset)
    {
        set_center(iCenter + aOffset);
    }

    vec2 const& view::size() const
    {
        return iSize;
    }

    void view::set_size(vec2 const& aSize)
    {
        iSize = aSize;
        iTransformation = invalid;
    }

    void view::zoom(scalar aFactor)
    {
        set_size(iSize * aFactor);
    }

    scalar view::rotation() const
    {
        return iRotation;
    }

    void view::set_rotation(scalar aAngle)
    {
        iRotation = aAngle;
        iTransformation = invalid;
    }

    void view::rotate(scalar aAngle)
    {
        set_rotation(rotation() + aAngle);
    }

    mat33 const& view::transformation() const
    {
        if (!iTransformation)
        {
            // todo

            scalar const cosine = std::cos(rotation());
            scalar const sine = std::sin(rotation());
            scalar const tx = -center().x * cosine - center().y * sine + center().x;
            scalar const ty = center().x * sine - center().y * cosine + center().y;

            scalar const a = 2.0 / size().x;
            scalar const b = -2.0 / size().y;
            scalar const c = -a * center().x;
            scalar const d = -b * center().y;

            iTransformation = mat33
            {
                { a * cosine, a * sine, a * tx + c },
                { -b * sine, b * cosine, b * ty + d },
                { 0.0, 0.0, 1.0 }
            };
        }
        return iTransformation.value();
    }
}