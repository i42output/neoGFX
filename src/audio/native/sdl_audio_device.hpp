// sdl_audio_device.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <SDL.h>
#include <neogfx/audio/audio_device.hpp>

namespace neogfx
{
	class sdl_audio_device : public audio_device
	{
	public:
		sdl_audio_device(const std::string& aName);
		~sdl_audio_device();
	public:
		bool is_open() const override;
		void open(const audio_spec& aAudioSpec = audio_spec{}, audio_spec_requirements aRequirements = audio_spec_requirements::RequireNone) override;
		void close() override;
	private:
		static void callback(void *userdata, Uint8* stream, int len);
	private:
		SDL_AudioDeviceID iId;
	};
}