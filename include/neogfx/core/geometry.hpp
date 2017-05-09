// geometry.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <array>
#include <type_traits>
#include <ostream>
#include <boost/optional.hpp>
#include "numerical.hpp"

namespace neogfx
{ 
	typedef double coordinate_value_type;
	typedef coordinate_value_type coordinate;
	typedef coordinate_value_type dimension;
	typedef coordinate_value_type coordinate_delta;

	typedef std::array<coordinate, 3> xyz;

	template <typename CoordinateType>
	class basic_delta 
	{ 
		// types
	public:
		typedef CoordinateType coordinate_type;
		// construction
	public:
		basic_delta() : dx(0), dy(0) {}
		basic_delta(coordinate_type dx, coordinate_type dy) : dx(dx), dy(dy) {}
		basic_delta(coordinate_type aSquareDelta) : dx(aSquareDelta), dy(aSquareDelta) {}
		template <typename CoordinateType2>
		basic_delta(const basic_delta<CoordinateType2>& other) :
			dx(static_cast<CoordinateType>(other.dx)), dy(static_cast<CoordinateType>(other.dy)) {}
		// operations
	public:
		bool operator==(const basic_delta& other) const { return dx == other.dx && dy == other.dy; }
		bool operator!=(const basic_delta& other) const { return !operator==(other); }
		basic_delta& operator+=(const basic_delta& other) { dx += other.dx; dy += other.dy; return *this; }
		basic_delta& operator-=(const basic_delta& other) { dx -= other.dx; dy -= other.dy; return *this; }
		basic_delta& operator*=(const basic_delta& other) { dx *= other.dx; dy *= other.dy; return *this; }
		basic_delta& operator/=(const basic_delta& other) { dx /= other.dx; dy /= other.dy; return *this; }
		basic_delta operator-() const { return basic_delta(-dx, -dy); }
		basic_delta ceil() const { return basic_delta(std::ceil(dx), std::ceil(dy)); }
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
		basic_size() : cx(0), cy(0) {}
		basic_size(dimension_type cx, dimension_type cy) : cx(cx), cy(cy) {}
		basic_size(dimension_type aSquareDimension) : cx(aSquareDimension), cy(aSquareDimension) {}
		template <typename CoordinateType2>
		basic_size(const basic_size<CoordinateType2>& other) :
			cx(static_cast<CoordinateType>(other.cx)), cy(static_cast<CoordinateType>(other.cy)) {}
		basic_size(const basic_delta<CoordinateType>& other) : cx(other.dx), cy(other.dy) {}
		// operations
	public:
		delta_type to_delta() const { return delta_type(cx, cy); }
		bool empty() const { return cx == 0 || cy == 0; }
		bool operator==(const basic_size& other) const { return cx == other.cx && cy == other.cy; }
		bool operator!=(const basic_size& other) const { return !operator==(other); }
		basic_size& operator+=(const basic_size& other) { cx += other.cx; cy += other.cy; return *this; }
		basic_size& operator+=(const basic_delta<CoordinateType>& other) { cx += other.dx; cy += other.dy; return *this; }
		basic_size& operator+=(const dimension_type other) { cx += other; cy += other; return *this; }
		basic_size& operator-=(const basic_size& other) { cx -= other.cx; cy -= other.cy; return *this; }
		basic_size& operator-=(const basic_delta<CoordinateType>& other) { cx -= other.dx; cy -= other.dy; return *this; }
		basic_size& operator-=(const dimension_type other) { cx -= other; cy -= other; return *this; }
		basic_size& operator*=(const basic_size& other) { cx *= other.cx; cy *= other.cy; return *this; }
		basic_size& operator*=(const dimension_type other) { cx *= other; cy *= other; return *this; }
		basic_size& operator/=(const basic_size& other) { cx /= other.cx; cy /= other.cy; return *this; }
		basic_size& operator/=(const dimension_type other) { cx /= other; cy /= other; return *this; }
		basic_size ceil() const { return basic_size(std::ceil(cx), std::ceil(cy)); }
		basic_size floor() const { return basic_size(std::floor(cx), std::floor(cy)); }
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
	class basic_point
	{
		// types
	public:
		typedef CoordinateType coordinate_type;
		// construction
	public:
		basic_point() : x(0), y(0) {}
		template <typename Scalar>
		basic_point(const basic_vector<Scalar, 2>& other) : x(static_cast<coordinate_type>(other[0])), y(static_cast<coordinate_type>(other[1])) {}
		basic_point(CoordinateType x, CoordinateType y) : x(x), y(y) {}
		template <typename CoordinateType2>
		basic_point(const basic_point<CoordinateType2>& other) :
			x(static_cast<CoordinateType>(other.x)), y(static_cast<CoordinateType>(other.y)) {}
		basic_point(const basic_delta<CoordinateType>& other) : x(other.dx), y(other.dy) {}
		basic_point(const basic_size<CoordinateType>& other) : x(other.cx), y(other.cy) {}
		// operations
	public:
		basic_vector<coordinate_type, 2> to_vector() const { return basic_vector<coordinate_type, 2>(x, y); }
		basic_vector<coordinate_type, 3> to_vector3(coordinate_type fill = 1.0) const { return basic_vector<coordinate_type, 3>(x, y, fill); }
		bool operator==(const basic_point& other) const { return x == other.x && y == other.y; }
		bool operator!=(const basic_point& other) const { return !operator==(other); }
		basic_point& operator+=(const basic_point& other) { x += other.x; y += other.y; return *this; }
		basic_point& operator-=(const basic_point& other) { x -= other.x; y -= other.y; return *this; }
		basic_point& operator*=(const basic_point& other) { x *= other.x; y *= other.y; return *this; }
		basic_point& operator/=(const basic_point& other) { x /= other.x; y /= other.y; return *this; }
		basic_point& operator+=(const basic_delta<coordinate_type>& other) { x += static_cast<coordinate_type>(other.dx); y += static_cast<coordinate_type>(other.dy); return *this; }
		basic_point& operator-=(const basic_delta<coordinate_type>& other) { x -= static_cast<coordinate_type>(other.dx); y -= static_cast<coordinate_type>(other.dy); return *this; }
		basic_point& operator+=(const basic_size<coordinate_type>& other) { x += static_cast<coordinate_type>(other.cx); y += static_cast<coordinate_type>(other.cy); return *this; }
		basic_point& operator-=(const basic_size<coordinate_type>& other) { x -= static_cast<coordinate_type>(other.cx); y -= static_cast<coordinate_type>(other.cy); return *this; }
		basic_point operator-() const { return basic_point(-x, -y); }
		basic_point ceil() const { return basic_point(std::ceil(x), std::ceil(y)); }
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

