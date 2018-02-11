// shapes.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <vector>
#include <neogfx/core/geometry.hpp>

namespace neogfx
{
	enum class rect_type
	{
		FilledTriangleFan,
		FilledTriangles,
		Outline
	};

	std::vector<xyz> rect_vertices(const rect& aRect, dimension aPixelAdjust, rect_type aType);

	std::vector<xyz> arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, bool aIncludeCentre, uint32_t aArcSegments = 0);
	std::vector<xyz> circle_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, bool aIncludeCentre, uint32_t aArcSegments = 0);
	std::vector<xyz> rounded_rect_vertices(const rect& aRect, dimension aRadius, bool aIncludeCentre, uint32_t aArcSegments = 0);
}