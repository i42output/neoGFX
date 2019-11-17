// audio_device.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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

namespace neogfx
{
	class audio_device : public i_audio_device
	{
	public:
		audio_device(const std::string& aName);
		~audio_device();
	public:
		const std::string& name() const override;
	public:
		const audio_spec& spec() const override;
	protected:
		void set_spec(const optional_audio_spec& aSpec);
	private:
		std::string iName;
		optional_audio_spec iSpec;
	};
}