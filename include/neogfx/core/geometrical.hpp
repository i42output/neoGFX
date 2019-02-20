// geometrical.hpp
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
#include <array>
#include <type_traits>
#include <ostream>
#include <optional>
#include "numerical.hpp"

namespace neogfx
{ 
    typedef scalar default_geometry_value_type;
    typedef default_geometry_value_type coordinate;
    typedef default_geometry_value_type dimension;
    typedef default_geometry_value_type coordinate_delta;

    typedef vec2 xy;
    typedef vec3 xyz;

    enum class logical_coordinate_system
    {
        Specified,
        AutomaticGui,
        AutomaticGame
    };

    template <logical_coordinate_system>
    struct is_gui 
    {
        static constexpr bool value = false;
    };
    template <>
    struct is_gui<logical_coordinate_system::AutomaticGui>
    {
        static constexpr bool value = true;
    };
    template <logical_coordinate_system>
    struct is_game
    {
        static constexpr bool value = false;
    };
    template <>
    struct is_game<logical_coordinate_system::AutomaticGame>
    {
        static constexpr bool value = true;
    };

    typedef std::optional<logical_coordinate_system> optional_logical_coordinate_system;

    struct logical_coordinates
    {
        vec2 bottomLeft;
        vec2 topRight;
        bool is_gui_orientation() const
        {
            return bottomLeft.y > topRight.y;
        }
        bool is_game_orientation() const
        {
            return !is_gui_orientation();
        }
        friend bool operator==(const logical_coordinates& lhs, const logical_coordinates& rhs)
        {
            return lhs.bottomLeft == rhs.bottomLeft && lhs.topRight == rhs.topRight;
        }
        friend bool operator!=(const logical_coordinates& lhs, const logical_coordinates& rhs)
        {
            return !(lhs == rhs);
        }
    };

    template <typename CoordinateType>
    class basic_delta 
    { 
        // types
    public:
        typedef CoordinateType coordinate_type;
        // construction
    public:
        constexpr basic_delta() : dx(0), dy(0) {}
        constexpr basic_delta(coordinate_type dx, coordinate_type dy) : dx(dx), dy(dy) {}
        constexpr basic_delta(coordinate_type aSquareDelta) : dx(aSquareDelta), dy(aSquareDelta) {}
        template <typename CoordinateType2>
        constexpr basic_delta(const basic_delta<CoordinateType2>& other) :
            dx(static_cast<CoordinateType>(other.dx)), dy(static_cast<CoordinateType>(other.dy)) {}
        // operations
    public:
        basic_vector<coordinate_type, 2> to_vec2() const { return basic_vector<coordinate_type, 2>{ dx, dy }; }
        basic_vector<coordinate_type, 3> to_vec3() const { return basic_vector<coordinate_type, 3>{ dx, dy, 0.0 }; }
        bool operator==(const basic_delta& other) const { return dx == other.dx && dy == other.dy; }
        bool operator!=(const basic_delta& other) const { return !operator==(other); }
        basic_delta& operator+=(const basic_delta& other) { dx += other.dx; dy += other.dy; return *this; }
        basic_delta& operator-=(const basic_delta& other) { dx -= other.dx; dy -= other.dy; return *this; }
        basic_delta& operator*=(const basic_delta& other) { dx *= other.dx; dy *= other.dy; return *this; }
        basic_delta& operator/=(const basic_delta& other) { dx /= other.dx; dy /= other.dy; return *this; }
        basic_delta operator-() const { return basic_delta(-dx, -dy); }
        basic_delta ceil() const { return basic_delta(std::ceil(dx), std::ceil(dy)); }
        basic_delta floor() const { return basic_delta(std::floor(dx), std::floor(dy)); }
        basic_delta min(const basic_delta& other) const { return basic_delta{ std::min(dx, other.dx), std::min(dy, other.dy) }; }
        basic_delta max(const basic_delta& other) const { return basic_delta{ std::max(dx, other.dx), std::max(dy, other.dy) }; }
        // attributes
    public:
        coordinate_type dx;
        coordinate_type dy;
    };

