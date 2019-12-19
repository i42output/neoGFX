// swizzle.hpp
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

namespace neogfx
{
    namespace math
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

        template <typename V, uint32_t S>
        struct swizzle_rebind
        {
            typedef typename V::template rebind<S>::type type;
        };

        template <typename V>
        struct swizzle_rebind<V, 1>
        {
            typedef typename V::value_type type;
        };

        template <typename V, uint32_t S>
        using swizzle_rebind_t = typename swizzle_rebind<V, S>::type;

        template <typename V, typename A, uint32_t S, uint32_t... Indexes>
        struct swizzle
        {
        public:
            typedef V vector_type;
            typedef A array_type;
            typedef typename array_type::value_type value_type;
        private:
            template <uint32_t Index, uint32_t... Indexes>
            struct first
            {
                static constexpr uint32_t value = Index;
            };
        public:
            swizzle& operator=(const value_type& aRhs)
            {
                static_assert(greater_than<vector_type::Size, Indexes...>::result, "Swizzle too big");
                assign(aRhs, &v[Indexes]...);
                return *this;
            }
            template <typename T, typename SFINAE = std::enable_if_t<std::is_same_v<std::decay_t<T>, swizzle_rebind_t<vector_type, S>>, sfinae>>
            swizzle& operator=(const T& aRhs)
            {
                static_assert(greater_than<vector_type::Size, Indexes...>::result, "Swizzle too big");
                assign(std::begin(aRhs.v), &v[Indexes]...);
                return *this;
            }
        public:
            template <typename DestIter>
            void copy(DestIter aDestination) const
            {
                do_copy(aDestination, &v[Indexes]...);
            }
        private:
            template <typename Next, typename... Rest>
            void assign(value_type aValue, Next aNext, Rest... aRest)
            {
                *aNext = aValue;
                assign(aValue, aRest...);
            }
            template <typename... Rest>
            void assign(value_type, Rest...)
            {
                /* finished */
            }
            template <typename SourceIter, typename Next, typename... Rest>
            void assign(SourceIter aSource, Next aNext, Rest... aRest)
            {
                *aNext = *aSource++;
                assign(aSource, aRest...);
            }
            template <typename SourceIter, typename... Rest>
            void assign(SourceIter, Rest...)
            {
                /* finished */
            }
            template <typename DestIter, typename Next, typename... Rest>
            void do_copy(DestIter aDestination, Next aNext, Rest... aRest) const
            {
                *aDestination++ = *aNext;
                do_copy(aDestination, aRest...);
            }
            template <typename DestIter, typename... Rest>
            void do_copy(DestIter, Rest...) const
            {
                /* finished */
            }
        public:
            array_type v;
        };

        namespace operators
        {
            template <typename V, typename A, uint32_t S, uint32_t... Indexes1>
            inline neogfx::math::swizzle_rebind_t<V, S> operator~(const neogfx::math::swizzle<V, A, S, Indexes1...>& aArg)
            {
                neogfx::math::swizzle_rebind_t<V, S> result;
                aArg.copy(&result.v[0]);
                return result;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline neogfx::math::swizzle_rebind_t<V, S> operator+(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs + ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline neogfx::math::swizzle_rebind_t<V, S> operator-(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs - ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline neogfx::math::swizzle_rebind_t<V, S> operator*(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs * ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline neogfx::math::swizzle_rebind_t<V, S> operator/(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs / ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline bool operator<(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs < ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline bool operator<=(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs <= ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline bool operator>(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs > ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline bool operator>=(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs >= ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline bool operator==(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs == ~aRhs;
            }

            template <typename V, typename A, uint32_t S, uint32_t... Indexes1, uint32_t... Indexes2>
            inline bool operator!=(const neogfx::math::swizzle<V, A, S, Indexes1...>& aLhs, const neogfx::math::swizzle<V, A, S, Indexes2...>& aRhs)
            {
                return ~aLhs != ~aRhs;
            }
        }
        using namespace operators;
    }
    using namespace math::operators;
}
using namespace neogfx::math::operators;

