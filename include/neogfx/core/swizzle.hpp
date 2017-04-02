// swizzle.hpp
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

namespace neogfx
{ 
	namespace detail
	{
		template<uint32_t... Rest>
		struct greater_than
		{
			// Will replace this enum hack with constexpr when I upgrade my compiler from VS2013 :/
			enum { result = true };
		};

		template<uint32_t Lhs, uint32_t Rhs, uint32_t... Rest>
		struct greater_than<Lhs, Rhs, Rest...>
		{
			// Will replace this enum hack with constexpr when I upgrade my compiler from VS2013 :/
			enum { result = Lhs > Rhs && greater_than<Lhs, Rest...>::result };
		};
	}

	template <typename V, uint32_t S, uint32_t... Indexes>
	struct swizzle
	{
	private:
		typedef V vector_type;
		typedef typename vector_type::template rebind<S>::type sizzled_vector_type;
		typedef typename vector_type::value_type value_type;
		typedef typename vector_type::array_type array_type;
	private:
		template <uint32_t Index, uint32_t... Indexes>
		struct first
		{
			static constexpr uint32_t value = Index;
		};
	public:
		swizzle& operator=(const sizzled_vector_type& aRhs)
		{
			static_assert(detail::greater_than<vector_type::Size, Indexes...>::result, "Swizzle too big");
			assign(std::begin(aRhs.v), &v[Indexes]...);
			return *this;
		}
		sizzled_vector_type operator*() const
		{
			static_assert(detail::greater_than<vector_type::Size, Indexes...>::result, "Swizzle too big");
			return sizzled_vector_type(v[Indexes]...);
		}
		operator sizzled_vector_type() const 
		{ 
			return **this;
		}
		template <uint32_t S2 = S>
		operator typename std::enable_if<S2 == 1, value_type>::type() const
		{
			return v[first<Indexes...>::value];
		}
	private:
		template <typename SourceIter, typename Next, typename... Rest>
		void assign(SourceIter aSource, Next aNext, Rest... aRest)
		{
			*aNext = *aSource++;
			assign(aSource, aRest...);
		}
		template <typename SourceIter, typename... Rest>
		void assign(SourceIter aFirst, Rest... aRest)
		{
			/* finished */
		}
	public:
		array_type v;
	};

	template <typename V, uint32_t S, uint32_t Index1, uint32_t Index2>
	inline typename V::value_type operator+(const swizzle<V, S, Index1>& aLhs, const swizzle<V, S, Index2>& aRhs)
	{
		return static_cast<typename V::value_type>(aLhs) + static_cast<typename V::value_type>(aRhs);
	}

	template <typename V, uint32_t S, uint32_t Index1, uint32_t Index2>
	inline typename V::value_type operator-(const swizzle<V, S, Index1>& aLhs, const swizzle<V, S, Index2>& aRhs)
	{
		return static_cast<typename V::value_type>(aLhs) - static_cast<typename V::value_type>(aRhs);
	}

	template <typename V, uint32_t S, uint32_t Index1, uint32_t Index2>
	inline typename V::value_type operator*(const swizzle<V, S, Index1>& aLhs, const swizzle<V, S, Index2>& aRhs)
	{
		return static_cast<typename V::value_type>(aLhs) * static_cast<typename V::value_type>(aRhs);
	}

	template <typename V, uint32_t S, uint32_t Index1, uint32_t Index2>
	inline typename V::value_type operator/(const swizzle<V, S, Index1>& aLhs, const swizzle<V, S, Index2>& aRhs)
	{
		return static_cast<typename V::value_type>(aLhs) / static_cast<typename V::value_type>(aRhs);
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline typename swizzle<V, S, Indexes1...>::sizzled_vector_type operator+(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs + *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline typename swizzle<V, S, Indexes1...>::sizzled_vector_type operator-(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs - *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline typename swizzle<V, S, Indexes1...>::sizzled_vector_type operator*(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs * *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline typename swizzle<V, S, Indexes1...>::sizzled_vector_type operator/(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs / *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline bool operator<(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs < *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline bool operator<=(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs <= *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline bool operator>(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs > *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline bool operator>=(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs >= *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline bool operator==(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs == *aRhs;
	}

	template <typename V, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
	inline bool operator!=(const swizzle<V, S, Indexes1...>& aLhs, const swizzle<V, S, Indexes2...>& aRhs)
	{
		return *aLhs != *aRhs;
	}
}
