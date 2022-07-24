// geometrical.hpp
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
#include <array>
#include <type_traits>
#include <ostream>
#include <optional>
#include <neolib/app/i_setting_value.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/alignment.hpp>

namespace neogfx
{ 
    typedef scalar default_geometry_value_type;
    typedef default_geometry_value_type coordinate;
    typedef default_geometry_value_type dimension;
    typedef default_geometry_value_type coordinate_delta;

    typedef int32_t coordinate_i32;
    typedef int32_t dimension_i32;
    typedef int32_t coordinate_delta_i32;

    typedef uint32_t coordinate_u32;
    typedef uint32_t dimension_u32;
    typedef uint32_t coordinate_delta_u32;

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

    typedef optional<logical_coordinate_system> optional_logical_coordinate_system;

    struct logical_coordinates
    {
        typedef logical_coordinates abstract_type; // todo

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

    typedef neolib::optional<logical_coordinates> optional_logical_coordinates;

    template <typename CoordinateType>
    class basic_delta 
    { 
        typedef basic_delta<CoordinateType> self_type;
        // types
    public:
        typedef self_type abstract_type; // todo: abstract type
        typedef CoordinateType coordinate_type;
        // construction
    public:
        constexpr basic_delta() : dx{}, dy{} {}
        constexpr basic_delta(coordinate_type dx, coordinate_type dy) : dx{ dx }, dy{ dy } {}
        constexpr explicit basic_delta(coordinate_type aSquareDelta) : dx{ aSquareDelta }, dy{ aSquareDelta } {}
        template <typename CoordinateType2>
        constexpr basic_delta(const basic_delta<CoordinateType2>& other) :
            dx{ static_cast<CoordinateType>(other.dx) }, dy{ static_cast<CoordinateType>(other.dy) } {}
        // operations
    public:
        basic_vector<coordinate_type, 2> to_vec2() const { return basic_vector<coordinate_type, 2>{ dx, dy }; }
        basic_vector<coordinate_type, 3> to_vec3() const { return basic_vector<coordinate_type, 3>{ dx, dy, 0.0 }; }
        auto operator<=>(const basic_delta&) const = default;
        basic_delta& operator+=(coordinate_type s) { dx +=s; dy += s; return *this; }
        basic_delta& operator-=(coordinate_type s) { dx -=s; dy -= s; return *this; }
        basic_delta& operator*=(coordinate_type s) { dx *=s; dy *= s; return *this; }
        basic_delta& operator/=(coordinate_type s) { dx /=s; dy /= s; return *this; }
        basic_delta& operator+=(const basic_delta& other) { dx += other.dx; dy += other.dy; return *this; }
        basic_delta& operator-=(const basic_delta& other) { dx -= other.dx; dy -= other.dy; return *this; }
        basic_delta& operator*=(const basic_delta& other) { dx *= other.dx; dy *= other.dy; return *this; }
        basic_delta& operator/=(const basic_delta& other) { dx /= other.dx; dy /= other.dy; return *this; }
        basic_delta operator-() const { return basic_delta{ -dx, -dy }; }
        basic_delta abs() const { return basic_delta{ std::abs(dx), std::abs(dy) }; }
        basic_delta ceil() const { return basic_delta{ std::ceil(dx), std::ceil(dy) }; }
        basic_delta floor() const { return basic_delta{ std::floor(dx), std::floor(dy) }; }
        basic_delta round() const { return basic_delta{ std::round(dx), std::round(dy) }; }
        basic_delta min(const basic_delta& other) const { return basic_delta{ std::min(dx, other.dx), std::min(dy, other.dy) }; }
        basic_delta max(const basic_delta& other) const { return basic_delta{ std::max(dx, other.dx), std::max(dy, other.dy) }; }
        basic_delta with_dx(coordinate_type dx) const { return basic_delta{ dx, dy }; }
        basic_delta with_dy(coordinate_type dy) const { return basic_delta{ dx, dy }; }
        coordinate_type magnitude() const { return std::sqrt(dx * dx + dy * dy); }
    public:
        template <typename T>
        basic_delta<T> as() const
        {
            return *this;
        }
        // attributes
    public:
        coordinate_type dx;
        coordinate_type dy;
    };

