// simd.hpp
/*
  neogfx C++ App/Game Engine
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
#include <array>
#include <thread>
#include <cstdlib>
#ifdef USE_AVX
#include <immintrin.h>
#endif
#ifdef USE_EMM
#include <emmintrin.h>
#endif

namespace neogfx
{ 
    inline double simd_fma_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2)
    {
#ifdef USE_AVX
        alignas(32) __m256d lhs = _mm256_set_pd(x1, y1, z1, w1);
        alignas(32) __m256d rhs = _mm256_set_pd(x2, y2, z2, w2);
        alignas(32) __m256d ans = _mm256_mul_pd(lhs, rhs);
        return ans.m256d_f64[0] + ans.m256d_f64[1] + ans.m256d_f64[2] + ans.m256d_f64[3];
#else
        return x1 * x2 + y1 * y2 + z1 * z2 + w1 * w2;
#endif
    }

    inline void simd_mul_4d(double x1, double x2, double y1, double y2, double z1, double z2, double w1, double w2, double& a, double& b, double& c, double& d)
    {
#ifdef USE_AVX
        alignas(32) __m256d lhs = _mm256_set_pd(x1, y1, z1, w1);
        alignas(32) __m256d rhs = _mm256_set_pd(x2, y2, z2, w2);
        alignas(32) __m256d ans = _mm256_mul_pd(lhs, rhs);
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

    /////////////////////////////////////////////////////////////////////////////
    // The Software is provided "AS IS" and possibly with faults. 
    // Intel disclaims any and all warranties and guarantees, express, implied or
    // otherwise, arising, with respect to the software delivered hereunder,
    // including but not limited to the warranty of merchantability, the warranty
    // of fitness for a particular purpose, and any warranty of non-infringement
    // of the intellectual property rights of any third party.
    // Intel neither assumes nor authorizes any person to assume for it any other
    // liability. Customer will use the software at its own risk. Intel will not
    // be liable to customer for any direct or indirect damages incurred in using
    // the software. In no event will Intel be liable for loss of profits, loss of
    // use, loss of data, business interruption, nor for punitive, incidental,
    // consequential, or special damages of any kind, even if advised of
    // the possibility of such damages.
    //
    // Copyright (c) 2003 Intel Corporation
    //
    // Third-party brands and names are the property of their respective owners
    //
    ///////////////////////////////////////////////////////////////////////////
    // Random Number Generation for SSE / SSE2
    // Source File
    // Version 0.1
    // Author Kipp Owens, Rajiv Parikh
    ////////////////////////////////////////////////////////////////////////

    namespace detail
    {
#ifdef USE_EMM
        inline __m128i& simd_rand_seed()
        {
            alignas(16) thread_local __m128i tSeed;
            return tSeed;
        }
#endif
    }

    inline void simd_srand(uint32_t seed)
    {
#ifdef USE_EMM
        detail::simd_rand_seed() = _mm_set_epi32(seed, seed + 1, seed, seed + 1);
#else
        std::srand(seed);
#endif
    }

    inline void simd_srand(std::thread::id seed)
    {
        simd_srand(static_cast<uint32_t>(std::hash<std::thread::id>{}(seed)));
    }

    inline uint32_t simd_rand()
    {
        thread_local std::array<uint32_t, 4> result = {};
        thread_local std::size_t resultCounter = 4;
        if (resultCounter < 4)
            return result[resultCounter++];
#ifdef USE_EMM
        alignas(16) __m128i cur_seed_split;
        alignas(16) __m128i multiplier;
        alignas(16) __m128i adder;
        alignas(16) __m128i mod_mask;
        alignas(16) __m128i sra_mask;
        alignas(16) __m128i ans;
        alignas(16) static const uint32_t mult[4] =
        { 214013, 17405, 214013, 69069 };
        alignas(16) static const uint32_t gadd[4] =
        { 2531011, 10395331, 13737667, 1 };
        alignas(16) static const uint32_t mask[4] =
        { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
        alignas(16) static const uint32_t masklo[4] =
        { 0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF };

        adder = _mm_load_si128((__m128i*) gadd);
        multiplier = _mm_load_si128((__m128i*) mult);
        mod_mask = _mm_load_si128((__m128i*) mask);
        sra_mask = _mm_load_si128((__m128i*) masklo);

        cur_seed_split = _mm_shuffle_epi32(detail::simd_rand_seed(), _MM_SHUFFLE(2, 3, 0, 1));

        detail::simd_rand_seed() = _mm_mul_epu32(detail::simd_rand_seed(), multiplier);

        multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
        cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);

        detail::simd_rand_seed() = _mm_and_si128(detail::simd_rand_seed(), mod_mask);

        cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
        cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));

        detail::simd_rand_seed() = _mm_or_si128(detail::simd_rand_seed(), cur_seed_split);
        detail::simd_rand_seed() = _mm_add_epi32(detail::simd_rand_seed(), adder);

        _mm_storeu_si128(&ans, detail::simd_rand_seed());
        result = { ans.m128i_u32[0], ans.m128i_u32[1], ans.m128i_u32[2], ans.m128i_u32[3] };
#else
        result = { std::rand(), std::rand(), std::rand(), std::rand() };
#endif
        resultCounter = 0;
        return result[resultCounter];
    }

    template <typename T>
    inline T simd_rand(T aUpper)
    {
        return static_cast<T>(simd_rand() % static_cast<uint32_t>(aUpper));
    }
}
