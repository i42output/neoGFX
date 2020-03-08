// numerical.hpp
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
#include <type_traits>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <ostream>
#include <optional>
#include <boost/math/constants/constants.hpp>
#include <neolib/vecarray.hpp>
#include <neogfx/core/swizzle.hpp>
#include <neogfx/core/swizzle_array.hpp>

namespace neogfx
{ 
    namespace math
    {
        using namespace boost::math::constants;

        typedef double scalar;
        typedef double angle;

        namespace constants
        {
            template <typename T>
            constexpr T zero = static_cast<T>(0.0);
            template <typename T>
            constexpr T one = static_cast<T>(1.0);
            template <typename T>
            constexpr T two = static_cast<T>(2.0);
        }

        template <typename T, typename SFINAE = std::enable_if_t<std::is_scalar_v<T>, sfinae>>
        inline T lerp(T aX1, T aX2, double aAmount)
        {
            double x1 = aX1;
            double x2 = aX2;
            return static_cast<T>((x2 - x1) * aAmount + x1);
        }

        inline angle to_rad(angle aDegrees)
        {
            return aDegrees / 180.0 * pi<angle>();
        }

        inline angle to_deg(angle aRadians)
        {
            return aRadians * 180.0 / pi<angle>();
        }

        struct column_vector {};
        struct row_vector {};

        template <typename T, uint32_t Size, typename Type = column_vector, bool IsScalar = std::is_scalar<T>::value>
        class basic_vector;

