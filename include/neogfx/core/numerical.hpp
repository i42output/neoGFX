// numerical.hpp
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
#include <type_traits>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <ostream>
#include <boost/optional.hpp>
#include <boost/math/constants/constants.hpp>
#include "swizzle.hpp"

namespace neogfx
{ 
	typedef double scalar;
	typedef double angle;

	inline angle to_rad(angle aDegrees)
	{
		return aDegrees / 180.0 * boost::math::constants::pi<angle>();
	}

	inline angle to_deg(angle aRadians)
	{
		return aRadians * 180.0 / boost::math::constants::pi<angle>();
	}

	struct column_vector {};
	struct row_vector {};

	template <typename T, uint32_t Size, typename Type = column_vector, bool IsScalar=std::is_scalar<T>::value>
	class basic_vector;

	/* todo: specializations that use SIMD intrinsics. */
	template <typename T, uint32_t _Size, typename Type>
	class basic_vector<T, _Size, Type, true>
	{
	public:
		enum : uint32_t { Size = _Size };
		typedef Type type;
	public:
		typedef T value_type;
		typedef value_type input_reference_type;
		typedef basic_vector<value_type, Size, Type> vector_type;
		typedef uint32_t size_type;
		typedef std::array<value_type, Size> array_type;
		typedef typename array_type::const_iterator const_iterator;
		typedef typename array_type::iterator iterator;
	public:
		template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2, Type> type; };
	public:
		basic_vector() : v{} {}
		explicit basic_vector(value_type value) : v{{value}} {}
		template <typename... Arguments>
		explicit basic_vector(value_type value, Arguments... aArguments) : v{{value, aArguments...}} {}
		basic_vector(const array_type& a) : v(a) {}
		basic_vector(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neogfx::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::uninitialized_fill(v.begin() + (values.end() - values.begin()), v.end(), value_type()); }
		basic_vector(const basic_vector& other) : v(other.v) {}
		basic_vector(basic_vector&& other) : v(std::move(other.v)) {}
		template <typename T2>
		basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
		basic_vector& operator=(const basic_vector& other) { v = other.v; return *this; }
		basic_vector& operator=(basic_vector&& other) { v = std::move(other.v); return *this; }
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
		bool operator==(const basic_vector& right) const { return v == right.v; }
		bool operator!=(const basic_vector& right) const { return v != right.v; }
		basic_vector& operator+=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
		basic_vector& operator-=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
		basic_vector& operator*=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
		basic_vector& operator/=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
		basic_vector& operator+=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] += right.v[index]; return *this; }
		basic_vector& operator-=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] -= right.v[index]; return *this; }
		basic_vector& operator*=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] *= right.v[index]; return *this; }
		basic_vector& operator/=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] /= right.v[index]; return *this; }
		basic_vector operator-() const { basic_vector result; for (uint32_t index = 0; index < Size; ++index) result.v[index] = -v[index]; return result; }
		scalar magnitude() const { scalar ss = 0; for (uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
		basic_vector normalized() const { basic_vector result; scalar m = magnitude(); for (uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] / m; return result; }
	public:
		union
		{
			array_type v;

			swizzle<vector_type, 1, 0> x;
			swizzle<vector_type, 1, 1> y;
			swizzle<vector_type, 1, 2> z;
			swizzle<vector_type, 2, 0, 0> xx;
			swizzle<vector_type, 2, 0, 1> xy;
			swizzle<vector_type, 2, 0, 2> xz;
			swizzle<vector_type, 2, 1, 0> yx;
			swizzle<vector_type, 2, 1, 1> yy;
			swizzle<vector_type, 2, 1, 2> yz;
			swizzle<vector_type, 2, 2, 0> zx;
			swizzle<vector_type, 2, 2, 1> zy;
			swizzle<vector_type, 2, 2, 2> zz;
			swizzle<vector_type, 3, 0, 0, 0> xxx;
			swizzle<vector_type, 3, 0, 0, 1> xxy;
			swizzle<vector_type, 3, 0, 0, 2> xxz;
			swizzle<vector_type, 3, 0, 1, 0> xyx;
			swizzle<vector_type, 3, 0, 1, 1> xyy;
			swizzle<vector_type, 3, 0, 1, 2> xyz;
			swizzle<vector_type, 3, 1, 0, 0> yxx;
			swizzle<vector_type, 3, 1, 0, 1> yxy;
			swizzle<vector_type, 3, 1, 0, 2> yxz;
			swizzle<vector_type, 3, 1, 1, 0> yyx;
			swizzle<vector_type, 3, 1, 1, 1> yyy;
			swizzle<vector_type, 3, 1, 1, 2> yyz;
			swizzle<vector_type, 3, 1, 2, 0> yzx;
			swizzle<vector_type, 3, 1, 2, 1> yzy;
			swizzle<vector_type, 3, 1, 2, 2> yzz;
			swizzle<vector_type, 3, 2, 0, 0> zxx;
			swizzle<vector_type, 3, 2, 0, 1> zxy;
			swizzle<vector_type, 3, 2, 0, 2> zxz;
			swizzle<vector_type, 3, 2, 1, 0> zyx;
			swizzle<vector_type, 3, 2, 1, 1> zyy;
			swizzle<vector_type, 3, 2, 1, 2> zyz;
			swizzle<vector_type, 3, 2, 2, 0> zzx;
			swizzle<vector_type, 3, 2, 2, 1> zzy;
			swizzle<vector_type, 3, 2, 2, 2> zzz;
		};
	};

	template <typename T, uint32_t _Size, typename Type>
	class basic_vector<T, _Size, Type, false>
	{
	public:
		enum : uint32_t { Size = _Size };
		typedef Type type;
	public:
		typedef T value_type;
		typedef const value_type& input_reference_type;
		typedef basic_vector<value_type, Size, Type> vector_type;
		typedef uint32_t size_type;
		typedef std::array<value_type, Size> array_type;
		typedef typename array_type::const_iterator const_iterator;
		typedef typename array_type::iterator iterator;
	public:
		template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2, Type> type; };
	public:
		basic_vector() : v{} {}
		explicit basic_vector(value_type value) : v{{value}} {}
		template <typename... Arguments>
		explicit basic_vector(const value_type& value, Arguments&&... aArguments) : v{{value, std::forward<Arguments>(aArguments)...}} {}
		template <typename... Arguments>
		explicit basic_vector(value_type&& value, Arguments&&... aArguments) : v{{std::move(value), std::forward<Arguments>(aArguments)...}} {}
		basic_vector(std::initializer_list<value_type> values) { if (values.size() > Size) throw std::out_of_range("neogfx::basic_vector: initializer list too big"); std::copy(values.begin(), values.end(), v.begin()); std::fill(v.begin() + (values.end() - values.begin()), v.end(), value_type()); }
		basic_vector(const basic_vector& other) : v(other.v) {}
		basic_vector(basic_vector&& other) : v(std::move(other.v)) {}
		template <typename T2>
		basic_vector(const basic_vector<T2, Size, Type>& other) { std::transform(other.begin(), other.end(), v.begin(), [](T2 source) { return static_cast<value_type>(source); }); }
		basic_vector& operator=(const basic_vector& other) { v = other.v; return *this; }
		basic_vector& operator=(basic_vector&& other) { v = std::move(other.v); return *this; }
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
		bool operator==(const basic_vector& right) const { return v == right.v; }
		bool operator!=(const basic_vector& right) const { return v != right.v; }
		basic_vector& operator+=(scalar value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
		basic_vector& operator-=(scalar value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
		basic_vector& operator*=(scalar value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
		basic_vector& operator/=(scalar value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
		basic_vector& operator+=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] += value; return *this; }
		basic_vector& operator-=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] -= value; return *this; }
		basic_vector& operator*=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] *= value; return *this; }
		basic_vector& operator/=(input_reference_type value) { for (uint32_t index = 0; index < Size; ++index) v[index] /= value; return *this; }
		basic_vector& operator+=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] += right.v[index]; return *this; }
		basic_vector& operator-=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] -= right.v[index]; return *this; }
		basic_vector& operator*=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] *= right.v[index]; return *this; }
		basic_vector& operator/=(const basic_vector& right) { for (uint32_t index = 0; index < Size; ++index) v[index] /= right.v[index]; return *this; }
		basic_vector operator-() const { basic_vector result; for (uint32_t index = 0; index < Size; ++index) result.v[index] = -v[index]; return result; }
		scalar magnitude() const { scalar ss = 0; for (uint32_t index = 0; index < Size; ++index) ss += (v[index] * v[index]); return std::sqrt(ss); }
		basic_vector normalized() const { basic_vector result; scalar m = magnitude(); for (uint32_t index = 0; index < Size; ++index) result.v[index] = v[index] / m; return result; }
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

	typedef boost::optional<vector1> optional_vector1;
	typedef boost::optional<vector2> optional_vector2;
	typedef boost::optional<vector3> optional_vector3;
	typedef boost::optional<vector4> optional_vector4;

	typedef boost::optional<vec1> optional_vec1;
	typedef boost::optional<vec2> optional_vec2;
	typedef boost::optional<vec3> optional_vec3;
	typedef boost::optional<vec4> optional_vec4;

	typedef boost::optional<col_vec1> optional_col_vec1;
	typedef boost::optional<col_vec2> optional_col_vec2;
	typedef boost::optional<col_vec3> optional_col_vec3;
	typedef boost::optional<col_vec4> optional_col_vec4;

	typedef boost::optional<row_vec1> optional_row_vec1;
	typedef boost::optional<row_vec2> optional_row_vec2;
	typedef boost::optional<row_vec3> optional_row_vec3;
	typedef boost::optional<row_vec4> optional_row_vec4;

	typedef std::vector<vec2> vec2_list;
	typedef std::vector<vec3> vec3_list;

	typedef boost::optional<vec2_list> optional_vec2_list;
	typedef boost::optional<vec3_list> optional_vec3_list;

	typedef basic_vector<float, 1> vector1f;
	typedef basic_vector<float, 2> vector2f;
	typedef basic_vector<float, 3> vector3f;
	typedef basic_vector<float, 4> vector4f;

	typedef vector1f vec1f;
	typedef vector2f vec2f;
	typedef vector3f vec3f;
	typedef vector4f vec4f;

	struct aabb
	{
		vec3 min;
		vec3 max;
	};

	typedef boost::optional<aabb> optional_aabb;

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
	inline basic_vector<T, D, Type, IsScalar> operator*(const basic_vector<T, D, Type, IsScalar>& left, const basic_vector<T, D, Type, IsScalar>& right)
	{
		basic_vector<T, D, Type, IsScalar> result = left;
		result *= right;
		return result;
	}

	template <typename T, uint32_t D, typename Type, bool IsScalar>
	inline basic_vector<T, D, Type, IsScalar> operator/(const basic_vector<T, D, Type, IsScalar>& left, const basic_vector<T, D, Type, IsScalar>& right)
	{
		basic_vector<T, D, Type, IsScalar> result = left;
		result /= right;
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
		for (uitn32_t index = 0; index < D; ++index)
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
	inline basic_vector<T, 3, Type, IsScalar> operator*(const basic_vector<T, 3, Type, IsScalar>& left, const basic_vector<T, 3, Type, IsScalar>& right)
	{
		return basic_vector<T, 3, Type, IsScalar>{ left[0] * right[0], left[1] * right[1], left[2] * right[2] };
	}

	template <typename T, typename Type, bool IsScalar>
	inline basic_vector<T, 3, Type, IsScalar> operator/(const basic_vector<T, 3, Type, IsScalar>& left, const basic_vector<T, 3, Type, IsScalar>& right)
	{
		return basic_vector<T, 3, Type, IsScalar>{ left[0] / right[0], left[1] / right[1], left[2] / right[2] };
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

	/* todo: specializations that use SIMD intrinsics. */
	template <typename T, uint32_t Rows, uint32_t Columns>
	class basic_matrix
	{
	public:
		typedef T value_type;
		typedef basic_vector<T, Columns, row_vector> row_type;
		typedef basic_vector<T, Rows, column_vector> column_type;
		typedef std::array<column_type, Columns> array_type;
		typedef typename basic_vector<T, Columns>::input_reference_type input_reference_type;
	public:
		basic_matrix() : m{{}} {}
		basic_matrix(std::initializer_list<column_type> aColumns) { std::copy(aColumns.begin(), aColumns.end(), m.begin()); }
		basic_matrix(const basic_matrix& other) : m{other.m} {}
		basic_matrix(basic_matrix&& other) : m{std::move(other.m)} {}
		template <typename T2>
		basic_matrix(const basic_matrix<T2, Rows, Columns>& other)
		{
			for (uint32_t column = 0; column < Columns; ++column)
				for (uint32_t row = 0; row < Rows; ++row)
					(*this)[column][row] = static_cast<value_type>(other[column][row]);
		}
		basic_matrix& operator=(const basic_matrix& other) { m = other.m; return *this; }
		basic_matrix& operator=(basic_matrix&& other) { m = std::move(other.m); return *this; }
	public:
		std::pair<uint32_t, uint32_t> size() const { return std::make_pair(Rows, Columns); }
		const column_type& operator[](uint32_t aColumn) const { return m[aColumn]; }
		column_type& operator[](uint32_t aColumn) { return m[aColumn]; }
	public:
		basic_matrix& operator+=(input_reference_type value) { for (uint32_t column = 0; column < Columns; ++column) m[column] += value; return *this; }
		basic_matrix& operator-=(input_reference_type value) { for (uint32_t column = 0; column < Columns; ++column) m[column] -= value; return *this; }
		basic_matrix& operator*=(input_reference_type value) { for (uint32_t column = 0; column < Columns; ++column) m[column] *= value; return *this; }
		basic_matrix& operator/=(input_reference_type value) { for (uint32_t column = 0; column < Columns; ++column) m[column] /= value; return *this; }
		basic_matrix& operator+=(const basic_matrix& right) { for (uint32_t column = 0; column < Columns; ++column) m[column] += right.m[column]; return *this; }
		basic_matrix& operator-=(const basic_matrix& right) { for (uint32_t column = 0; column < Columns; ++column) m[column] -= right.m[column]; return *this; }
		basic_matrix& operator*=(const basic_matrix& right) 
		{ 
			basic_matrix result;
			for (uint32_t column = 0; column < Columns; ++column)
				for (uint32_t row = 0; row < Rows; ++row)
					for (uint32_t index = 0; index < Columns; ++index)
						result[column][row] += (m[index][row] * right[column][index]);
			*this = result;
			return *this;
		}
		basic_matrix operator-() const
		{
			basic_matrix result = *this;
			for (uint32_t column = 0; column < Columns; ++column)
				for (uint32_t row = 0; row < Rows; ++row)
					result[column][row] = -result[column][row];
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

	typedef boost::optional<matrix11> optional_matrix11;
	typedef boost::optional<matrix22> optional_matrix22;
	typedef boost::optional<matrix21> optional_matrix21;
	typedef boost::optional<matrix12> optional_matrix12;
	typedef boost::optional<matrix33> optional_matrix33;
	typedef boost::optional<matrix31> optional_matrix31;
	typedef boost::optional<matrix32> optional_matrix32;
	typedef boost::optional<matrix13> optional_matrix13;
	typedef boost::optional<matrix23> optional_matrix23;
	typedef boost::optional<matrix44> optional_matrix44;
	typedef boost::optional<matrix41> optional_matrix41;
	typedef boost::optional<matrix42> optional_matrix42;
	typedef boost::optional<matrix43> optional_matrix43;
	typedef boost::optional<matrix14> optional_matrix14;
	typedef boost::optional<matrix24> optional_matrix24;
	typedef boost::optional<matrix34> optional_matrix34;

	typedef boost::optional<mat11> optional_mat11;
	typedef boost::optional<mat22> optional_mat22;
	typedef boost::optional<mat21> optional_mat21;
	typedef boost::optional<mat12> optional_mat12;
	typedef boost::optional<mat33> optional_mat33;
	typedef boost::optional<mat31> optional_mat31;
	typedef boost::optional<mat32> optional_mat32;
	typedef boost::optional<mat13> optional_mat13;
	typedef boost::optional<mat23> optional_mat23;
	typedef boost::optional<mat44> optional_mat44;
	typedef boost::optional<mat41> optional_mat41;
	typedef boost::optional<mat42> optional_mat42;
	typedef boost::optional<mat43> optional_mat43;
	typedef boost::optional<mat14> optional_mat14;
	typedef boost::optional<mat24> optional_mat24;
	typedef boost::optional<mat34> optional_mat34;

	template <typename T, uint32_t Rows, uint32_t Columns>
	inline basic_matrix<T, Rows, Columns> operator+(const basic_matrix<T, Rows, Columns>& left, scalar right)
	{
		basic_matrix<T, Rows, Columns> result = left;
		result += right;
		return result;
	}

	template <typename T, uint32_t Rows, uint32_t Columns>
	inline basic_matrix<T, Rows, Columns> operator-(const basic_matrix<T, Rows, Columns>& left, scalar right)
	{
		basic_matrix<T, Rows, Columns> result = left;
		result -= right;
		return result;
	}

	template <typename T, uint32_t Rows, uint32_t Columns>
	inline basic_matrix<T, Rows, Columns> operator*(const basic_matrix<T, Rows, Columns>& left, scalar right)
	{
		basic_matrix<T, Rows, Columns> result = left;
		result *= right;
		return result;
	}

	template <typename T, uint32_t Rows, uint32_t Columns>
	inline basic_matrix<T, Rows, Columns> operator/(const basic_matrix<T, Rows, Columns>& left, scalar right)
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
}
