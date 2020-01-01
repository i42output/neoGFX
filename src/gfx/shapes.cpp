// shapes.cpp
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
#include <neogfx/gfx/shapes.hpp>

namespace neogfx
{
    vertices arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const point& aOrigin, mesh_type aType, uint32_t aArcSegments)
    {
        vertices result;
        angle arc = (aEndAngle != aStartAngle ? aEndAngle - aStartAngle : boost::math::constants::two_pi<angle>());
        uint32_t arcSegments = aArcSegments;
        if (arcSegments == 0)
            arcSegments = static_cast<uint32_t>(std::ceil(std::sqrt(aRadius) * 10.0) * arc / boost::math::constants::two_pi<angle>());
        angle theta = arc / static_cast<angle>(arcSegments);
        if (aType == mesh_type::TriangleFan)
        {
            result.reserve(arcSegments + 2);
            result.push_back(xyz{ aOrigin.x, aOrigin.y });
        }
        else if (aType == mesh_type::Triangles)
            result.reserve(arcSegments * 3);
        else if (aType == mesh_type::Outline)
            result.reserve(arcSegments + 1);
        auto c = std::cos(theta);
        auto s = std::sin(theta);
        auto startCoordinate = mat22{ { std::cos(aStartAngle), std::sin(aStartAngle) },{ -std::sin(aStartAngle), std::cos(aStartAngle) } } *
            vec2{ aRadius, 0.0 };
        coordinate x = startCoordinate.x;
        coordinate y = startCoordinate.y;
        for (uint32_t i = 0; i < arcSegments; ++i)
        {
            if (aType == mesh_type::Triangles)
                result.push_back(xyz{ aOrigin.x, aOrigin.y });
            result.push_back(xyz{ x + aCentre.x, y + aCentre.y });
            coordinate t = x;
            x = c * x - s * y;
            y = s * t + c * y;
            if (aType == mesh_type::Triangles)
                result.push_back(xyz{ x + aCentre.x, y + aCentre.y });
        }
        if (aStartAngle == aEndAngle)
        {
            if (aType == mesh_type::TriangleFan)
                result.push_back(result[1]);
            else if (aType == mesh_type::Outline)
                result.push_back(result[0]);
        }
        return result;
    }

    vertices circle_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, mesh_type aType, uint32_t aArcSegments)
    {
        return arc_vertices(aCentre, aRadius, aStartAngle, aStartAngle, aCentre, aType, aArcSegments);
    }

    vertices rounded_rect_vertices(const rect& aRect, dimension aRadius, mesh_type aType, uint32_t aArcSegments)
    {
        vertices result;
        auto const topLeft = arc_vertices(
            aRect.top_left() + point{ aRadius, aRadius },
            aRadius,
            boost::math::constants::pi<coordinate>(),
            boost::math::constants::pi<coordinate>() * 1.5,
            aRect.centre(),
            aType, aArcSegments);
        auto const topRight = arc_vertices(
            aRect.top_right() + point{ -aRadius, aRadius },
            aRadius,
            boost::math::constants::pi<coordinate>() * 1.5,
            boost::math::constants::pi<coordinate>() * 2.0,
            aRect.centre(),
            aType, aArcSegments);
        auto const bottomRight = arc_vertices(
            aRect.bottom_right() + point{ -aRadius, -aRadius },
            aRadius,
            0.0,
            boost::math::constants::pi<coordinate>() * 0.5,
            aRect.centre(),
            aType, aArcSegments);
        auto const bottomLeft = arc_vertices(
            aRect.bottom_left() + point{ aRadius, -aRadius },
            aRadius,
            boost::math::constants::pi<coordinate>() * 0.5,
            boost::math::constants::pi<coordinate>(),
            aRect.centre(),
            aType, aArcSegments);
        std::array<xyz, 8> const remainingCoordinates =
        {
            xyz{ (aRect.top_left() + point{ 0.0, aRadius }).x, (aRect.top_left() + point{ 0.0, aRadius }).y },
            xyz{ (aRect.top_left() + point{ aRadius, 0.0 }).x, (aRect.top_left() + point{ aRadius, 0.0 }).y },
            xyz{ (aRect.top_right() + point{ -aRadius, 0.0 }).x, (aRect.top_right() + point{ -aRadius, 0.0 }).y },
            xyz{ (aRect.top_right() + point{ 0.0, aRadius }).x, (aRect.top_right() + point{ 0.0, aRadius }).y },
            xyz{ (aRect.bottom_right() + point{ 0.0, -aRadius }).x, (aRect.bottom_right() + point{ 0.0, -aRadius }).y },
            xyz{ (aRect.bottom_right() + point{ -aRadius, 0.0 }).x, (aRect.bottom_right() + point{ -aRadius, 0.0 }).y },
            xyz{ (aRect.bottom_left() + point{ aRadius, 0.0 }).x, (aRect.bottom_left() + point{ aRadius, 0.0 }).y },
            xyz{ (aRect.bottom_left() + point{ 0.0, -aRadius }).x, (aRect.bottom_left() + point{ 0.0, -aRadius }).y }
        };
        if (aType == mesh_type::TriangleFan || aType == mesh_type::Outline)
        {
            result.reserve(topLeft.size() + topRight.size() + bottomRight.size() + bottomLeft.size() + (aType == mesh_type::TriangleFan ? 10 : 9));
            if (aType == mesh_type::TriangleFan)
                result.push_back(xyz{ aRect.centre().x, aRect.centre().y });
            result.insert(result.end(), remainingCoordinates[0]);
            result.insert(result.end(), topLeft.begin(), topLeft.end());
            result.insert(result.end(), remainingCoordinates[1]);
            result.insert(result.end(), remainingCoordinates[2]);
            result.insert(result.end(), topRight.begin(), topRight.end());
            result.insert(result.end(), remainingCoordinates[3]);
            result.insert(result.end(), remainingCoordinates[4]);
            result.insert(result.end(), bottomRight.begin(), bottomRight.end());
            result.insert(result.end(), remainingCoordinates[5]);
            result.insert(result.end(), remainingCoordinates[6]);
            result.insert(result.end(), bottomLeft.begin(), bottomLeft.end());
            result.insert(result.end(), remainingCoordinates[7]);
            result.push_back(result[aType == mesh_type::TriangleFan ? 1 : 0]);
        }
        else if (aType == mesh_type::Triangles)
        {
            result.reserve(topLeft.size() + topRight.size() + bottomRight.size() + bottomLeft.size() + (remainingCoordinates.size() - 1) * 3 + 3);
            result.insert(result.end(), topLeft.begin(), topLeft.end());
            result.insert(result.end(), topRight.begin(), topRight.end());
            result.insert(result.end(), bottomRight.begin(), bottomRight.end());
            result.insert(result.end(), bottomLeft.begin(), bottomLeft.end());
            for (std::size_t i = 0u; i < remainingCoordinates.size() - 1; ++i)
            {
                result.insert(result.end(), xyz{ aRect.centre().x, aRect.centre().y });
                result.insert(result.end(), remainingCoordinates[i]);
                result.insert(result.end(), remainingCoordinates[i + 1u]);
            }
            result.insert(result.end(), xyz{ aRect.centre().x, aRect.centre().y });
            result.insert(result.end(), remainingCoordinates[7]);
            result.insert(result.end(), remainingCoordinates[0]);
        }
        return result;
    }
}