        /* todo: specializations that use SIMD intrinsics. */
        template <typename T, uint32_t _Size, typename Type>
        class basic_vector<T, _Size, Type, true> : public swizzle_array<basic_vector<T, _Size, Type, true>, T, _Size>
        {
            typedef basic_vector<T, _Size, Type, true> self_type;
            typedef swizzle_array<basic_vector<T, _Size, Type, true>, T, _Size> base_type;
        public:
            typedef self_type abstract_type; // todo: abstract base; std::array?
        public:
            enum : uint32_t { Size = _Size };
            typedef Type type;
        public:
            typedef T value_type;
            typedef basic_vector<value_type, Size, Type> vector_type;
            typedef uint32_t size_type;
            typedef std::array<value_type, Size> array_type;
            typedef typename array_type::const_iterator const_iterator;
            typedef typename array_type::iterator iterator;
        public:
            template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2, Type> type; };
        public:
            basic_vector() : base_type{} {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, typename std::enable_if<Size == 1, SFINAE>::type = 0) : base_type{ {x} } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, typename std::enable_if<Size == 2, SFINAE>::type = 0) : base_type{ {x, y} } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, value_type z, typename std::enable_if<Size == 3, SFINAE>::type = 0) : base_type{ {x, y, z} } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, value_type z, value_type w, typename std::enable_if<Size == 4, SFINAE>::type = 0) : base_type{ { x, y, z, w } } {}
            template <typename... Arguments>
            explicit basic_vector(value_type value, Arguments... aArguments) : base_type{ {value, aArguments...} } {}
            explicit basic_vector(const array_type& v) : base_type{ v } {}
            template <typename V, typename A, uint32_t S, uint32_t... Indexes>
            basic_vector(const swizzle<V, A, S, Indexes...>& aSwizzle) : self_type{ ~aSwizzle } {}
            basic_vector(const self_type& other) : base_type{ other.v } {}
            basic_vector(self_type&& other) : base_type{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if<Size2 < Size, SFINAE>::type = 0) : base_type{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            self_type& operator=(const self_type& other) { v = other.v; return *this; }
            self_type& operator=(self_type&& other) { v = std::move(other.v); return *this; }
            self_type& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neogfx::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static uint32_t size() { return Size; }
            value_type operator[](uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](uint32_t aIndex) { return v[aIndex]; }
            const_iterator begin() const { return v.begin(); }
            const_iterator end() const { return v.end(); }
            iterator begin() { return v.begin(); }
            iterator end() { return v.end(); }
            operator const array_type&() const { return v; }
        public:
            template <typename T2>
            basic_vector<T2, Size, Type> as() const
            {
                return basic_vector<T2, Size, Type>{ *this };
            }
        public:
            bool operator==(const self_type& right) const { return v == right.v; }
            bool operator!=(const self_type& right) const { return v != right.v; }
            self_type& operator+=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
            self_type& operator-=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
            self_type& operator*=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
            self_type& operator/=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
            self_type& operator+=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] += right.v[index]; return *this; }
            self_type& operator-=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] -= right.v[index]; return *this; }
            self_type& operator*=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] *= right.v[index]; return *this; }
            self_type& operator/=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] /= right.v[index]; return *this; }
            self_type operator-() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result.v[index] = -v[index]; return result; }
            self_type scale(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            self_type normalized() const { self_type result; value_type im = constants::one<value_type> / magnitude(); for (uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            self_type min(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            self_type max(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            value_type min() const { value_type result = v[0]; for (uint32_t index = 1; index < Size; ++index) result = std::min(v[index], result); return result; }
            self_type ceil() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            self_type floor() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            self_type round() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const self_type& right) const { value_type total = 0; for (uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const self_type& right) const
            {
                value_type result = constants::zero<value_type>;
                for (uint32_t index = 0; index < Size; ++index)
                    result += (v[index] * right[index]);
                return result;
            }
            template <typename SFINAE = self_type>
            std::enable_if_t<Size == 3, SFINAE> cross(const self_type& right) const
            {
                return self_type{ 
                    base_type::y * right.base_type::z - base_type::z * right.base_type::y, 
                    base_type::z * right.base_type::x - base_type::x * right.base_type::z, 
                    base_type::x * right.base_type::y - base_type::y * right.base_type::x };
            }
        public:
            using base_type::v;
        };

        template <typename T, uint32_t _Size, typename Type>
        class basic_vector<T, _Size, Type, false>
        {
            typedef basic_vector<T, _Size, Type, false> self_type;
        public:
            typedef self_type abstract_type; // todo: abstract base; std::array?
        public:
            enum : uint32_t { Size = _Size };
            typedef Type type;
        public:
            typedef T value_type;
            typedef basic_vector<value_type, Size, Type> vector_type;
            typedef uint32_t size_type;
            typedef std::array<value_type, Size> array_type;
            typedef typename array_type::const_iterator const_iterator;
            typedef typename array_type::iterator iterator;
        public:
            template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2, Type> type; };
        public:
            basic_vector() : v{} {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, typename std::enable_if<Size == 1, SFINAE>::type = 0) : v{ { x } } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, typename std::enable_if<Size == 2, SFINAE>::type = 0) : v{ { x, y } } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, value_type z, typename std::enable_if<Size == 3, SFINAE>::type = 0) : v{ { x, y, z } } {}
            template <typename SFINAE = int>
            explicit basic_vector(value_type x, value_type y, value_type z, value_type w, typename std::enable_if<Size == 4, SFINAE>::type = 0) : v{ { x, y, z, w } } {}
            template <typename... Arguments>
            explicit basic_vector(const value_type& value, Arguments&&... aArguments) : v{ {value, std::forward<Arguments>(aArguments)...} } {}
            template <typename... Arguments>
            explicit basic_vector(value_type&& value, Arguments&&... aArguments) : v{ {std::move(value), std::forward<Arguments>(aArguments)...} } {}
            explicit basic_vector(const array_type& v) : v{ v } {}
            basic_vector(const self_type& other) : v{ other.v } {}
            basic_vector(self_type&& other) : v{ std::move(other.v) } {}
            template <typename T2>
            basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            template <typename T2, uint32_t Size2, typename SFINAE = int>
            basic_vector(const basic_vector<T2, Size2, Type>& other, typename std::enable_if<Size2 < Size, SFINAE>::type = 0) : v{} { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
            self_type& operator=(const self_type& other) { v = other.v; return *this; }
            self_type& operator=(self_type&& other) { v = std::move(other.v); return *this; }
            self_type& operator=(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neogfx::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type{}); return *this; }
        public:
            static uint32_t size() { return Size; }
            const value_type& operator[](uint32_t aIndex) const { return v[aIndex]; }
            value_type& operator[](uint32_t aIndex) { return v[aIndex]; }
            const_iterator begin() const { return v.begin(); }
            const_iterator end() const { return v.end(); }
            iterator begin() { return v.begin(); }
            iterator end() { return v.end(); }
            operator const array_type&() const { return v; }
        public:
            template <typename T2>
            basic_vector<T2, Size, Type> as() const
            {
                return basic_vector<T2, Size, Type>{ *this };
            }
        public:
            bool operator==(const basic_vector& right) const { return v == right.v; }
            bool operator!=(const basic_vector& right) const { return v != right.v; }
            self_type& operator+=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
            self_type& operator-=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
            self_type& operator*=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
            self_type& operator/=(value_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
            self_type& operator+=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] += right.v[index]; return *this; }
            self_type& operator-=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] -= right.v[index]; return *this; }
            self_type& operator*=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] *= right.v[index]; return *this; }
            self_type& operator/=(const self_type& right) { for (uint32_t index = 0; index < Size; ++index) v[index] /= right.v[index]; return *this; }
            self_type operator-() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result.v[index] = -v[index]; return result; }
            self_type scale(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = v[index] * right[index]; return result; }
            value_type magnitude() const { value_type ss = constants::zero<value_type>; for (uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
            self_type normalized() const { self_type result; value_type im = constants::one<value_type> / magnitude(); for (uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] * im; return result; }
            self_type min(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::min(v[index], right.v[index]); return result; }
            self_type max(const self_type& right) const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::max(v[index], right.v[index]); return result; }
            self_type ceil() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::ceil(v[index]); return result; }
            self_type floor() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::floor(v[index]); return result; }
            self_type round() const { self_type result; for (uint32_t index = 0; index < Size; ++index) result[index] = std::round(v[index]); return result; }
            value_type distance(const self_type& right) const { value_type total = constants::zero<value_type>; for (uint32_t index = 0; index < Size; ++index) total += ((v[index] - right.v[index]) * (v[index] - right.v[index])); return std::sqrt(total); }
            value_type dot(const self_type& right) const
            {
                value_type result = constants::zero<value_type>;
                for (uint32_t index = 0; index < Size; ++index)
                    result += ((*this)[index] * right[index]);
                return result;
            }
            std::enable_if_t<Size == 3, self_type> cross(const self_type& right) const
            {
                auto const& x = v[0];
                auto const& y = v[1];
                auto const& z = v[2];
                auto const& xx = right.v[0];
                auto const& yy = right.v[1];
                auto const& zz = right.v[2];
                return self_type{ y * zz - z * yy, z * xx - x * zz, x * yy - y * xx };
            }
        public:
            array_type v;
        };

        template <typename T, uint32_t Size, typename Type>
        inline bool operator<(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v < aRhs.v;
        }

        template <typename T, uint32_t Size, typename Type>
        inline bool operator<=(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v <= aRhs.v;
        }

        template <typename T, uint32_t Size, typename Type>
        inline bool operator>(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v > aRhs.v;
        }

        template <typename T, uint32_t Size, typename Type>
        inline bool operator>=(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v >= aRhs.v;
        }

        template <typename T, uint32_t Size, typename Type>
        inline bool operator==(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v == aRhs.v;
        }

        template <typename T, uint32_t Size, typename Type>
        inline bool operator!=(const basic_vector<T, Size, Type>& aLhs, const basic_vector<T, Size, Type>& aRhs)
        {
            return aLhs.v != aRhs.v;
        }

        typedef basic_vector<double, 1> vector1;
        typedef basic_vector<double, 2> vector2;
        typedef basic_vector<double, 3> vector3;
        typedef basic_vector<double, 4> vector4;

        typedef vector1 vec1;
        typedef vector2 vec2;
        typedef vector3 vec3;
        typedef vector4 vec4;

        typedef vec1 col_vec1;
        typedef vec2 col_vec2;
        typedef vec3 col_vec3;
        typedef vec4 col_vec4;

        typedef basic_vector<double, 1, row_vector> row_vec1;
        typedef basic_vector<double, 2, row_vector> row_vec2;
        typedef basic_vector<double, 3, row_vector> row_vec3;
        typedef basic_vector<double, 4, row_vector> row_vec4;

        typedef std::optional<vector1> optional_vector1;
        typedef std::optional<vector2> optional_vector2;
        typedef std::optional<vector3> optional_vector3;
        typedef std::optional<vector4> optional_vector4;

        typedef std::optional<vec1> optional_vec1;
        typedef std::optional<vec2> optional_vec2;
        typedef std::optional<vec3> optional_vec3;
        typedef std::optional<vec4> optional_vec4;

        typedef std::optional<col_vec1> optional_col_vec1;
        typedef std::optional<col_vec2> optional_col_vec2;
        typedef std::optional<col_vec3> optional_col_vec3;
        typedef std::optional<col_vec4> optional_col_vec4;

        typedef std::optional<row_vec1> optional_row_vec1;
        typedef std::optional<row_vec2> optional_row_vec2;
        typedef std::optional<row_vec3> optional_row_vec3;
        typedef std::optional<row_vec4> optional_row_vec4;

        typedef std::vector<vec2> vec2_list;
        typedef std::vector<vec3> vec3_list;

        typedef std::optional<vec2_list> optional_vec2_list;
        typedef std::optional<vec3_list> optional_vec3_list;

        typedef vec2_list vertices_2d;
        typedef vec3_list vertices;

        typedef optional_vec2_list optional_vertices_2d_t;
        typedef optional_vec3_list optional_vertices_t;

        typedef basic_vector<float, 1> vector1f;
        typedef basic_vector<float, 2> vector2f;
        typedef basic_vector<float, 3> vector3f;
        typedef basic_vector<float, 4> vector4f;

        typedef vector1f vec1f;
        typedef vector2f vec2f;
        typedef vector3f vec3f;
        typedef vector4f vec4f;

        typedef basic_vector<int32_t, 1> vector1i32;
        typedef basic_vector<int32_t, 2> vector2i32;
        typedef basic_vector<int32_t, 3> vector3i32;
        typedef basic_vector<int32_t, 4> vector4i32;

        typedef vector1i32 vec1i32;
        typedef vector2i32 vec2i32;
        typedef vector3i32 vec3i32;
        typedef vector4i32 vec4i32;

        typedef basic_vector<uint32_t, 1> vector1u32;
        typedef basic_vector<uint32_t, 2> vector2u32;
        typedef basic_vector<uint32_t, 3> vector3u32;
        typedef basic_vector<uint32_t, 4> vector4u32;

        typedef vector1u32 vec1u32;
        typedef vector2u32 vec2u32;
        typedef vector3u32 vec3u32;
        typedef vector4u32 vec4u32;

        template <std::size_t VertexCount>
        using vec3_array = neolib::vecarray<vec3, VertexCount, VertexCount, neolib::check<neolib::vecarray_overflow>, std::allocator<vec3>>;

        template <std::size_t VertexCount>
        using vec2_array = neolib::vecarray<vec2, VertexCount, VertexCount, neolib::check<neolib::vecarray_overflow>, std::allocator<vec2>>;

        typedef std::array<vec3, 3> triangle;
        typedef std::array<vec3, 4> quad;

        typedef std::array<vec2, 3> triangle_2d;
        typedef std::array<vec2, 4> quad_2d;

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator+(const basic_vector<T, D, Type, IsScalar>& left, const basic_vector<T, D, Type, IsScalar>& right)
        {
            basic_vector<T, D, Type, IsScalar> result = left;
            result += right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator-(const basic_vector<T, D, Type, IsScalar>& left, const basic_vector<T, D, Type, IsScalar>& right)
        {
            basic_vector<T, D, Type, IsScalar> result = left;
            result -= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator+(const basic_vector<T, D, Type, IsScalar>& left, const T& right)
        {
            basic_vector<T, D, Type, IsScalar> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] += right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator+(const T& left, const basic_vector<T, D, Type, IsScalar>& right)
        {
            basic_vector<T, D, Type, IsScalar> result = right;
            for (uint32_t i = 0; i < D; ++i)
                result[i] += left;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator-(const basic_vector<T, D, Type, IsScalar>& left, const T& right)
        {
            basic_vector<T, D, Type, IsScalar> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] -= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator-(const T& left, const basic_vector<T, D, Type, IsScalar>& right)
        {
            basic_vector<T, D, Type, IsScalar> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = left - right[i];
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator*(const basic_vector<T, D, Type, IsScalar>& left, const T& right)
        {
            basic_vector<T, D, Type, IsScalar> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] *= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator*(const T& left, const basic_vector<T, D, Type, IsScalar>& right)
        {
            basic_vector<T, D, Type, IsScalar> result = right;
            for (uint32_t i = 0; i < D; ++i)
                result[i] *= left;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator/(const basic_vector<T, D, Type, IsScalar>& left, const T& right)
        {
            basic_vector<T, D, Type, IsScalar> result = left;
            for (uint32_t i = 0; i < D; ++i)
                result[i] /= right;
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator/(const T& left, const basic_vector<T, D, Type, IsScalar>& right)
        {
            basic_vector<T, D, Type, IsScalar> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = left / right[i];
            return result;
        }

        template <typename T, uint32_t D, typename Type, bool IsScalar>
        inline basic_vector<T, D, Type, IsScalar> operator%(const basic_vector<T, D, Type, IsScalar>& left, const T& right)
        {
            basic_vector<T, D, Type, IsScalar> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = std::fmod(left[i], right);
            return result;
        }

        template <typename T, uint32_t D, bool IsScalar>
        inline T operator*(const basic_vector<T, D, row_vector, IsScalar>& left, const basic_vector<T, D, column_vector, IsScalar>& right)
        {
            T result = {};
            for (uint32_t index = 0; index < D; ++index)
                result += (left[index] * right[index]);
            return result;
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator+(const basic_vector<T, 3, Type, IsScalar>& left, const basic_vector<T, 3, Type, IsScalar>& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left[0] + right[0], left[1] + right[1], left[2] + right[2] };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator-(const basic_vector<T, 3, Type, IsScalar>& left, const basic_vector<T, 3, Type, IsScalar>& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left[0] - right[0], left[1] - right[1], left[2] - right[2] };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator+(const basic_vector<T, 3, Type, IsScalar>& left, const T& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left[0] + right, left[1] + right, left[2] + right };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator+(const T& left, const basic_vector<T, 3, Type, IsScalar>& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left + right[0], left + right[1], left + right[2] };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator-(const basic_vector<T, 3, Type, IsScalar>& left, const T& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left[0] - right, left[1] - right, left[2] - right };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator-(const T& left, const basic_vector<T, 3, Type, IsScalar>& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left - right[0], left - right[1], left - right[2] };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator*(const basic_vector<T, 3, Type, IsScalar>& left, const T& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left[0] * right, left[1] * right, left[2] * right };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator*(const T& left, const basic_vector<T, 3, Type, IsScalar>& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left * right[0], left * right[1], left * right[2] };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator/(const basic_vector<T, 3, Type, IsScalar>& left, const T& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ left[0] / right, left[1] / right, left[2] / right };
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> operator%(const basic_vector<T, 3, Type, IsScalar>& left, const T& right)
        {
            return basic_vector<T, 3, Type, IsScalar>{ std::fmod(left[0], right), std::fmod(left[1], right), std::fmod(left[2], right) };
        }

        template <typename T, bool IsScalar>
        inline T operator*(const basic_vector<T, 3, row_vector, IsScalar>& left, const basic_vector<T, 3, column_vector, IsScalar>& right)
        {
            return left[0] * right[0] + left[1] * right[1] + left[2] * right[2];
        }

        template <typename T, typename Type, bool IsScalar>
        inline basic_vector<T, 3, Type, IsScalar> midpoint(const basic_vector<T, 3, Type, IsScalar>& left, const basic_vector<T, 3, Type, IsScalar>& right)
        {
            return (left + right) / constants::two<T>;
        }

        template <typename T, uint32_t Size, typename Type, bool IsScalar>
        inline basic_vector<T, Size, Type, IsScalar> lerp(const basic_vector<T, Size, Type, IsScalar>& aV1, const basic_vector<T, Size, Type, IsScalar>& aV2, double aAmount)
        {
            basic_vector<T, Size, Type, IsScalar> result;
            for (uint32_t i = 0; i < Size; ++i)
            {
                double x1 = aV1[i];
                double x2 = aV2[i];
                result[i] = static_cast<T>((x2 - x1) * aAmount + x1);
            }
            return result;
        }

        /* todo: specializations that use SIMD intrinsics. */
        template <typename T, uint32_t Rows, uint32_t Columns>
        class basic_matrix
        {
            typedef basic_matrix<T, Rows, Columns> self_type;
        public:
            typedef self_type abstract_type; // todo: abstract base
        public:
            typedef T value_type;
            typedef basic_vector<T, Columns, row_vector> row_type;
            typedef basic_vector<T, Rows, column_vector> column_type;
            typedef std::array<column_type, Columns> array_type;
        public:
            template <typename T2>
            struct rebind { typedef basic_matrix<T2, Rows, Columns> type; };
        public:
            basic_matrix() : m{ {} } {}
            basic_matrix(std::initializer_list<std::initializer_list<value_type>> aColumns) { std::copy(aColumns.begin(), aColumns.end(), m.begin()); }
            basic_matrix(const self_type& other) : m{ other.m } {}
            basic_matrix(self_type&& other) : m{ std::move(other.m) } {}
            template <typename T2>
            basic_matrix(const basic_matrix<T2, Rows, Columns>& other)
            {
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        (*this)[column][row] = static_cast<value_type>(other[column][row]);
            }
            self_type& operator=(const self_type& other) { m = other.m; return *this; }
            self_type& operator=(self_type&& other) { m = std::move(other.m); return *this; }
        public:
            template <typename T2>
            basic_matrix<T2, Rows, Columns> as() const
            {
                return basic_matrix<T2, Rows, Columns>{ *this };
            }
        public:
            std::pair<uint32_t, uint32_t> size() const { return std::make_pair(Rows, Columns); }
            const column_type& operator[](uint32_t aColumn) const { return m[aColumn]; }
            column_type& operator[](uint32_t aColumn) { return m[aColumn]; }
            const value_type* data() const { return &m[0].v[0]; }
        public:
            bool operator==(const self_type& right) const { return m == right.m; }
            bool operator!=(const self_type& right) const { return m != right.m; }
            self_type& operator+=(const self_type& right) { for (uint32_t column = 0; column < Columns; ++column) m[column] += right.m[column]; return *this; }
            self_type& operator-=(const self_type& right) { for (uint32_t column = 0; column < Columns; ++column) m[column] -= right.m[column]; return *this; }
            self_type& operator*=(const self_type& right)
            {
                self_type result;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        for (uint32_t index = 0; index < Columns; ++index)
                            result[column][row] += (m[index][row] * right[column][index]);
                *this = result;
                return *this;
            }
            self_type operator-() const
            {
                self_type result = *this;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        result[column][row] = -result[column][row];
                return result;
            }
            self_type round_to(value_type aEpsilon) const
            {
                self_type result;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                    {
                         std::modf((*this)[column][row] / aEpsilon + 0.5, &result[column][row]);
                         result[column][row] *= aEpsilon;
                    }
                return result;
            }
            basic_matrix<T, Columns, Rows> transposed() const
            {
                basic_matrix<T, Columns, Rows> result;
                for (uint32_t column = 0; column < Columns; ++column)
                    for (uint32_t row = 0; row < Rows; ++row)
                        result[row][column] = m[column][row];
                return result;
            }
            template <typename SFINAE = self_type>
            static const std::enable_if_t<Rows == Columns, SFINAE>& identity()
            {
                auto make_identity = []()
                {
                    self_type result;
                    for (uint32_t diag = 0; diag < Rows; ++diag)
                        result[diag][diag] = static_cast<value_type>(1.0);
                    return result;
                };
                static self_type const sIdentity = make_identity();
                return sIdentity;
            }
            bool is_identity() const
            {
                return this == &identity();
            }
        private:
            array_type m;
        };

        typedef basic_matrix<double, 1, 1> matrix11;
        typedef basic_matrix<double, 2, 2> matrix22;
        typedef basic_matrix<double, 2, 1> matrix21;
        typedef basic_matrix<double, 1, 2> matrix12;
        typedef basic_matrix<double, 3, 3> matrix33;
        typedef basic_matrix<double, 3, 1> matrix31;
        typedef basic_matrix<double, 3, 2> matrix32;
        typedef basic_matrix<double, 1, 3> matrix13;
        typedef basic_matrix<double, 2, 3> matrix23;
        typedef basic_matrix<double, 4, 4> matrix44;
        typedef basic_matrix<double, 4, 1> matrix41;
        typedef basic_matrix<double, 4, 2> matrix42;
        typedef basic_matrix<double, 4, 3> matrix43;
        typedef basic_matrix<double, 1, 4> matrix14;
        typedef basic_matrix<double, 2, 4> matrix24;
        typedef basic_matrix<double, 3, 4> matrix34;

        typedef matrix11 matrix1;
        typedef matrix22 matrix2;
        typedef matrix33 matrix3;
        typedef matrix44 matrix4;

        typedef matrix11 mat11;
        typedef matrix22 mat22;
        typedef matrix21 mat21;
        typedef matrix12 mat12;
        typedef matrix33 mat33;
        typedef matrix31 mat31;
        typedef matrix32 mat32;
        typedef matrix13 mat13;
        typedef matrix23 mat23;
        typedef matrix44 mat44;
        typedef matrix41 mat41;
        typedef matrix42 mat42;
        typedef matrix43 mat43;
        typedef matrix14 mat14;
        typedef matrix24 mat24;
        typedef matrix34 mat34;

        typedef mat11 mat1;
        typedef mat22 mat2;
        typedef mat33 mat3;
        typedef mat44 mat4;

        typedef std::optional<matrix11> optional_matrix11;
        typedef std::optional<matrix22> optional_matrix22;
        typedef std::optional<matrix21> optional_matrix21;
        typedef std::optional<matrix12> optional_matrix12;
        typedef std::optional<matrix33> optional_matrix33;
        typedef std::optional<matrix31> optional_matrix31;
        typedef std::optional<matrix32> optional_matrix32;
        typedef std::optional<matrix13> optional_matrix13;
        typedef std::optional<matrix23> optional_matrix23;
        typedef std::optional<matrix44> optional_matrix44;
        typedef std::optional<matrix41> optional_matrix41;
        typedef std::optional<matrix42> optional_matrix42;
        typedef std::optional<matrix43> optional_matrix43;
        typedef std::optional<matrix14> optional_matrix14;
        typedef std::optional<matrix24> optional_matrix24;
        typedef std::optional<matrix34> optional_matrix34;

        typedef std::optional<matrix11> optional_matrix1;
        typedef std::optional<matrix22> optional_matrix2;
        typedef std::optional<matrix33> optional_matrix3;
        typedef std::optional<matrix44> optional_matrix4;

        typedef std::optional<mat11> optional_mat11;
        typedef std::optional<mat22> optional_mat22;
        typedef std::optional<mat21> optional_mat21;
        typedef std::optional<mat12> optional_mat12;
        typedef std::optional<mat33> optional_mat33;
        typedef std::optional<mat31> optional_mat31;
        typedef std::optional<mat32> optional_mat32;
        typedef std::optional<mat13> optional_mat13;
        typedef std::optional<mat23> optional_mat23;
        typedef std::optional<mat44> optional_mat44;
        typedef std::optional<mat41> optional_mat41;
        typedef std::optional<mat42> optional_mat42;
        typedef std::optional<mat43> optional_mat43;
        typedef std::optional<mat14> optional_mat14;
        typedef std::optional<mat24> optional_mat24;
        typedef std::optional<mat34> optional_mat34;

        typedef std::optional<mat11> optional_mat1;
        typedef std::optional<mat22> optional_mat2;
        typedef std::optional<mat33> optional_mat3;
        typedef std::optional<mat44> optional_mat4;

        typedef basic_matrix<float, 1, 1> matrix11f;
        typedef basic_matrix<float, 2, 2> matrix22f;
        typedef basic_matrix<float, 2, 1> matrix21f;
        typedef basic_matrix<float, 1, 2> matrix12f;
        typedef basic_matrix<float, 3, 3> matrix33f;
        typedef basic_matrix<float, 3, 1> matrix31f;
        typedef basic_matrix<float, 3, 2> matrix32f;
        typedef basic_matrix<float, 1, 3> matrix13f;
        typedef basic_matrix<float, 2, 3> matrix23f;
        typedef basic_matrix<float, 4, 4> matrix44f;
        typedef basic_matrix<float, 4, 1> matrix41f;
        typedef basic_matrix<float, 4, 2> matrix42f;
        typedef basic_matrix<float, 4, 3> matrix43f;
        typedef basic_matrix<float, 1, 4> matrix14f;
        typedef basic_matrix<float, 2, 4> matrix24f;
        typedef basic_matrix<float, 3, 4> matrix34f;

        typedef matrix11f mat11f;
        typedef matrix22f mat22f;
        typedef matrix21f mat21f;
        typedef matrix12f mat12f;
        typedef matrix33f mat33f;
        typedef matrix31f mat31f;
        typedef matrix32f mat32f;
        typedef matrix13f mat13f;
        typedef matrix23f mat23f;
        typedef matrix44f mat44f;
        typedef matrix41f mat41f;
        typedef matrix42f mat42f;
        typedef matrix43f mat43f;
        typedef matrix14f mat14f;
        typedef matrix24f mat24f;
        typedef matrix34f mat34f;

        typedef matrix11f mat1f;
        typedef matrix22f mat2f;
        typedef matrix33f mat3f;
        typedef matrix44f mat4f;

        typedef std::optional<matrix11f> optional_matrix11f;
        typedef std::optional<matrix22f> optional_matrix22f;
        typedef std::optional<matrix21f> optional_matrix21f;
        typedef std::optional<matrix12f> optional_matrix12f;
        typedef std::optional<matrix33f> optional_matrix33f;
        typedef std::optional<matrix31f> optional_matrix31f;
        typedef std::optional<matrix32f> optional_matrix32f;
        typedef std::optional<matrix13f> optional_matrix13f;
        typedef std::optional<matrix23f> optional_matrix23f;
        typedef std::optional<matrix44f> optional_matrix44f;
        typedef std::optional<matrix41f> optional_matrix41f;
        typedef std::optional<matrix42f> optional_matrix42f;
        typedef std::optional<matrix43f> optional_matrix43f;
        typedef std::optional<matrix14f> optional_matrix14f;
        typedef std::optional<matrix24f> optional_matrix24f;
        typedef std::optional<matrix34f> optional_matrix34f;

        typedef std::optional<matrix11f> optional_matrix1f;
        typedef std::optional<matrix22f> optional_matrix2f;
        typedef std::optional<matrix33f> optional_matrix3f;
        typedef std::optional<matrix44f> optional_matrix4f;

        typedef std::optional<mat11f> optional_mat11f;
        typedef std::optional<mat22f> optional_mat22f;
        typedef std::optional<mat21f> optional_mat21f;
        typedef std::optional<mat12f> optional_mat12f;
        typedef std::optional<mat33f> optional_mat33f;
        typedef std::optional<mat31f> optional_mat31f;
        typedef std::optional<mat32f> optional_mat32f;
        typedef std::optional<mat13f> optional_mat13f;
        typedef std::optional<mat23f> optional_mat23f;
        typedef std::optional<mat44f> optional_mat44f;
        typedef std::optional<mat41f> optional_mat41f;
        typedef std::optional<mat42f> optional_mat42f;
        typedef std::optional<mat43f> optional_mat43f;
        typedef std::optional<mat14f> optional_mat14f;
        typedef std::optional<mat24f> optional_mat24f;
        typedef std::optional<mat34f> optional_mat34f;

        typedef std::optional<mat11f> optional_mat1f;
        typedef std::optional<mat22f> optional_mat2f;
        typedef std::optional<mat33f> optional_mat3f;
        typedef std::optional<mat44f> optional_mat4f;

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result += right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result -= right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator*(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result *= right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator/(const basic_matrix<T, Rows, Columns>& left, typename basic_matrix<T, Rows, Columns>::value_type right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result /= right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = right;
            result += left;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            return -right + left;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator*(scalar left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = right;
            result *= left;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator+(const basic_matrix<T, Rows, Columns>& left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result += right;
            return result;
        }

        template <typename T, uint32_t Rows, uint32_t Columns>
        inline basic_matrix<T, Rows, Columns> operator-(const basic_matrix<T, Rows, Columns>& left, const basic_matrix<T, Rows, Columns>& right)
        {
            basic_matrix<T, Rows, Columns> result = left;
            result -= right;
            return result;
        }

        template <typename T, uint32_t D1, uint32_t D2>
        inline basic_matrix<T, D1, D1> operator*(const basic_matrix<T, D1, D2>& left, const basic_matrix<T, D2, D1>& right)
        {
            if (&left == &left.identity())
                return right;
            if (&right == &right.identity())
                return left;
            basic_matrix<T, D1, D1> result;
            for (uint32_t column = 0; column < D1; ++column)
                for (uint32_t row = 0; row < D1; ++row)
                    for (uint32_t index = 0; index < D2; ++index)
                        result[column][row] += (left[index][row] * right[column][index]);
            return result;
        }

        template <typename T, uint32_t D, bool IsScalar>
        inline basic_vector<T, D, column_vector, IsScalar> operator*(const basic_matrix<T, D, D>& left, const basic_vector<T, D, column_vector, IsScalar>& right)
        {
            basic_vector<T, D, column_vector, IsScalar> result;
            for (uint32_t row = 0; row < D; ++row)
                for (uint32_t index = 0; index < D; ++index)
                    result[row] += (left[index][row] * right[index]);
            return result;
        }

        template <typename T, uint32_t D, bool IsScalar>
        inline basic_vector<T, D, row_vector, IsScalar> operator*(const basic_vector<T, D, row_vector, IsScalar>& left, const basic_matrix<T, D, D>& right)
        {
            basic_vector<T, D, row_vector, IsScalar> result;
            for (uint32_t column = 0; column < D; ++column)
                for (uint32_t index = 0; index < D; ++index)
                    result[column] += (left[index] * right[column][index]);
            return result;
        }

        template <typename T, uint32_t D, bool IsScalar>
        inline basic_matrix<T, D, D> operator*(const basic_vector<T, D, column_vector, IsScalar>& left, const basic_vector<T, D, row_vector, IsScalar>& right)
        {
            basic_matrix<T, D, D> result;
            for (uint32_t column = 0; column < D; ++column)
                for (uint32_t row = 0; row < D; ++row)
                    result[column][row] = (left[row] * right[column]);
            return result;
        }

        template <typename T, uint32_t D>
        inline basic_matrix<T, D, D> without_translation(const basic_matrix<T, D, D>& matrix)
        {
            auto result = matrix;
            for (uint32_t row = 0; row < D - 1; ++row)
                result[D - 1][row] = 0.0;
            return result;
        }

        template <typename Elem, typename Traits, typename T, uint32_t Size, typename Type, bool IsScalar>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_vector<T, Size, Type, IsScalar>& aVector)
        {
            aStream << "[";
            for (uint32_t i = 0; i < Size; ++i)
            {
                if (i != 0)
                    aStream << ", ";
                aStream << aVector[i];
            }
            aStream << "]";
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, uint32_t Rows, uint32_t Columns>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_matrix<T, Rows, Columns>& aMatrix)
        {
            aStream << "[";
            for (uint32_t row = 0; row < Rows; ++row)
            {
                if (row != 0)
                    aStream << ", ";
                aStream << "[";
                for (uint32_t column = 0; column < Columns; ++column)
                {
                    if (column != 0)
                        aStream << ", ";
                    aStream << aMatrix[column][row];
                }
                aStream << "]";
            }
            aStream << "]";
            return aStream;
        }

        template <typename Elem, typename Traits, typename T, uint32_t Rows, uint32_t Columns>
        inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const std::optional<basic_matrix<T, Rows, Columns>>& aMatrix)
        {
            if (aMatrix != std::nullopt)
                aStream << *aMatrix;
            else
                aStream << "[null]";
            return aStream;
        }

        struct aabb
        {
            vec3 min;
            vec3 max;
            aabb() : min{}, max{} {}
            aabb(const vec3& aMin, const vec3& aMax) : min{ aMin }, max{ aMax } {}
        };

        inline bool operator==(const aabb& left, const aabb& right)
        {
            return left.min == right.min && left.max == right.max;
        }

        inline bool operator!=(const aabb& left, const aabb& right)
        {
            return !(left == right);
        }

        inline bool operator<(const aabb& left, const aabb& right)
        {
            return std::tie(left.min.z, left.min.y, left.min.x, left.max.z, left.max.y, left.max.x) <
                std::tie(right.min.z, right.min.y, right.min.x, right.max.z, right.max.y, right.max.x);
        }

        typedef std::optional<aabb> optional_aabb;

        inline aabb aabb_union(const aabb& left, const aabb& right)
        {
            return aabb{ left.min.min(right.min), left.max.max(right.max) };
        }

        inline scalar aabb_volume(const aabb& a)
        {
            auto extents = a.max - a.min;
            return extents.x * extents.y * (extents.z != 0.0 ? extents.z : 1.0);
        }

        inline bool aabb_contains(const aabb& outer, const aabb& inner)
        {
            return inner.min >= outer.min && inner.max <= outer.max;
        }

        inline bool aabb_contains(const aabb& outer, const vec3& point)
        {
            return point >= outer.min && point <= outer.max;
        }

        inline bool aabb_intersects(const aabb& first, const aabb& second)
        {
            if (first.max.x < second.min.x)
                return false;
            if (first.min.x > second.max.x)
                return false;
            if (first.max.y < second.min.y)
                return false;
            if (first.min.y > second.max.y)
                return false;
            if (first.max.z < second.min.z)
                return false;
            if (first.min.z > second.max.z)
                return false;
            return true;
        }

        struct aabb_2d
        {
            vec2 min;
            vec2 max;
            aabb_2d() : min{}, max{} {}
            aabb_2d(const vec2& aMin, const vec2& aMax) : min{ aMin }, max{ aMax } {}
            aabb_2d(const aabb& aAabb) : min{ aAabb.min.xy }, max{ aAabb.max.xy } {}
        };

        inline aabb_2d to_aabb_2d(const vertices& vertices)
        {
            aabb_2d result = !vertices.empty() ? aabb_2d{ vertices[0].xy, vertices[0].xy } : aabb_2d{};
            for (auto const& v : vertices)
            {
                result.min = result.min.min(v.xy);
                result.max = result.max.max(v.xy);
            }
            return result;
        }

        inline bool operator==(const aabb_2d& left, const aabb_2d& right)
        {
            return left.min == right.min && left.max == right.max;
        }

        inline bool operator!=(const aabb_2d& left, const aabb_2d& right)
        {
            return !(left == right);
        }

        inline bool operator<(const aabb_2d& left, const aabb_2d& right)
        {
            return std::tie(left.min.y, left.min.x, left.max.y, left.max.x) <
                std::tie(right.min.y, right.min.x, right.max.y, right.max.x);
        }

        typedef std::optional<aabb_2d> optional_aabb_2d;

        inline aabb_2d aabb_union(const aabb_2d& left, const aabb_2d& right)
        {
            return aabb_2d{ left.min.min(right.min), left.max.max(right.max) };
        }

        inline scalar aabb_volume(const aabb_2d& a)
        {
            auto extents = a.max - a.min;
            return extents.x * extents.y;
        }

        inline bool aabb_contains(const aabb_2d& outer, const aabb_2d& inner)
        {
            return inner.min >= outer.min && inner.max <= outer.max;
        }

        inline bool aabb_contains(const aabb_2d& outer, const vec2& point)
        {
            return point >= outer.min && point <= outer.max;
        }

        inline bool aabb_intersects(const aabb_2d& first, const aabb_2d& second)
        {
            if (first.max.x < second.min.x)
                return false;
            if (first.min.x > second.max.x)
                return false;
            if (first.max.y < second.min.y)
                return false;
            if (first.min.y > second.max.y)
                return false;
            return true;
        }

        // 3D helpers

        template <typename T, bool IsScalar>
        inline basic_vector<T, 3, column_vector, IsScalar> operator*(const basic_matrix<T, 4, 4>& left, const basic_vector<T, 3, column_vector, IsScalar>& right)
        {
            return (left * basic_vector<T, 4, column_vector, IsScalar>{ right.x, right.y, right.z, 1.0 }).xyz;
        }

        template <typename T, bool IsScalar>
        inline std::vector<basic_vector<T, 3, column_vector, IsScalar>> operator*(const basic_matrix<T, 4, 4>& left, const std::vector<basic_vector<T, 3, column_vector, IsScalar>>& right)
        {
            std::vector<basic_vector<T, 3, column_vector, IsScalar>> result;
            result.reserve(right.size());
            for (auto const& v : right)
                result.push_back(left * v);
            return result;
        }

        inline mat33 rotation_matrix(const vec3& axis, scalar angle, scalar epsilon = 0.00001)
        {
            if (std::abs(angle) <= epsilon)
                return mat33::identity();
            else if (std::abs(angle - boost::math::constants::pi<scalar>()) <= epsilon)
                return -mat33::identity();
            scalar const s = std::sin(angle);
            scalar const c = std::cos(angle);
            scalar const a = 1.0 - c;
            scalar const ax = a * axis.x;
            scalar const ay = a * axis.y;
            scalar const az = a * axis.z;
            return mat33{
                { ax * axis.x + c, ax * axis.y + axis.z * s, ax * axis.z - axis.y * s },
                { ay * axis.x - axis.z * s, ay * axis.y + c, ay * axis.z + axis.x * s },
                { az * axis.x + axis.y * s, az * axis.y - axis.x * s, az * axis.z + c } }.round_to(epsilon);
        }

        inline mat33 rotation_matrix(const vec3& vectorA, const vec3& vectorB, scalar epsilon = 0.00001)
        {
            auto const nva = vectorA.normalized();
            auto const nvb = vectorB.normalized();
            return rotation_matrix(nva.cross(nvb).normalized(), std::acos(nva.dot(nvb)), epsilon);
        }

        inline mat33 rotation_matrix(const vec3& angles)
        {
            scalar ax = angles.x;
            scalar ay = angles.y;
            scalar az = angles.z;
            if (ax != 0.0 || ay != 0.0)
            {
                mat33 rx = { { 1.0, 0.0, 0.0 },{ 0.0, std::cos(ax), std::sin(ax) },{ 0.0, -std::sin(ax), std::cos(ax) } };
                mat33 ry = { { std::cos(ay), 0.0, -std::sin(ay) },{ 0.0, 1.0, 0.0 },{ std::sin(ay), 0.0, std::cos(ay) } };
                mat33 rz = { { std::cos(az), std::sin(az), 0.0 },{ -std::sin(az), std::cos(az), 0.0 },{ 0.0, 0.0, 1.0 } };
                return rz * ry * rx;
            }
            else
            {
                return mat33{ { std::cos(az), std::sin(az), 0.0 },{ -std::sin(az), std::cos(az), 0.0 },{ 0.0, 0.0, 1.0 } };
            }
        }

        inline mat44 affine_rotation_matrix(const vec3& angles)
        {
            scalar ax = angles.x;
            scalar ay = angles.y;
            scalar az = angles.z;
            if (ax != 0.0 || ay != 0.0)
            {
                mat44 rx = { { 1.0, 0.0, 0.0, 0.0 },{ 0.0, std::cos(ax), std::sin(ax), 0.0 },{ 0.0, -std::sin(ax), std::cos(ax), 0.0 },{0.0, 0.0, 0.0, 1.0} };
                mat44 ry = { { std::cos(ay), 0.0, -std::sin(ay), 0.0 },{ 0.0, 1.0, 0.0, 0.0 },{ std::sin(ay), 0.0, std::cos(ay), 0.0 },{0.0, 0.0, 0.0, 1.0} };
                mat44 rz = { { std::cos(az), std::sin(az), 0.0, 0.0 },{ -std::sin(az), std::cos(az), 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 },{0.0, 0.0, 0.0, 1.0} };
                return rz * ry * rx;
            }
            else
            {
                return mat44{ { std::cos(az), std::sin(az), 0.0, 0.0 },{ -std::sin(az), std::cos(az), 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 },{0.0, 0.0, 0.0, 1.0} };
            }
        }
    }

    using namespace math;
}
