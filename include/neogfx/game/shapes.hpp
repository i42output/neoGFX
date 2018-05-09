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

	template <std::size_t VertexCount>
	using temp_vec3_buffer = neolib::vecarray<vec3, VertexCount, VertexCount, neolib::check<neolib::vecarray_overflow>, std::allocator<vec3>>;

	template <std::size_t VertexCount>
	inline void calc_rect_vertices(temp_vec3_buffer<VertexCount>& aResult, const rect& aRect, dimension aPixelAdjust, rect_type aType)
	{
		aResult.clear();
		if (aType == rect_type::FilledTriangleFan) // fill
		{
			aResult.push_back(aRect.centre().to_vec3());
			aResult.push_back(aRect.top_left().to_vec3());
			aResult.push_back(aRect.top_right().to_vec3());
			aResult.push_back(aRect.bottom_right().to_vec3());
			aResult.push_back(aRect.bottom_left().to_vec3());
			aResult.push_back(aRect.top_left().to_vec3());
		}
		else if (aType == rect_type::FilledTriangles) // fill
		{
			aResult.push_back(aRect.top_left().to_vec3());
			aResult.push_back(aRect.top_right().to_vec3());
			aResult.push_back(aRect.bottom_left().to_vec3());
			aResult.push_back(aRect.top_right().to_vec3());
			aResult.push_back(aRect.bottom_right().to_vec3());
			aResult.push_back(aRect.bottom_left().to_vec3());
		}
		else // draw (outline)
		{
			aResult.push_back(xyz{ aRect.top_left().x, aRect.top_left().y + aPixelAdjust });
			aResult.push_back(xyz{ aRect.top_right().x, aRect.top_right().y + aPixelAdjust });
			aResult.push_back(xyz{ aRect.top_right().x - aPixelAdjust, aRect.top_right().y });
			aResult.push_back(xyz{ aRect.bottom_right().x - aPixelAdjust, aRect.bottom_right().y });
			aResult.push_back(xyz{ aRect.bottom_right().x, aRect.bottom_right().y - aPixelAdjust });
			aResult.push_back(xyz{ aRect.bottom_left().x, aRect.bottom_left().y - aPixelAdjust });
			aResult.push_back(xyz{ aRect.bottom_left().x + aPixelAdjust, aRect.bottom_left().y });
			aResult.push_back(xyz{ aRect.top_left().x + aPixelAdjust, aRect.top_left().y });
		}
	}

	template <typename Container>
	inline typename Container::iterator insert_back_rect_vertices(Container& aResult, const rect& aRect, dimension aPixelAdjust, rect_type aType)
	{
		temp_vec3_buffer<8> temp;
		calc_rect_vertices(temp, aRect, aPixelAdjust, aType);
		return aResult.insert(aResult.end(), temp.begin(), temp.end());
	}

	std::vector<xyz> rect_vertices(const rect& aRect, dimension aPixelAdjust, rect_type aType);
	std::vector<xyz> arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, bool aIncludeCentre, uint32_t aArcSegments = 0);
	std::vector<xyz> circle_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, bool aIncludeCentre, uint32_t aArcSegments = 0);
	std::vector<xyz> rounded_rect_vertices(const rect& aRect, dimension aRadius, bool aIncludeCentre, uint32_t aArcSegments = 0);
}