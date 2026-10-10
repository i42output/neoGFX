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
#include <iterator>
#include <limits>

#include <neolib/ecs/chrono.hpp>

#include <neogfx/audio/audio_playback.hpp>

#include <mod_tracker/player.hpp>

namespace mod_tracker
{
    namespace
    {
        // sequencer positions are in tocks, and one tock is one flick
        constexpr std::int64_t TOCKS_PER_SECOND = neolib::ecs::chrono::flicks::period::den;
        constexpr ng::audio_sample_rate DEVICE_SAMPLE_RATE = 44100u;
        // the audio clip has to have a duration; this one is open-ended for all practical purposes (centuries)
        constexpr ng::game::sequencer_duration AUDIO_CLIP_DURATION = std::numeric_limits<ng::game::sequencer_duration>::max() / 2;
        // pattern clips for the next pass of a looping song are laid out this far ahead of the playhead
        constexpr double PATTERN_CLIP_LOOKAHEAD_S = 2.0;
    }

    player::player() :
        iDevice{ ng::service<ng::i_audio>().create_playback_device(ng::audio_data_format{ ng::audio_sample_format::F32, 2u, DEVICE_SAMPLE_RATE }) },
        iSequencer{ ng::service<ng::game::i_sequencer>() },
        iSequence{ iSequencer.create_sequence() }
    {
        iDevice.start();
    }

    player::~player()
    {
        unload();
        iSequencer.delete_sequence(iSequence);
        iDevice.stop();
    }

    ng::audio_sample_rate player::sample_rate() const
    {
        return iDevice.data_format().sampleRate;
    }

    bool player::loaded() const
    {
        return iModule != nullptr;
    }

    void player::load(std::shared_ptr<module_data const> aModule, std::shared_ptr<timeline const> aTimeline, render_options const& aOptions)
    {
        unload();

        iModule = std::move(aModule);
        iTimeline = std::move(aTimeline);
        iAudio = ng::make_ref<module_bitstream>(iModule, iTimeline, aOptions);
        iAudio->song_stream().set_looping(iLooping);

        iAudioTrack = iSequencer.create_track(iSequence);
        iPatternTrack = iSequencer.create_track(iSequence);

        // the module is one open-ended audio clip: the playback payload starts the device pulling from it when
        // the playhead enters the clip, and restarts it from the right frame whenever the playhead jumps
        // nothing about ordinary playback needs restarting: the module carries on through any stall of the
        // GUI by itself, so the playback only counts a jump backwards as a seek (seeks, pauses and stops stop
        // the playback themselves, so the restart that follows them is certain)
        iAudioClip = ng::game::make_sequencer_clip<ng::audio_playback>(iDevice, *iAudio, 1.0f, AUDIO_CLIP_DURATION);
        iSequencer.add_clip(iAudioClip, *iAudioTrack, 0, AUDIO_CLIP_DURATION);

        iLoopPasses.clear();
        iClipsEnd = 0u;
        if (!iTimeline->rows.empty())
            add_pattern_clips(0u, 0u);

        iSequencer.stop(iSequence);
        iLastPosition = 0u;
        iPaused = false;
    }

    void player::unload()
    {
        if (!loaded())
            return;
        stop_audio();
        iSequencer.stop(iSequence);
        // deleting the tracks deletes their clips; the audio clip is held here too so that its playback is
        // known to be stopped before the module it plays is released
        iSequencer.remove_all(iSequence);
        iAudioTrack = std::nullopt;
        iPatternTrack = std::nullopt;
        iAudioClip = nullptr;
        iAudio = nullptr;
        iLoopPasses.clear();
        iModule = nullptr;
        iTimeline = nullptr;
        iPaused = false;
    }

    module_data const& player::song_module() const
    {
        return *iModule;
    }

    timeline const& player::song() const
    {
        return *iTimeline;
    }

    player::state player::current_state() const
    {
        if (!loaded())
            return state::Empty;
        if (iSequencer.is_playing(iSequence))
            return state::Playing;
        return iPaused ? state::Paused : state::Stopped;
    }

    bool player::looping() const
    {
        return iLooping;
    }

    void player::set_looping(bool aLooping)
    {
        iLooping = aLooping;
        if (iAudio != nullptr)
            iAudio->song_stream().set_looping(aLooping);
    }

