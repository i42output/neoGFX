// easing.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
/* TERMS OF USE - EASING EQUATIONS
 * Open source under the BSD License.
 * Copyright(c)2001 Robert Penner
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met :
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of the author nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <neogfx/neogfx.hpp>

namespace neogfx
{

	template <typename T>
	inline T ease_linear(T t)
	{
		return t;
	}

	template <typename T>
	inline T ease_in_sine(T t)
	{
		return 1.0 - std::cos(t * half_pi<T>());
	}

	template <typename T>
	inline T ease_out_sine(T t)
	{
		return 1.0 - ease_in_sine(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_sine(T t)
	{
		return (t < 0.5 ? ease_in_sine(t * 2.0) : 1.0 + ease_out_sine((t - 0.5) * 2.0)) / 2.0;
	}

	template <typename T>
	inline T ease_in_quad(T t)
	{
		return t * t;
	}

	template <typename T>
	inline T ease_out_quad(T t)
	{
		return 1.0 - ease_in_quad(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_quad(T t)
	{
		return (t < 0.5 ? ease_in_quad(t * 2.0) : 1.0 + ease_out_quad((t - 0.5) * 2.0)) / 2.0;
	}

	template <typename T>
	inline T ease_in_cubic(T t)
	{
		return t * t * t;
	}

	template <typename T>
	inline T ease_out_cubic(T t)
	{
		return 1.0 - ease_in_cubic(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_cubic(T t)
	{
		return (t < 0.5 ? ease_in_cubic(t * 2.0) : 1.0 + ease_out_cubic((t - 0.5) * 2.0)) / 2.0;
	}

	template <typename T>
	inline T ease_in_quart(T t)
	{
		return t * t * t * t;
	}

	template <typename T>
	inline T ease_out_quart(T t)
	{
		return 1.0 - ease_in_quart(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_quart(T t)
	{
		return (t < 0.5 ? ease_in_quart(t * 2.0) : 1.0 + ease_out_quart((t - 0.5) * 2.0)) / 2.0;
	}

	template <typename T>
	inline T ease_in_quint(T t)
	{
		return t * t * t * t * t;
	}

	template <typename T>
	inline T ease_out_quint(T t)
	{
		return 1.0 - ease_in_quint(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_quint(T t)
	{
		return (t < 0.5 ? ease_in_quint(t * 2.0) : 1.0 + ease_out_quint((t - 0.5) * 2.0)) / 2.0;
	}

	template <typename T>
	inline T ease_in_expo(T t)
	{
		return std::pow(2.0, 10 * (t - 1.0));
	}

	template <typename T>
	inline T ease_out_expo(T t)
	{
		return 1.0 - ease_in_expo(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_expo(T t)
	{
		return (t < 0.5 ? ease_in_expo(t * 2.0) : 1.0 + ease_out_expo((t - 0.5) * 2.0)) / 2.0;
	}

	template <typename T>
	inline T ease_in_circ(T t)
	{
		return 1.0 - std::sqrt(1.0 - t * t);
	}

	template <typename T>
	inline T ease_out_circ(T t)
	{
		return 1.0 - ease_in_circ(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_circ(T t)
	{
		return (t < 0.5 ? ease_in_circ(t * 2.0) : 1.0 + ease_out_circ((t - 0.5) * 2.0)) / 2.0;
	}

	template <typename T, typename Arg = T>
	inline T ease_in_back(T t, Arg s = 1.70158)
	{
		return t * t * ((s + 1.0) * t - s);
	}

	template <typename T, typename Arg = T>
	inline T ease_out_back(T t, T s = 1.70158)
	{
		return 1.0 - ease_in_back(1.0 - t, s);
	}

	template <typename T, typename Arg = T>
	inline T ease_in_out_back(T t, Arg s = 1.70158)
	{
		return (t < 0.5 ? ease_in_back(t * 2.0, s) : 1.0 + ease_out_back((t - 0.5) * 2.0, s)) / 2.0;
	}

	template <typename T, typename Arg = T>
	inline T ease_in_elastic(T t, Arg a = 0.5, Arg p = 0.25)
	{
		if (t == 0.0) 
			return 0;
		else if (t == 1.0) 
			return 1.0;
		T s;
		if (a < 1.0) 
		{
			a = 1.0;
			s = p / 4.0;
		}
		else 
			s = p / (2 * pi<T>()) * std::asin(1.0 / a);
		return -(a * std::pow(2.0, 10.0 * (t - 1.0)) * std::sin(((t - 1.0) - s) * (two_pi<T>()) / p));
	}

	template <typename T, typename Arg = T>
	inline T ease_out_elastic(T t, Arg a = 0.5, Arg p = 0.25)
	{
		return 1.0 - ease_in_elastic(1.0 - t, a, p);
	}

	template <typename T, typename Arg = T>
	inline T ease_in_out_elastic(T t, Arg a = 0.5, Arg p = 0.25)
	{
		return (t < 0.5 ? ease_in_elastic(t * 2.0, a, p) : 1.0 + ease_out_elastic((t - 0.5) * 2.0, a, p)) / 2.0;
	}

	template <typename T>
	inline T ease_out_bounce(T t)
	{
		if (t < (1.0 / 2.75))
		{
			return (7.5625 * t * t);
		}
		else if (t < (2.0 / 2.75))
		{
			t -= (1.5 / 2.75);
			return (7.5625 * t * t + 0.75);
		}
		else if (t < (2.5 / 2.75))
		{
			t -= (2.25 / 2.75);
			return (7.5625 * t * t + 0.9375);
		}
		else
		{
			t -= (2.625 / 2.75);
			return (7.5625 * t * t + 0.984375);
		}
	}

	template <typename T>
	inline T ease_in_bounce(T t)
	{
		return 1.0 - ease_out_bounce(1.0 - t);
	}

	template <typename T>
	inline T ease_in_out_bounce(T t)
	{
		return (t < 0.5 ? ease_in_bounce(t * 2.0) : 1.0 + ease_out_bounce((t - 0.5) * 2.0)) / 2.0;
	}

	enum class easing : uint32_t
	{
		Linear,
		InQuad,
		OutQuad,
		InOutQuad,
		InCubic,
		OutCubic,
		InOutCubic,
		InQuart,
		OutQuart,
		InOutQuart,
		InQuint,
		OutQuint,
		InOutQuint,
		InSine,
		OutSine,
		InOutSine,
		InExpo,
		OutExpo,
		InOutExpo,
		InCirc,
		OutCirc,
		InOutCirc,
		InElastic,
		OutElastic,
		InOutElastic,
		InBack,
		OutBack,
		InOutBack,
		InBounce,
		OutBounce,
		InOutBounce
	};

	inline uint32_t easing_count()
	{
		return static_cast<uint32_t>(easing::InOutBounce) + 1u;
	}

	inline easing to_easing(uint32_t index)
	{
		return static_cast<easing>(index);
	}

	template <typename T>
	inline T ease(easing e, T t)
	{
		switch (e)
		{
		case easing::Linear:
			return ease_linear(t);
		case easing::InQuad:
			return ease_in_quad(t);
		case easing::OutQuad:
			return ease_out_quad(t);
		case easing::InOutQuad:
			return ease_in_out_quad(t);
		case easing::InCubic:
			return ease_in_cubic(t);
		case easing::OutCubic:
			return ease_out_cubic(t);
		case easing::InOutCubic:
			return ease_in_out_cubic(t);
		case easing::InQuart:
			return ease_in_quart(t);
		case easing::OutQuart:
			return ease_out_quart(t);
		case easing::InOutQuart:
			return ease_in_out_quart(t);
		case easing::InQuint:
			return ease_in_quint(t);
		case easing::OutQuint:
			return ease_out_quint(t);
		case easing::InOutQuint:
			return ease_in_out_quint(t);
		case easing::InSine:
			return ease_in_sine(t);
		case easing::OutSine:
			return ease_out_sine(t);
		case easing::InOutSine:
			return ease_in_out_sine(t);
		case easing::InExpo:
			return ease_in_expo(t);
		case easing::OutExpo:
			return ease_out_expo(t);
		case easing::InOutExpo:
			return ease_in_out_expo(t);
		case easing::InCirc:
			return ease_in_circ(t);
		case easing::OutCirc:
			return ease_out_circ(t);
		case easing::InOutCirc:
			return ease_in_out_circ(t);
		case easing::InElastic:
			return ease_in_elastic(t);
		case easing::OutElastic:
			return ease_out_elastic(t);
		case easing::InOutElastic:
			return ease_in_out_elastic(t);
		case easing::InBack:
			return ease_in_back(t);
		case easing::OutBack:
			return ease_out_back(t);
		case easing::InOutBack:
			return ease_in_out_back(t);
		case easing::InBounce:
			return ease_in_bounce(t);
		case easing::OutBounce:
			return ease_out_bounce(t);
		case easing::InOutBounce:
			return ease_in_out_bounce(t);
		default:
			throw std::logic_error("neogfx::easing: unknown easing type");
		}
	}

	template <typename T>
	inline T ease(easing e, T t, T b, T c, T d)
	{
		return ease(e, t / d) * c + b;
	}
		
	inline const std::string& to_string(easing e)
	{
		static const std::unordered_map<easing, std::string> STRING_MAP =
		{ {
			{ easing::Linear,		"Linear"s },
			{ easing::InQuad,		"InQuad"s },
			{ easing::OutQuad,		"OutQuad"s },
			{ easing::InOutQuad,	"InOutQuad"s },
			{ easing::InCubic,		"InCubic"s },
			{ easing::OutCubic,	"OutCubic"s },
			{ easing::InOutCubic,	"InOutCubic"s },
			{ easing::InQuart,		"InQuart"s },
			{ easing::OutQuart,	"OutQuart"s },
			{ easing::InOutQuart,	"InOutQuart"s },
			{ easing::InQuint,		"InQuint"s },
			{ easing::OutQuint,	"OutQuint"s },
			{ easing::InOutQuint,	"InOutQuint"s },
			{ easing::InSine,		"InSine"s },
			{ easing::OutSine,		"OutSine"s },
			{ easing::InOutSine,	"InOutSine"s },
			{ easing::InExpo,		"InExpo"s },
			{ easing::OutExpo,		"OutExpo"s },
			{ easing::InOutExpo,	"InOutExpo"s },
			{ easing::InCirc,		"InCirc"s },
			{ easing::OutCirc,		"OutCirc"s },
			{ easing::InOutCirc,	"InOutCirc"s },
			{ easing::InElastic,	"InElastic"s },
			{ easing::OutElastic,	"OutElastic"s },
			{ easing::InOutElastic,"InOutElastic"s },
			{ easing::InBack,		"InBack"s },
			{ easing::OutBack,		"OutBack"s },
			{ easing::InOutBack,	"InOutBack"s },
			{ easing::InBounce,	"InBounce"s },
			{ easing::OutBounce,	"OutBounce"s },
			{ easing::InOutBounce,	"InOutBounce"s }
		} };
		auto s = STRING_MAP.find(e);
		if (s != STRING_MAP.end())
			return s->second;
		throw std::logic_error("neogfx::to_string: unknown easing type");
	}
}
