// shapes.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/core/numerical.hpp>
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

    template <typename Vertex, std::size_t VertexCount, typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline void calc_rect_vertices(neolib::static_vector<Vertex, VertexCount>& aResult, const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aType, const optional_mat44f& aTransformation = {})
    {
        auto const& transformableRect = (aTransformation ? aRect.with_centered_origin() : aRect).as<typename Vertex::value_type>();
        aResult.clear();
        switch(aType)
        {
        case mesh_type::TriangleFan:
            aResult.push_back(transformableRect.center().to_vec3());
            aResult.push_back(transformableRect.top_left().to_vec3());
            aResult.push_back(transformableRect.top_right().to_vec3());
            aResult.push_back(transformableRect.bottom_right().to_vec3());
            aResult.push_back(transformableRect.bottom_left().to_vec3());
            aResult.push_back(transformableRect.top_left().to_vec3());
            break;
        case mesh_type::Triangles:
            aResult.push_back(transformableRect.top_left().to_vec3());
            aResult.push_back(transformableRect.top_right().to_vec3());
            aResult.push_back(transformableRect.bottom_left().to_vec3());
            aResult.push_back(transformableRect.top_right().to_vec3());
            aResult.push_back(transformableRect.bottom_right().to_vec3());
            aResult.push_back(transformableRect.bottom_left().to_vec3());
            break;
        case mesh_type::Outline:
            aResult.push_back(transformableRect.top_left().to_vec3());
            aResult.push_back(transformableRect.top_right().to_vec3());
            aResult.push_back(transformableRect.top_right().to_vec3());
            aResult.push_back(transformableRect.bottom_right().to_vec3());
            aResult.push_back(transformableRect.bottom_right().to_vec3());
            aResult.push_back(transformableRect.bottom_left().to_vec3());
            aResult.push_back(transformableRect.bottom_left().to_vec3());
            aResult.push_back(transformableRect.top_left().to_vec3());
            break;
        }
        auto const center = aRect.center().to_vec3().as<typename Vertex::value_type>();
        if (aTransformation)
            for (auto& v : aResult)
                v = *aTransformation * v + center;
    }

    template <typename Vertex, typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline neolib::static_vector<Vertex, 8> const& rect_vertices(const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aType, const optional_mat44f& aTransformation = {})
    {
        thread_local neolib::static_vector<Vertex, 8> result;
        calc_rect_vertices(result, aRect, aType, aTransformation);
        return result;
    };
    template <typename Vertex>
    std::vector<Vertex> arc_vertices(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const point& aOrigin, mesh_type aType, std::uint32_t aArcSegments = 0);
    template <typename Vertex>
    std::vector<Vertex> circle_vertices(const point& aCenter, dimension aRadius, angle aStartAngle, mesh_type aType, std::uint32_t aArcSegments = 0);
    template <typename Vertex>
    std::vector<Vertex> rounded_rect_vertices(const rect& aRect, dimension aRadius, mesh_type aType, std::uint32_t aArcSegments = 0);
}