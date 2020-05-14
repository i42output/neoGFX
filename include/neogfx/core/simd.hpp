// simd.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#ifdef USE_AVX
#include <immintrin.h>
#endif

namespace neogfx
{ 
    inline double simd_fma_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2)
    {
#ifdef USE_AVX
        __m256d lhs = _mm256_set_pd(x1, y1, z1, w1);
        __m256d rhs = _mm256_set_pd(x2, y2, z2, w2);
        __m256d ans = _mm256_mul_pd(lhs, rhs);
        return ans.m256d_f64[0] + ans.m256d_f64[1] + ans.m256d_f64[2] + ans.m256d_f64[3];
#else
        return x1 * x2 + y1 * y2 + z1 * z2 + w1 * w2;
#endif
    }

    inline void simd_mul_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2, double& a, double& b, double& c, double& d)
    {
#ifdef USE_AVX
        __m256d lhs = _mm256_set_pd(x1, y1, z1, w1);
        __m256d rhs = _mm256_set_pd(x2, y2, z2, w2);
        __m256d ans = _mm256_mul_pd(lhs, rhs);
        a = ans.m256d_f64[0];
        b = ans.m256d_f64[1];
        c = ans.m256d_f64[2];
        d = ans.m256d_f64[3];
#else
        a = x1 * x2;
        b = y1 * y2;
        c = z1 * z2;
        d = w1 * w2;
#endif
    }
}