	class basic_surface;

	template <typename CoordinateType>
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
		// construction
	public:
		basic_rect() {}
		basic_rect(const point_type& coordinates, const size_type& dimensions) : point_type(coordinates), size_type(dimensions) {}
		basic_rect(const point_type& topLeft, const point_type& bottomRight) : 
			point_type(topLeft), 
			size_type(static_cast<CoordinateType>(bottomRight.x - topLeft.x), static_cast<CoordinateType>(bottomRight.y - topLeft.y)) {}
		basic_rect(const size_type& dimensions) : point_type(), size_type(dimensions) {}
		basic_rect(coordinate_type x, coordinate_type y, dimension_type cx, dimension_type cy) : point_type(x, y), size_type(cx, cy) {}
		template <typename CoordinateType2>
		basic_rect(const basic_rect<CoordinateType2>& other) : point_type(other), size_type(other) {}
		// assignment
	public:
		basic_rect& operator=(const point_type& coordinates) { static_cast<point_type&>(*this) = coordinates; return *this; }
		basic_rect& operator=(const size_type& dimensions) { static_cast<size_type&>(*this) = dimensions; return *this; }
		// operations
	public:
		basic_vector<basic_vector<coordinate_type, 2>, 4> to_vector() const { return basic_vector<basic_vector<coordinate_type, 2>, 4>(top_left().to_vector(), top_right().to_vector(), bottom_right().to_vector(), bottom_left().to_vector()); }
		const point_type& position() const { return *this; }
		point_type& position() { return *this; }
		const size_type& extents() const { return *this; }
		size_type& extents() { return *this; }
		coordinate_type left() const { return x; }
		coordinate_type top() const { return y; }
		coordinate_type right() const { return x + cx; }
		coordinate_type bottom() const { return y + cy; }
		point_type top_left() const { return point_type(x, y); }
		point_type top_right() const { return point_type(x + cx, y); }
		point_type bottom_left() const { return point_type(x, y + cy); }
		point_type bottom_right() const { return point_type(x + cx, y + cy); }
		dimension_type width() const { return cx; }
		dimension_type height() const { return cy; }
		bool operator==(const basic_rect& other) const { return x == other.x && y == other.y && cx == other.cx && cy == other.cy; }
		bool operator!=(const basic_rect& other) const { return !operator==(other); }
		basic_rect& operator*=(const basic_rect& other) { position() *= other.position(); extents() *= other.extents(); return *this; }
		basic_rect& operator*=(const size_type& size) { position() *= size; extents() *= size; return *this; }
		basic_rect& operator/=(const basic_rect& other) { position() /= other.position(); extents() /= other.extents(); return *this; }
		basic_rect& operator/=(const size_type& size) { position() /= size; extents() /= size; return *this; }
		bool contains(const point_type& point) const { return point.x >= left() && point.y >= top() && point.x < right() && point.y < bottom(); }
		bool contains_x(const point_type& point) const { return point.x >= left() && point.x < right(); }
		bool contains_y(const point_type& point) const { return point.y >= top() && point.y < bottom(); }
		bool contains(const basic_rect& other) const { return other.left() >= left() && other.top() >= top() && other.right() <= right() && other.bottom() <= bottom(); }
		point_type centre() const { return point_type(left() + static_cast<CoordinateType>(width() / 2), top() + static_cast<CoordinateType>(height() / 2)); }
		basic_rect& move(const point_type& aOffset) { x += aOffset.x; y += aOffset.y; return *this; }
		basic_rect& inflate(const delta_type& delta) { x -= delta.dx; y -= delta.dy; cx += delta.dx * static_cast<CoordinateType>(2); cy += delta.dy * static_cast<CoordinateType>(2); return *this; }
		basic_rect& inflate(const size_type& size) { return inflate(delta_type(size.cx, size.cy)); }
		basic_rect& inflate(CoordinateType dx, CoordinateType dy) { return inflate(delta_type(dx, dy)); }
		basic_rect& deflate(const delta_type& delta) { return inflate(-delta); }
		basic_rect& deflate(const size_type& size) { return inflate(-size.cx, -size.cy); }
		basic_rect& deflate(CoordinateType dx, CoordinateType dy) { return inflate(-dx, -dy); }
		basic_rect intersection(const basic_rect& other) const
		{
			point_type topLeft(std::max(left(), other.left()), std::max(top(), other.top()));
			point_type bottomRight(std::min(right(), other.right()), std::min(bottom(), other.bottom()));
			basic_rect candidate(topLeft, bottomRight);
			if (contains(candidate.centre()) && other.contains(candidate.centre()))
				return candidate;
			else
				return basic_rect();
		}
		basic_rect combine(const basic_rect& other) const
		{
			point_type topLeft(std::min(left(), other.left()), std::min(top(), other.top()));
			point_type bottomRight(std::max(right(), other.right()), std::max(bottom(), other.bottom()));
			return basic_rect(topLeft, bottomRight);
		}
	};

