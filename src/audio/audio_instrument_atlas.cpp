// audio_instrument_atlas.cpp
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
#include <sstream>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#ifdef _WIN32
#define MA_ENABLE_WASAPI
#endif
#include "3rdparty/miniaudio/miniaudio.h"

#include <neolib/core/string_utils.hpp>
#include <neolib/file/file.hpp>
#include <neolib/file/zip.hpp>
#include <neogfx/audio/audio_instrument_atlas.hpp>
#include <neogfx/audio/audio_bitstream.hpp>
#include <neogfx/audio/audio_oscillator.hpp>

struct smbContext;
smbContext* smbCreateContext();
void smbDestroyContext(smbContext* context);
void smbPitchShift(smbContext* context, float pitchShift, long numSampsToProcess, long fftFrameSize, long osamp, float sampleRate, float* indata, float* outdata);

namespace neogfx
{
	audio_instrument_atlas::audio_instrument_atlas()
	{
		auto atlasFile = neolib::program_directory() + "/music.zip";

		if (std::filesystem::exists(atlasFile))
		{
			neolib::zip zipFile(atlasFile);
			std::istringstream metaDataFile{ zipFile.extract_to_string(zipFile.index_of("meta.json")) };
			boost::property_tree::ptree metaData;
			boost::property_tree::read_json(metaDataFile, metaData);
			for (auto const& program : metaData.get_child("midi.programs"))
			{
				auto instrumentId = static_cast<neogfx::instrument>(program.second.get<std::uint32_t>("program"));
				auto sampleIndexFile = program.second.get<std::string>("sfz");
				std::istringstream sampleIndex{ zipFile.extract_to_string(zipFile.index_of(sampleIndexFile)) };

				std::optional<note> lokey;
				std::optional<note> pitch_keycenter;
				std::optional<note> hikey;
				std::optional<std::string> sample;

				auto add_sample_info = [&]()
				{
					if (!lokey || !hikey || !pitch_keycenter || !sample)
						return;
					sample_info sampleInfo{ sample.value(), lokey.value(), pitch_keycenter.value(), hikey.value() };
					for (auto key = sampleInfo.midiKeyLow; key <= sampleInfo.midiKeyHigh; key = static_cast<note>(static_cast<std::uint32_t>(key) + 1))
					{
						if (iSamples[instrumentId].find(key) == iSamples[instrumentId].end()) // todo: groups
							iSamples[instrumentId][key] = sampleInfo;
					}
					lokey = std::nullopt;
					hikey = std::nullopt;
					pitch_keycenter = std::nullopt;
					sample = std::nullopt;
				};

				std::string line;
				while (std::getline(sampleIndex, line))
				{
					std::vector<std::string> tokens = neolib::tokens(line, " ="s, 4);
					while (tokens.size() >= 2)
					{
						try
						{
							if (tokens[0] == "lokey")
								lokey = string_to_note(tokens.at(1));
							else if (tokens[0] == "hikey")
								hikey = string_to_note(tokens.at(1));
							else if (tokens[0] == "pitch_keycenter")
								pitch_keycenter = string_to_note(tokens.at(1));
							else if (tokens[0] == "sample")
								sample = tokens.at(1);
							tokens.erase(tokens.begin(), std::next(tokens.begin(), 2));
						}
						catch (...)
						{
							// todo, lokey, hikey et al can be MIDI note notation rather than integer
						}
					}
					add_sample_info();
				}
			}
		}
	}

