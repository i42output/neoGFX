// i_audio.hpp
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
#include <neogfx/audio/i_audio_device.hpp>

#pragma once

namespace neogfx
{
	typedef std::any audio_context;

	class i_audio : public i_service
	{
	public:
		virtual ~i_audio() = default;
	public:
		virtual i_vector<i_audio_device_info> const& available_devices() = 0;
		virtual i_audio_device& create_device(i_audio_device_info const& aDeviceInfo, audio_data_format const& aDataFormat) = 0;
		virtual i_audio_device& create_playback_device(audio_data_format const& aDataFormat) = 0;
		virtual i_audio_device& create_capture_device(audio_data_format const& aDataFormat) = 0;
		virtual void destroy_device(i_audio_device& aDevice) = 0;
	public:
		static uuid const& iid() { static uuid const sIid{ 0xabec285f, 0x3bc2, 0x4d97, 0xbf24, { 0x12, 0x8a, 0xe9, 0x39, 0x2f, 0x71 } }; return sIid; }
	};
}