	typedef basic_rect<coordinate> rect;

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator*(const basic_rect<CoordinateType>& left, const basic_rect<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
		ret *= right;
		return ret;
	}

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator*(const basic_rect<CoordinateType>& left, const basic_size<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
		ret *= right;
		return ret;
	}

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator/(const basic_rect<CoordinateType>& left, const basic_rect<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
		ret /= right;
		return ret;
	}

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator/(const basic_rect<CoordinateType>& left, const basic_size<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
		ret /= right;
		return ret;
	}

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator+(const basic_rect<CoordinateType>& left, const basic_point<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
		ret.basic_point::operator+=(right);
		return ret;
	}

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator+(const basic_rect<CoordinateType>& left, const basic_delta<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
		ret.basic_size::operator+=(right);
		return ret;
	}

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator-(const basic_rect<CoordinateType>& left, const basic_point<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
		ret.basic_point::operator-=(right);
		return ret;
	}

	template <typename CoordinateType>
	inline basic_rect<CoordinateType> operator-(const basic_rect<CoordinateType>& left, const basic_delta<CoordinateType>& right)
	{
		basic_rect<CoordinateType> ret = left;
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
		basic_margins& operator+=(dimension_type amount) { left += amount; top += amount; right += amount; bottom += amount; return *this; }
		basic_margins& operator-=(dimension_type amount) { left -= amount; top -= amount; right -= amount; bottom -= amount; return *this; }
	public:
		point_type top_left() const { return point_type(left, top); }
		size_type size() const { return size_type(left + right, top + bottom); }
		// attributes
	public:
		dimension_type left;
		dimension_type top;
		dimension_type right;
		dimension_type bottom;
	};

	typedef basic_margins<dimension> margins;

