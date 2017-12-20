// shapes.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/game/shapes.hpp>

namespace neogfx
{
	std::vector<xyz> rect_vertices(const rect& aRect, dimension aPixelAdjust, rect_type aType)
	{
		std::vector<xyz> result;
		result.reserve(16);
		if (aType == rect_type::FilledTriangleFan) // fill
		{
			result.push_back(aRect.centre().to_vec3());
			result.push_back(aRect.top_left().to_vec3());
			result.push_back(aRect.top_right().to_vec3());
			result.push_back(aRect.bottom_right().to_vec3());
			result.push_back(aRect.bottom_left().to_vec3());
			result.push_back(aRect.top_left().to_vec3());
		}
		else if (aType == rect_type::FilledTriangles) // fill
		{
			result.push_back(aRect.top_left().to_vec3());
			result.push_back(aRect.top_right().to_vec3());
			result.push_back(aRect.bottom_left().to_vec3());
			result.push_back(aRect.top_right().to_vec3());
			result.push_back(aRect.bottom_right().to_vec3());
			result.push_back(aRect.bottom_left().to_vec3());
		}
		else // draw (outline)
		{
			result.push_back(xyz{ aRect.top_left().x, aRect.top_left().y + aPixelAdjust });
			result.push_back(xyz{ aRect.top_right().x, aRect.top_right().y + aPixelAdjust });
			result.push_back(xyz{ aRect.top_right().x - aPixelAdjust, aRect.top_right().y });
			result.push_back(xyz{ aRect.bottom_right().x - aPixelAdjust, aRect.bottom_right().y });
			result.push_back(xyz{ aRect.bottom_right().x, aRect.bottom_right().y - aPixelAdjust });
			result.push_back(xyz{ aRect.bottom_left().x, aRect.bottom_left().y - aPixelAdjust });
			result.push_back(xyz{ aRect.bottom_left().x + aPixelAdjust, aRect.bottom_left().y });
			result.push_back(xyz{ aRect.top_left().x + aPixelAdjust, aRect.top_left().y });
		}
		return std::move(result);
	};

	std::vector<xyz> arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, bool aIncludeCentre, uint32_t aArcSegments)
	{
		std::vector<xyz> result;
		angle arc = (aEndAngle != aStartAngle ? aEndAngle - aStartAngle : boost::math::constants::two_pi<angle>());
		uint32_t arcSegments = aArcSegments;
		if (arcSegments == 0)
			arcSegments = static_cast<uint32_t>(std::ceil(std::sqrt(aRadius) * 10.0) * arc / boost::math::constants::two_pi<angle>());
		angle theta = arc / static_cast<angle>(arcSegments);
		result.reserve((arcSegments + (aIncludeCentre ? 2 : 1)) * 2);
		if (aIncludeCentre)
		{
			result.push_back(xyz{ aCentre.x, aCentre.y });
		}
		auto c = std::cos(theta);
		auto s = std::sin(theta);
		auto startCoordinate = mat22{ { std::cos(aStartAngle), std::sin(aStartAngle) },{ -std::sin(aStartAngle), std::cos(aStartAngle) } } *
			vec2{ aRadius, 0.0 };
		coordinate x = startCoordinate.x;
		coordinate y = startCoordinate.y;
		for (uint32_t i = 0; i < arcSegments; ++i)
		{
			result.push_back(xyz{ x + aCentre.x, y + aCentre.y });
			coordinate t = x;
			x = c * x - s * y;
			y = s * t + c * y;
		}
		return result;
	}

	std::vector<xyz> circle_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, bool aIncludeCentre, uint32_t aArcSegments)
	{
		auto result = arc_vertices(aCentre, aRadius, aStartAngle, aStartAngle, aIncludeCentre, aArcSegments);
		result.push_back(result[aIncludeCentre ? 1 : 0]);
		return result;
	}

	std::vector<xyz> rounded_rect_vertices(const rect& aRect, dimension aRadius, bool aIncludeCentre, uint32_t aArcSegments)
	{
		std::vector<xyz> result;
		auto topLeft = arc_vertices(
			aRect.top_left() + point{ aRadius, aRadius },
			aRadius,
			boost::math::constants::pi<coordinate>(),
			boost::math::constants::pi<coordinate>() * 1.5,
			false, aArcSegments);
		auto topRight = arc_vertices(
			aRect.top_right() + point{ -aRadius, aRadius },
			aRadius,
			boost::math::constants::pi<coordinate>() * 1.5,
			boost::math::constants::pi<coordinate>() * 2.0,
			false, aArcSegments);
		auto bottomRight = arc_vertices(
			aRect.bottom_right() + point{ -aRadius, -aRadius },
			aRadius,
			0.0,
			boost::math::constants::pi<coordinate>() * 0.5,
			false, aArcSegments);
		auto bottomLeft = arc_vertices(
			aRect.bottom_left() + point{ aRadius, -aRadius },
			aRadius,
			boost::math::constants::pi<coordinate>() * 0.5,
			boost::math::constants::pi<coordinate>(),
			false, aArcSegments);
		result.reserve(topLeft.size() + topRight.size() + bottomRight.size() + bottomLeft.size() + (aIncludeCentre ? 9 : 8));
		if (aIncludeCentre)
		{
			result.push_back(xyz{ aRect.centre().x, aRect.centre().y });
		}
		result.insert(result.end(), xyz{ (aRect.top_left() + point{ 0.0, aRadius }).x, (aRect.top_left() + point{ 0.0, aRadius }).y });
		result.insert(result.end(), topLeft.begin(), topLeft.end());
		result.insert(result.end(), xyz{ (aRect.top_left() + point{ aRadius, 0.0 }).x, (aRect.top_left() + point{ aRadius, 0.0 }).y });
		result.insert(result.end(), xyz{ (aRect.top_right() + point{ -aRadius, 0.0 }).x, (aRect.top_right() + point{ -aRadius, 0.0 }).y });
		result.insert(result.end(), topRight.begin(), topRight.end());
		result.insert(result.end(), xyz{ (aRect.top_right() + point{ 0.0, aRadius }).x, (aRect.top_right() + point{ 0.0, aRadius }).y });
		result.insert(result.end(), xyz{ (aRect.bottom_right() + point{ 0.0, -aRadius }).x, (aRect.bottom_right() + point{ 0.0, -aRadius }).y });
		result.insert(result.end(), bottomRight.begin(), bottomRight.end());
		result.insert(result.end(), xyz{ (aRect.bottom_right() + point{ -aRadius, 0.0 }).x, (aRect.bottom_right() + point{ -aRadius, 0.0 }).y });
		result.insert(result.end(), xyz{ (aRect.bottom_left() + point{ aRadius, 0.0 }).x, (aRect.bottom_left() + point{ aRadius, 0.0 }).y });
		result.insert(result.end(), bottomLeft.begin(), bottomLeft.end());
		result.insert(result.end(), xyz{ (aRect.bottom_left() + point{ 0.0, -aRadius }).x, (aRect.bottom_left() + point{ 0.0, -aRadius }).y });
		result.push_back(result[aIncludeCentre ? 1 : 0]);
		return result;
	}
}