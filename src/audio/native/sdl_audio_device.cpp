// sdl_audio.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>
#include "sdl_audio_device.hpp"

namespace neogfx
{
	namespace
	{
		inline int convert_requirements(audio_spec_requirements aRequirements)
		{
			int result = SDL_AUDIO_ALLOW_ANY_CHANGE;
			if ((aRequirements & audio_spec_requirements::RequireFrequency) != audio_spec_requirements::RequireFrequency)
				result &= ~SDL_AUDIO_ALLOW_FREQUENCY_CHANGE;
			if ((aRequirements & audio_spec_requirements::RequireFormat) != audio_spec_requirements::RequireFormat)
				result &= ~SDL_AUDIO_ALLOW_FORMAT_CHANGE;
			if ((aRequirements & audio_spec_requirements::RequireChannels) != audio_spec_requirements::RequireChannels)
				result &= ~SDL_AUDIO_ALLOW_CHANNELS_CHANGE;
			return result;
		}

		inline SDL_AudioFormat convert_format(audio_format aFormat)
		{
			switch (aFormat)
			{
			case audio_format::S8:
				return AUDIO_S8;
			case audio_format::U8:
				return AUDIO_U8;
			case audio_format::S16LSB:
				return AUDIO_S16LSB;
			case audio_format::S16MSB:
				return AUDIO_S16MSB;
			case audio_format::S16SYS:
				return AUDIO_S16SYS;
			case audio_format::U16LSB:
				return AUDIO_U16LSB;
			case audio_format::U16MSB:
				return AUDIO_U16MSB;
			case audio_format::U16SYS:
				return AUDIO_U16SYS;
			case audio_format::S32LSB:
				return AUDIO_S32LSB;
			case audio_format::S32MSB:
				return AUDIO_S32MSB;
			case audio_format::S32SYS:
				return AUDIO_S32SYS;
			case audio_format::F32LSB:
				return AUDIO_F32LSB;
			case audio_format::F32MSB:
				return AUDIO_F32MSB;
			case audio_format::F32SYS:
				return AUDIO_F32SYS;
			default:
				throw unknown_audio_format();
			}
		}

		inline audio_format convert_format(SDL_AudioFormat aFormat)
		{
			switch (aFormat)
			{
			case AUDIO_S8:
				return audio_format::S8;
			case AUDIO_U8:
				return audio_format::U8;
			case AUDIO_S16LSB:
				return audio_format::S16LSB;
			case AUDIO_S16MSB:
				return audio_format::S16MSB;
			case AUDIO_U16LSB:
				return audio_format::U16LSB;
			case AUDIO_U16MSB:
				return audio_format::U16MSB;
			case AUDIO_S32LSB:
				return audio_format::S32LSB;
			case AUDIO_S32MSB:
				return audio_format::S32MSB;
			case AUDIO_F32LSB:
				return audio_format::F32LSB;
			case AUDIO_F32MSB:
				return audio_format::F32MSB;
			default:
				throw unknown_audio_format();
			}
		}
	}

	sdl_audio_device::sdl_audio_device(const std::string& aName) : 
		audio_device{ aName }, iId{ 0 }
	{
	}

	sdl_audio_device::~sdl_audio_device()
	{
		if (is_open())
			close();
	}

	bool sdl_audio_device::is_open() const
	{
		return iId != 0;
	}

	void sdl_audio_device::open(const audio_spec& aAudioSpec, audio_spec_requirements aRequirements)
	{
		if (is_open())
			throw already_open();
		SDL_AudioSpec desired =
		{
			aAudioSpec.frequency(),
			convert_format(aAudioSpec.format()),
			aAudioSpec.channels(),
			0,
			aAudioSpec.samples(),
			0,
			0,
			&callback,
			this
		};
		SDL_AudioSpec obtained = {};
		iId = SDL_OpenAudioDevice(name().c_str(), 0, &desired, &obtained, convert_requirements(aRequirements));
		set_spec(audio_spec{ obtained.freq, convert_format(obtained.format), obtained.channels, obtained.samples, obtained.size, obtained.silence });
	}

	void sdl_audio_device::close()
	{
		if (!is_open())
			throw not_open();
		SDL_CloseAudioDevice(iId);
		iId = 0;
		set_spec(boost::none);
	}

	void sdl_audio_device::callback(void *userdata, Uint8* stream, int len)
	{
	}
}