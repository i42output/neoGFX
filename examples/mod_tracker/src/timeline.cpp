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

// The song laid out in time (the scan), and real-time playback from it (the stream).

#include <mod_tracker/replayer.hpp>

#include <algorithm>
#include <iterator>
#include <unordered_map>

namespace mod_tracker
{
    namespace
    {
        // a playback position this close to where the song is counts as carrying on rather than a seek
        constexpr double CONTINUITY_TOLERANCE_S = 0.1;
        // how far apart (at least) the scan keeps snapshots of the engine; a seek advances from the one before it
        constexpr double SNAPSHOT_INTERVAL_S = 0.5;
    }

    std::size_t timeline::row_at(std::uint64_t aFrame) const
    {
        auto const next = std::upper_bound(rows.begin(), rows.end(), aFrame,
            [](std::uint64_t aLhs, row_info const& aRhs) { return aLhs < aRhs.frame; });
        return next == rows.begin() ? 0u : static_cast<std::size_t>(std::distance(rows.begin(), next) - 1);
    }

    std::size_t timeline::snapshot_for(std::size_t aRow) const
    {
        auto const next = std::upper_bound(snapshotRows.begin(), snapshotRows.end(), aRow);
        return next == snapshotRows.begin() ? 0u : static_cast<std::size_t>(std::distance(snapshotRows.begin(), next) - 1);
    }

    std::optional<std::uint64_t> timeline::song_frame(std::uint64_t aPosition, bool aLooping) const
    {
        if (aPosition < frameCount)
            return aPosition;
        if (!aLooping || loop_length() == 0u)
            return {};
        return rows[*loopRow].frame + (aPosition - frameCount) % loop_length();
    }

    std::uint64_t timeline::pass_end(std::uint64_t aPosition) const
    {
        if (aPosition < frameCount || loop_length() == 0u)
            return frameCount;
        return frameCount + ((aPosition - frameCount) / loop_length() + 1u) * loop_length();
    }

    timeline scan(module_data const& aModule, render_options const& aOptions, std::stop_token aStop, std::atomic<float>& aProgress)
    {
        aProgress.store(0.0f);
        timeline result;
        result.sampleRate = aOptions.sampleRate;
        result.channels = aModule.channels;
        auto const maximumFrames = static_cast<std::uint64_t>(aOptions.maximumDuration_s * aOptions.sampleRate);
        auto const snapshotInterval = static_cast<std::uint64_t>(SNAPSHOT_INTERVAL_S * aOptions.sampleRate);

        replayer engine{ aModule, aOptions };
        struct visit_info
        {
            std::size_t row;
            replayer_state timing;
        };
        // order position << 32 | row, for every row played, with the timing the row started with; a row
        // played again (other than by a pattern loop) means the song has looped
        std::unordered_map<std::uint64_t, visit_info> visited;
        std::uint64_t lastSnapshotFrame = 0u;
        engine.set_row_handler([&](replayer& aEngine)
            {
                if (aStop.stop_requested())
                    throw scan_cancelled{};
                auto const& state = aEngine.state();
                // the previous row's effects have all been applied by now, so its speed and tempo are known
                if (!result.rows.empty())
                {
                    result.rows.back().speed = state.speed;
                    result.rows.back().tempo = state.tempo;
                }
                auto const visit = (static_cast<std::uint64_t>(state.orderPosition) << 32u) | state.row;
                if (state.patternLoopJump)
                {
                    // a pattern loop replays rows of this pattern; they don't count as the song looping
                    std::erase_if(visited, [&](auto const& aVisit)
                        {
                            return (aVisit.first >> 32u) == state.orderPosition && (aVisit.first & 0xFFFFFFFFu) >= state.row;
                        });
                }
                else if (auto const existing = visited.find(visit); existing != visited.end())
                {
                    result.loopRow = existing->second.row;
                    result.loopTiming = existing->second.timing;
                    return false;
                }
                if (aEngine.frames() >= maximumFrames)
                    return false;
                replayer_state timing;
                timing.speed = state.speed;
                timing.tempo = state.tempo;
                timing.tickFraction = state.tickFraction;
                visited.insert_or_assign(visit, visit_info{ result.rows.size(), timing });
                if (result.snapshots.empty() || aEngine.frames() - lastSnapshotFrame >= snapshotInterval)
                {
                    result.snapshotRows.push_back(result.rows.size());
                    result.snapshots.push_back(state);
                    lastSnapshotFrame = aEngine.frames();
                }
                result.rows.push_back(row_info{ state.orderPosition, state.pattern, state.row, aEngine.frames(), state.speed, state.tempo });
                if (!aModule.orders.empty())
                    aProgress.store(std::max(aProgress.load(), static_cast<float>(state.orderPosition) / static_cast<float>(aModule.orders.size())));
                return true;
            });
        while (!engine.ended())
            engine.render(nullptr, aOptions.sampleRate);
        if (!result.rows.empty() && !result.loopRow)
        {
            // a song that stops never reaches another row to complete the last one's details
            result.rows.back().speed = engine.state().speed;
            result.rows.back().tempo = engine.state().tempo;
        }
        result.frameCount = engine.frames();
        aProgress.store(1.0f);
        return result;
    }

