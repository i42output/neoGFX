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
#include <neolib/core/string_utils.hpp>
#include <neolib/file/file.hpp>
#include <neolib/file/zip.hpp>
#include <neogfx/audio/audio_instrument_atlas.hpp>

#pragma once

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
		// todo - create bitstream for every note
		return false;
	}

	i_audio_bitstream& audio_instrument_atlas::instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate, note aNote)
	{
		if (iSamples.find(aInstrument) == iSamples.end())
			throw audio_instrument_not_found(aInstrument);
		throw std::logic_error("neogfx::audio_instrument_atlas::instrument: todo");
	}
}