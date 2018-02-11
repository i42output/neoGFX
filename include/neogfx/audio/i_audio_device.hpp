// i_audio_device.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/audio/audio_spec.hpp>

namespace neogfx
{
	class i_audio_device
	{
	public:
		struct already_open : std::logic_error { already_open() : std::logic_error("neogfx::i_audio_device::already_open") {} };
		struct not_open : std::logic_error { not_open() : std::logic_error("neogfx::i_audio_device::not_open") {} };
	public:
		virtual const std::string& name() const = 0;
	public:
		virtual bool is_open() const = 0;
		virtual void open(const audio_spec& aAudioSpec = audio_spec{}, audio_spec_requirements aRequirements = audio_spec_requirements::RequireNone) = 0;
		virtual void close() = 0;
	public:
		virtual const audio_spec& spec() const = 0;
	};
}