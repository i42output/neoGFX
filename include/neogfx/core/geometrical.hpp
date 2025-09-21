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
    using default_geometry_value_type = scalar;
    using coordinate                  = default_geometry_value_type;
    using dimension                   = default_geometry_value_type;
    using coordinate_delta            = default_geometry_value_type;

    using coordinate_i32              = std::int32_t;
    using dimension_i32               = std::int32_t;
    using coordinate_delta_i32        = std::int32_t;

    using coordinate_u32              = std::uint32_t;
    using dimension_u32               = std::uint32_t;
    using coordinate_delta_u32        = std::uint32_t;

    using xy    = vec2;
    using xyz   = vec3;

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

    using optional_logical_coordinate_system = optional<logical_coordinate_system> ;

    struct logical_coordinates
    {
        using abstract_type = logical_coordinates; // todo

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

    // Primarily for 2D use so 3D support limited
    template <typename CoordinateType>
    class basic_delta 
    { 
        // types
    public:
        using abstract_type   = basic_delta; // todo: abstract type
        using coordinate_type = CoordinateType;
        // construction
    public:
        constexpr basic_delta() : dx{}, dy{}, dz{} {}
        constexpr basic_delta(coordinate_type dx, coordinate_type dy, coordinate_type dz = {}) : dx{ dx }, dy{ dy }, dz{ dz } {}
        constexpr explicit basic_delta(coordinate_type aSquareDelta) : dx{ aSquareDelta }, dy{ aSquareDelta }, dz{} {}
        template <typename CoordinateType2>
        constexpr basic_delta(const basic_delta<CoordinateType2>& aOther) :
            dx{ static_cast<CoordinateType>(aOther.dx) }, dy{ static_cast<CoordinateType>(aOther.dy) }, dz{ static_cast<CoordinateType>(aOther.dz) } {}
        // operations
    public:
        basic_vector<coordinate_type, 2> to_vec2() const { return basic_vector<coordinate_type, 2>{ dx, dy }; }
        basic_vector<coordinate_type, 3> to_vec3() const { return basic_vector<coordinate_type, 3>{ dx, dy, dz }; }
        auto operator<=>(const basic_delta&) const = default;
        basic_delta& operator+=(coordinate_type s) { dx +=s; dy += s; return *this; }
        basic_delta& operator-=(coordinate_type s) { dx -=s; dy -= s; return *this; }
        basic_delta& operator*=(coordinate_type s) { dx *=s; dy *= s; return *this; }
        basic_delta& operator/=(coordinate_type s) { dx /=s; dy /= s; return *this; }
        basic_delta& operator+=(const basic_delta& aOther) { dx += aOther.dx; dy += aOther.dy; dz += aOther.dz; return *this; }
        basic_delta& operator-=(const basic_delta& aOther) { dx -= aOther.dx; dy -= aOther.dy; dz -= aOther.dz; return *this; }
        basic_delta& operator*=(const basic_delta& aOther) { dx *= aOther.dx; dy *= aOther.dy; dz *= aOther.dz; return *this; }
        basic_delta& operator/=(const basic_delta& aOther) { dx /= aOther.dx; dy /= aOther.dy; if (aOther.dz != static_cast<coordinate_type>(0.0)) dz /= aOther.dz; return *this; }
        basic_delta operator-() const { return basic_delta{ -dx, -dy, -dz }; }
        basic_delta abs() const { return basic_delta{ std::abs(dx), std::abs(dy), std::abs(dz) }; }
        basic_delta ceil() const { return basic_delta{ std::ceil(dx), std::ceil(dy), std::ceil(dz) }; }
        basic_delta floor() const { return basic_delta{ std::floor(dx), std::floor(dy), std::floor(dz) }; }
        basic_delta round() const { return basic_delta{ std::round(dx), std::round(dy), std::round(dz) }; }
        basic_delta min(const basic_delta& aOther) const { return basic_delta{ std::min(dx, aOther.dx), std::min(dy, aOther.dy), std::min(dz, aOther.dz) }; }
        basic_delta max(const basic_delta& aOther) const { return basic_delta{ std::max(dx, aOther.dx), std::max(dy, aOther.dy), std::max(dz, aOther.dz) }; }
        basic_delta with_dx(coordinate_type dx) const { return basic_delta{ dx, dy, dz }; }
        basic_delta with_dy(coordinate_type dy) const { return basic_delta{ dx, dy, dz }; }
        coordinate_type magnitude() const { return std::sqrt(dx * dx + dy * dy + dz * dz); }
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
        coordinate_type dz;
    };

    using delta = basic_delta<coordinate>;

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

    // Primarily for 2D use so 3D support limited
    template <typename CoordinateType>
    class basic_size 
    { 
        // types
    public:
        using abstract_type   = basic_size; // todo: abstract type
        using coordinate_type = CoordinateType;
        using dimension_type  = CoordinateType;
        using delta_type      = basic_delta<dimension_type>;
        // construction
    public:
        constexpr basic_size() : cx{}, cy{}, cz{} {}
        template <typename Scalar>
        constexpr basic_size(const basic_vector<Scalar, 2>& aOther) : cx{ static_cast<dimension_type>(aOther.x) }, cy{ static_cast<dimension_type>(aOther.y) }, cz{} {}
        template <typename Scalar>
        constexpr basic_size(const basic_vector<Scalar, 3>& aOther) : cx{ static_cast<dimension_type>(aOther.x) }, cy{ static_cast<dimension_type>(aOther.y) }, cz{ static_cast<dimension_type>(aOther.z) } {}
        constexpr basic_size(dimension_type cx, dimension_type cy, dimension_type cz = {}) : cx{ cx }, cy{ cy }, cz{ cz } {}
        constexpr explicit basic_size(dimension_type aSquareDimension) : cx{ aSquareDimension }, cy{ aSquareDimension }, cz{} {}
        template <typename CoordinateType2>
        constexpr basic_size(const basic_size<CoordinateType2>& aOther) :
            cx{ static_cast<CoordinateType>(aOther.cx) }, cy{ static_cast<CoordinateType>(aOther.cy) }, cz{ static_cast<CoordinateType>(aOther.cz) } {}
        constexpr basic_size(const basic_delta<CoordinateType>& aOther) : cx{ aOther.dx }, cy{ aOther.dy }, cz{ aOther.dz } {}
        // operations
    public:
        basic_vector<dimension_type, 2> to_vec2() const { throw_on_bad_size(*this); return basic_vector<dimension_type, 2>{ cx, cy }; }
        basic_vector<dimension_type, 3> to_vec3() const { throw_on_bad_size(*this); return basic_vector<dimension_type, 3>{ cx, cy, cz }; }
        basic_vector<dimension_type, 2> to_vec2_inf() const { return basic_vector<dimension_type, 2>{ cx, cy }; }
        basic_vector<dimension_type, 3> to_vec3_inf() const { return basic_vector<dimension_type, 3>{ cx, cy, cz }; }
        delta_type to_delta() const { return delta_type(cx, cy, cz); }
        bool empty() const { return cx == 0 || cy == 0; }
        auto operator<=>(const basic_size&) const = default;
        basic_size operator-() const { throw_on_bad_size(*this); return basic_size{ -cx, -cy, -cz }; }
        basic_size& operator+=(const basic_size& aOther) { throw_on_bad_size(aOther); cx += aOther.cx; cy += aOther.cy; cz += aOther.cz; return *this; }
        basic_size& operator+=(const basic_delta<CoordinateType>& aOther) { throw_on_bad_size(aOther); cx += aOther.dx; cy += aOther.dy; cz += aOther.dz; return *this; }
        basic_size& operator+=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx += amount; cy += amount; return *this; }
        basic_size& operator-=(const basic_size& aOther) { throw_on_bad_size(aOther); cx -= aOther.cx; cy -= aOther.cy; cz -= aOther.cz; return *this; }
        basic_size& operator-=(const basic_delta<CoordinateType>& aOther) { throw_on_bad_size(aOther); cx -= aOther.dx; cy -= aOther.dy; cz -= aOther.dz; return *this; }
        basic_size& operator-=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx -= amount; cy -= amount; return *this; }
        basic_size& operator*=(const basic_size& aOther) { throw_on_bad_size(aOther); cx *= aOther.cx; cy *= aOther.cy; cz *= aOther.cz; return *this; }
        basic_size& operator*=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx *= amount; cy *= amount; return *this; }
        basic_size& operator/=(const basic_size& aOther) { throw_on_bad_size(aOther); cx /= aOther.cx; cy /= aOther.cy; if (aOther.cz != static_cast<dimension_type>(0.0)) cz /= aOther.cz; return *this; }
        basic_size& operator/=(dimension_type amount) { throw_on_bad_size(basic_size{ amount }); cx /= amount; cy /= amount; return *this; }
        basic_size ceil() const { return basic_size{ !cx_inf() ? std::ceil(cx) : cx, !cy_inf() ? std::ceil(cy) : cy, !cz_inf() ? std::ceil(cz) : cz }; }
        basic_size floor() const { return basic_size{ !cx_inf() ? std::floor(cx) : cx, !cy_inf() ? std::floor(cy) : cy, !cz_inf() ? std::floor(cz) : cz }; }
        basic_size round() const { return basic_size{ !cx_inf() ? std::round(cx) : cx, !cy_inf() ? std::round(cy) : cy, !cz_inf() ? std::round(cz) : cz }; }
        basic_size min(const basic_size& aOther) const { return basic_size{ std::min(cx, aOther.cx), std::min(cy, aOther.cy), std::min(cz, aOther.cz) }; }
        basic_size max(const basic_size& aOther) const { return basic_size{ std::max(cx, aOther.cx), std::max(cy, aOther.cy), std::max(cz, aOther.cz) }; }
        bool less_than(const basic_size& aOther) const { return cx < aOther.cx && cy < aOther.cy; };
        bool less_than_or_equal(const basic_size& aOther) const { return cx <= aOther.cx && cy <= aOther.cy; };
        bool greater_than(const basic_size& aOther) const { return cy > aOther.cy && cy > aOther.cy; };
        bool greater_than_or_equal(const basic_size& aOther) const { return cy >= aOther.cy && cy >= aOther.cy; };
        dimension_type magnitude() const { throw_on_bad_size(*this); return std::sqrt(cx * cx + cy * cy); }
        basic_size with_cx(dimension_type d) const { return basic_size{ d, cy, cz }; }
        basic_size with_cy(dimension_type d) const { return basic_size{ cx, d, cz }; }
        basic_size with_cz(dimension_type d) const { return basic_size{ cx, cy, d }; }
        basic_size aspect_ratio() const { return cx >= cy ? basic_size{ 1.0, cy / cx, cz } : basic_size{ cx / cy, 1.0, cz }; }
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
        bool cz_inf() const { return cz == max_dimension(); }
    private:
        void throw_on_bad_size(const basic_size& rhs) const { if ((rhs.cx != 0.0 && cx_inf()) && (rhs.cy != 0.0 && cy_inf()) && (rhs.cz != 0.0 && cz_inf())) throw bad_size(); }
        // helpers
    public:
        static constexpr dimension_type max_dimension() { return std::numeric_limits<dimension_type>::infinity(); }
        static constexpr basic_size max_size() { return basic_size{ max_dimension(), max_dimension() }; }
        // attributes
    public:
        dimension_type cx;
        dimension_type cy;
        dimension_type cz;
    };

    using size = basic_size<coordinate>;

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

    // Primarily for 2D use so 3D support limited
    template <typename CoordinateType>
    class basic_point
    {
        // types
    public:
        using abstract_type   = basic_point; // todo: abstract type
        using coordinate_type = CoordinateType;
        // construction
    public:
        constexpr basic_point() : x{}, y{}, z{} {}
        template <typename Scalar>
        constexpr basic_point(const basic_vector<Scalar, 2>& aOther) : 
            x{ static_cast<coordinate_type>(aOther.x) }, 
            y{ static_cast<coordinate_type>(aOther.y) },
            z{} {}
        template <typename Scalar>
        constexpr basic_point(const basic_vector<Scalar, 3>& aOther) : 
            x{ static_cast<coordinate_type>(aOther.x) }, 
            y{ static_cast<coordinate_type>(aOther.y) },
            z{} {}
        constexpr basic_point(CoordinateType x, CoordinateType y, CoordinateType z = {}) : 
            x{ x }, y{ y }, z{ z } {}
        template <typename CoordinateType2>
        constexpr basic_point(const basic_point<CoordinateType2>& aOther) :
            x{ static_cast<CoordinateType>(aOther.x) }, 
            y{ static_cast<CoordinateType>(aOther.y) },
            z{ static_cast<CoordinateType>(aOther.z) } {}
        constexpr basic_point(const basic_delta<CoordinateType>& aOther) : 
            x{ aOther.dx }, y{ aOther.dy }, z{} {}
        constexpr basic_point(const basic_size<CoordinateType>& aOther) : 
            x{ aOther.cx }, y{ aOther.cy }, z{} {}
        // operations
    public:
        basic_delta<coordinate_type> to_delta() const { return basic_delta<CoordinateType>{ x, y }; }
        basic_size<coordinate_type> to_size() const { return basic_size<CoordinateType>{ x, y }; }
        basic_vector<coordinate_type, 2> to_vec2() const { return basic_vector<coordinate_type, 2>{ x, y }; }
        basic_vector<coordinate_type, 3> to_vec3() const { return basic_vector<coordinate_type, 3>{ x, y, z }; }
        explicit operator basic_delta<coordinate_type>() const { return to_delta(); }
        explicit operator basic_size<coordinate_type>() const { return to_size(); }
        auto operator<=>(const basic_point&) const = default;
        basic_point& operator+=(const basic_point& aOther) { x += aOther.x; y += aOther.y; z += aOther.z; return *this; }
        basic_point& operator-=(const basic_point& aOther) { x -= aOther.x; y -= aOther.y; z -= aOther.z; return *this; }
        basic_point& operator*=(const basic_point& aOther) { x *= aOther.x; y *= aOther.y; z *= aOther.z; return *this; }
        basic_point& operator/=(const basic_point& aOther) { x /= aOther.x; y /= aOther.y; if (aOther.z != static_cast<coordinate_type>(0.0)) z /= aOther.z; return *this; }
        basic_point& operator+=(coordinate_type amount) { x += amount; y += amount; return *this; }
        basic_point& operator-=(coordinate_type amount) { x -= amount; y -= amount; return *this; }
        basic_point& operator*=(coordinate_type amount) { x *= amount; y *= amount; return *this; }
        basic_point& operator/=(coordinate_type amount) { x /= amount; y /= amount; return *this; }
        basic_point& operator+=(const basic_delta<coordinate_type>& aOther) { x += static_cast<coordinate_type>(aOther.dx); y += static_cast<coordinate_type>(aOther.dy); z += static_cast<coordinate_type>(aOther.dz); return *this; }
        basic_point& operator-=(const basic_delta<coordinate_type>& aOther) { x -= static_cast<coordinate_type>(aOther.dx); y -= static_cast<coordinate_type>(aOther.dy); z -= static_cast<coordinate_type>(aOther.dz); return *this; }
        basic_point& operator+=(const basic_size<coordinate_type>& aOther) { x += static_cast<coordinate_type>(aOther.cx); y += static_cast<coordinate_type>(aOther.cy); z += static_cast<coordinate_type>(aOther.cz); return *this; }
        basic_point& operator-=(const basic_size<coordinate_type>& aOther) { x -= static_cast<coordinate_type>(aOther.cx); y -= static_cast<coordinate_type>(aOther.cy); z -= static_cast<coordinate_type>(aOther.cz); return *this; }
        basic_point operator-() const { return basic_point{ -x, -y, -z }; }
        basic_point abs() const { return basic_point{ std::abs(x), std::abs(y), std::abs(z) }; }
        basic_point ceil() const { return basic_point{ std::ceil(x), std::ceil(y), std::ceil(z) }; }
        basic_point floor() const { return basic_point{ std::floor(x), std::floor(y), std::floor(z) }; }
        basic_point round() const { return basic_point{ std::round(x), std::round(y), std::round(z) }; }
        basic_point min(const basic_point& aOther) const { return basic_point{ std::min(x, aOther.x), std::min(y, aOther.y), std::min(z, aOther.z) }; }
        basic_point max(const basic_point& aOther) const { return basic_point{ std::max(x, aOther.x), std::max(y, aOther.y), std::max(z, aOther.z) }; }
        basic_point min_max(const basic_point& aOther) const { return basic_point{ std::min(x, aOther.x), std::max(y, aOther.y), aOther.z }; }
        basic_point max_min(const basic_point& aOther) const { return basic_point{ std::max(x, aOther.x), std::min(y, aOther.y), aOther.z }; }
        basic_point mid(const basic_point& aOther) const { return basic_point{ (x + aOther.x) / static_cast<coordinate_type>(2.0), (y + aOther.y) / static_cast<coordinate_type>(2.0), (z + aOther.z) / static_cast<coordinate_type>(2.0) }; }
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

    using point = basic_point<coordinate>;

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
        // types
    public:
        using abstract_type   = basic_box_areas; // todo: abstract type
        using dimension_type  = DimensionType;
        using coordinate_type = dimension_type;
        using point_type      = basic_point<coordinate_type>;
        using size_type       = basic_size<dimension_type>;
        // construction
    public:
        basic_box_areas() : left{}, top{}, right{}, bottom{} {}
        basic_box_areas(dimension_type all) : left(all), top(all), right(all), bottom(all) {}
        basic_box_areas(dimension_type left, dimension_type top, dimension_type right, dimension_type bottom) : left(left), top(top), right(right), bottom(bottom) {}
        basic_box_areas(dimension_type leftRight, dimension_type topBottom) : left(leftRight), top(topBottom), right(leftRight), bottom(topBottom) {}
        basic_box_areas(const size_type& aDimensions) : basic_box_areas{ aDimensions.cx, aDimensions.cy } {};
        basic_box_areas(const point_type& aTopLeft, const point_type& aBottomRight) : left(aTopLeft.x), top(aTopLeft.y), right(aBottomRight.x), bottom(aBottomRight.y) {}
        template <typename DimensionType2>
        basic_box_areas(const basic_box_areas<DimensionType2>& aOther) :
            left(static_cast<dimension_type>(aOther.left)), top(static_cast<dimension_type>(aOther.top)), right(static_cast<dimension_type>(aOther.right)), bottom(static_cast<dimension_type>(aOther.bottom)) {}
        // operations
    public:
        auto operator<=>(const basic_box_areas&) const = default;
        basic_box_areas operator-() const { return basic_box_areas{ -left, -top, -right, -bottom }; }
        basic_box_areas& operator+=(const basic_box_areas& aOther) { left += aOther.left; top += aOther.top; right += aOther.right; bottom += aOther.bottom; return *this; }
        basic_box_areas& operator+=(dimension_type amount) { left += amount; top += amount; right += amount; bottom += amount; return *this; }
        basic_box_areas& operator-=(const basic_box_areas& aOther) { left -= aOther.left; top -= aOther.top; right -= aOther.right; bottom -= aOther.bottom; return *this; }
        basic_box_areas& operator-=(dimension_type amount) { left -= amount; top -= amount; right -= amount; bottom -= amount; return *this; }
        basic_box_areas& operator*=(const basic_box_areas& aOther) { left *= aOther.left; top *= aOther.top; right *= aOther.right; bottom *= aOther.bottom; return *this; }
        basic_box_areas& operator*=(dimension_type amount) { left *= amount; top *= amount; right *= amount; bottom *= amount; return *this; }
        basic_box_areas& operator/=(const basic_box_areas& aOther) { left /= aOther.left; top /= aOther.top; right /= aOther.right; bottom /= aOther.bottom; return *this; }
        basic_box_areas& operator/=(dimension_type amount) { left /= amount; top /= amount; right /= amount; bottom /= amount; return *this; }
    public:
        basic_box_areas with_left(dimension_type d) const { return basic_box_areas{ d, top, right, bottom }; }
        basic_box_areas with_top(dimension_type d) const { return basic_box_areas{ left, d, right, bottom }; }
        basic_box_areas with_right(dimension_type d) const { return basic_box_areas{ left, top, d, bottom }; }
        basic_box_areas with_bottom(dimension_type d) const { return basic_box_areas{ left, top, right, d }; }
    public:
        point_type top_left() const { return point_type{ left, top }; }
        point_type bottom_right() const { return point_type{ right, bottom }; }
        size_type size() const { return size_type{ left + right, top + bottom }; }
    public:
        basic_box_areas ceil() const { return basic_box_areas{ std::ceil(left), std::ceil(top), std::ceil(right), std::ceil(bottom) }; }
        basic_box_areas floor() const { return basic_box_areas{ std::floor(left), std::floor(top), std::floor(right), std::floor(bottom) }; }
        basic_box_areas round() const { return basic_box_areas{ std::round(left), std::round(top), std::round(right), std::round(bottom) }; }
        // attributes
    public:
        dimension_type left;
        dimension_type top;
        dimension_type right;
        dimension_type bottom;
    };

    using box_areas = basic_box_areas<double>;

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

    enum class scale_to_fit : std::uint32_t
    {
        Minimum         = 0x00000001,
        Maximum         = 0x00000002
    };

    // Primarily for 2D use so 3D support limited
    template <typename CoordinateType, logical_coordinate_system CoordinateSystem = logical_coordinate_system::AutomaticGui>
    class basic_rect :
        public basic_point<CoordinateType>,
        public basic_size<CoordinateType>
    {
        // types
    public:
        using abstract_type   = basic_rect; // todo: abstract type
        using coordinate_type = CoordinateType;
        using dimension_type  = coordinate_type;
    private:
        using delta_type      = basic_delta<coordinate_type>;
        using size_type       = basic_size<dimension_type>;
        using point_type      = basic_point<coordinate_type>;
        using padding_type    = basic_box_areas<dimension_type>;
    public:
        using point_type::x;
        using point_type::y;
        using point_type::z;
        using size_type::cx;
        using size_type::cy;
        using size_type::cz;
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
        basic_rect(const aabb_2df& aBoundingBox) : basic_rect{ aBoundingBox.min.x, aBoundingBox.min.y, aBoundingBox.max.x, aBoundingBox.max.y } {}
        basic_rect(const basic_box_areas<coordinate_type>& aBoxAreas) : basic_rect{ aBoxAreas.top_left(), aBoxAreas.bottom_right() } {}
    public:
        template <typename CoordinateType2, logical_coordinate_system CoordinateSystem2>
        basic_rect(const basic_rect<CoordinateType2, CoordinateSystem2>& aOther) : point_type{ aOther }, size_type{ aOther } {}
        // assignment
    public:
        template <typename CoordinateType2, logical_coordinate_system CoordinateSystem2>
        basic_rect& operator=(const basic_rect<CoordinateType2, CoordinateSystem2>& aOther) { static_cast<point_type&>(*this) = aOther; static_cast<size_type&>(*this) = aOther; epsilon = aOther.epsilon; return *this; }
        basic_rect& operator=(const point_type& coordinates) { static_cast<point_type&>(*this) = coordinates; return *this; }
        basic_rect& operator=(const size_type& dimensions) { static_cast<size_type&>(*this) = dimensions; return *this; }
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
        point_type top_left() const { return point_type(left(), top(), z); }
        point_type top_right() const { return point_type(right(), top(), z); }
        point_type bottom_left() const { return point_type(left(), bottom(), z); }
        point_type bottom_right() const { return point_type(right(), bottom(), z); }
        dimension_type width() const { return cx; }
        dimension_type height() const { return cy; }
        bool operator==(const basic_rect& aOther) const { return x == aOther.x && y == aOther.y && cx == aOther.cx && cy == aOther.cy; }
        bool operator!=(const basic_rect& aOther) const { return !operator==(aOther); }
        basic_rect& operator*=(const basic_rect& aOther) { position() *= aOther.position(); extents() *= aOther.extents(); return *this; }
        basic_rect& operator*=(const size_type& size) { position() *= size; extents() *= size; return *this; }
        basic_rect& operator*=(dimension_type value) { position() *= value; extents() *= value; return *this; }
        basic_rect& operator/=(const basic_rect& aOther) { position() /= aOther.position(); extents() /= aOther.extents(); return *this; }
        basic_rect& operator/=(const size_type& size) { position() /= size; extents() /= size; return *this; }
        basic_rect& operator/=(dimension_type value) { position() /= value; extents() /= value; return *this; }
        bool contains_x(const point_type& point) const { return point.x >= left() && point.x < right(); }
        bool contains_y(const point_type& point) const { if constexpr (gui) return point.y >= top() && point.y < bottom(); else return point.y >= bottom() && point.y < top(); }
        bool contains(const point_type& point) const { return contains_x(point) && contains_y(point); }
        bool contains(const basic_rect& aOther) const
        { 
            if constexpr (gui) 
                return aOther.left() >= left() && aOther.right() <= right() && aOther.top() >= top() && aOther.bottom() <= bottom(); 
            else
                return aOther.left() >= left() && aOther.right() <= right() && aOther.bottom() >= bottom() && aOther.top() <= top();
        }
        point_type center() const 
        { 
            if constexpr (gui)
                return point_type{ left() + static_cast<coordinate_type>(width()) / two, top() + static_cast<coordinate_type>(height()) / two };
            else
                return point_type{ left() + static_cast<coordinate_type>(width()) / two, bottom() + static_cast<coordinate_type>(height()) / two };
        }
        basic_rect& translate(const point_type& aOffset) { x += aOffset.x; y += aOffset.y; return *this; }
        basic_rect translated(const point_type& aOffset) const { basic_rect result = *this; result.x += aOffset.x; result.y += aOffset.y; return result; }
        basic_rect& indent(const point_type& aOffset) { x += aOffset.x; y += aOffset.y; cx -= aOffset.x; cy -= aOffset.y; return *this; }
        basic_rect& inflate(const delta_type& delta) { x -= delta.dx; y -= delta.dy; cx += delta.dx * two; cy += delta.dy * two; return *this; }
        basic_rect& inflate(const size_type& size) { return inflate(delta_type(size.cx, size.cy)); }
        basic_rect& inflate(const padding_type& padding) { return inflate(padding.left, padding.top, padding.right, padding.bottom); }
        basic_rect& inflate(coordinate_type dx, coordinate_type dy) { return inflate(delta_type(dx, dy)); }
        basic_rect& inflate(coordinate_type left, coordinate_type top, coordinate_type right, coordinate_type bottom) { x -= left; y -= top; cx += (left + right); cy += (top + bottom); return *this; }
        template <typename... Args>
        basic_rect inflated(Args&&... aArgs) const
        {
            auto result = *this;
            result.inflate(std::forward<Args>(aArgs)...);
            return result;
        }
        basic_rect& deflate(const delta_type& delta) { return inflate(-delta); }
        basic_rect& deflate(const size_type& size) { return inflate(-size.cx, -size.cy); }
        basic_rect& deflate(const padding_type& padding) { return deflate(padding.left, padding.top, padding.right, padding.bottom); }
        basic_rect& deflate(coordinate_type dx, coordinate_type dy) { return inflate(-dx, -dy); }
        basic_rect& deflate(coordinate_type left, coordinate_type top, coordinate_type right, coordinate_type bottom) { return inflate(-left, -top, -right, -bottom); }
        template <typename... Args>
        basic_rect deflated(Args&&... aArgs) const
        {
            auto result = *this;
            result.deflate(std::forward<Args>(aArgs)...);
            return result;
        }
        template <typename... T>
        friend basic_rect inflate_rect(const basic_rect& aRect, T&&... aAmount)
        {
            auto result = aRect;
            result.inflate(std::forward<T>(aAmount)...);
            return result;
        }
        template <typename... T>
        friend basic_rect deflate_rect(const basic_rect& aRect, T&&... aAmount)
        {
            auto result = aRect;
            result.deflate(std::forward<T>(aAmount)...);
            return result;
        }
        basic_rect intersection(const basic_rect& aOther) const
        {
            if constexpr (gui)
            {
                basic_rect candidate{ top_left().max(aOther.top_left()), bottom_right().min(aOther.bottom_right()) };
                if (contains(candidate.center()) && aOther.contains(candidate.center()))
                    return candidate;
                else
                    return basic_rect{};
            }
            else
            {
                basic_rect candidate{ bottom_left().max(aOther.bottom_left()), top_right().min(aOther.top_right()) };
                if (contains(candidate.center()) && aOther.contains(candidate.center()))
                    return candidate;
                else
                    return basic_rect{};
            }
        }
        bool intersects(const basic_rect& aOther) const
        {
            auto const& test = intersection(aOther);
            return test != basic_rect{};
        }
        basic_rect& center_on(const basic_rect& aOther)
        {
            *this = centered_on(aOther);
            return *this;
        }
        basic_rect centered_on(const basic_rect& aOther) const
        {
            auto result = *this;
            result.position() += (aOther.center() - result.center());
            return result;
        }
        basic_rect& scale_to_fit(const basic_rect& aOther, neogfx::scale_to_fit aScaleToFit = neogfx::scale_to_fit::Minimum)
        {
            *this = scaled_to_fit(aOther, aScaleToFit);
            return *this;
        }
        basic_rect scaled_to_fit(const basic_rect& aOther, neogfx::scale_to_fit aScaleToFit = neogfx::scale_to_fit::Minimum) const
        {
            auto result = *this;
            scalar coefficient = 1.0;
            switch (aScaleToFit)
            {
            case neogfx::scale_to_fit::Minimum:
                if (result.cx >= result.cy)
                    coefficient = aOther.cx / result.cx;
                else
                    coefficient = aOther.cy / result.cy;
                break;
            case neogfx::scale_to_fit::Maximum:
                coefficient = std::max(aOther.cx / result.cx, aOther.cy / result.cy);
                break;
            default:
                // do nothing
                break;
            }
            result.cx *= coefficient;
            result.cy *= coefficient;
            result.center_on(aOther);
            return result;
        }
        basic_rect& combine(const basic_rect& aOther)
        {
            *this = combined(aOther);
            return *this;
        }
        basic_rect combined(const basic_rect& aOther) const
        {
            if constexpr (gui)
                return basic_rect{ top_left().min(aOther.top_left()), bottom_right().max(aOther.bottom_right()) };
            else
                return basic_rect{ bottom_left().min(aOther.bottom_left()), top_right().max(aOther.top_right()) };
        }
        basic_rect with_position(const point_type& p) const
        {
            auto result = *this;
            result.position().x = p.x;
            result.position().y = p.y;
            return result;
        }
        basic_rect with_x(coordinate_type c) const
        {
            auto result = *this;
            result.position().x = c;
            return result;
        }
        basic_rect with_y(coordinate_type c) const
        {
            auto result = *this;
            result.position().y = c;
            return result;
        }
        basic_rect with_extents(const size_type& e) const
        {
            auto result = *this;
            result.extents().cx = e.cx;
            result.extents().cy = e.cy;
            return result;
        }
        basic_rect with_cx(dimension_type d) const
        { 
            auto result = *this;
            result.extents().cx = d;
            return result;
        }
        basic_rect with_cy(dimension_type d) const
        { 
            auto result = *this;
            result.extents().cy = d;
            return result;
        }
        basic_rect with_centered_origin() const
        {
            return basic_rect{ point_type{ -extents() / two }, extents() };
        }
        basic_rect ceil() const { return basic_rect{ point_type::ceil(), size_type::ceil() }; }
        basic_rect floor() const { return basic_rect{ point_type::floor(), size_type::floor() }; }
        basic_rect round() const { return basic_rect{ point_type::round(), size_type::round() }; }
        basic_rect with_epsilon(const size_type& aEpsilon = default_epsilon) const
        {
            basic_rect result = *this;
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
        aabb_2df to_aabb_2df() const
        {
            if constexpr (gui)
                return aabb_2df{ top_left().to_vec2().as<float>(), bottom_right().to_vec2().as<float>() };
            else
                return aabb_2df{ bottom_left().to_vec2().as<float>(), top_right().to_vec2().as<float>() };
        }
        basic_vector<coordinate_type, 4> to_vec4() const
        {
            return basic_vector<coordinate_type, 4>{ top_left().x, top_left().y, bottom_right().x, bottom_right().y };
        }
        quad to_quad() const
        {
            return quad{ top_left().to_vec3(), top_right().to_vec3(), bottom_right().to_vec3(), bottom_left().to_vec3() };
        }
        quadf to_quadf() const
        {
            return quadf{ top_left().to_vec3().as<float>(), top_right().to_vec3().as<float>(), bottom_right().to_vec3().as<float>(), bottom_left().to_vec3().as<float>() };
        }
    public:
        template <typename T>
        std::enable_if_t<!std::is_same_v<T, coordinate_type>, basic_rect<T, CoordinateSystem>> as() const
        {
            return basic_rect<T, CoordinateSystem>{ *this };
        }
        template <typename T>
        std::enable_if_t<std::is_same_v<T, coordinate_type>, basic_rect const&> as() const
        {
            return *this;
        }
    public:
        size_type epsilon = size_type{ zero };
    };

    using gui_rect  = basic_rect<coordinate, logical_coordinate_system::AutomaticGui>;
    using game_rect = basic_rect<coordinate, logical_coordinate_system::AutomaticGame>;
    using rect      = gui_rect;

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
        using coordinate_type = CoordinateType;
        using delta_type      = basic_delta<coordinate_type>;
        using point_type      = basic_point<coordinate_type> ;
        using line            = basic_line<coordinate_type> ;
        // construction
    public:
        basic_line(const point_type& a, const point_type& b) : a(a), b(b), d(b - a), m(d.dx != 0 ? d.dy / d.dx : 0), c(a.y - m * a.x) {}
        template <typename CoordinateType2>
        basic_line(const basic_line<CoordinateType2>& aOther) :
            a(static_cast<point_type>(aOther.a)), b(static_cast<point_type>(aOther.b)), d(b - a), m(d.dx != 0 ? d.dy / d.dx : 0), c(a.y - m * a.x) {}
        // operations
    public:
        bool operator==(const basic_line& aOther) const { return (a == aOther.a && b == aOther.b) || (a == aOther.b & b == aOther.a); }
        bool operator!=(const basic_line& aOther) const { return !operator==(aOther); }
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
        bool intersection(const line& aOther, point& pointOfIntersection) const
        {
            if (d.dx == 0 || aOther.d.dx == 0)
            {
                if (d.dx == aOther.d.dx)
                    return false;
                else if (d.dx == 0)
                {
                    pointOfIntersection.x = a.x;
                    pointOfIntersection.y = aOther.m * a.x + aOther.c;
                }
                else // (aOther.d.dx == 0)
                {
                    pointOfIntersection.x = aOther.a.x;
                    pointOfIntersection.y = m * aOther.a.x + c;
                }
            }
            else if (d.dy == 0 || aOther.d.dy == 0)
            {
                if (d.dy == aOther.d.dy)
                    return false;
                else if (d.dy == 0)
                {
                    pointOfIntersection.y = a.y;
                    pointOfIntersection.x = (a.y - aOther.c) / aOther.m;
                }
                else // (aOther.d.dy == 0)
                {
                    pointOfIntersection.y = aOther.a.y;
                    pointOfIntersection.x = (aOther.a.y - c) / m;
                }
            }
            else
            {
                if (m == aOther.m)
                    return false;
                pointOfIntersection.x = (aOther.c - c) / (m - aOther.m);
                pointOfIntersection.y = pointOfIntersection.x * m + c;
            }
            return within_bounding_rect(pointOfIntersection) && aOther.within_bounding_rect(pointOfIntersection);
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

    using line      = basic_line<coordinate>;

    using size_f32 = basic_size<float>;
    using delta_f32 = basic_delta<float>;
    using point_f32 = basic_point<float>;
    using rect_f32 = basic_rect<float>;

    using size_i32  = basic_size<std::int32_t>;
    using delta_i32 = basic_delta<std::int32_t>;
    using point_i32 = basic_point<std::int32_t>;
    using rect_i32  = basic_rect<std::int32_t>;

    using size_u32  = basic_size<std::uint32_t>;
    using delta_u32 = basic_delta<std::uint32_t>;
    using point_u32 = basic_point<std::uint32_t>;
    using rect_u32  = basic_rect<std::uint32_t>;

    template <typename DimensionType>
    using basic_margin = basic_box_areas<DimensionType>;

    template <typename DimensionType>
    using basic_border = basic_box_areas<DimensionType>;

    template <typename DimensionType>
    using basic_padding = basic_box_areas<DimensionType>;

    using margin  = basic_margin<dimension> ;
    using border  = basic_border<dimension> ;
    using padding = basic_padding<dimension> ;

    using optional_dimension = optional<dimension>;
    using optional_angle     = optional<angle>;
    using optional_delta     = optional<delta>;
    using optional_point     = optional<point>;
    using optional_size      = optional<size>;
    using optional_rect      = optional<rect>;
    using optional_delta_u32 = optional<delta_u32>;
    using optional_point_u32 = optional<point_u32>;
    using optional_size_u32  = optional<size_u32>;
    using optional_rect_u32  = optional<rect_u32>;
    using optional_margin    = optional<margin>;
    using optional_border    = optional<border>;
    using optional_padding   = optional<padding>;
    using optional_vector1   = optional<vector1>;
    using optional_vector2   = optional<vector2>;
    using optional_vector3   = optional<vector3>;
    using optional_vector4   = optional<vector4>;

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
    template <typename T>
    struct hash<neogfx::basic_point<T>>
    {
        size_t operator()(const neogfx::basic_point<T>& aPoint) const
        {
            return std::hash<T>()(aPoint.x) ^ std::hash<T>()(aPoint.y);
        }
    };

    template <typename T>
    struct hash<neogfx::basic_size<T>>
    {
        size_t operator()(const neogfx::basic_size<T>& aSize) const
        {
            return std::hash<T>()(aSize.cx) ^ std::hash<T>()(aSize.cy);
        }
    };

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
