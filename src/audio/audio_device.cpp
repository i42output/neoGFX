// audio_device.cpp
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
#include <neogfx/audio/audio_device.hpp>

namespace neogfx
{
    audio_device::audio_device(std::string const& aName) : 
        iName{ aName }
    {
    }

    audio_device::~audio_device()
    {
    }

    std::string const& audio_device::name() const
    {
        return iName;
    }

    const audio_spec& audio_device::spec() const
    {
        if (!is_open())
            throw not_open();
        return *iSpec;
    }

    void audio_device::set_spec(const optional_audio_spec& aSpec)
    {
        iSpec = aSpec;
    }
}