    void player::play()
    {
        if (!loaded())
            return;
        iSequencer.play(iSequence);
        iPaused = false;
    }

    void player::pause()
    {
        if (current_state() != state::Playing)
            return;
        iSequencer.pause(iSequence);
        // the sequencer stops advancing the clip but the device would carry on pulling from the module; when
        // playback resumes the module carries on from where the audio actually got to
        stop_audio();
        iPaused = true;
    }

    void player::stop()
    {
        if (!loaded())
            return;
        stop_audio();
        iAudio->song_stream().force_seek(0u);
        iSequencer.stop(iSequence);
        remove_loop_clips(std::numeric_limits<std::uint64_t>::max());
        iLastPosition = 0u;
        iPaused = false;
    }

    void player::seek(std::size_t aRow)
    {
        if (!loaded() || aRow >= iTimeline->rows.size())
            return;
        // the device stops pulling from the module before the module is told where to seek to, so that the
        // request can only apply to the restarted playback; it starts exactly on the row however late in the
        // frame the sequencer restarts it, and a short hop isn't taken for playback carrying on
        stop_audio();
        iAudio->song_stream().force_seek(iTimeline->rows[aRow].frame);
        iSequencer.seek(iSequence, to_position(iTimeline->rows[aRow].frame));
        // seeks are made to positions in the first pass, so the clips laid out for later passes start over
        remove_loop_clips(std::numeric_limits<std::uint64_t>::max());
        iLastPosition = iTimeline->rows[aRow].frame;
    }

    void player::seek_order(std::int32_t aOrderPosition)
    {
        if (!loaded() || aOrderPosition < 0 || aOrderPosition >= static_cast<std::int32_t>(iModule->orders.size()))
            return;
        auto const& rows = iTimeline->rows;
        auto const reaches = [&](row_info const& aRow) { return aRow.orderPosition == static_cast<std::uint32_t>(aOrderPosition); };
        // search from the end of the current visit to the current order position, so that asking for the
        // current position goes to its next visit (or back to its first) rather than to the row already playing
        auto from = std::next(rows.begin(), static_cast<std::ptrdiff_t>(current_row()));
        auto const currentOrderPosition = from->orderPosition;
        from = std::find_if(from, rows.end(), [&](row_info const& aRow) { return aRow.orderPosition != currentOrderPosition; });
        auto target = std::find_if(from, rows.end(), reaches);
        if (target == rows.end())
            target = std::find_if(rows.begin(), rows.end(), reaches);
        if (target != rows.end())
            seek(static_cast<std::size_t>(std::distance(rows.begin(), target)));
    }

    void player::seek_relative(std::int32_t aRows)
    {
        if (!loaded() || iTimeline->rows.empty())
            return;
        auto const last = static_cast<std::int32_t>(iTimeline->rows.size()) - 1;
        seek(static_cast<std::size_t>(std::clamp(static_cast<std::int32_t>(current_row()) + aRows, 0, last)));
    }

    bool player::muted(std::uint32_t aChannel) const
    {
        return iAudio != nullptr && iAudio->song_stream().muted(aChannel);
    }

    void player::set_muted(std::uint32_t aChannel, bool aMuted)
    {
        if (iAudio != nullptr && aChannel < MAX_CHANNELS)
            iAudio->song_stream().set_muted(aChannel, aMuted);
    }

    float player::take_level(std::uint32_t aChannel)
    {
        if (iAudio == nullptr || aChannel >= MAX_CHANNELS)
            return 0.0f;
        return std::min(iAudio->song_stream().take_peak(aChannel), 1.0f);
    }

    void player::update()
    {
        if (loaded() && iSequencer.is_playing(iSequence))
        {
            auto const now = position();
            if (!iLooping || iTimeline->loop_length() == 0u)
            {
                // the module falls silent at the end of the pass it is in; the transport stops there too
                if (now >= iTimeline->pass_end(iLastPosition))
                    stop();
                else
                    iLastPosition = now;
            }
            else
            {
                auto const lookahead = static_cast<std::uint64_t>(PATTERN_CLIP_LOOKAHEAD_S * iTimeline->sampleRate);
                while (iClipsEnd < now + lookahead)
                    add_pattern_clips(*iTimeline->loopRow, iClipsEnd);
                // passes the playhead has left behind are done with, so the track doesn't grow without end
                remove_loop_clips(now);
                iLastPosition = now;
            }
        }
        iSequencer.update();
    }

