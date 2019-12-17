// swizzle_array.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/swizzle.hpp>

namespace neogfx
{
    namespace math
    {
        template <typename V, typename T, uint32_t Size>
        struct swizzle_array
        {
            typedef T value_type;
            typedef std::array<value_type, Size> array_type;
            typedef V vector_type;

            union
            {
                array_type v;
                struct // todo: alignment, padding?
                {
                    value_type x;
                    value_type y;
                    value_type z;
                };
                swizzle<vector_type, array_type, 2, 0, 0> xx;
                swizzle<vector_type, array_type, 2, 0, 1> xy;
                swizzle<vector_type, array_type, 2, 0, 2> xz;
                swizzle<vector_type, array_type, 2, 1, 0> yx;
                swizzle<vector_type, array_type, 2, 1, 1> yy;
                swizzle<vector_type, array_type, 2, 1, 2> yz;
                swizzle<vector_type, array_type, 2, 2, 0> zx;
                swizzle<vector_type, array_type, 2, 2, 1> zy;
                swizzle<vector_type, array_type, 2, 2, 2> zz;
                swizzle<vector_type, array_type, 3, 0, 0, 0> xxx;
                swizzle<vector_type, array_type, 3, 0, 0, 1> xxy;
                swizzle<vector_type, array_type, 3, 0, 0, 2> xxz;
                swizzle<vector_type, array_type, 3, 0, 1, 0> xyx;
                swizzle<vector_type, array_type, 3, 0, 1, 1> xyy;
                swizzle<vector_type, array_type, 3, 0, 1, 2> xyz;
                swizzle<vector_type, array_type, 3, 1, 0, 0> yxx;
                swizzle<vector_type, array_type, 3, 1, 0, 1> yxy;
                swizzle<vector_type, array_type, 3, 1, 0, 2> yxz;
                swizzle<vector_type, array_type, 3, 1, 1, 0> yyx;
                swizzle<vector_type, array_type, 3, 1, 1, 1> yyy;
                swizzle<vector_type, array_type, 3, 1, 1, 2> yyz;
                swizzle<vector_type, array_type, 3, 1, 2, 0> yzx;
                swizzle<vector_type, array_type, 3, 1, 2, 1> yzy;
                swizzle<vector_type, array_type, 3, 1, 2, 2> yzz;
                swizzle<vector_type, array_type, 3, 2, 0, 0> zxx;
                swizzle<vector_type, array_type, 3, 2, 0, 1> zxy;
                swizzle<vector_type, array_type, 3, 2, 0, 2> zxz;
                swizzle<vector_type, array_type, 3, 2, 1, 0> zyx;
                swizzle<vector_type, array_type, 3, 2, 1, 1> zyy;
                swizzle<vector_type, array_type, 3, 2, 1, 2> zyz;
                swizzle<vector_type, array_type, 3, 2, 2, 0> zzx;
                swizzle<vector_type, array_type, 3, 2, 2, 1> zzy;
                swizzle<vector_type, array_type, 3, 2, 2, 2> zzz;
            };
        };

        template <typename V, typename T>
        struct swizzle_array<V, T, 1>
        {
            typedef T value_type;
            typedef std::array<value_type, 1> array_type;
            typedef V vector_type;

            union
            {
                array_type v;
                struct // todo: alignment, padding?
                {
                    value_type x;
                };
                swizzle<vector_type, array_type, 2, 0, 0> xx;
                swizzle<vector_type, array_type, 3, 0, 0, 0> xxx;
            };
        };

        template <typename V, typename T>
        struct swizzle_array<V, T, 2>
        {
            typedef T value_type;
            typedef std::array<value_type, 2> array_type;
            typedef V vector_type;

            union
            {
                array_type v;
                struct // todo: alignment, padding?
                {
                    value_type x;
                    value_type y;
                };
                swizzle<vector_type, array_type, 2, 0, 0> xx;
                swizzle<vector_type, array_type, 2, 0, 1> xy;
                swizzle<vector_type, array_type, 2, 1, 0> yx;
                swizzle<vector_type, array_type, 2, 1, 1> yy;
                swizzle<vector_type, array_type, 3, 0, 0, 0> xxx;
                swizzle<vector_type, array_type, 3, 0, 0, 1> xxy;
                swizzle<vector_type, array_type, 3, 0, 1, 0> xyx;
                swizzle<vector_type, array_type, 3, 0, 1, 1> xyy;
                swizzle<vector_type, array_type, 3, 1, 0, 0> yxx;
                swizzle<vector_type, array_type, 3, 1, 0, 1> yxy;
                swizzle<vector_type, array_type, 3, 1, 1, 0> yyx;
                swizzle<vector_type, array_type, 3, 1, 1, 1> yyy;
            };
        };

        template <typename V, typename T>
        struct swizzle_array<V, T, 3>
        {
            typedef T value_type;
            typedef std::array<value_type, 3> array_type;
            typedef V vector_type;

            union
            {
                array_type v;
                struct // todo: alignment, padding?
                {
                    value_type x;
                    value_type y;
                    value_type z;
                };
                swizzle<vector_type, array_type, 2, 0, 0> xx;
                swizzle<vector_type, array_type, 2, 0, 1> xy;
                swizzle<vector_type, array_type, 2, 0, 2> xz;
                swizzle<vector_type, array_type, 2, 1, 0> yx;
                swizzle<vector_type, array_type, 2, 1, 1> yy;
                swizzle<vector_type, array_type, 2, 1, 2> yz;
                swizzle<vector_type, array_type, 2, 2, 0> zx;
                swizzle<vector_type, array_type, 2, 2, 1> zy;
                swizzle<vector_type, array_type, 2, 2, 2> zz;
                swizzle<vector_type, array_type, 3, 0, 0, 0> xxx;
                swizzle<vector_type, array_type, 3, 0, 0, 1> xxy;
                swizzle<vector_type, array_type, 3, 0, 0, 2> xxz;
                swizzle<vector_type, array_type, 3, 0, 1, 0> xyx;
                swizzle<vector_type, array_type, 3, 0, 1, 1> xyy;
                swizzle<vector_type, array_type, 3, 0, 1, 2> xyz;
                swizzle<vector_type, array_type, 3, 1, 0, 0> yxx;
                swizzle<vector_type, array_type, 3, 1, 0, 1> yxy;
                swizzle<vector_type, array_type, 3, 1, 0, 2> yxz;
                swizzle<vector_type, array_type, 3, 1, 1, 0> yyx;
                swizzle<vector_type, array_type, 3, 1, 1, 1> yyy;
                swizzle<vector_type, array_type, 3, 1, 1, 2> yyz;
                swizzle<vector_type, array_type, 3, 1, 2, 0> yzx;
                swizzle<vector_type, array_type, 3, 1, 2, 1> yzy;
                swizzle<vector_type, array_type, 3, 1, 2, 2> yzz;
                swizzle<vector_type, array_type, 3, 2, 0, 0> zxx;
                swizzle<vector_type, array_type, 3, 2, 0, 1> zxy;
                swizzle<vector_type, array_type, 3, 2, 0, 2> zxz;
                swizzle<vector_type, array_type, 3, 2, 1, 0> zyx;
                swizzle<vector_type, array_type, 3, 2, 1, 1> zyy;
                swizzle<vector_type, array_type, 3, 2, 1, 2> zyz;
                swizzle<vector_type, array_type, 3, 2, 2, 0> zzx;
                swizzle<vector_type, array_type, 3, 2, 2, 1> zzy;
                swizzle<vector_type, array_type, 3, 2, 2, 2> zzz;
            };
        };
    }
}