    typedef basic_delta<coordinate> delta;

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator+(const basic_delta<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_delta<CoordinateType> ret = left;
        ret += right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator-(const basic_delta<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_delta<CoordinateType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator*(const basic_delta<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_delta<CoordinateType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator/(const basic_delta<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_delta<CoordinateType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType>
    class basic_size 
    { 
        // types
    public:
        typedef CoordinateType coordinate_type;
        typedef CoordinateType dimension_type;
        typedef basic_delta<dimension_type> delta_type;
        // construction
    public:
        constexpr basic_size() : cx(0), cy(0) {}
        template <typename Scalar>
        constexpr basic_size(const basic_vector<Scalar, 2>& other) : cx(static_cast<dimension_type>(other.x)), cy(static_cast<dimension_type>(other.y)) {}
        template <typename Scalar>
        constexpr basic_size(const basic_vector<Scalar, 3>& other) : cx(static_cast<dimension_type>(other.x)), cy(static_cast<dimension_type>(other.y)) {}
        constexpr basic_size(dimension_type cx, dimension_type cy) : cx(cx), cy(cy) {}
        constexpr basic_size(dimension_type aSquareDimension) : cx(aSquareDimension), cy(aSquareDimension) {}
        template <typename CoordinateType2>
        constexpr basic_size(const basic_size<CoordinateType2>& other) :
            cx(static_cast<CoordinateType>(other.cx)), cy(static_cast<CoordinateType>(other.cy)) {}
        constexpr basic_size(const basic_delta<CoordinateType>& other) : cx(other.dx), cy(other.dy) {}
        // operations
    public:
        basic_vector<dimension_type, 2> to_vec2() const { return basic_vector<dimension_type, 2>{ cx, cy }; }
        basic_vector<dimension_type, 3> to_vec3() const { return basic_vector<dimension_type, 3>{ cx, cy, 0.0 }; }
        delta_type to_delta() const { return delta_type(cx, cy); }
        bool empty() const { return cx == 0 || cy == 0; }
        bool operator==(const basic_size& other) const { return cx == other.cx && cy == other.cy; }
        bool operator!=(const basic_size& other) const { return !operator==(other); }
        basic_size operator-() const { return basic_size{ -cx, -cy }; }
        basic_size& operator+=(const basic_size& other) { cx += other.cx; cy += other.cy; return *this; }
        basic_size& operator+=(const basic_delta<CoordinateType>& other) { cx += other.dx; cy += other.dy; return *this; }
        basic_size& operator+=(dimension_type amount) { cx += amount; cy += amount; return *this; }
        basic_size& operator-=(const basic_size& other) { cx -= other.cx; cy -= other.cy; return *this; }
        basic_size& operator-=(const basic_delta<CoordinateType>& other) { cx -= other.dx; cy -= other.dy; return *this; }
        basic_size& operator-=(dimension_type amount) { cx -= amount; cy -= amount; return *this; }
        basic_size& operator*=(const basic_size& other) { cx *= other.cx; cy *= other.cy; return *this; }
        basic_size& operator*=(dimension_type amount) { cx *= amount; cy *= amount; return *this; }
        basic_size& operator/=(const basic_size& other) { cx /= other.cx; cy /= other.cy; return *this; }
        basic_size& operator/=(dimension_type amount) { cx /= amount; cy /= amount; return *this; }
        basic_size ceil() const { return basic_size(std::ceil(cx), std::ceil(cy)); }
        basic_size floor() const { return basic_size(std::floor(cx), std::floor(cy)); }
        basic_size min(const basic_size& other) const { return basic_size{ std::min(cx, other.cx), std::min(cy, other.cy) }; }
        basic_size max(const basic_size& other) const { return basic_size{ std::max(cx, other.cx), std::max(cy, other.cy) }; }
        dimension_type magnitude() const { return std::sqrt(cx * cx + cy * cy); }
        // helpers
    public:
        static constexpr dimension_type max_dimension() { return std::numeric_limits<dimension_type>::max(); }
        static constexpr basic_size max_size() { return basic_size{ max_dimension(), max_dimension() }; }
        // attributes
    public:
        dimension_type cx;
        dimension_type cy;
    };

    typedef basic_size<coordinate> size;

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator+(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret += right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator-(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator*(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator/(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator+(const basic_size<CoordinateType>& left, typename basic_size<CoordinateType>::dimension_type right)
    {
        basic_size<CoordinateType> ret = left;
        ret += right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator-(const basic_size<CoordinateType>& left, typename basic_size<CoordinateType>::dimension_type right)
    {
        basic_size<CoordinateType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator*(const basic_size<CoordinateType>& left, typename basic_size<CoordinateType>::dimension_type right)
    {
        basic_size<CoordinateType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator/(const basic_size<CoordinateType>& left, typename basic_size<CoordinateType>::dimension_type right)
    {
        basic_size<CoordinateType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType>
    inline bool operator<(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        return std::tie(left.cx, left.cy) < std::tie(right.cx, right.cy);
    }

    template <typename CoordinateType>
    inline bool operator<=(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        return std::tie(left.cx, left.cy) <= std::tie(right.cx, right.cy);
    }

    template <typename CoordinateType>
    inline bool operator>(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        return std::tie(left.cx, left.cy) > std::tie(right.cx, right.cy);
    }

    template <typename CoordinateType>
    inline bool operator>=(const basic_size<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        return std::tie(left.cx, left.cy) >= std::tie(right.cx, right.cy);
    }

    template <typename CoordinateType>
    class basic_point
    {
        // types
    public:
        typedef CoordinateType coordinate_type;
        // construction
    public:
        constexpr basic_point() : x(0), y(0) {}
        template <typename Scalar>
        constexpr basic_point(const basic_vector<Scalar, 2>& other) : x(static_cast<coordinate_type>(other.x)), y(static_cast<coordinate_type>(other.y)) {}
        template <typename Scalar>
        constexpr basic_point(const basic_vector<Scalar, 3>& other) : x(static_cast<coordinate_type>(other.x)), y(static_cast<coordinate_type>(other.y)) {}
        constexpr basic_point(CoordinateType x, CoordinateType y) : x(x), y(y) {}
        template <typename CoordinateType2>
        constexpr basic_point(const basic_point<CoordinateType2>& other) :
            x(static_cast<CoordinateType>(other.x)), y(static_cast<CoordinateType>(other.y)) {}
        constexpr basic_point(const basic_delta<CoordinateType>& other) : x(other.dx), y(other.dy) {}
        constexpr basic_point(const basic_size<CoordinateType>& other) : x(other.cx), y(other.cy) {}
        // operations
    public:
        basic_vector<coordinate_type, 2> to_vec2() const { return basic_vector<coordinate_type, 2>{ x, y }; }
        basic_vector<coordinate_type, 3> to_vec3(coordinate_type z = 0.0) const { return basic_vector<coordinate_type, 3>{ x, y, z }; }
        bool operator==(const basic_point& other) const { return x == other.x && y == other.y; }
        bool operator!=(const basic_point& other) const { return !operator==(other); }
        basic_point& operator+=(const basic_point& other) { x += other.x; y += other.y; return *this; }
        basic_point& operator-=(const basic_point& other) { x -= other.x; y -= other.y; return *this; }
        basic_point& operator*=(const basic_point& other) { x *= other.x; y *= other.y; return *this; }
        basic_point& operator/=(const basic_point& other) { x /= other.x; y /= other.y; return *this; }
        basic_point& operator+=(coordinate_type amount) { x += amount; y += amount; return *this; }
        basic_point& operator-=(coordinate_type amount) { x -= amount; y -= amount; return *this; }
        basic_point& operator*=(coordinate_type amount) { x *= amount; y *= amount; return *this; }
        basic_point& operator/=(coordinate_type amount) { x /= amount; y /= amount; return *this; }
        basic_point& operator+=(const basic_delta<coordinate_type>& other) { x += static_cast<coordinate_type>(other.dx); y += static_cast<coordinate_type>(other.dy); return *this; }
        basic_point& operator-=(const basic_delta<coordinate_type>& other) { x -= static_cast<coordinate_type>(other.dx); y -= static_cast<coordinate_type>(other.dy); return *this; }
        basic_point& operator+=(const basic_size<coordinate_type>& other) { x += static_cast<coordinate_type>(other.cx); y += static_cast<coordinate_type>(other.cy); return *this; }
        basic_point& operator-=(const basic_size<coordinate_type>& other) { x -= static_cast<coordinate_type>(other.cx); y -= static_cast<coordinate_type>(other.cy); return *this; }
        basic_point operator-() const { return basic_point(-x, -y); }
        basic_point ceil() const { return basic_point(std::ceil(x), std::ceil(y)); }
        basic_point floor() const { return basic_point(std::floor(x), std::floor(y)); }
        basic_point min(const basic_point& other) const { return basic_point{ std::min(x, other.x), std::min(y, other.y) }; }
        basic_point max(const basic_point& other) const { return basic_point{ std::max(x, other.x), std::max(y, other.y) }; }
        // attributes
    public:
        coordinate_type x;
        coordinate_type y;
    };

    typedef basic_point<coordinate> point;

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator+(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret += right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator-(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret -= right;
        return basic_delta<CoordinateType>(ret.x, ret.y);
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator*(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator/(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator+(const basic_point<CoordinateType>& left, typename basic_point<CoordinateType>::coordinate_type amount)
    {
        basic_point<CoordinateType> ret = left;
        ret += amount;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator-(const basic_point<CoordinateType>& left, typename basic_point<CoordinateType>::coordinate_type amount)
    {
        basic_point<CoordinateType> ret = left;
        ret -= amount;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator*(const basic_point<CoordinateType>& left, typename basic_point<CoordinateType>::coordinate_type amount)
    {
        basic_point<CoordinateType> ret = left;
        ret *= amount;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator/(const basic_point<CoordinateType>& left, typename basic_point<CoordinateType>::coordinate_type amount)
    {
        basic_point<CoordinateType> ret = left;
        ret /= amount;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator+(const basic_point<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret += right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator-(const basic_point<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret -= right;
        return basic_delta<CoordinateType>(ret.x, ret.y);
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator*(const basic_point<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator/(const basic_point<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret /= right;
        return basic_delta<CoordinateType>(ret.x, ret.y);
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator+(const basic_point<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret += right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator-(const basic_point<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator*(const basic_point<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_point<CoordinateType> operator/(const basic_point<CoordinateType>& left, const basic_size<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator+(const basic_size<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret += basic_size<CoordinateType>{ right.x, right.y };
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator-(const basic_size<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret -= basic_size<CoordinateType>{ right.x, right.y };
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator*(const basic_size<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret *= basic_size<CoordinateType>{ right.x, right.y };
        return ret;
    }

    template <typename CoordinateType>
    inline basic_size<CoordinateType> operator/(const basic_size<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_size<CoordinateType> ret = left;
        ret /= basic_size<CoordinateType>{ right.x, right.y };
        return ret;
    }

    template <typename CoordinateType>
    inline bool operator<(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        return std::tie(left.x, left.y) < std::tie(right.x, right.y);
    }

    template <typename CoordinateType>
    inline bool operator==(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        return left.x == right.x && left.y == right.y;
    }

    template <typename CoordinateType>
    inline bool operator<=(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        return left < right || left == right;
    }

    template <typename CoordinateType>
    inline bool operator>(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        return right < left;
    }

    template <typename CoordinateType>
    inline bool operator>=(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        return right < left || left == right;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem = logical_coordinate_system::AutomaticGui>
    class basic_rect :
        public basic_point<CoordinateType>,
        public basic_size<CoordinateType>
    {
        // types
    public:
        typedef CoordinateType coordinate_type;
        typedef coordinate_type dimension_type;
    private:
        typedef basic_delta<CoordinateType> delta_type;
        typedef basic_size<dimension_type> size_type;
        typedef basic_point<CoordinateType> point_type;
    public:
        using point_type::x;
        using point_type::y;
        using size_type::cx;
        using size_type::cy;
        // constants
    public:
        static constexpr bool gui = is_gui<CoordinateSystem>::value;
        static constexpr bool game = is_game<CoordinateSystem>::value;
        // construction
    public:
        basic_rect() {}
        basic_rect(const point_type& coordinates, const size_type& dimensions) : point_type{ coordinates }, size_type{ dimensions } {}
        basic_rect(const point_type& leftCorner, const point_type& rightCorner) : point_type{ leftCorner }, size_type{ std::abs<CoordinateType>(rightCorner.x - leftCorner.x), std::abs<CoordinateType>(rightCorner.y - leftCorner.y) } {}
        basic_rect(const size_type& dimensions) : point_type{}, size_type{ dimensions } {}
        basic_rect(coordinate_type x0, coordinate_type y0, coordinate_type x1, coordinate_type y1) : point_type{ x0, y0 }, size_type{ x1 - x0, y1 - y0 } {}
    public:
        template <typename CoordinateType2, logical_coordinate_system CoordinateSystem2>
        basic_rect(const basic_rect<CoordinateType2, CoordinateSystem2>& other) : point_type{ other }, size_type{ other } {}
        // assignment
    public:
        template <typename CoordinateType2, logical_coordinate_system CoordinateSystem2>
        basic_rect& operator=(const basic_rect<CoordinateType2, CoordinateSystem2>& other) { static_cast<point_type&>(*this) = other; static_cast<size_type&>(*this) = other;  return *this; }
        basic_rect& operator=(const point_type& coordinates) { static_cast<point_type&>(*this) = coordinates; return *this; }
        basic_rect& operator=(const size_type& dimensions) { static_cast<size_type&>(*this) = dimensions; return *this; }
        // operations
    public:
        basic_vector<basic_vector<coordinate_type, 2>, 4> to_vector() const 
        { 
            if constexpr (gui)
                return basic_vector<basic_vector<coordinate_type, 2>, 4>(top_left().to_vector(), top_right().to_vector(), bottom_right().to_vector(), bottom_left().to_vector()); 
            else
                return basic_vector<basic_vector<coordinate_type, 2>, 4>(botttom_left().to_vector(), bottom_right().to_vector(), top_right().to_vector(), top_left().to_vector());
        }
        const point_type& position() const { return *this; }
        point_type& position() { return *this; }
        const size_type& extents() const { return *this; }
        size_type& extents() { return *this; }
        coordinate_type left() const { return x; }
        coordinate_type top() const { if constexpr (gui) return y; else return y + cy; }
        coordinate_type right() const { return x + cx; }
        coordinate_type bottom() const { if constexpr (gui) return y + cy; else return y; }
        point_type top_left() const { return point_type(left(), top()); }
        point_type top_right() const { return point_type(right(), top()); }
        point_type bottom_left() const { return point_type(left(), bottom()); }
        point_type bottom_right() const { return point_type(right(), bottom()); }
        dimension_type width() const { return cx; }
        dimension_type height() const { return cy; }
        bool operator==(const basic_rect& other) const { return x == other.x && y == other.y && cx == other.cx && cy == other.cy; }
        bool operator!=(const basic_rect& other) const { return !operator==(other); }
        basic_rect& operator*=(const basic_rect& other) { position() *= other.position(); extents() *= other.extents(); return *this; }
        basic_rect& operator*=(const size_type& size) { position() *= size; extents() *= size; return *this; }
        basic_rect& operator*=(dimension_type value) { position() *= value; extents() *= value; return *this; }
        basic_rect& operator/=(const basic_rect& other) { position() /= other.position(); extents() /= other.extents(); return *this; }
        basic_rect& operator/=(const size_type& size) { position() /= size; extents() /= size; return *this; }
        basic_rect& operator/=(dimension_type value) { position() /= value; extents() /= value; return *this; }
        bool contains_x(const point_type& point) const { return point.x >= left() && point.x < right(); }
        bool contains_y(const point_type& point) const { if constexpr (gui) return point.y >= top() && point.y < bottom(); else return point.y >= bottom() && point.y < top(); }
        bool contains(const point_type& point) const { return contains_x(point) && contains_y(point); }
        bool contains(const basic_rect& other) const 
        { 
            if constexpr (gui) 
                return other.left() >= left() && other.right() <= right() && other.top() >= top() && other.bottom() <= bottom(); 
            else
                return other.left() >= left() && other.right() <= right() && other.bottom() >= bottom() && other.top() <= top();
        }
        point_type centre() const 
        { 
            if constexpr (gui)
                return point_type{ left() + static_cast<CoordinateType>(width() / 2), top() + static_cast<CoordinateType>(height() / 2) };
            else
                return point_type{ left() + static_cast<CoordinateType>(width() / 2), bottom() + static_cast<CoordinateType>(height() / 2) };
        }
        basic_rect& move(const point_type& aOffset) { x += aOffset.x; y += aOffset.y; return *this; }
        basic_rect& inflate(const delta_type& delta) { x -= delta.dx; y -= delta.dy; cx += delta.dx * static_cast<CoordinateType>(2); cy += delta.dy * static_cast<CoordinateType>(2); return *this; }
        basic_rect& inflate(const size_type& size) { return inflate(delta_type(size.cx, size.cy)); }
        basic_rect& inflate(CoordinateType dx, CoordinateType dy) { return inflate(delta_type(dx, dy)); }
        basic_rect& deflate(const delta_type& delta) { return inflate(-delta); }
        basic_rect& deflate(const size_type& size) { return inflate(-size.cx, -size.cy); }
        basic_rect& deflate(CoordinateType dx, CoordinateType dy) { return inflate(-dx, -dy); }
        basic_rect intersection(const basic_rect& other) const
        {
            if constexpr (gui)
            {
                basic_rect candidate{ top_left().max(other.top_left()), bottom_right().min(other.bottom_right()) };
                if (contains(candidate.centre()) && other.contains(candidate.centre()))
                    return candidate;
                else
                    return basic_rect{};
            }
            else
            {
                basic_rect candidate{ bottom_left().max(other.bottom_left()), top_right().min(other.top_right()) };
                if (contains(candidate.centre()) && other.contains(candidate.centre()))
                    return candidate;
                else
                    return basic_rect{};
            }
        }
        basic_rect combine(const basic_rect& other) const
        {
            if constexpr (gui)
                return basic_rect{ top_left().min(other.top_left()), bottom_right().max(other.bottom_right()) };
            else
                return basic_rect{ bottom_left().min(other.bottom_left()), top_right().max(other.top_right()) };
        }
        basic_rect with_centred_origin() const
        {
            return basic_rect{ point_type{ -extents() / 2.0 }, extents() };
        }
        basic_rect ceil() const { return basic_rect(point_type::ceil(), size_type::ceil()); }
        basic_rect floor() const { return basic_rect(point_type::floor(), size_type::floor()); }
        aabb_2d to_aabb_2d() const 
        { 
            if constexpr (gui)
                return aabb_2d{ top_left().to_vec2(), bottom_right().to_vec2() };
            else
                return aabb_2d{ bottom_left().to_vec2(), top_right().to_vec2() };
        }
    };

    typedef basic_rect<coordinate, logical_coordinate_system::AutomaticGui> gui_rect;
    typedef basic_rect<coordinate, logical_coordinate_system::AutomaticGame> game_rect;
    typedef gui_rect rect;

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator*(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_rect<CoordinateType, CoordinateSystem>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator*(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_size<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret *= right;
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator*(const basic_rect<CoordinateType, CoordinateSystem>& left, typename basic_rect<CoordinateType, CoordinateSystem>::dimension_type value)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret *= value;
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator/(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_rect<CoordinateType, CoordinateSystem>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator/(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_size<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator/(const basic_rect<CoordinateType, CoordinateSystem>& left, typename basic_rect<CoordinateType, CoordinateSystem>::dimension_type value)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret /= value;
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator+(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_point<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_point::operator+=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator+(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_size<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size::operator+=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator+(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_delta<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size::operator+=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator-(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_point<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_point::operator-=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator-(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_size<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size::operator-=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator-(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_delta<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size::operator-=(right);
        return ret;
    }

    template <typename CoordinateType>
    class basic_line
    {
        // types
    private:
        typedef CoordinateType coordinate_type;
        typedef basic_delta<coordinate_type> delta_type;
        typedef basic_point<coordinate_type> point_type;
        typedef basic_line<coordinate_type> line;
        // construction
    public:
        basic_line(const point_type& a, const point_type& b) : a(a), b(b), d(b - a), m(d.dx != 0 ? d.dy / d.dx : 0), c(a.y - m * a.x) {}
        template <typename CoordinateType2>
        basic_line(const basic_line<CoordinateType2>& other) :
            a(static_cast<point_type>(other.a)), b(static_cast<point_type>(other.b)), d(b - a), m(d.dx != 0 ? d.dy / d.dx : 0), c(a.y - m * a.x) {}
        // operations
    public:
        bool operator==(const basic_line& other) const { return (a == other.a && b == other.b) || (a == other.b & b == other.a); }
        bool operator!=(const basic_line& other) const { return !operator==(other); }
        basic_line from(const point_type& a) const 
        {
            if (a == this->a)
                return *this;
            else
                return basic_line(this->b, this->a);
        }
        coordinate_type delta_x() const { return d.dx; }
        coordinate_type delta_y() const { return d.dy; }
        bool is_vertex(const point_type& v) const { return v == a || v == b; }
        bool within_bounding_rect(const point_type& p) const
        {
            return p.x >= std::min(a.x, b.x) && p.x <= std::max(a.x, b.x) &&
                p.y >= std::min(a.y, b.y) && p.y <= std::max(a.y, b.y);
        }
        bool intersection(const line& other, point& pointOfIntersection) const
        {
            if (d.dx == 0 || other.d.dx == 0)
            {
                if (d.dx == other.d.dx)
                    return false;
                else if (d.dx == 0)
                {
                    pointOfIntersection.x = a.x;
                    pointOfIntersection.y = other.m * a.x + other.c;
                }
                else // (other.d.dx == 0)
                {
                    pointOfIntersection.x = other.a.x;
                    pointOfIntersection.y = m * other.a.x + c;
                }
            }
            else if (d.dy == 0 || other.d.dy == 0)
            {
                if (d.dy == other.d.dy)
                    return false;
                else if (d.dy == 0)
                {
                    pointOfIntersection.y = a.y;
                    pointOfIntersection.x = (a.y - other.c) / other.m;
                }
                else // (other.d.dy == 0)
                {
                    pointOfIntersection.y = other.a.y;
                    pointOfIntersection.x = (other.a.y - c) / m;
                }
            }
            else
            {
                if (m == other.m)
                    return false;
                pointOfIntersection.x = (other.c - c) / (m - other.m);
                pointOfIntersection.y = pointOfIntersection.x * m + c;
            }
            return within_bounding_rect(pointOfIntersection) && other.within_bounding_rect(pointOfIntersection);
        }
        // attributes
    public:
        point_type a;
        point_type b;
    private:
        delta_type d;
        coordinate_type m;
        coordinate_type c;
    };

    typedef basic_line<coordinate> line;

    template <typename DimensionType>
    class basic_margins
    {
        // types
    public:
        typedef DimensionType dimension_type;
        typedef dimension_type coordinate_type;
        typedef basic_point<coordinate_type> point_type;
        typedef basic_size<dimension_type> size_type;
        // construction
    public:
        basic_margins() : left{}, top{}, right{}, bottom{} {}
        basic_margins(dimension_type all) : left(all), top(all), right(all), bottom(all) {}
        basic_margins(dimension_type left, dimension_type top, dimension_type right, dimension_type bottom) : left(left), top(top), right(right), bottom(bottom) {}
        basic_margins(dimension_type leftRight, dimension_type topBottom) : left(leftRight), top(topBottom), right(leftRight), bottom(topBottom) {}
        template <typename DimensionType2>
        basic_margins(const basic_margins<DimensionType2>& other) :
            left(static_cast<dimension_type>(other.left)), top(static_cast<dimension_type>(other.top)), right(static_cast<dimension_type>(other.right)), bottom(static_cast<dimension_type>(other.bottom)) {}
        // operations
    public:
        bool operator==(const basic_margins& other) const { return left == other.left && top == other.top && right == other.right && bottom == other.bottom; }
        bool operator!=(const basic_margins& other) const { return !operator == (other); }
        basic_margins operator-() const { return basic_margins{ -left, -top, -right, -bottom }; }
        basic_margins& operator+=(const basic_margins& other) { left += other.left; top += other.top; right += other.right; bottom += other.bottom; return *this; }
        basic_margins& operator+=(dimension_type amount) { left += amount; top += amount; right += amount; bottom += amount; return *this; }
        basic_margins& operator-=(const basic_margins& other) { left -= other.left; top -= other.top; right -= other.right; bottom -= other.bottom; return *this; }
        basic_margins& operator-=(dimension_type amount) { left -= amount; top -= amount; right -= amount; bottom -= amount; return *this; }
        basic_margins& operator*=(const basic_margins& other) { left *= other.left; top *= other.top; right *= other.right; bottom *= other.bottom; return *this; }
        basic_margins& operator*=(dimension_type amount) { left *= amount; top *= amount; right *= amount; bottom *= amount; return *this; }
        basic_margins& operator/=(const basic_margins& other) { left /= other.left; top /= other.top; right /= other.right; bottom /= other.bottom; return *this; }
        basic_margins& operator/=(dimension_type amount) { left /= amount; top /= amount; right /= amount; bottom /= amount; return *this; }
    public:
        point_type top_left() const { return point_type{ left, top }; }
        size_type size() const { return size_type{ left + right, top + bottom }; }
        // attributes
    public:
        dimension_type left;
        dimension_type top;
        dimension_type right;
        dimension_type bottom;
    };

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator+(const basic_margins<DimensionType>& left, const basic_margins<DimensionType>& right)
    {
        basic_margins<DimensionType> ret = left;
        ret += right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator-(const basic_margins<DimensionType>& left, const basic_margins<DimensionType>& right)
    {
        basic_margins<DimensionType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator*(const basic_margins<DimensionType>& left, const basic_margins<DimensionType>& right)
    {
        basic_margins<DimensionType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator/(const basic_margins<DimensionType>& left, const basic_margins<DimensionType>& right)
    {
        basic_margins<DimensionType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator+(const basic_margins<DimensionType>& left, typename basic_margins<DimensionType>::dimension_type right)
    {
        basic_margins<DimensionType> ret = left;
        ret += right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator-(const basic_margins<DimensionType>& left, typename basic_margins<DimensionType>::dimension_type right)
    {
        basic_margins<DimensionType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator*(const basic_margins<DimensionType>& left, typename basic_margins<DimensionType>::dimension_type right)
    {
        basic_margins<DimensionType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_margins<DimensionType> operator/(const basic_margins<DimensionType>& left, typename basic_margins<DimensionType>::dimension_type right)
    {
        basic_margins<DimensionType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename DimensionType>
    inline bool operator<(const basic_margins<DimensionType>& left, const basic_margins<DimensionType>& right)
    {
        return std::tie(left.left, left.top, left.right, left.bottom) < std::tie(right.left, right.top, right.right, right.bottom);
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator-(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_margins<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_point::operator+=(right.top_left());
        ret.basic_size::operator-=(right.size());
        return ret;
    }

    typedef basic_size<uint32_t> size_u32;
    typedef basic_delta<uint32_t> delta_u32;
    typedef basic_point<uint32_t> point_u32;
    typedef basic_rect<uint32_t> rect_u32;

    typedef basic_margins<dimension> margins;

    typedef std::optional<dimension> optional_dimension;
    typedef std::optional<angle> optional_angle;
    typedef std::optional<delta> optional_delta;
    typedef std::optional<point> optional_point;
    typedef std::optional<size> optional_size;
    typedef std::optional<rect> optional_rect;
    typedef std::optional<delta_u32> optional_delta_u32;
    typedef std::optional<point_u32> optional_point_u32;
    typedef std::optional<size_u32> optional_size_u32;
    typedef std::optional<rect_u32> optional_rect_u32;
    typedef std::optional<margins> optional_margins;
    typedef std::optional<vector1> optional_vector1;
    typedef std::optional<vector2> optional_vector2;
    typedef std::optional<vector3> optional_vector3;
    typedef std::optional<vector4> optional_vector4;

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_point<T>& aPoint)
    {
        aStream << "(" << aPoint.x << ", " << aPoint.y << ")";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_size<T>& aSize)
    {
        aStream << "{" << aSize.cx << ", " << aSize.cy << "}";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_rect<T>& aRect)
    {
        aStream << "[" << aRect.top_left() << " -> " << aRect.bottom_right() << ", " << aRect.extents() << "]";
        return aStream;
    }
}

namespace std 
{
    template <> struct hash<neogfx::rect>
    {
        size_t operator()(const neogfx::rect& aRect) const
        {
            return std::hash<neogfx::rect::coordinate_type>()(aRect.x) ^ 
                std::hash<neogfx::rect::coordinate_type>()(aRect.y) ^
                std::hash<neogfx::rect::coordinate_type>()(aRect.cx) ^
                std::hash<neogfx::rect::coordinate_type>()(aRect.cy);
        }
    };
}
