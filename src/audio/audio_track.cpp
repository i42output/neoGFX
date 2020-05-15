// audio_track.cpp
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
#include <neogfx/audio/audio_track.hpp>

namespace neogfx
{
    audio_track::audio_track(const audio_spec& aSpec) : 
        iSpec{ aSpec }
    {
    }

    const audio_spec& audio_track::spec() const
    {
        return iSpec;
    }

    void audio_track::add_sample(i_audio_sample& aSample)
    {
        iItems.push_back(item_sample{ std::shared_ptr<i_audio_sample>{std::shared_ptr<i_audio_sample>{}, &aSample} });
    }

    void audio_track::add_sample(std::shared_ptr<i_audio_sample> aSample)
    {
        iItems.push_back(item_sample{ aSample });
    }

    void audio_track::add_silence(double aDuration)
    {
        iItems.push_back(item_silence{ aDuration });
    }

    void audio_track::repeat_start(uint32_t aRepeatCount)
    {
        iItems.push_back(item_repeat_start{ aRepeatCount });
    }

    void audio_track::repeat_end()
    {
        iItems.push_back(item_repeat_end{});
    }

    audio_track::item_index audio_track::item_count() const
    {
        return static_cast<item_index>(iItems.size());
    }

    const audio_track::value_type& audio_track::item(item_index aItemIndex) const
    {
        if (aItemIndex < iItems.size())
            return iItems[aItemIndex];
        throw bad_item_index();
    }

    audio_track::value_type& audio_track::item(item_index aItemIndex)
    {
        if (aItemIndex < iItems.size())
            return iItems[aItemIndex];
        throw bad_item_index();
    }
}