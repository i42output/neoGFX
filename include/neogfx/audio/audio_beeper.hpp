// audio_beeper.hpp
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
#include <neogfx/audio/i_audio_beeper.hpp>
#include <neogfx/audio/audio_beeper_sample.hpp>

namespace neogfx
{
    class audio_beeper : public i_audio_beeper
    {
    public:
        audio_beeper(i_audio_playback_device& aDevice);
    public:
        void beep(double aDuration, double aFrequency) override;
        void beep(const audio_envelope& aEnvelope, double aFrequency) override;
        void silence(double aDuration) override;
        void repeat_start(uint32_t aRepeatCount) override;
        void repeat_end() override;
        void clear() override;
    private:
        i_audio_playback_device& iDevice;
        i_audio_track& iTrack;
        audio_beeper_sample iSample;
    };
}