// color.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2018 Leigh Johnston
  
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
#include <neogfx/core/hsl_color.hpp>
#include <neogfx/core/hsv_color.hpp>

namespace neogfx
{
	typedef colour color;
	typedef optional_colour optional_color;
	typedef colour_or_gradient color_or_gradient;
	typedef optional_colour_or_gradient optional_color_or_gradient;
}