    typedef basic_delta<coordinate> delta;

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator+(const basic_delta<CoordinateType>& left, CoordinateType s)
    {
        basic_delta<CoordinateType> ret = left;
        ret += s;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator-(const basic_delta<CoordinateType>& left, CoordinateType s)
    {
        basic_delta<CoordinateType> ret = left;
        ret -= s;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator*(const basic_delta<CoordinateType>& left, CoordinateType s)
    {
        basic_delta<CoordinateType> ret = left;
        ret *= s;
        return ret;
    }

    template <typename CoordinateType>
    inline basic_delta<CoordinateType> operator/(const basic_delta<CoordinateType>& left, CoordinateType s)
    {
        basic_delta<CoordinateType> ret = left;
        ret /= s;
        return ret;
    }

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
    inline bool nearly_equal(basic_delta<CoordinateType> const& lhs, basic_delta<CoordinateType> const& rhs, scalar epsilon = 0.00001)
    {
        return nearly_equal(lhs.dx, rhs.dx, epsilon) && nearly_equal(lhs.dy, rhs.dy, epsilon);
    }

    struct bad_size : std::logic_error { bad_size() : std::logic_error{ "neogfx::bad_size" } {} };

    template <typename CoordinateType>
    class basic_size 
    { 
        typedef basic_size<CoordinateType> self_type;
        // types
    public:
        typedef self_type abstract_type; // todo: abstract type
        typedef CoordinateType coordinate_type;
        typedef CoordinateType dimension_type;
        typedef basic_delta<dimension_type> delta_type;
        // construction
    public:
        constexpr basic_size() : cx{}, cy{} {}
        template <typename Scalar>
        constexpr basic_size(const basic_vector<Scalar, 2>& other) : cx{ static_cast<dimension_type>(other.x) }, cy{ static_cast<dimension_type>(other.y) } {}
        template <typename Scalar>
        constexpr basic_size(const basic_vector<Scalar, 3>& other) : cx{ static_cast<dimension_type>(other.x) }, cy{ static_cast<dimension_type>(other.y) } {}
        constexpr basic_size(dimension_type cx, dimension_type cy) : cx{ cx }, cy{ cy } {}
        constexpr explicit basic_size(dimension_type aSquareDimension) : cx{ aSquareDimension }, cy{ aSquareDimension } {}
        template <typename CoordinateType2>
        constexpr basic_size(const basic_size<CoordinateType2>& other) :
            cx{ static_cast<CoordinateType>(other.cx) }, cy{ static_cast<CoordinateType>(other.cy) } {}
        constexpr basic_size(const basic_delta<CoordinateType>& other) : cx{ other.dx }, cy{ other.dy } {}
        // operations
    public:
        basic_vector<dimension_type, 2> to_vec2() const { throw_on_bad_size(*this); return basic_vector<dimension_type, 2>{ cx, cy }; }
        basic_vector<dimension_type, 3> to_vec3() const { throw_on_bad_size(*this); return basic_vector<dimension_type, 3>{ cx, cy, 0.0 }; }
        basic_vector<dimension_type, 2> to_vec2_inf() const { return basic_vector<dimension_type, 2>{ cx, cy }; }
        basic_vector<dimension_type, 3> to_vec3_inf() const { return basic_vector<dimension_type, 3>{ cx, cy, 0.0 }; }
        delta_type to_delta() const { return delta_type(cx, cy); }
        bool empty() const { return cx == 0 || cy == 0; }
        auto operator<=>(const basic_size&) const = default;
        basic_size operator-() const { throw_on_bad_size(*this); return basic_size{ -cx, -cy }; }
        basic_size& operator+=(const basic_size& other) { throw_on_bad_size(other); cx += other.cx; cy += other.cy; return *this; }
        basic_size& operator+=(const basic_delta<CoordinateType>& other) { throw_on_bad_size(other); cx += other.dx; cy += other.dy; return *this; }
        basic_size& operator+=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx += amount; cy += amount; return *this; }
        basic_size& operator-=(const basic_size& other) { throw_on_bad_size(other); cx -= other.cx; cy -= other.cy; return *this; }
        basic_size& operator-=(const basic_delta<CoordinateType>& other) { throw_on_bad_size(other); cx -= other.dx; cy -= other.dy; return *this; }
        basic_size& operator-=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx -= amount; cy -= amount; return *this; }
        basic_size& operator*=(const basic_size& other) { throw_on_bad_size(other); cx *= other.cx; cy *= other.cy; return *this; }
        basic_size& operator*=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx *= amount; cy *= amount; return *this; }
        basic_size& operator/=(const basic_size& other) { throw_on_bad_size(other); cx /= other.cx; cy /= other.cy; return *this; }
        basic_size& operator/=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx /= amount; cy /= amount; return *this; }
        basic_size ceil() const { return basic_size{ !cx_inf() ? std::ceil(cx) : cx, !cy_inf() ? std::ceil(cy) : cy }; }
        basic_size floor() const { return basic_size{ !cx_inf() ? std::floor(cx) : cx, !cy_inf() ? std::floor(cy) : cy }; }
        basic_size round() const { return basic_size{ !cx_inf() ? std::round(cx) : cx, !cy_inf() ? std::round(cy) : cy }; }
        basic_size min(const basic_size& other) const { return basic_size{ std::min(cx, other.cx), std::min(cy, other.cy) }; }
        basic_size max(const basic_size& other) const { return basic_size{ std::max(cx, other.cx), std::max(cy, other.cy) }; }
        bool less_than(const basic_size& other) const { return cx < other.cx && cy < other.cy; };
        bool less_than_or_equal(const basic_size& other) const { return cx <= other.cx && cy <= other.cy; };
        bool greater_than(const basic_size& other) const { return cy > other.cy && cy > other.cy; };
        bool greater_than_or_equal(const basic_size& other) const { return cy >= other.cy && cy >= other.cy; };
        dimension_type magnitude() const { throw_on_bad_size(*this); return std::sqrt(cx * cx + cy * cy); }
        basic_size with_cx(dimension_type d) const { return basic_size{ d, cy }; }
        basic_size with_cy(dimension_type d) const { return basic_size{ cx, d }; }
        basic_size aspect_ratio() const { return cx >= cy ? basic_size{ 1.0, cy / cx } : basic_size{ cx / cy, 1.0 }; }
        basic_size with_aspect_ratio_min(const basic_size& aspectRatio) const { return basic_size{ std::min(cx, cy) } * aspectRatio; };
        basic_size with_aspect_ratio_max(const basic_size& aspectRatio) const { return basic_size{ std::max(cx, cy) } * aspectRatio; };
    public:
        template <typename T>
        basic_size<T> as() const
        {
            return *this;
        }
    public:
        bool cx_inf() const { return cx == max_dimension(); }
        bool cy_inf() const { return cy == max_dimension(); }
    private:
        void throw_on_bad_size(const basic_size& rhs) const { if ((rhs.cx != 0.0 && cx_inf()) && (rhs.cy != 0.0 && cy_inf())) throw bad_size(); }
        // helpers
    public:
        static constexpr dimension_type max_dimension() { return std::numeric_limits<dimension_type>::infinity(); }
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
    inline bool nearly_equal(basic_size<CoordinateType> const& lhs, basic_size<CoordinateType> const& rhs, scalar epsilon = 0.00001)
    {
        return nearly_equal(lhs.cx, rhs.cx, epsilon) && nearly_equal(lhs.cy, rhs.cy, epsilon);
    }

