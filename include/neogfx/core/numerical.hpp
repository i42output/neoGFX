// numerical.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c)  2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/numerical.hpp>

namespace neogfx
{
    namespace math
    {
        using namespace boost::math::constants;
        using namespace neolib::math;
    }

    using namespace boost::math::constants;
    using namespace neolib::math;

    using neolib::math::operators::operator~;
    using neolib::math::operators::operator+;
    using neolib::math::operators::operator-;
    using neolib::math::operators::operator*;
    using neolib::math::operators::operator/;
    using neolib::math::operators::operator<;
    using neolib::math::operators::operator>;
    using neolib::math::operators::operator<=;
    using neolib::math::operators::operator>=;
    using neolib::math::operators::operator==;
    using neolib::math::operators::operator!=;

    using neolib::math::nearly_equal;

    // why? most of these operations would normally be indeterminate; only use these functions if inf is being used as a special a sentinal value (e.g. neoGFX unbounded dimensions)
    template <typename T>
    inline T inf_add(T lhs, T rhs)
    {
        if (lhs != std::abs(std::numeric_limits<T>::infinity()) && rhs != std::abs(std::numeric_limits<T>::infinity()))
            return lhs + rhs;
        else if (lhs == std::numeric_limits<T>::infinity() && rhs == std::numeric_limits<T>::infinity())
            return std::numeric_limits<T>::infinity();
        else if (lhs == -std::numeric_limits<T>::infinity() && rhs == -std::numeric_limits<T>::infinity())
            return -std::numeric_limits<T>::infinity();
        else if (lhs == -std::numeric_limits<T>::infinity() && rhs == std::numeric_limits<T>::infinity())
            return 0.0;
        else if (lhs == std::numeric_limits<T>::infinity() && rhs == -std::numeric_limits<T>::infinity())
            return 0.0;
        else if (lhs == std::numeric_limits<T>::infinity() || rhs == std::numeric_limits<T>::infinity())
            return std::numeric_limits<T>::infinity();
        else
            return -std::numeric_limits<T>::infinity();
    }

    // why? most of these operations would normally be indeterminate; only use these functions if inf is being used as a special a sentinal value (e.g. neoGFX unbounded dimensions)
    template <typename T>
    inline T inf_multiply(T lhs, T rhs)
    {
        if (lhs == 0.0 || rhs == 0.0)
            return 0.0;
        else if (lhs != std::numeric_limits<T>::infinity() && rhs != std::numeric_limits<T>::infinity())
            return lhs * rhs;
        else if ((lhs > 0.0 && rhs > 0.0) || (lhs < 0.0 && rhs < 0.0))
            return std::numeric_limits<T>::infinity();
        else
            return -std::numeric_limits<T>::infinity();
    }

    // why? most of these operations would normally be indeterminate; only use these functions if inf is being used as a special a sentinal value (e.g. neoGFX unbounded dimensions)
    template <typename T, uint32_t D>
    inline basic_vector<T, D, column_vector> inf_multiply(const basic_matrix<T, D, D>& left, const basic_vector<T, D, column_vector>& right)
    {
        if (left.is_identity())
            return right;
        basic_vector<T, D, column_vector> result;
        for (uint32_t row = 0; row < D; ++row)
            for (uint32_t index = 0; index < D; ++index)
                result[row] = inf_add(result[row], inf_multiply(left[index][row], right[index]));
        return result;
    }

    // why? most of these operations would normally be indeterminate; only use these functions if inf is being used as a special a sentinal value (e.g. neoGFX unbounded dimensions)
    template <typename T, uint32_t D>
    inline basic_vector<T, D, row_vector> inf_multiply(const basic_vector<T, D, row_vector>& left, const basic_matrix<T, D, D>& right)
    {
        if (right.is_identity())
            return left;
        basic_vector<T, D, row_vector> result;
        for (uint32_t column = 0; column < D; ++column)
            for (uint32_t index = 0; index < D; ++index)
                result[column] = inf_add(result[column], inf_multiply(left[index], right[column][index]));
        return result;
    }

    template <typename T>
    inline T mix(const T& aLhs, const T& aRhs, double aMixValue)
    {
        if constexpr (std::is_scalar_v<T>)
            return static_cast<T>(static_cast<double>(aLhs) * (1.0 - aMixValue) + static_cast<double>(aRhs) * aMixValue);
        else
            return aLhs;
    }

    template <typename T1, typename T2>
    inline T1 mix(const T1& aLhs, const T2&, double)
    {
        return aLhs;
    }

    template <typename T>
    inline T mix(const std::optional<T>& aLhs, const std::optional<T>& aRhs, double aMixValue)
    {
        if (!aLhs.has_value() && !aRhs.has_value())
            return T{};
        else if (aLhs.has_value() && !aRhs.has_value())
            return *aLhs;
        else if (!aLhs.has_value() && aRhs.has_value())
            return *aRhs;
        else
            return mix(*aLhs, *aRhs, aMixValue);
    }
}
