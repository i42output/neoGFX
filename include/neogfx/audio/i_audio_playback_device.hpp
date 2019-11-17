// i_audio_playback_device.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/audio/i_audio_device.hpp>
#include <neogfx/audio/i_audio_sample.hpp>
#include <neogfx/audio/i_audio_track.hpp>
#include <neogfx/audio/i_audio_beeper.hpp>

namespace neogfx
{
    class i_audio_playback_device : public i_audio_device
    {
    public:
        virtual i_audio_sample& load_sample(const std::string& aUri) = 0;
        virtual i_audio_sample& create_sample(double aDuration) = 0;
        virtual void destroy_sample(i_audio_sample& aSample) = 0;
    public:
        virtual i_audio_track& create_track() = 0;
        virtual void destroy_track(i_audio_track& aTrack) = 0;
    public:
        virtual i_audio_beeper& beeper() = 0;
    };
}