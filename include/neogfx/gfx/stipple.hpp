// stipple.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <bit>

namespace neogfx
{
    /// @todo i_stipple
    struct stipple
    {
        stipple()
        {
        }
        template <typename T, typename = std::enable_if_t<std::is_same_v<T, double>, void>>
        stipple(std::initializer_list<T> aPattern, scalar aFactor = 1.0, scalar aPosition = 0.0) :
            pattern{ aPattern }, position{ aPosition }
        {
            std::transform(pattern.cbegin(), pattern.cend(), pattern.begin(), [&](scalar x) { return x * aFactor; });
        }
        template <std::integral T>
        stipple(T aPattern, scalar aFactor = 1.0, scalar aPosition = 0.0) :
            pattern{}, position{ aPosition }
        {
            auto bits = static_cast<std::uint16_t>(aPattern);
            int left = 16;
            while (left)
            {
                int const countOne = std::countr_one(bits);
                pattern.push_back(countOne * aFactor);
                bits >>= countOne;
                left -= countOne;
                int const countZero = std::min(left, std::countr_zero(bits));
                pattern.push_back(countZero * aFactor);
                bits >>= countZero;
                left -= countZero;
            }
            if (pattern.empty())
                pattern = { aFactor, 0.0 };
        }

        neolib::static_vector<scalar, 16> pattern = { 1.0, 0.0 };
        scalar position = 0.0;
    };
}