    stream::stream(module_data const& aModule, timeline const& aTimeline, render_options const& aOptions) :
        iTimeline{ aTimeline },
        iReplayer{ aModule, aOptions },
        iContinuityTolerance{ static_cast<std::uint64_t>(CONTINUITY_TOLERANCE_S * aOptions.sampleRate) }
    {
        // a new engine is exactly where the scan started, so playback from the top needs no seek
        iReplayer.set_row_handler([this](replayer&) { return row_starting(); });
    }

    void stream::render(std::uint64_t aPosition, float* aOutput, std::uint64_t aFrames)
    {
        auto const seekRequest = iSeekRequest.load(std::memory_order_acquire);
        if (seekRequest != iSeenSeekRequest)
        {
            iSeenSeekRequest = seekRequest;
            seek(iSeekTarget.load(std::memory_order_relaxed));
        }
        else if (aPosition != iNextCursor)
        {
            // playback was restarted; close to where the song already is means carry on, so that a pause and
            // resume (or the sequencer restarting playback after a stall) is seamless
            auto const distance = aPosition > iNextPosition ? aPosition - iNextPosition : iNextPosition - aPosition;
            if (distance > iContinuityTolerance)
                seek(aPosition);
        }
        iNextCursor = aPosition + aFrames;
        iNextPosition += aFrames;
        iReplayer.render(aOutput, aFrames, iMuted.load(std::memory_order_relaxed));
        for (std::uint32_t channel = 0u; channel < iTimeline.channels && channel < MAX_CHANNELS; ++channel)
        {
            auto const peak = iReplayer.take_peak(channel);
            if (peak > iPeaks[channel].load(std::memory_order_relaxed))
                iPeaks[channel].store(peak, std::memory_order_relaxed);
        }
    }

    bool stream::looping() const
    {
        return iLooping.load(std::memory_order_relaxed);
    }

    void stream::set_looping(bool aLooping)
    {
        iLooping.store(aLooping, std::memory_order_relaxed);
    }

    bool stream::muted(std::uint32_t aChannel) const
    {
        return aChannel < 64u && (iMuted.load(std::memory_order_relaxed) & (std::uint64_t{ 1u } << aChannel)) != 0u;
    }

    void stream::set_muted(std::uint32_t aChannel, bool aMuted)
    {
        if (aChannel >= 64u)
            return;
        if (aMuted)
            iMuted.fetch_or(std::uint64_t{ 1u } << aChannel, std::memory_order_relaxed);
        else
            iMuted.fetch_and(~(std::uint64_t{ 1u } << aChannel), std::memory_order_relaxed);
    }

    void stream::force_seek(std::uint64_t aPosition)
    {
        iSeekTarget.store(aPosition, std::memory_order_relaxed);
        iSeekRequest.fetch_add(1u, std::memory_order_release);
    }

    float stream::take_peak(std::uint32_t aChannel)
    {
        return aChannel < MAX_CHANNELS ? iPeaks[aChannel].exchange(0.0f, std::memory_order_relaxed) : 0.0f;
    }

    void stream::seek(std::uint64_t aPosition)
    {
        iNextPosition = aPosition;
        auto const frame = iTimeline.song_frame(aPosition, looping());
        if (!frame || iTimeline.rows.empty() || iTimeline.snapshots.empty())
        {
            iReplayer.end();
            return;
        }
        // from the latest snapshot at or before the row, to the exact frame, without mixing
        auto const snapshot = iTimeline.snapshot_for(iTimeline.row_at(*frame));
        auto const row = iTimeline.snapshotRows[snapshot];
        iReplayer.restore(iTimeline.snapshots[snapshot]);
        iNextRow = row;
        iReplayer.render(nullptr, *frame - iTimeline.rows[row].frame);
    }

    bool stream::row_starting()
    {
        if (iNextRow >= iTimeline.rows.size())
        {
            // the end of the song: carry on into the loop (as the song itself would), taking the timing it had
            // the first time through so that every pass lasts exactly as long as the scan says
            if (!looping() || !iTimeline.loopRow)
                return false;
            iNextRow = *iTimeline.loopRow;
            if (iTimeline.loopTiming)
                iReplayer.restore_timing(*iTimeline.loopTiming);
        }
        ++iNextRow;
        return true;
    }
}
