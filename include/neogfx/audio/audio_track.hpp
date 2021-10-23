// audio_track.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/audio/audio_spec.hpp>
#include <neogfx/audio/i_audio_track.hpp>

namespace neogfx
{
    class audio_track : public i_audio_track
    {
    public:
        audio_track(const audio_spec& aSpec);
    public:
        const audio_spec& spec() const override;
    public:
        void add_sample(i_audio_sample& aSample) override;
        void add_sample(std::shared_ptr<i_audio_sample> aSample) override;
        void add_silence(double aDuration) override;
        void repeat_start(uint32_t aRepeatCount) override;
        void repeat_end() override;
    public:
        item_index item_count() const override;
        value_type const& item(item_index aItemIndex) const override;
        value_type& item(item_index aItemIndex) override;
    private:
        audio_spec iSpec;
        std::vector<value_type> iItems;
    };
}