// i_audio_oscillator.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/audio/audio_primitives.hpp>
#include <neogfx/audio/i_audio_bitstream.hpp>

namespace neogfx
{
    enum class oscillator_function : std::uint32_t
    {
        Custom      = 0,
        Sine        = 1,
        Square      = 2,
        Triangle    = 3,
        Sawtooth    = 4
    };

    class i_audio_oscillator : public i_audio_bitstream
    {
    public:
        typedef i_audio_oscillator abstract_type;
    public:
        virtual float frequency() const = 0;
        virtual void set_frequency(float aFrequency) = 0;
        virtual oscillator_function function() const = 0;
        virtual void set_function(oscillator_function aFunction) = 0;
        virtual void set_function(std::function<float(float)> const& aFunction) = 0; ///< Note: not plugin-safe.
    };
}