// audio_playback_device.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/audio/audio_playback_device.hpp>
#include <neogfx/audio/audio_track.hpp>
#include <neogfx/audio/audio_beeper.hpp>

namespace neogfx
{
    audio_playback_device::audio_playback_device()
    {
    }

    i_audio_sample& audio_playback_device::load_sample(std::string const& aUri)
    {
        // todo
        throw std::logic_error("audio_playback_device::load_sample: not yet implemented");
    }

    i_audio_sample& audio_playback_device::create_sample(double aDuration)
    {
        // todo
        throw std::logic_error("audio_playback_device::create_sample: not yet implemented");
    }

    void audio_playback_device::destroy_sample(i_audio_sample& aSample)
    {
        for (auto i = iSamples.begin(); i != iSamples.end(); ++i)
            if (&**i == &aSample)
            {
                iSamples.erase(i);
                return;
            }
    }

    i_audio_track& audio_playback_device::create_track()
    {
        auto newTrack = std::make_shared<audio_track>(spec());
        iTracks.push_back(newTrack);
        return *newTrack;
    }

    void audio_playback_device::destroy_track(i_audio_track& aTrack)
    {
        for (auto i = iTracks.begin(); i != iTracks.end(); ++i)
            if (&**i == &aTrack)
            {
                iTracks.erase(i);
                return;
            }
    }

    i_audio_beeper& audio_playback_device::beeper()
    {
        if (iBeeper == nullptr)
            iBeeper = std::make_unique<audio_beeper>(*this);
        return *iBeeper;
    }
}