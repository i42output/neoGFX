// audio_playback_device.hpp
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
#include <neogfx/audio/i_audio_playback_device.hpp>

namespace neogfx
{
    class audio_playback_device : public i_audio_playback_device
    {
    public:
        audio_playback_device();
    public:
        i_audio_sample& load_sample(const std::string& aUri) override;
        i_audio_sample& create_sample(double aDuration) override;
        void destroy_sample(i_audio_sample& aSample) override;
    public:
        i_audio_track& create_track() override;
        void destroy_track(i_audio_track& aTrack) override;
    public:
        i_audio_beeper& beeper() override;
    private:
        std::vector<std::shared_ptr<i_audio_sample>> iSamples;
        std::vector<std::shared_ptr<i_audio_track>> iTracks;
        std::unique_ptr<i_audio_beeper> iBeeper;
    };
}