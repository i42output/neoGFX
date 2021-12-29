// audio_device.hpp
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
#include <neogfx/audio/i_audio_device.hpp>
#include <neogfx/audio/i_audio_waveform.hpp>

#pragma once

namespace neogfx
{
	class audio_device_info : public i_audio_device_info
	{
	public:
		audio_device_info(audio_device_id aId, audio_device_type aType, i_string const& aName, bool aIsDefault, vector<audio_data_format> const& aDataFormats);
		audio_device_info(i_audio_device_info const& aOther);
		~audio_device_info() override;
	public:
		audio_device_id id() const override;
		audio_device_type type() const override;
		i_string const& name() const override;
		bool is_default() const override;
	public:
		i_vector<audio_data_format> const& data_formats() const override;
	private:
		audio_device_id iId;
		audio_device_type iType;
		string iName;
		bool iIsDefault;
		vector<audio_data_format> iDataFormats;
	};

	class audio_device : public reference_counted<i_audio_device>
	{
	public:
		audio_device(audio_context aContext, i_audio_device_info const& aDeviceInfo, audio_data_format const& aDataFormat);
		~audio_device();
	public:
		i_audio_device_info const& info() const final;
		audio_data_format const& data_format() const final;
	public:
		void start() final;
		void stop() final;
	public:
		void play(i_audio_waveform& aWaveform, std::chrono::duration<double> const& aDuration) final;
	private:
		mutable std::recursive_mutex iMutex;
		audio_device_info iInfo;
		audio_data_format iDataFormat;
		audio_device_config iConfig;
		audio_device_handle iHandle;
		struct source
		{
			ref_ptr<i_audio_waveform> bitstream; // todo: should be i_audio_bitstream
			std::optional<std::chrono::steady_clock::time_point> expiryTime;
		};
		std::vector<source> iSources;
	};
}