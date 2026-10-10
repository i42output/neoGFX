/*
neogfx C++ App/Game Engine - Examples - MOD Tracker
Copyright(C) 2026 Leigh Johnston

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

#include <mod_tracker/mod_tracker.hpp>

#include <algorithm>

#include <mod_tracker/module_bitstream.hpp>

namespace mod_tracker
{
    namespace
    {
        // the song is mixed into a buffer of this many frames at a time; it is allocated up front so that the
        // audio thread never allocates
        constexpr ng::audio_frame_count MIX_FRAMES = 1024u;
    }

    module_bitstream::module_bitstream(std::shared_ptr<module_data const> aModule, std::shared_ptr<timeline const> aTimeline, render_options const& aOptions) :
        audio_bitstream{ aOptions.sampleRate },
        iModule{ std::move(aModule) },
        iTimeline{ std::move(aTimeline) },
        iStream{ *iModule, *iTimeline, aOptions },
        iMix(MIX_FRAMES * 2u)
    {
    }

    ng::audio_frame_count module_bitstream::length() const
    {
        return 0u;
    }

    void module_bitstream::generate(ng::audio_channel aChannel, ng::audio_frame_count aFrameCount, float aGain, float* aOutputFrames)
    {
        generate_from(aChannel, iCursor, aFrameCount, aGain, aOutputFrames);
        iCursor += aFrameCount;
    }

    void module_bitstream::generate_from(ng::audio_channel aChannel, ng::audio_frame_index aFrameFrom, ng::audio_frame_count aFrameCount, float aGain, float* aOutputFrames)
    {
        auto const outputChannels = ng::channel_count(aChannel);
        if (outputChannels == 0u)
            return;
        auto const gain = amplitude() * aGain;
        // the mix is stereo; it goes to the front left and right channels of the output if it has them, and
        // is folded down into every channel if it hasn't
        bool const stereo =
            (aChannel & ng::audio_channel::FrontLeft) != ng::audio_channel::None &&
            (aChannel & ng::audio_channel::FrontRight) != ng::audio_channel::None;
        auto const left = stereo ? ng::channel_index(aChannel, ng::audio_channel::FrontLeft) : 0u;
        auto const right = stereo ? ng::channel_index(aChannel, ng::audio_channel::FrontRight) : 0u;
        for (ng::audio_frame_count done = 0u; done < aFrameCount;)
        {
            auto const frames = std::min(aFrameCount - done, MIX_FRAMES);
            std::fill(iMix.begin(), std::next(iMix.begin(), static_cast<std::ptrdiff_t>(frames * 2u)), 0.0f);
            iStream.render(aFrameFrom + done, iMix.data(), frames);
            auto output = aOutputFrames + done * outputChannels;
            for (ng::audio_frame_count frame = 0u; frame < frames; ++frame, output += outputChannels)
            {
                auto const l = iMix[frame * 2u] * gain;
                auto const r = iMix[frame * 2u + 1u] * gain;
                if (stereo)
                {
                    output[left] += l;
                    output[right] += r;
                }
                else
                    for (std::uint64_t channel = 0u; channel < outputChannels; ++channel)
                        output[channel] += (l + r) * 0.5f;
            }
            done += frames;
        }
    }

    stream& module_bitstream::song_stream()
    {
        return iStream;
    }
}
