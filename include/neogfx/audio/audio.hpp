// audio.hpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/audio/i_audio.hpp>
#include <neogfx/audio/audio_device.hpp>
#include <neogfx/audio/audio_instrument_atlas.hpp>

#pragma once

namespace neogfx
{
	class audio : public i_audio
	{
	public:
		audio();
		~audio() override;
	public:
		i_vector<i_audio_device_info> const& available_devices() override;
		i_audio_device& create_device(i_audio_device_info const& aDeviceInfo, audio_data_format const& aDataFormat) override;
		i_audio_device& create_playback_device(audio_data_format const& aDataFormat) override;
		i_audio_device& create_capture_device(audio_data_format const& aDataFormat) override;
		void destroy_device(i_audio_device& aDevice) override;
	public:
		i_audio_instrument_atlas& instrument_atlas() override;
	private:
		i_audio_device_info const* find_device(audio_device_type aType, audio_data_format const& aDataFormat);
	private:
		audio_context iContext;
		vector<audio_device_info> iDeviceInfos;
		vector<ref_ptr<i_audio_device>> iDevices;
		audio_instrument_atlas iInstrumentAtlas;
	};
}