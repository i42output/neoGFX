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
#pragma once

#include <neogfx/neogfx.hpp>
#include <vector>
#include <neolib/vecarray.hpp>
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    enum class mesh_type
    {
        TriangleFan,
        Triangles,
        Outline
    };

    struct unsupported_mesh_type : std::logic_error { unsupported_mesh_type() : std::logic_error("neogfx::unsupported_mesh_type") {} };

    template <std::size_t VertexCount>
    using temp_vec3_buffer = neolib::vecarray<vec3, VertexCount, VertexCount, neolib::check<neolib::vecarray_overflow>, std::allocator<vec3>>;

    template <std::size_t VertexCount, typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline void calc_rect_vertices(temp_vec3_buffer<VertexCount>& aResult, const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aType, scalar aZpos = 0.0)
    {
        aResult.clear();
        switch(aType)
        {
        case mesh_type::TriangleFan:
            aResult.push_back(aRect.centre().to_vec3(aZpos));
            aResult.push_back(aRect.top_left().to_vec3(aZpos));
            aResult.push_back(aRect.top_right().to_vec3(aZpos));
            aResult.push_back(aRect.bottom_right().to_vec3(aZpos));
            aResult.push_back(aRect.bottom_left().to_vec3(aZpos));
            aResult.push_back(aRect.top_left().to_vec3(aZpos));
            break;
        case mesh_type::Triangles:
            aResult.push_back(aRect.top_left().to_vec3(aZpos));
            aResult.push_back(aRect.top_right().to_vec3(aZpos));
            aResult.push_back(aRect.bottom_left().to_vec3(aZpos));
            aResult.push_back(aRect.top_right().to_vec3(aZpos));
            aResult.push_back(aRect.bottom_right().to_vec3(aZpos));
            aResult.push_back(aRect.bottom_left().to_vec3(aZpos));
            break;
        case mesh_type::Outline:
            aResult.push_back(xyz{ aRect.top_left().x, aRect.top_left().y, aZpos });
            aResult.push_back(xyz{ aRect.top_right().x, aRect.top_right().y, aZpos });
            aResult.push_back(xyz{ aRect.top_right().x, aRect.top_right().y, aZpos });
            aResult.push_back(xyz{ aRect.bottom_right().x, aRect.bottom_right().y, aZpos });
            aResult.push_back(xyz{ aRect.bottom_right().x, aRect.bottom_right().y, aZpos });
            aResult.push_back(xyz{ aRect.bottom_left().x, aRect.bottom_left().y, aZpos });
            aResult.push_back(xyz{ aRect.bottom_left().x, aRect.bottom_left().y, aZpos });
            aResult.push_back(xyz{ aRect.top_left().x, aRect.top_left().y, aZpos });
            break;
        }
    }

    typedef temp_vec3_buffer<8> vec3_rect_vertices;

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    vec3_rect_vertices rect_vertices(const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aType, scalar aZpos = 0.0)
    {
        vec3_rect_vertices result;
        calc_rect_vertices(result, aRect, aType, aZpos);
        return result;
    };

    template <typename Container, typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline typename Container::iterator back_insert_rect_vertices(Container& aResult, const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aType, scalar aZpos = 0.0)
    {
        vec3_rect_vertices temp = rect_vertices(aRect, aType, aZpos);
        return aResult.insert(aResult.end(), temp.begin(), temp.end());
    }

    vertices arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const point& aOrigin, mesh_type aType, uint32_t aArcSegments = 0);
    vertices circle_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, mesh_type aType, uint32_t aArcSegments = 0);
    vertices rounded_rect_vertices(const rect& aRect, dimension aRadius, mesh_type aType, uint32_t aArcSegments = 0);
}