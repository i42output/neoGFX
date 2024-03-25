// path.hpp
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

#include <neolib/core/vecarray.hpp>

namespace neogfx
{
    enum class path_shape : uint32_t
    {
        Vertices,
        Quads,
        Lines,
        LineLoop,
        LineStrip,
        ConvexPolygon
    };

    template <typename PointType>
    class basic_path
    {
        // types
    public:
        typedef PointType point_type;
        typedef typename point_type::coordinate_type coordinate_type;
        typedef typename point_type::coordinate_type coordinate_delta_type;
        typedef typename point_type::coordinate_type dimension_type;
        typedef basic_size<coordinate_type> size_type;
        typedef basic_delta<coordinate_type> delta_type;
        typedef basic_rect<coordinate_type> mesh_type;
        typedef basic_line<coordinate_type> line_type;
        typedef neolib::vecarray<point_type, 16, -1> sub_path_type;
        typedef neolib::vecarray<sub_path_type, 1, -1> sub_paths_type;
        typedef typename sub_paths_type::size_type sub_paths_size_type;
        struct clip_rect_list : std::vector<mesh_type>
        {
            bool contains(const point_type& aPoint) const
            {
                for (auto const& m : *this)
                    if (m.contains(aPoint))
                        return true;
                return false;
            }
        };
    public:
        struct missing_move_to : std::logic_error { missing_move_to() : std::logic_error("neogfx::basic_path::missing_move_to") {} };
    private:
        struct intersect
        {
            // construction
        public:
            intersect() : iX(), iSkip() {}
            intersect(coordinate_type aX, bool aSkip = false) : iX(aX), iSkip(aSkip) {}
            // operations
        public:
            coordinate_type x() const { return iX; }
            bool skip() const { return iSkip; }
            bool operator==(const intersect& aOther) const { return iX == aOther.iX; }
            bool operator<(const intersect& aOther) const { return iX < aOther.iX; }
            // attributes
        private:
            coordinate_type iX;
            bool iSkip;
        };
        typedef std::vector<intersect> intersect_list;
        // construction
    public:
        basic_path(path_shape aShape = path_shape::ConvexPolygon, sub_paths_size_type aPathCountHint = 0) : iShape(aShape)
        {
            iSubPaths.reserve(aPathCountHint);
        }
        basic_path(const mesh_type& aRect, path_shape aShape = path_shape::ConvexPolygon) : iShape(aShape)
        {
            move_to(aRect.top_left());
            line_to(aRect.top_right());
            line_to(aRect.bottom_right());
            line_to(aRect.bottom_left());
            line_to(aRect.top_left());
        }
        // operations
    public:
        path_shape shape() const 
        {        
            return iShape; 
        }
        void set_shape(path_shape aShape) 
        { 
            iShape = aShape; 
        }
        point_type position() const 
        { 
            return iPosition; 
        }
        void set_position(point_type aPosition) 
        { 
            iPosition = aPosition; 
            iBoundingRect = invalid;
        }
        const sub_paths_type& sub_paths() const 
        { 
            return iSubPaths; 
        }
        sub_paths_type& sub_paths()
        { 
            return iSubPaths; 
        }
        vertices to_vertices(const typename sub_paths_type::value_type& aPath) const
        {
            vertices result;
            result.reserve((aPath.size() + 1) * (iShape == path_shape::Quads ? 6 : 1));
            if (aPath.size() >= 2)
            {
                if (iShape == path_shape::ConvexPolygon)
                {
                    auto const& boundingRect = bounding_rect(false);
                    auto const& center = boundingRect.center();
                    result.push_back(xyz{ center.x + position().x, center.y + position().y });
                }
                for (auto vi = aPath.begin(); vi != aPath.end(); ++vi)
                {
                    switch (iShape)
                    {
                    case path_shape::Quads:
                        if (vi + 1 != aPath.end())
                        {
                            result.push_back(xyz{ vi->x + position().x, vi->y + position().y });
                            result.push_back(xyz{ (vi + 1)->x + position().x, (vi + 1)->y + position().y });
                            result.push_back(xyz{ vi->x + position().x, vi->y + position().y });
                            result.push_back(xyz{ (vi + 1)->x + position().x, (vi + 1)->y + position().y });
                        }
                        break;
                    case path_shape::ConvexPolygon:
                    default:
                        result.push_back(xyz{ vi->x + position().x, vi->y + position().y });
                        break;
                    }
                }
                if (iShape == path_shape::LineLoop && aPath[0] == aPath[aPath.size() - 1])
                {
                    result.pop_back();
                }
                else if (iShape == path_shape::ConvexPolygon && aPath[0] != aPath[aPath.size() - 1])
                {
                    result.push_back(xyz{ aPath[0].x, aPath[0].y });
                }
            }
            return result;
        }
        void move_to(const point_type& aPoint, sub_paths_size_type aLineCountHint = 0)
        {
            iPointFrom = aPoint;
            iLineCountHint = aLineCountHint;
        }
        void move_to(coordinate_type aX, coordinate_type aY, sub_paths_size_type aLineCountHint = 0)
        {
            move_to(point_type(aX, aY), aLineCountHint);
        }
        void line_to(const point_type& aPoint)
        {
            if (iPointFrom)
            {
                iSubPaths.push_back(sub_path_type{});
                if (iLineCountHint != 0)
                {
                    iSubPaths.back().reserve(iLineCountHint + 1);
                    iLineCountHint = 0;
                }
                iSubPaths.back().push_back(*iPointFrom);
                iPointFrom = std::nullopt;
            }
            else
            {
                if (iSubPaths.empty())
                    throw missing_move_to();
            }
            if (iSubPaths.back().empty() || iSubPaths.back().back() != aPoint)
                iSubPaths.back().push_back(aPoint);
            iBoundingRect = invalid;
        }
        void line_to(coordinate_type aX, coordinate_type aY)
        {
            line_to(point_type{ aX, aY });
        }
        void add_rect(const mesh_type& aRectangle);
        void inflate(const delta_type& aDelta)
        {
            auto const boundingRect = bounding_rect(false);
            auto const center = boundingRect.center();
            for (auto& segment : iSubPaths)
                for (auto& point : segment)
                {
                    if (point.x < center.x)
                        point.x -= aDelta.dx;
                    else
                        point.x += aDelta.dx;
                    if (point.y < center.y)
                        point.y -= aDelta.dy;
                    else
                        point.y += aDelta.dy;
                }
            iBoundingRect = invalid;
        }
        void inflate(coordinate_delta_type aDeltaX, coordinate_delta_type aDeltaY)
        {
            inflate(delta_type(aDeltaX, aDeltaY));
        }
        void deflate(const delta_type& aDeltas)
        {
            inflate(-aDeltas);
        }
        void deflate(coordinate_delta_type aDeltaX, coordinate_delta_type aDeltaY)
        {
            inflate(delta_type(-aDeltaX, -aDeltaY));
        }
        mesh_type bounding_rect(bool aOffsetPosition = true, size_type aPixelWidthAdjustment = size_type{}) const;
        clip_rect_list clip_rects(const point& aOrigin) const;
        // attributes
    private:
        path_shape iShape;
        point_type iPosition;
        std::optional<point_type> iPointFrom;
        sub_paths_type iSubPaths;
        sub_paths_size_type iLineCountHint;
        mutable cache<std::tuple<bool, size_type, mesh_type>> iBoundingRect;
    };
}

#include "path.inl"

namespace neogfx
{
    typedef basic_path<point> path;
    typedef std::optional<path> optional_path;
}