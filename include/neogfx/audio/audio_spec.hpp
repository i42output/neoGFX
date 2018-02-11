// audio_spec.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
#include <boost/optional.hpp>

namespace neogfx
{
	enum class audio_format
	{
		S8,
		U8,
		S16LSB,
		S16 = S16LSB,
		S16MSB,
		S16SYS,
		U16LSB,
		U16 = U16LSB,
		U16MSB,
		U16SYS,
		S32LSB,
		S32 = S32LSB,
		S32MSB,
		S32SYS,
		F32LSB,
		F32 = F32LSB,
		F32MSB,
		F32SYS
	};

	struct unknown_audio_format : std::logic_error { unknown_audio_format() : std::logic_error("neogfx::unknown_audio_format") {} };

	class audio_spec
	{
	public:
		audio_spec() :
			iFrequency{ 48000 }, iFormat{ audio_format::F32 }, iChannels{ 2 }, iSamples{ 4096 }, iSize{ 0 }, iSilence{ 0 }
		{
		}
		audio_spec(int32_t aFrequency, audio_format aFormat, uint8_t aChannels, uint16_t aSamples = 4096) :
			iFrequency{ aFrequency }, iFormat{ aFormat }, iChannels{ aChannels }, iSamples{ aSamples }, iSize{ 0 }, iSilence{ 0 }
		{
		}
		audio_spec(int32_t aFrequency, audio_format aFormat, uint8_t aChannels, uint16_t aSamples, uint32_t aSize, uint8_t aSilence) :
			iFrequency{ aFrequency }, iFormat{ aFormat }, iChannels{ aChannels }, iSamples{ aSamples }, iSize{ aSize }, iSilence{ aSilence }
		{
		}
	public:
		int32_t frequency() const { return iFrequency; }
		audio_format format() const { return iFormat; }
		uint8_t channels() const { return iChannels; }
		uint16_t samples() const { return iSamples; }
		uint32_t size() const { return iSize; }
		uint8_t silence() const { return iSilence; }
	private:
		int32_t iFrequency;
		audio_format iFormat;
		uint8_t iChannels;
		uint16_t iSamples;
		uint32_t iSize;
		uint8_t iSilence;
	};

	typedef boost::optional<audio_spec> optional_audio_spec;

	enum class audio_spec_requirements
	{
		RequireNone = 0x00,
		RequireFrequency = 0x01,
		RequireFormat = 0x02,
		RequireChannels = 0x04,
		RequireAll = RequireFrequency | RequireFormat | RequireChannels
	};

	inline constexpr audio_spec_requirements operator|(audio_spec_requirements aLhs, audio_spec_requirements aRhs)
	{
		return static_cast<audio_spec_requirements>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr audio_spec_requirements operator&(audio_spec_requirements aLhs, audio_spec_requirements aRhs)
	{
		return static_cast<audio_spec_requirements>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}
}