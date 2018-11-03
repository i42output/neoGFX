// shape_factory.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/vecarray.hpp>
#include <neogfx/core/shapes.hpp>

namespace neogfx
{
	vertices_t rect_vertices(const rect& aRect, dimension aPixelAdjust, mesh_type aType, scalar aZpos)
	{
		vertices_t result;
		result.reserve(16);
		back_insert_rect_vertices(result, aRect, aPixelAdjust, aType, aZpos);
		return std::move(result);
	};

	vertices_t arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, mesh_type aType, uint32_t aArcSegments)
	{
		bool const includeCentre = (aType == mesh_type::Triangles || aType == mesh_type::TriangleFan);
		vertices_t result;
		angle arc = (aEndAngle != aStartAngle ? aEndAngle - aStartAngle : boost::math::constants::two_pi<angle>());
		uint32_t arcSegments = aArcSegments;
		if (arcSegments == 0)
			arcSegments = static_cast<uint32_t>(std::ceil(std::sqrt(aRadius) * 10.0) * arc / boost::math::constants::two_pi<angle>());
		angle theta = arc / static_cast<angle>(arcSegments);
		result.reserve((arcSegments + (includeCentre ? 2 : 1)) * 2);
		if (includeCentre) // include centre
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

	vertices_t circle_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, mesh_type aType, uint32_t aArcSegments)
	{
		bool const includeCentre = (aType == mesh_type::Triangles || aType == mesh_type::TriangleFan);
		auto result = arc_vertices(aCentre, aRadius, aStartAngle, aStartAngle, aType, aArcSegments);
		result.push_back(result[includeCentre ? 1 : 0]);
		return result;
	}

	vertices_t rounded_rect_vertices(const rect& aRect, dimension aRadius, mesh_type aType, uint32_t aArcSegments)
	{
		bool const includeCentre = (aType == mesh_type::Triangles || aType == mesh_type::TriangleFan);
		vertices_t result;
		auto topLeft = arc_vertices(
			aRect.top_left() + point{ aRadius, aRadius },
			aRadius,
			boost::math::constants::pi<coordinate>(),
			boost::math::constants::pi<coordinate>() * 1.5,
			aType, aArcSegments);
		auto topRight = arc_vertices(
			aRect.top_right() + point{ -aRadius, aRadius },
			aRadius,
			boost::math::constants::pi<coordinate>() * 1.5,
			boost::math::constants::pi<coordinate>() * 2.0,
			aType, aArcSegments);
		auto bottomRight = arc_vertices(
			aRect.bottom_right() + point{ -aRadius, -aRadius },
			aRadius,
			0.0,
			boost::math::constants::pi<coordinate>() * 0.5,
			aType, aArcSegments);
		auto bottomLeft = arc_vertices(
			aRect.bottom_left() + point{ aRadius, -aRadius },
			aRadius,
			boost::math::constants::pi<coordinate>() * 0.5,
			boost::math::constants::pi<coordinate>(),
			aType, aArcSegments);
		result.reserve(topLeft.size() + topRight.size() + bottomRight.size() + bottomLeft.size() + (includeCentre ? 10 : 9));
		if (includeCentre)
			result.push_back(xyz{ aRect.centre().x, aRect.centre().y });
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
		result.push_back(result[includeCentre ? 1 : 0]);
		return result;
	}
}