	typedef boost::optional<dimension> optional_dimension;
	typedef boost::optional<angle> optional_angle;
	typedef boost::optional<point> optional_point;
	typedef boost::optional<size> optional_size;
	typedef boost::optional<rect> optional_rect;
	typedef boost::optional<margins> optional_margins;
	typedef boost::optional<vector1> optional_vector1;
	typedef boost::optional<vector2> optional_vector2;
	typedef boost::optional<vector3> optional_vector3;
	typedef boost::optional<vector4> optional_vector4;

	enum units_e
	{
		UnitsPixels,
		UnitsPoints,
		UnitsPicas,
		UnitsEms,
		UnitsMillimetres,
		UnitsMillimeters = UnitsMillimetres,
		UnitsCentimetres,
		UnitsCentimeters = UnitsCentimetres,
		UnitsInches,
		UnitsPercentage
	};

	class i_device_resolution
	{
	public:
		virtual dimension horizontal_dpi() const = 0;
		virtual dimension vertical_dpi() const = 0;
	};

	class i_device_metrics : public i_device_resolution
	{
	public:
		virtual bool metrics_available() const = 0;
		virtual size extents() const = 0;
		virtual dimension em_size() const = 0;
	};

	class i_units_context
	{
	public:
		virtual const i_device_metrics& device_metrics() const = 0;
		virtual units_e units() const = 0;
		virtual units_e set_units(units_e aUnits) const = 0;
	};

	class units_context : public i_units_context
	{
	public:
		units_context(const i_device_metrics& aDeviceMetrics);
	public:
		virtual const i_device_metrics& device_metrics() const;
		virtual units_e units() const;
		virtual units_e set_units(units_e aUnits) const;
	private:
		const i_device_metrics& iDeviceMetrics;
		mutable units_e iUnits;
	};

	class units_converter
	{
	public:
		struct bad_parameter : std::logic_error { bad_parameter() : std::logic_error("neogfx::units_converter::bad_parameter") {} };
		struct device_metrics_unavailable : std::logic_error { device_metrics_unavailable() : std::logic_error("neogfx::units_converter::device_metrics_unavailable") {} };
		// construction
	public:
		units_converter() = delete;
		units_converter(const i_units_context& aContext);
		~units_converter();
		// operations
	public:
		units_e saved_units() const;
		units_e units() const;
		units_e set_units(units_e aUnits) const;
	public:
		vector2 to_device_units(const vector2& aValue) const;
		dimension to_device_units(dimension aValue) const;
		delta to_device_units(const delta& aValue) const;
		size to_device_units(const size& aValue) const;
		point to_device_units(const point& aValue) const;
		rect to_device_units(const rect& aValue) const;
		margins to_device_units(const margins& aValue) const;
		vector2 to_device_units(const size& aExtents, const vector2& aValue) const;
		dimension to_device_units(const size& aExtents, dimension aValue) const;
		delta to_device_units(const size& aExtents, const delta& aValue) const;
		size to_device_units(const size& aExtents, const size& aValue) const;
		point to_device_units(const size& aExtents, const point& aValue) const;
		rect to_device_units(const size& aExtents, const rect& aValue) const;
		vector2 from_device_units(const vector2& aValue) const;
		dimension from_device_units(dimension aValue) const;
		delta from_device_units(const delta& aValue) const;
		size from_device_units(const size& aValue) const;
		point from_device_units(const point& aValue) const;
		rect from_device_units(const rect& aValue) const;		
		margins from_device_units(const margins& aValue) const;
		vector2 from_device_units(const size& aExtents, const vector2& aValue) const;
		dimension from_device_units(const size& aExtents, dimension aValue) const;
		delta from_device_units(const size& aExtents, const delta& aValue) const;
		size from_device_units(const size& aExtents, const size& aValue) const;
		point from_device_units(const size& aExtents, const point& aValue) const;
		rect from_device_units(const size& aExtents, const rect& aValue) const;		
	public:
		template <typename T, uint32_t D>
		basic_vector<T, D> to_device_units(const basic_vector<T, D>& aValue)
		{
			basic_vector<T, D> result;
			for (uint32_t i = 0; i < D; ++i)
				result[i] = to_device_units(aValue[i]);
			return result;
		}
		template <typename T, uint32_t D>
		basic_vector<T, D> to_device_units(const size& aExtents, const basic_vector<T, D>& aValue)
		{
			basic_vector<T, D> result;
			for (uint32_t i = 0; i < D; ++i)
				result[i] = to_device_units(aExtents, aValue[i]);
			return result;
		}
		template <typename T, uint32_t D>
		basic_vector<T, D> from_device_units(const basic_vector<T, D>& aValue)
		{
			basic_vector<T, D> result;
			for (uint32_t i = 0; i < D; ++i)
				result[i] = from_device_units(aValue[i]);
			return result;
		}
		template <typename T, uint32_t D>
		basic_vector<T, D> from_device_units(const size& aExtents, const basic_vector<T, D>& aValue)
		{
			basic_vector<T, D> result;
			for (uint32_t i = 0; i < D; ++i)
				result[i] = from_device_units(aExtents, aValue[i]);
			return result;
		}
		// attributes
	private:
		const i_units_context& iContext;
		units_e iSavedUnits;
	};

