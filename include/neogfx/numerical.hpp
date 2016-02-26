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

namespace neogfx
{ 
	typedef double scalar;

	template <typename T, uint32_t D>
	class basic_vector
	{
	public:
		typedef T value_type;
	public:
		basic_vector() : iContents{} {}
		template <typename T2, typename... Arguments>
		explicit basic_vector(T2&& aValue, Arguments&&... aArguments) { static_assert(sizeof...(Arguments) <= D - 1, "Invalid number of arguments"); unpack_assign(0, std::forward<T2>(aValue), std::forward<Arguments>(aArguments)...); }
		basic_vector(const basic_vector& aOther) { std::copy(std::begin(aOther.iContents), std::end(aOther.iContents), std::begin(iContents)); }
		basic_vector(basic_vector&& aOther) { std::move(std::begin(aOther.iContents), std::end(aOther.iContents), std::begin(iContents)); }
	public:
		uint32_t size() const { return D; }
		const value_type& operator[](uint32_t aIndex) const { return iContents[aIndex]; }
		value_type& operator[](uint32_t aIndex) { return iContents[aIndex]; }
	public:
		basic_vector& operator+=(scalar aValue) { for (std::size_t i = 0; i < size(); ++i) iContents[i] += aValue; return *this; }
		basic_vector& operator-=(scalar aValue) { for (std::size_t i = 0; i < size(); ++i) iContents[i] -= aValue; return *this; }
		basic_vector& operator*=(scalar aValue)	{ for (std::size_t i = 0; i < size(); ++i) iContents[i] *= aValue; return *this; }
		basic_vector& operator/=(scalar aValue) { for (std::size_t i = 0; i < size(); ++i) iContents[i] /= aValue; return *this; }
		basic_vector& operator+=(const basic_vector& other) { for (std::size_t i = 0; i < size(); ++i) iContents[i] += other[i]; return *this; }
		basic_vector& operator-=(const basic_vector& other) { for (std::size_t i = 0; i < size(); ++i) iContents[i] -= other[i]; return *this; }
		basic_vector& operator*=(const basic_vector& other)	{ for (std::size_t i = 0; i < size(); ++i) iContents[i] *= other[i]; return *this; }
		basic_vector& operator/=(const basic_vector& other) { for (std::size_t i = 0; i < size(); ++i) iContents[i] /= other[i]; return *this; }
	private:
		template <typename T2, typename... Arguments>
		void unpack_assign(std::size_t aIndex, T2&& aValue, Arguments&&... aArguments) { iContents[aIndex] = aValue; unpack_assign(aIndex + 1, std::forward<Arguments>(aArguments)...); }
		template <typename Xyzzy = value_type>
		typename std::enable_if<std::is_scalar<Xyzzy>::value, void>::type unpack_assign(std::size_t aIndex) { std::uninitialized_fill_n(&iContents[aIndex], size() - aIndex, value_type()); }
		template <typename Xyzzy = value_type>
		typename std::enable_if<!std::is_scalar<Xyzzy>::value, void>::type unpack_assign(std::size_t aIndex) {}
	private:
		value_type iContents[D];
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

	template <typename T, uint32_t Rows, uint32_t Columns>
	class basic_matrix
	{
	public:
		typedef T value_type;
		typedef basic_vector<T, Columns> row_type;
		typedef basic_vector<T, Rows> column_type;
	public:
		basic_matrix() : iContents{} {}
		template <typename T2, typename... Arguments>
		explicit basic_matrix(T2&& aValue, Arguments&&... aArguments) { static_assert(sizeof...(Arguments) <= Columns - 1, "Invalid number of arguments"); unpack_assign(0, std::forward<T2>(aValue), std::forward<Arguments>(aArguments)...); }
		basic_matrix(const basic_matrix& aOther) { std::copy(std::begin(aOther.iContents), std::end(aOther.iContents), std::begin(iContents)); }
		basic_matrix(basic_matrix&& aOther) { std::move(std::begin(aOther.iContents), std::end(aOther.iContents), std::begin(iContents)); }
	public:
		std::pair<uint32_t, uint32_t> size() const { return std::make_pair(Rows, Columns); }
		const column_type& operator[](uint32_t aColumn) const { return iContents[aColumn]; }
		column_type& operator[](uint32_t aColumn) { return iContents[aColumn]; }
	public:
		basic_matrix& operator+=(scalar aValue) { for (std::size_t i = 0; i < size(); ++i) iContents[i] += aValue; return *this; }
		basic_matrix& operator-=(scalar aValue) { for (std::size_t i = 0; i < size(); ++i) iContents[i] -= aValue; return *this; }
		basic_matrix& operator*=(scalar aValue)	{ for (std::size_t i = 0; i < size(); ++i) iContents[i] *= aValue; return *this; }
		basic_matrix& operator/=(scalar aValue) { for (std::size_t i = 0; i < size(); ++i) iContents[i] /= aValue; return *this; }
	private:
		template <typename T2, typename... Arguments>
		void unpack_assign(std::size_t aIndex, T2&& aValue, Arguments&&... aArguments) { iContents[aIndex] = aValue; unpack_assign(aIndex + 1, std::forward<Arguments>(aArguments)...); }
		template <typename Xyzzy = value_type>
		typename std::enable_if<std::is_scalar<Xyzzy>::value, void>::type unpack_assign(std::size_t aIndex) { std::uninitialized_fill_n(&iContents[aIndex], size() - aIndex, value_type()); }
		template <typename Xyzzy = value_type>
		typename std::enable_if<!std::is_scalar<Xyzzy>::value, void>::type unpack_assign(std::size_t aIndex) {}
	private:
		column_type iContents[Columns];
	};

	typedef basic_matrix<double, 1, 1> matrix11;
	typedef basic_matrix<double, 2, 2> matrix22;
	typedef basic_matrix<double, 3, 3> matrix33;
	typedef basic_matrix<double, 4, 4> matrix44;
}
