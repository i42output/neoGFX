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

#include "neogfx.hpp"
#include <type_traits>
#include <array>
#include <algorithm>
#include <ostream>
#include "swizzle.hpp"

namespace neogfx
{ 
	typedef double scalar;

	template <typename T, uint32_t Size, bool IsScalar=std::is_scalar<T>::value>
	class basic_vector;

	template <typename T, uint32_t _Size>
	class basic_vector<T, _Size, true>
	{
	public:
		enum : uint32_t { Size = _Size };
	public:
		typedef T value_type;
		typedef basic_vector<value_type, Size> vector_type;
		typedef uint32_t size_type;
		typedef std::array<value_type, Size> array_type;
	public:
		template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2> type; };
	public:
		basic_vector() : v{} {}
		explicit basic_vector(value_type aValue) : v{{aValue}} {}
		template <typename... Arguments>
		explicit basic_vector(value_type aValue, Arguments... aArguments) : v{{aValue, aArguments...}} {}
		basic_vector(std::initializer_list<value_type> aValues) { std::copy(aValues.begin(), aValues.end(), v.begin()); }
		basic_vector(const basic_vector& aOther) : v(aOther.v) {}
		basic_vector(basic_vector&& aOther) : v(std::move(aOther.v)) {}
		basic_vector& operator=(const basic_vector& aOther) { v = aOther.v; return *this; }
		basic_vector& operator=(basic_vector&& aOther) { v = std::move(aOther.v); return *this; }
	public:
		static uint32_t size() { return Size; }
		const value_type& operator[](uint32_t aIndex) const { return v[aIndex]; }
		value_type& operator[](uint32_t aIndex) { return v[aIndex]; }
	public:
		basic_vector& operator+=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](value_type aElement) { return aElement + aValue; }); return *this; }
		basic_vector& operator-=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](value_type aElement) { return aElement - aValue; }); return *this; }
		basic_vector& operator*=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](value_type aElement) { return aElement * aValue; }); return *this; }
		basic_vector& operator/=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](value_type aElement) { return aElement / aValue; }); return *this; }
		basic_vector& operator+=(const basic_vector& other) { std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](value_type aLhs, value_type aRhs) { return aLhs + aRhs; }); return *this; }
		basic_vector& operator-=(const basic_vector& other) { std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](value_type aLhs, value_type aRhs) { return aLhs - aRhs; }); return *this; }
		basic_vector& operator*=(const basic_vector& other)	{ std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](value_type aLhs, value_type aRhs) { return aLhs * aRhs; }); return *this; }
		basic_vector& operator/=(const basic_vector& other) { std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](value_type aLhs, value_type aRhs) { return aLhs / aRhs; }); return *this; }
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

	template <typename T, uint32_t _Size>
	class basic_vector<T, _Size, false>
	{
	public:
		enum : uint32_t { Size = _Size };
	public:
		typedef T value_type;
		typedef basic_vector<value_type, Size> vector_type;
		typedef uint32_t size_type;
		typedef std::array<value_type, Size> array_type;
	public:
		template <uint32_t Size2> struct rebind { typedef basic_vector<T, Size2> type; };
	public:
		basic_vector() : v{} {}
		explicit basic_vector(value_type aValue) : v{{aValue}} {}
		template <typename... Arguments>
		explicit basic_vector(const value_type& aValue, Arguments&&... aArguments) : v{{aValue, std::forward<Arguments>(aArguments)...}} {}
		template <typename... Arguments>
		explicit basic_vector(value_type&& aValue, Arguments&&... aArguments) : v{{std::move(aValue), std::forward<Arguments>(aArguments)...}} {}
		basic_vector(std::initializer_list<value_type> aValues) { std::copy(aValues.begin(), aValues.end(), v.begin()); }
		basic_vector(const basic_vector& aOther) : v(aOther.v) {}
		basic_vector(basic_vector&& aOther) : v(std::move(aOther.v)) {}
		basic_vector& operator=(const basic_vector& aOther) { v = aOther.v; return *this; }
		basic_vector& operator=(basic_vector&& aOther) { v = std::move(aOther.v); return *this; }
	public:
		static uint32_t size() { return Size; }
		const value_type& operator[](uint32_t aIndex) const { return v[aIndex]; }
		value_type& operator[](uint32_t aIndex) { return v[aIndex]; }
	public:
		basic_vector& operator+=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](const value_type& aElement) { return aElement + aValue; }); return *this; }
		basic_vector& operator-=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](const value_type& aElement) { return aElement - aValue; }); return *this; }
		basic_vector& operator*=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](const value_type& aElement) { return aElement * aValue; }); return *this; }
		basic_vector& operator/=(scalar aValue) { std::transform(std::begin(v), std::end(v), std::begin(v), [aValue](const value_type& aElement) { return aElement / aValue; }); return *this; }
		basic_vector& operator+=(const basic_vector& other) { std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](const value_type& aLhs, const value_type& aRhs) { return aLhs + aRhs; }); return *this; }
		basic_vector& operator-=(const basic_vector& other) { std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](const value_type& aLhs, const value_type& aRhs) { return aLhs - aRhs; }); return *this; }
		basic_vector& operator*=(const basic_vector& other)	{ std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](const value_type& aLhs, const value_type& aRhs) { return aLhs * aRhs; }); return *this; }
		basic_vector& operator/=(const basic_vector& other) { std::transform(std::begin(v), std::end(v), std::begin(other.v), std::begin(v), [](const value_type& aLhs, const value_type& aRhs) { return aLhs / aRhs; }); return *this; }
	public:
		array_type v;
	};

	typedef basic_vector<double, 1> vector1;
	typedef basic_vector<double, 2> vector2;
	typedef basic_vector<double, 3> vector3;
	typedef basic_vector<double, 4> vector4;

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator+(const basic_vector<T, D>& left, const basic_vector<T, D>& right)
	{
		basic_vector<T, D> result = left;
		result += right;
		return result;
	}

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator-(const basic_vector<T, D>& left, const basic_vector<T, D>& right)
	{
		basic_vector<T, D> result = left;
		result -= right;
		return result;
	}

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator*(const basic_vector<T, D>& left, const basic_vector<T, D>& right)
	{
		basic_vector<T, D> result = left;
		result *= right;
		return result;
	}

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator/(const basic_vector<T, D>& left, const basic_vector<T, D>& right)
	{
		basic_vector<T, D> result = left;
		result /= right;
		return result;
	}

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator+(const basic_vector<T, D>& left, const T& right)
	{
		basic_vector<T, D> result = left;
		for (uint32_t i = 0; i < D; ++i)
			result[i] += right;
		return result;
	}

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator-(const basic_vector<T, D>& left, const T& right)
	{
		basic_vector<T, D> result = left;
		for (uint32_t i = 0; i < D; ++i)
			result[i] -= right;
		return result;
	}

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator*(const basic_vector<T, D>& left, const T& right)
	{
		basic_vector<T, D> result = left;
		for (uint32_t i = 0; i < D; ++i)
			result[i] *= right;
		return result;
	}

	template <typename T, uint32_t D>
	inline basic_vector<T, D> operator/(const basic_vector<T, D>& left, const T& right)
	{
		basic_vector<T, D> result = left;
		for (uint32_t i = 0; i < D; ++i)
			result[i] /= right;
		return result;
	}

	template <typename Elem, typename Traits, typename T, uint32_t Size>
	std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const basic_vector<T, Size>& aVector)
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

	template <typename T, uint32_t Rows, uint32_t Columns>
	class basic_matrix
	{
	public:
		typedef T value_type;
		typedef basic_vector<T, Columns> row_type;
		typedef basic_vector<T, Rows> column_type;
		typedef std::array<column_type, Columns> array_type;
	public:
		basic_matrix() : m{{}} {}
		basic_matrix(std::initializer_list<column_type> aColumns) { std::copy(aColumns.begin(), aColumns.end(), m.begin()); }
		basic_matrix(const basic_matrix& aOther) : m{aOther.m} {}
		basic_matrix(basic_matrix&& aOther) : m{std::move(aOther.m)} {}
		basic_matrix& operator=(const basic_matrix& aOther) { m = aOther.m; return *this; }
		basic_matrix& operator=(basic_matrix&& aOther) { m = std::move(aOther.m); return *this; }
	public:
		std::pair<uint32_t, uint32_t> size() const { return std::make_pair(Rows, Columns); }
		const column_type& operator[](uint32_t aColumn) const { return m[aColumn]; }
		column_type& operator[](uint32_t aColumn) { return m[aColumn]; }
	public:
		basic_matrix& operator+=(scalar aValue) { std::transform(std::begin(m), std::end(m), std::begin(m), [aValue](const column_type& aElement) { return aElement + aValue; }); return *this; }
		basic_matrix& operator-=(scalar aValue) { std::transform(std::begin(m), std::end(m), std::begin(m), [aValue](const column_type& aElement) { return aElement - aValue; }); return *this; }
		basic_matrix& operator*=(scalar aValue) { std::transform(std::begin(m), std::end(m), std::begin(m), [aValue](const column_type& aElement) { return aElement * aValue; }); return *this; }
		basic_matrix& operator/=(scalar aValue) { std::transform(std::begin(m), std::end(m), std::begin(m), [aValue](const column_type& aElement) { return aElement / aValue; }); return *this; }
	private:
		array_type m;
	};

	typedef basic_matrix<double, 1, 1> matrix11;
	typedef basic_matrix<double, 2, 2> matrix22;
	typedef basic_matrix<double, 3, 3> matrix33;
	typedef basic_matrix<double, 4, 4> matrix44;
}