	class scoped_units
	{
		// construction
	public:
		scoped_units(const i_units_context& aUnitsContext, units_e aNewUnits) :
			iUnitsContext1(aUnitsContext), iUnitsContext2(aUnitsContext), iSavedUnits1(aUnitsContext.units()), iSavedUnits2(aUnitsContext.units())
		{
			iUnitsContext1.set_units(aNewUnits);
			iUnitsContext2.set_units(aNewUnits);
		}
		scoped_units(const i_units_context& aUnitsContext1, const i_units_context& aUnitsContext2, units_e aNewUnits) :
			iUnitsContext1(aUnitsContext1), iUnitsContext2(aUnitsContext2), iSavedUnits1(aUnitsContext1.units()), iSavedUnits2(aUnitsContext2.units())
		{
			iUnitsContext1.set_units(aNewUnits);
			iUnitsContext2.set_units(aNewUnits);
		}
		~scoped_units()
		{
			restore_saved_units();
		}
		// operations
	public:
		units_e saved_units() const
		{
			return iSavedUnits1;
		}
		void restore_saved_units()
		{
			iUnitsContext1.set_units(iSavedUnits1);
			iUnitsContext2.set_units(iSavedUnits2);
		}
		// attributes
	private:
		const i_units_context& iUnitsContext1;
		const i_units_context& iUnitsContext2;
		units_e iSavedUnits1;
		units_e iSavedUnits2;
	};

	template<typename T>
	inline T as_units(const i_units_context& aSourceUnitsContext, units_e aUnits, const T& aValue)
	{
		if (aSourceUnitsContext.units() == aUnits)
			return aValue;
		units_converter uc(aSourceUnitsContext);
		uc.set_units(aUnits);
		T result = uc.to_device_units(aValue);
		uc.set_units(uc.saved_units());
		return uc.from_device_units(result);
	}

	template<typename T>
	inline T convert_units(const i_units_context& aSourceUnitsContext, units_e aNewUnits, const T& aValue)
	{
		if (aSourceUnitsContext.units() == aNewUnits)
			return aValue;
		T result = units_converter(aSourceUnitsContext).to_device_units(aValue);
		scoped_units su(aSourceUnitsContext, aNewUnits);
		return units_converter(aSourceUnitsContext).from_device_units(result);
	}

	template<typename T>
	inline T convert_units(const i_units_context& aSourceUnitsContext, const i_units_context& aDestinationUnitsContext, const T& aValue)
	{
		if (aSourceUnitsContext.units() == aDestinationUnitsContext.units())
		{
			if (aSourceUnitsContext.units() != UnitsPixels ||
				(aSourceUnitsContext.device_metrics().horizontal_dpi() == aDestinationUnitsContext.device_metrics().horizontal_dpi() &&
				aSourceUnitsContext.device_metrics().vertical_dpi() == aDestinationUnitsContext.device_metrics().vertical_dpi()))
			{
				return aValue;
			}
		}
		T result = units_converter(aSourceUnitsContext).to_device_units(aValue);
		{
			scoped_units su1(aSourceUnitsContext, UnitsMillimeters);
			result = units_converter(aSourceUnitsContext).from_device_units(aValue);
			scoped_units su2(aDestinationUnitsContext, UnitsMillimeters);
			result = units_converter(aDestinationUnitsContext).to_device_units(result);
		}
		return units_converter(aDestinationUnitsContext).from_device_units(result);
	}

	enum class alignment : uint32_t
	{
		None = 0x0000,
		Left = 0x0001,
		Right = 0x0002,
		Centre = 0x0004,
		Center = alignment::Centre,
		Justify = 0x0008,
		Top = 0x0010,
		VCentre = 0x0020,
		VCenter = alignment::VCentre,
		Bottom = 0x0040,
		Horizontal = Left | Centre | Right | Justify,
		Vertical = Top | VCentre | Bottom
	};

	inline constexpr alignment operator|(alignment aLhs, alignment aRhs)
	{
		return static_cast<alignment>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr alignment operator&(alignment aLhs, alignment aRhs)
	{
		return static_cast<alignment>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

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

