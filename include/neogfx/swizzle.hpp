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

#include "neogfx.hpp"

namespace neogfx
{ 
	template <typename V, uint32_t S, uint32_t... Indexes>
	class swizzle
	{
	private:
		typedef V vector_type;
		typedef typename vector_type::template rebind<S>::type sizzled_vector_type;
		typedef typename vector_type::value_type value_type;
	public:
		swizzle& operator=(const sizzled_vector_type& aRhs)
		{
			unpack_assign<0, Indexes...>(*this, aRhs);
			return *this;
		}
		operator sizzled_vector_type() const 
		{ 
			sizzled_vector_type result; 
			unpack_assign<0, Indexes...>(result, *this);
			return result;
		}
		const value_type& operator[](uint32_t aIndex) const
		{
			return iContents[aIndex];
		}
		value_type& operator[](uint32_t aIndex)
		{
			return iContents[aIndex];
		}
	private:
		template <uint32_t UnswizzledIndex, uint32_t SwizzledIndex, uint32_t... Rest> 
		void unpack_assign(sizzled_vector_type& aDestination, const swizzle& aSource) const
		{
			static_assert(UnswizzledIndex < S && SwizzledIndex < vector_type::vector_size, "Swizzle too big");
			aDestination[UnswizzledIndex] = aSource[SwizzledIndex];
			unpack_assign<UnswizzledIndex + 1, Rest...>(aDestination, aSource);
		}
		template <uint32_t>
		void unpack_assign(sizzled_vector_type&, const swizzle&) const
		{
			/* finished */
		}
		template <uint32_t UnswizzledIndex, uint32_t SwizzledIndex, uint32_t... Rest> 
		void unpack_assign(swizzle& aDestination, const sizzled_vector_type& aSource) const
		{
			static_assert(UnswizzledIndex < S && SwizzledIndex < vector_type::vector_size, "Swizzle too big");
			aDestination[SwizzledIndex] = aSource[UnswizzledIndex];
			unpack_assign<UnswizzledIndex + 1, Rest...>(aDestination, aSource);
		}
		template <uint32_t>
		void unpack_assign(swizzle&, const sizzled_vector_type&) const
		{
			/* finished */
		}
	private:
		value_type iContents[vector_type::vector_size];
	};
}
