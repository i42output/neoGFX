// i_fragment_shader.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/i_shader.hpp>

namespace neogfx
{
    class i_rendering_context;

    class i_fragment_shader : public i_shader
    {
    };

    class i_gradient_shader : public i_fragment_shader
    {
    public:
        virtual void set_gradient(i_rendering_context& aContext, gradient& aGradient, const rect& aBoundingBox) = 0; // todo: use abstract gradient and rect types when available
    };
}