    std::size_t player::current_row() const
    {
        if (!loaded() || iTimeline->rows.empty())
            return 0u;
        auto const clip = iSequencer.current_clip(*iPatternTrack);
        if (!clip)
        {
            // past the end of the song (it has stopped there, or the next pass isn't laid out yet)
            auto const frame = iTimeline->song_frame(position(), iLooping);
            return frame ? iTimeline->row_at(*frame) : iTimeline->rows.size() - 1u;
        }
        auto const& rows = iSequencer.clip(clip->id).payload<pattern_clip>();
        auto const first = std::next(iTimeline->rows.begin(), static_cast<std::ptrdiff_t>(rows.first_row()));
        auto const last = std::next(first, static_cast<std::ptrdiff_t>(rows.row_count()));
        auto const frame = first->frame + to_frame(clip->elapsed);
        auto const next = std::upper_bound(first, last, frame,
            [](std::uint64_t aFrame, row_info const& aRow) { return aFrame < aRow.frame; });
        return static_cast<std::size_t>(std::distance(iTimeline->rows.begin(), next == first ? first : std::prev(next)));
    }

    std::uint64_t player::current_frame() const
    {
        if (!loaded())
            return 0u;
        return iTimeline->song_frame(position(), iLooping).value_or(iTimeline->frameCount);
    }

    std::uint64_t player::current_pass() const
    {
        if (!loaded() || iTimeline->loop_length() == 0u || position() < iTimeline->frameCount)
            return 0u;
        return 1u + (position() - iTimeline->frameCount) / iTimeline->loop_length();
    }

    std::uint64_t player::position() const
    {
        return to_frame(iSequencer.position(iSequence));
    }

    ng::game::sequencer_position player::to_position(std::uint64_t aFrame) const
    {
        return static_cast<ng::game::sequencer_position>(aFrame) * TOCKS_PER_SECOND / static_cast<ng::game::sequencer_position>(iTimeline->sampleRate);
    }

    std::uint64_t player::to_frame(ng::game::sequencer_position aPosition) const
    {
        return static_cast<std::uint64_t>(std::max<ng::game::sequencer_position>(aPosition, 0) * static_cast<ng::game::sequencer_position>(iTimeline->sampleRate) / TOCKS_PER_SECOND);
    }

    void player::add_pattern_clips(std::size_t aFirstRow, std::uint64_t aStart)
    {
        // one pattern clip per visit to an order position, from aFirstRow to the end of the song, laid out from
        // playback position aStart; a song that jumps back to an order position it has already played gets a
        // second clip for the second visit
        auto const& rows = iTimeline->rows;
        auto const base = rows[aFirstRow].frame;
        bool const loopPass = (aStart != 0u);
        if (loopPass)
            iLoopPasses.push_back(loop_pass{ aStart + iTimeline->frameCount - base, {} });
        for (std::size_t first = aFirstRow; first < rows.size();)
        {
            auto last = first + 1u;
            while (last < rows.size() && rows[last].orderPosition == rows[first].orderPosition && rows[last].row > rows[last - 1u].row)
                ++last;
            auto const start = to_position(aStart + rows[first].frame - base);
            auto const end = to_position(aStart + (last < rows.size() ? rows[last].frame : iTimeline->frameCount) - base);
            auto const id = iSequencer.emplace_clip<pattern_clip>(*iPatternTrack, start, end - start, first, last - first);
            if (loopPass)
                iLoopPasses.back().clips.push_back(id);
            first = last;
        }
        iClipsEnd = aStart + iTimeline->frameCount - base;
    }

    void player::remove_loop_clips(std::uint64_t aBefore)
    {
        while (!iLoopPasses.empty() && iLoopPasses.front().end <= aBefore)
        {
            for (auto const id : iLoopPasses.front().clips)
                iSequencer.delete_clip(id);
            iLoopPasses.pop_front();
        }
        // with every further pass gone the next one is laid out afresh, straight after the first
        if (iLoopPasses.empty())
            iClipsEnd = std::min(iClipsEnd, iTimeline->frameCount);
    }

    void player::stop_audio()
    {
        if (iAudioClip != nullptr)
            iAudioClip->payload<ng::audio_playback>().stop();
    }
}