	bool audio_instrument_atlas::load_instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate)
	{
		if (iSamples.find(aInstrument) == iSamples.end())
			return false;
		for (auto key = note::MIDI0; key <= note::Ab9; key = static_cast<note>(static_cast<std::uint32_t>(key) + 1))
		{
			if (iSamples[aInstrument].find(key) != iSamples[aInstrument].end())
				(void)instrument(aInstrument, aSampleRate, key);
		}
		return true;
	}

	class pure_tone : public audio_bitstream<i_audio_bitstream>
	{
	public:
		pure_tone(audio_sample_rate aSampleRate, float aFrequency) :
			audio_bitstream<i_audio_bitstream>{ aSampleRate },
			iOscillator{ aSampleRate, aFrequency }
		{
		}
	public:
		audio_frame_count length() const override
		{
			return sample_rate();
		}
		void generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames) override
		{
			iOscillator.generate(aFrameCount, aOutputFrames);
		}
		void generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames) override
		{
			iOscillator.generate_from(aFrameFrom, aFrameCount, aOutputFrames);
		}
	private:
		audio_oscillator iOscillator;
	};

	class sample : public audio_bitstream<i_audio_bitstream>
	{
	public:
		sample(audio_sample_rate aSampleRate, std::vector<float>&& aPcmFrames) :
			audio_bitstream<i_audio_bitstream>{ aSampleRate },
			iPcmFrames{ aPcmFrames }
		{
		}
	public:
		audio_frame_count length() const override
		{
			return iPcmFrames.size();
		}
		void generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames) override
		{
			generate_from(aChannel, iCursor, aFrameCount, aOutputFrames);
		}
		void generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames) override
		{
			std::copy(std::next(iPcmFrames.begin(), aFrameFrom), std::next(iPcmFrames.begin(), aFrameFrom + aFrameCount), aOutputFrames);
			iCursor = aFrameFrom + aFrameCount;
		}
	private:
		std::vector<float> iPcmFrames;
		audio_frame_index iCursor = 0ULL;
	};

	i_audio_bitstream& audio_instrument_atlas::instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate, note aNote)
	{
		note_key const key{ aInstrument, aSampleRate, aNote };

		auto existing = iNotes.find(key);
		if (existing != iNotes.end())
			return *existing->second;

		if (aInstrument == neogfx::instrument::PureTone)
		{
			iNotes[key] = make_ref<pure_tone>(aSampleRate, frequency(aNote));
			return *iNotes[key];
		}

		auto existingInstrument = iSamples.find(aInstrument);
		if (existingInstrument == iSamples.end())
			throw audio_instrument_not_found(aInstrument);
		auto existingNote = existingInstrument->second.find(aNote);
		if (existingNote == iSamples[aInstrument].end())
			throw audio_instrument_note_not_found(aInstrument, aNote);
		auto atlasFile = neolib::program_directory() + "/music.zip";
		if (!std::filesystem::exists(atlasFile))
			throw audio_instrument_atlas_file_found();
		neolib::zip zipFile(atlasFile);
		thread_local std::vector<std::uint8_t> buffer;
		buffer.clear();
		zipFile.extract_to(zipFile.index_of(existingNote->second.sampleFile), buffer);

		ma_decoder decoder;
		ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 1, static_cast<ma_uint32>(aSampleRate));
		ma_result result = ma_decoder_init_memory(buffer.data(), buffer.size(), &config, &decoder);
		if (result != MA_SUCCESS)
			throw audio_instrument_sample_decode_failure();

		std::vector<float> entireSample;

		thread_local std::array<float, 16384> partSample;
		std::uint64_t framesRead;
		for (;;)
		{
			ma_decoder_read_pcm_frames(&decoder, &partSample[0], partSample.size(), &framesRead);
			std::copy(partSample.begin(), std::next(partSample.begin(), framesRead), std::back_inserter(entireSample));
			if (framesRead < partSample.size())
				break;
		}

		if (aNote != existingNote->second.midiKeyPitchCentre)
		{
			auto context = smbCreateContext();
			auto const frequencyShift = frequency(aNote) / frequency(existingNote->second.midiKeyPitchCentre);
			smbPitchShift(context, frequencyShift, static_cast<long>(entireSample.size()), 4096, 32, static_cast<float>(aSampleRate), entireSample.data(), entireSample.data());
			smbDestroyContext(context);
		}

		iNotes[key] = make_ref<sample>(aSampleRate, std::move(entireSample));

		return *iNotes[key];
	}
}