    template <typename CoordinateType>
    class basic_point
    {
        typedef basic_point<CoordinateType> self_type;
        // types
    public:
        typedef self_type abstract_type; // todo: abstract type
        typedef CoordinateType coordinate_type;
        // construction
    public:
        constexpr basic_point() : x{}, y{}, z{} {}
        template <typename Scalar>
        constexpr basic_point(const basic_vector<Scalar, 2>& other) : 
            x{ static_cast<coordinate_type>(other.x) }, 
            y{ static_cast<coordinate_type>(other.y) },
            z{} {}
        template <typename Scalar>
        constexpr basic_point(const basic_vector<Scalar, 3>& other) : 
            x{ static_cast<coordinate_type>(other.x) }, 
            y{ static_cast<coordinate_type>(other.y) },
            z{} {}
        constexpr basic_point(CoordinateType x, CoordinateType y, CoordinateType z = {}) : 
            x{ x }, y{ y }, z{ z } {}
        template <typename CoordinateType2>
        constexpr basic_point(const basic_point<CoordinateType2>& other) :
            x{ static_cast<CoordinateType>(other.x) }, 
            y{ static_cast<CoordinateType>(other.y) },
            z{ static_cast<CoordinateType>(other.z) } {}
        constexpr basic_point(const basic_delta<CoordinateType>& other) : 
            x{ other.dx }, y{ other.dy }, z{} {}
        constexpr basic_point(const basic_size<CoordinateType>& other) : 
            x{ other.cx }, y{ other.cy }, z{} {}
        // operations
    public:
        basic_delta<coordinate_type> to_delta() const { return basic_delta<CoordinateType>{ x, y }; }
        basic_size<coordinate_type> to_size() const { return basic_size<CoordinateType>{ x, y }; }
        basic_vector<coordinate_type, 2> to_vec2() const { return basic_vector<coordinate_type, 2>{ x, y }; }
        basic_vector<coordinate_type, 3> to_vec3() const { return basic_vector<coordinate_type, 3>{ x, y, z }; }
        explicit operator basic_delta<coordinate_type>() const { return to_delta(); }
        explicit operator basic_size<coordinate_type>() const { return to_size(); }
        auto operator<=>(const basic_point&) const = default;
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
        basic_point operator-() const { return basic_point{ -x, -y, -z }; }
        basic_point abs() const { return basic_point{ std::abs(x), std::abs(y), std::abs(z) }; }
        basic_point ceil() const { return basic_point{ std::ceil(x), std::ceil(y), std::ceil(z) }; }
        basic_point floor() const { return basic_point{ std::floor(x), std::floor(y), std::floor(z) }; }
        basic_point round() const { return basic_point{ std::round(x), std::round(y), std::round(z) }; }
        basic_point min(const basic_point& other) const { return basic_point{ std::min(x, other.x), std::min(y, other.y), std::min(z, other.z) }; }
        basic_point max(const basic_point& other) const { return basic_point{ std::max(x, other.x), std::max(y, other.y), std::max(z, other.z) }; }
        basic_point min_max(const basic_point& other) const { return basic_point{ std::min(x, other.x), std::max(y, other.y), other.z }; }
        basic_point max_min(const basic_point& other) const { return basic_point{ std::max(x, other.x), std::min(y, other.y), other.z }; }
        basic_point mid(const basic_point& other) const { return basic_point{ (x + other.x) / static_cast<coordinate_type>(2.0), (y + other.y) / static_cast<coordinate_type>(2.0), (z + other.z) / static_cast<coordinate_type>(2.0) }; }
        basic_point with_x(coordinate_type x0) const { return basic_point{ x0, y, z }; }
        basic_point with_y(coordinate_type y0) const { return basic_point{ x, y0, z }; }
        basic_point with_z(coordinate_type z0) const { return basic_point{ x, y, z0 }; }
        coordinate_type magnitude() const { return std::sqrt(x * x + y * y + z * z); }
        template <typename T>
        basic_point<T> as() const
        {
            return *this;
        }
        // attributes
    public:
        coordinate_type x;
        coordinate_type y;
        coordinate_type z;
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
    inline basic_point<CoordinateType> operator-(const basic_point<CoordinateType>& left, const basic_point<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret -= right;
        return ret;
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
    inline basic_point<CoordinateType> operator-(const basic_point<CoordinateType>& left, const basic_delta<CoordinateType>& right)
    {
        basic_point<CoordinateType> ret = left;
        ret -= right;
        return ret;
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
    inline bool nearly_equal(basic_point<CoordinateType> const& lhs, basic_point<CoordinateType> const& rhs, scalar epsilon = 0.00001)
    {
        return nearly_equal(lhs.x, rhs.x, epsilon) && nearly_equal(lhs.y, rhs.y, epsilon);
    }

    template <typename DimensionType>
    class basic_box_areas
    {
        typedef basic_box_areas<DimensionType> self_type;
        // types
    public:
        typedef self_type abstract_type; // todo: abstract type
        typedef DimensionType dimension_type;
        typedef dimension_type coordinate_type;
        typedef basic_point<coordinate_type> point_type;
        typedef basic_size<dimension_type> size_type;
        // construction
    public:
        basic_box_areas() : left{}, top{}, right{}, bottom{} {}
        basic_box_areas(dimension_type all) : left(all), top(all), right(all), bottom(all) {}
        basic_box_areas(dimension_type left, dimension_type top, dimension_type right, dimension_type bottom) : left(left), top(top), right(right), bottom(bottom) {}
        basic_box_areas(dimension_type leftRight, dimension_type topBottom) : left(leftRight), top(topBottom), right(leftRight), bottom(topBottom) {}
        basic_box_areas(const size_type& aDimensions) : basic_box_areas{ aDimensions.cx, aDimensions.cy } {};
        basic_box_areas(const point_type& aTopLeft, const point_type& aBottomRight) : left(aTopLeft.x), top(aTopLeft.y), right(aBottomRight.x), bottom(aBottomRight.y) {}
        template <typename DimensionType2>
        basic_box_areas(const basic_box_areas<DimensionType2>& other) :
            left(static_cast<dimension_type>(other.left)), top(static_cast<dimension_type>(other.top)), right(static_cast<dimension_type>(other.right)), bottom(static_cast<dimension_type>(other.bottom)) {}
        // operations
    public:
        auto operator<=>(const self_type&) const = default;
        self_type operator-() const { return self_type{ -left, -top, -right, -bottom }; }
        self_type& operator+=(const self_type& other) { left += other.left; top += other.top; right += other.right; bottom += other.bottom; return *this; }
        self_type& operator+=(dimension_type amount) { left += amount; top += amount; right += amount; bottom += amount; return *this; }
        self_type& operator-=(const self_type& other) { left -= other.left; top -= other.top; right -= other.right; bottom -= other.bottom; return *this; }
        self_type& operator-=(dimension_type amount) { left -= amount; top -= amount; right -= amount; bottom -= amount; return *this; }
        self_type& operator*=(const self_type& other) { left *= other.left; top *= other.top; right *= other.right; bottom *= other.bottom; return *this; }
        self_type& operator*=(dimension_type amount) { left *= amount; top *= amount; right *= amount; bottom *= amount; return *this; }
        self_type& operator/=(const self_type& other) { left /= other.left; top /= other.top; right /= other.right; bottom /= other.bottom; return *this; }
        self_type& operator/=(dimension_type amount) { left /= amount; top /= amount; right /= amount; bottom /= amount; return *this; }
    public:
        point_type top_left() const { return point_type{ left, top }; }
        point_type bottom_right() const { return point_type{ right, bottom }; }
        size_type size() const { return size_type{ left + right, top + bottom }; }
    public:
        self_type ceil() const { return self_type{ std::ceil(left), std::ceil(top), std::ceil(right), std::ceil(bottom) }; }
        self_type floor() const { return self_type{ std::floor(left), std::floor(top), std::floor(right), std::floor(bottom) }; }
        self_type round() const { return self_type{ std::round(left), std::round(top), std::round(right), std::round(bottom) }; }
        // attributes
    public:
        dimension_type left;
        dimension_type top;
        dimension_type right;
        dimension_type bottom;
    };

    typedef basic_box_areas<double> box_areas;

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator+(const basic_box_areas<DimensionType>& left, const basic_box_areas<DimensionType>& right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret += right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator-(const basic_box_areas<DimensionType>& left, const basic_box_areas<DimensionType>& right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator*(const basic_box_areas<DimensionType>& left, const basic_box_areas<DimensionType>& right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator/(const basic_box_areas<DimensionType>& left, const basic_box_areas<DimensionType>& right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator+(const basic_box_areas<DimensionType>& left, typename basic_box_areas<DimensionType>::dimension_type right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret += right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator-(const basic_box_areas<DimensionType>& left, typename basic_box_areas<DimensionType>::dimension_type right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret -= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator*(const basic_box_areas<DimensionType>& left, typename basic_box_areas<DimensionType>::dimension_type right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret *= right;
        return ret;
    }

    template <typename DimensionType>
    inline basic_box_areas<DimensionType> operator/(const basic_box_areas<DimensionType>& left, typename basic_box_areas<DimensionType>::dimension_type right)
    {
        basic_box_areas<DimensionType> ret = left;
        ret /= right;
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem = logical_coordinate_system::AutomaticGui>
    class basic_rect :
        public basic_point<CoordinateType>,
        public basic_size<CoordinateType>
    {
        typedef basic_rect<CoordinateType, CoordinateSystem> self_type;
        // types
    public:
        typedef self_type abstract_type; // todo: abstract type
        typedef CoordinateType coordinate_type;
        typedef coordinate_type dimension_type;
    private:
        typedef basic_delta<coordinate_type> delta_type;
        typedef basic_size<dimension_type> size_type;
        typedef basic_point<coordinate_type> point_type;
        typedef basic_box_areas<dimension_type> padding_type;
    public:
        using point_type::x;
        using point_type::y;
        using size_type::cx;
        using size_type::cy;
        // constants
    public:
        static constexpr bool gui = is_gui<CoordinateSystem>::value;
        static constexpr bool game = is_game<CoordinateSystem>::value;
        static constexpr coordinate_type zero = constants::zero<coordinate_type>;
        static constexpr coordinate_type one = constants::one<coordinate_type>;
        static constexpr coordinate_type two = constants::two<coordinate_type>;
        static constexpr coordinate_type default_epsilon = static_cast<coordinate_type>(0.00001);
        // construction
    public:
        basic_rect() : epsilon { default_epsilon } {}
        basic_rect(const point_type& coordinates, const size_type& dimensions) : point_type{ coordinates }, size_type{ dimensions } {}
        basic_rect(const point_type& leftCorner, const point_type& rightCorner) : basic_rect{ leftCorner.x, leftCorner.y, rightCorner.x, rightCorner.y } {}
        explicit basic_rect(const point_type& coordinates) : point_type{ coordinates }, size_type{} {}
        explicit basic_rect(const size_type& dimensions) : point_type{}, size_type{ dimensions } {}
        basic_rect(coordinate_type x0, coordinate_type y0, coordinate_type x1, coordinate_type y1) : point_type{ x0, y0 }, size_type{ x1 - x0, y1 - y0 } {}
        basic_rect(const aabb_2d& aBoundingBox) : basic_rect{ aBoundingBox.min.x, aBoundingBox.min.y, aBoundingBox.max.x, aBoundingBox.max.y } {}
        basic_rect(const basic_box_areas<coordinate_type>& aBoxAreas) : basic_rect{ aBoxAreas.top_left(), aBoxAreas.bottom_right() } {}
    public:
        template <typename CoordinateType2, logical_coordinate_system CoordinateSystem2>
        basic_rect(const basic_rect<CoordinateType2, CoordinateSystem2>& other) : point_type{ other }, size_type{ other } {}
        // assignment
    public:
        template <typename CoordinateType2, logical_coordinate_system CoordinateSystem2>
        self_type& operator=(const basic_rect<CoordinateType2, CoordinateSystem2>& other) { static_cast<point_type&>(*this) = other; static_cast<size_type&>(*this) = other; epsilon = other.epsilon; return *this; }
        self_type& operator=(const point_type& coordinates) { static_cast<point_type&>(*this) = coordinates; return *this; }
        self_type& operator=(const size_type& dimensions) { static_cast<size_type&>(*this) = dimensions; return *this; }
        // operations
    public:
        basic_vector<basic_vector<coordinate_type, 2>, 4> to_vector() const 
        { 
            if constexpr (gui)
                return basic_vector<basic_vector<coordinate_type, 2>, 4>(top_left().to_vector(), top_right().to_vector(), bottom_right().to_vector(), bottom_left().to_vector()); 
            else
                return basic_vector<basic_vector<coordinate_type, 2>, 4>(bottom_left().to_vector(), bottom_right().to_vector(), top_right().to_vector(), top_left().to_vector());
        }
        const point_type& position() const { return *this; }
        point_type& position() { return *this; }
        const size_type& extents() const { return *this; }
        size_type& extents() { return *this; }
        coordinate_type left() const { return x; }
        coordinate_type top() const { if constexpr (gui) return y; else return (y + cy) - epsilon.cy; }
        coordinate_type right() const { return (x + cx) - epsilon.cx; }
        coordinate_type bottom() const { if constexpr (gui) return (y + cy) - epsilon.cy; else return y; }
        point_type top_left() const { return point_type(left(), top()); }
        point_type top_right() const { return point_type(right(), top()); }
        point_type bottom_left() const { return point_type(left(), bottom()); }
        point_type bottom_right() const { return point_type(right(), bottom()); }
        dimension_type width() const { return cx; }
        dimension_type height() const { return cy; }
        bool operator==(const basic_rect& other) const { return x == other.x && y == other.y && cx == other.cx && cy == other.cy; }
        bool operator!=(const basic_rect& other) const { return !operator==(other); }
        self_type& operator*=(const self_type& other) { position() *= other.position(); extents() *= other.extents(); return *this; }
        self_type& operator*=(const size_type& size) { position() *= size; extents() *= size; return *this; }
        self_type& operator*=(dimension_type value) { position() *= value; extents() *= value; return *this; }
        self_type& operator/=(const basic_rect& other) { position() /= other.position(); extents() /= other.extents(); return *this; }
        self_type& operator/=(const size_type& size) { position() /= size; extents() /= size; return *this; }
        self_type& operator/=(dimension_type value) { position() /= value; extents() /= value; return *this; }
        bool contains_x(const point_type& point) const { return point.x >= left() && point.x < right(); }
        bool contains_y(const point_type& point) const { if constexpr (gui) return point.y >= top() && point.y < bottom(); else return point.y >= bottom() && point.y < top(); }
        bool contains(const point_type& point) const { return contains_x(point) && contains_y(point); }
        bool contains(const self_type& other) const
        { 
            if constexpr (gui) 
                return other.left() >= left() && other.right() <= right() && other.top() >= top() && other.bottom() <= bottom(); 
            else
                return other.left() >= left() && other.right() <= right() && other.bottom() >= bottom() && other.top() <= top();
        }
        point_type center() const 
        { 
            if constexpr (gui)
                return point_type{ left() + static_cast<coordinate_type>(width()) / two, top() + static_cast<coordinate_type>(height()) / two };
            else
                return point_type{ left() + static_cast<coordinate_type>(width()) / two, bottom() + static_cast<coordinate_type>(height()) / two };
        }
        self_type& translate(const point_type& aOffset) { x += aOffset.x; y += aOffset.y; return *this; }
        self_type translated(const point_type& aOffset) const { self_type result = *this; result.x += aOffset.x; result.y += aOffset.y; return result; }
        self_type& indent(const point_type& aOffset) { x += aOffset.x; y += aOffset.y; cx -= aOffset.x; cy -= aOffset.y; return *this; }
        self_type& inflate(const delta_type& delta) { x -= delta.dx; y -= delta.dy; cx += delta.dx * two; cy += delta.dy * two; return *this; }
        self_type& inflate(const size_type& size) { return inflate(delta_type(size.cx, size.cy)); }
        self_type& inflate(const padding_type& padding) { return inflate(padding.left, padding.top, padding.right, padding.bottom); }
        self_type& inflate(coordinate_type dx, coordinate_type dy) { return inflate(delta_type(dx, dy)); }
        self_type& inflate(coordinate_type left, coordinate_type top, coordinate_type right, coordinate_type bottom) { x -= left; y -= top; cx += (left + right); cy += (top + bottom); return *this; }
        template <typename... Args>
        self_type inflated(Args&&... aArgs) const
        {
            auto result = *this;
            result.inflate(std::forward<Args>(aArgs)...);
            return result;
        }
        self_type& deflate(const delta_type& delta) { return inflate(-delta); }
        self_type& deflate(const size_type& size) { return inflate(-size.cx, -size.cy); }
        self_type& deflate(const padding_type& padding) { return deflate(padding.left, padding.top, padding.right, padding.bottom); }
        self_type& deflate(coordinate_type dx, coordinate_type dy) { return inflate(-dx, -dy); }
        self_type& deflate(coordinate_type left, coordinate_type top, coordinate_type right, coordinate_type bottom) { return inflate(-left, -top, -right, -bottom); }
        template <typename... Args>
        self_type deflated(Args&&... aArgs) const
        {
            auto result = *this;
            result.deflate(std::forward<Args>(aArgs)...);
            return result;
        }
        template <typename... T>
        friend self_type inflate_rect(const self_type& aRect, T&&... aAmount)
        {
            auto result = aRect;
            result.inflate(std::forward<T>(aAmount)...);
            return result;
        }
        template <typename... T>
        friend self_type deflate_rect(const self_type& aRect, T&&... aAmount)
        {
            auto result = aRect;
            result.deflate(std::forward<T>(aAmount)...);
            return result;
        }
        self_type intersection(const self_type& other) const
        {
            if constexpr (gui)
            {
                self_type candidate{ top_left().max(other.top_left()), bottom_right().min(other.bottom_right()) };
                if (contains(candidate.center()) && other.contains(candidate.center()))
                    return candidate;
                else
                    return basic_rect{};
            }
            else
            {
                self_type candidate{ bottom_left().max(other.bottom_left()), top_right().min(other.top_right()) };
                if (contains(candidate.center()) && other.contains(candidate.center()))
                    return candidate;
                else
                    return basic_rect{};
            }
        }
        self_type& center_on(const self_type& other)
        {
            *this = centered_on(other);
            return *this;
        }
        self_type centered_on(const self_type other) const
        {
            auto result = *this;
            result.position() += (other.center() - center());
            return result;
        }
        self_type& combine(const self_type& other)
        {
            *this = combined(other);
            return *this;
        }
        self_type combined(const self_type& other) const
        {
            if constexpr (gui)
                return self_type{ top_left().min(other.top_left()), bottom_right().max(other.bottom_right()) };
            else
                return self_type{ bottom_left().min(other.bottom_left()), top_right().max(other.top_right()) };
        }
        self_type with_position(const point_type& p) const
        {
            auto result = *this;
            result.position().x = p.x;
            result.position().y = p.y;
            return result;
        }
        self_type with_x(coordinate_type c) const
        {
            auto result = *this;
            result.position().x = c;
            return result;
        }
        self_type with_y(coordinate_type c) const
        {
            auto result = *this;
            result.position().y = c;
            return result;
        }
        self_type with_extents(const size_type& e) const
        {
            auto result = *this;
            result.extents().cx = e.cx;
            result.extents().cy = e.cy;
            return result;
        }
        self_type with_cx(dimension_type d) const
        { 
            auto result = *this;
            result.extents().cx = d;
            return result;
        }
        self_type with_cy(dimension_type d) const 
        { 
            auto result = *this;
            result.extents().cy = d;
            return result;
        }
        self_type with_centered_origin() const
        {
            return self_type{ point_type{ -extents() / two }, extents() };
        }
        self_type ceil() const { return self_type{ point_type::ceil(), size_type::ceil() }; }
        self_type floor() const { return self_type{ point_type::floor(), size_type::floor() }; }
        self_type round() const { return self_type{ point_type::round(), size_type::round() }; }
        self_type with_epsilon(const size_type& aEpsilon = default_epsilon) const
        {
            self_type result = *this;
            result.epsilon = aEpsilon;
            return result;
        }
        aabb_2d to_aabb_2d() const
        { 
            if constexpr (gui)
                return aabb_2d{ top_left().to_vec2(), bottom_right().to_vec2() };
            else
                return aabb_2d{ bottom_left().to_vec2(), top_right().to_vec2() };
        }
        basic_vector<coordinate_type, 4> to_vec4() const
        {
            return basic_vector<coordinate_type, 4>{ top_left().x, top_left().y, bottom_right().x, bottom_right().y };
        }
    public:
        template <typename T>
        std::enable_if_t<!std::is_same_v<T, coordinate_type>, basic_rect<T, CoordinateSystem>> as() const
        {
            return basic_rect<T, CoordinateSystem>{ *this };
        }
        template <typename T>
        std::enable_if_t<std::is_same_v<T, coordinate_type>, self_type const&> as() const
        {
            return *this;
        }
    public:
        size_type epsilon = size_type{ zero };
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
        ret.basic_point<CoordinateType>::operator+=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator+(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_size<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size<CoordinateType>::operator+=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator+(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_delta<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size<CoordinateType>::operator+=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator-(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_point<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_point<CoordinateType>::operator-=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator-(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_size<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size<CoordinateType>::operator-=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline basic_rect<CoordinateType, CoordinateSystem> operator-(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_delta<CoordinateType>& right)
    {
        basic_rect<CoordinateType, CoordinateSystem> ret = left;
        ret.basic_size<CoordinateType>::operator-=(right);
        return ret;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline bool operator<(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_rect<CoordinateType, CoordinateSystem>& right)
    {
        return std::forward_as_tuple(left.position(), left.extents()) < std::forward_as_tuple(right.position(), right.extents());
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline bool operator==(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_rect<CoordinateType, CoordinateSystem>& right)
    {
        return left.position() == right.position() && left.extents() == right.extents();
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline bool operator<=(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_rect<CoordinateType, CoordinateSystem>& right)
    {
        return left < right || left == right;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline bool operator>(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_rect<CoordinateType, CoordinateSystem>& right)
    {
        return right < left;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline bool operator>=(const basic_rect<CoordinateType, CoordinateSystem>& left, const basic_rect<CoordinateType, CoordinateSystem>& right)
    {
        return right < left || left == right;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline bool nearly_equal(basic_rect<CoordinateType, CoordinateSystem> const& lhs, basic_rect<CoordinateType, CoordinateSystem> const& rhs, scalar epsilon = 0.00001)
    {
        return nearly_equal(lhs.position(), rhs.position(), epsilon) && nearly_equal(lhs.extents(), rhs.extents(), epsilon);
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

    typedef basic_size<int32_t> size_i32;
    typedef basic_delta<int32_t> delta_i32;
    typedef basic_point<int32_t> point_i32;
    typedef basic_rect<int32_t> rect_i32;

    typedef basic_size<uint32_t> size_u32;
    typedef basic_delta<uint32_t> delta_u32;
    typedef basic_point<uint32_t> point_u32;
    typedef basic_rect<uint32_t> rect_u32;

    template <typename DimensionType>
    using basic_margin = basic_box_areas<DimensionType>;

    template <typename DimensionType>
    using basic_border = basic_box_areas<DimensionType>;

    template <typename DimensionType>
    using basic_padding = basic_box_areas<DimensionType>;

    typedef basic_margin<dimension> margin;
    typedef basic_border<dimension> border;
    typedef basic_padding<dimension> padding;

    typedef optional<dimension> optional_dimension;
    typedef optional<angle> optional_angle;
    typedef optional<delta> optional_delta;
    typedef optional<point> optional_point;
    typedef optional<size> optional_size;
    typedef optional<rect> optional_rect;
    typedef optional<delta_u32> optional_delta_u32;
    typedef optional<point_u32> optional_point_u32;
    typedef optional<size_u32> optional_size_u32;
    typedef optional<rect_u32> optional_rect_u32;
    typedef optional<margin> optional_margin;
    typedef optional<border> optional_border;
    typedef optional<padding> optional_padding;
    typedef optional<vector1> optional_vector1;
    typedef optional<vector2> optional_vector2;
    typedef optional<vector3> optional_vector3;
    typedef optional<vector4> optional_vector4;

    template <typename T>
    inline basic_point<T> operator*(basic_matrix<T, 3, 3> const& aTransformation, basic_point<T> const& aPoint)
    {
        return aTransformation * aPoint.to_vec3();
    }

    template <typename T>
    inline basic_size<T> operator*(basic_matrix<T, 3, 3> const& aTransformation, basic_size<T> const& aSize)
    {
        if (aSize == basic_size<T>::max_size())
            return aSize;
        return inf_multiply(aTransformation, aSize.to_vec3_inf());
    }

    template <typename T>
    inline basic_rect<T> operator*(basic_matrix<T, 3, 3> const& aTransformation, basic_rect<T> const& aRect)
    {
        return basic_rect<T>{ basic_point<T>{ aTransformation* aRect.top_left().to_vec3() }, basic_point<T>{ aTransformation * aRect.bottom_right().to_vec3() } };
    }

    template <typename T>
    inline basic_box_areas<T> operator*(basic_matrix<T, 3, 3> const& aTransformation, basic_box_areas<T> const& aBoxAreas)
    {
        return basic_box_areas<T>{ basic_point<T>{ aTransformation * aBoxAreas.top_left().to_vec3() }, basic_point<T>{ aTransformation * aBoxAreas.bottom_right().to_vec3() } };
    }

    inline point mix(const point& aLhs, const point& aRhs, double aMixValue)
    {
        return point{ mix(aLhs.x, aRhs.x, aMixValue), mix(aLhs.y, aRhs.y, aMixValue) };
    }

    inline vec2 mix(const vec2& aLhs, const vec2& aRhs, double aMixValue)
    {
        return vec2{ mix(aLhs.x, aRhs.x, aMixValue), mix(aLhs.y, aRhs.y, aMixValue) };
    }

    inline vec3 mix(const vec3& aLhs, const vec3& aRhs, double aMixValue)
    {
        return vec3{ mix(aLhs.x, aRhs.x, aMixValue), mix(aLhs.y, aRhs.y, aMixValue), mix(aLhs.z, aRhs.z, aMixValue) };
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_point<T>& aPoint)
    {
        aStream << "(" << aPoint.x << ", " << aPoint.y << ")";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, basic_point<T>& aPoint)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_as_whitespace{} });
        char ignore;
        aStream >> ignore >> aPoint.x >> aPoint.y >> ignore;
        aStream.imbue(previousImbued);
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_size<T>& aSize)
    {
        aStream << "{" << aSize.cx << ", " << aSize.cy << "}";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, basic_size<T>& aSize)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_as_whitespace{} });
        char ignore;
        aStream >> ignore >> aSize.cx >> aSize.cy >> ignore;
        aStream.imbue(previousImbued);
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_delta<T>& aDelta)
    {
        aStream << "{dx: " << aDelta.dx << ", dy: " << aDelta.dy << "}";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, basic_delta<T>& aDelta)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_as_whitespace{} });
        std::string ignore;
        aStream >> ignore >> aDelta.dx >> ignore >> aDelta.dy >> ignore;
        aStream.imbue(previousImbued);
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_rect<T>& aRect)
    {
        aStream << "[" << aRect.top_left() << " -> " << aRect.bottom_right() << ", " << aRect.extents() << "]";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_box_areas<T>& aBoxAreas)
    {
        aStream << "[" << aBoxAreas.top_left() << " -> " << aBoxAreas.bottom_right() << ", " << aBoxAreas.size() << "]";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const neolib::optional<basic_point<T>>& aPoint)
    {
        if (aPoint)
            return aStream << *aPoint;
        aStream << "(nullopt)";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const neolib::optional<basic_size<T>>& aSize)
    {
        if (aSize)
            return aStream << *aSize;
        aStream << "(nullopt)";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const neolib::optional<basic_delta<T>>& aDelta)
    {
        if (aDelta)
            return aStream << *aDelta;
        aStream << "(nullopt)";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const neolib::optional<basic_rect<T>>& aBoxAreas)
    {
        if (aBoxAreas)
            return aStream << *aBoxAreas;
        aStream << "(nullopt)";
        return aStream;
    }

    template <typename Elem, typename Traits, typename T>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const neolib::optional<basic_box_areas<T>>& aRect)
    {
        if (aRect)
            return aStream << *aRect;
        aStream << "(nullopt)";
        return aStream;
    }
}

define_setting_type(neogfx::delta)
define_setting_type(neogfx::size)
define_setting_type(neogfx::point)
define_setting_type(neogfx::rect)

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
