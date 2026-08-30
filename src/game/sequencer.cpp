// sequencer.cpp
/*
  neolib C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/game/sequencer.hpp>

#include <algorithm>
#include <chrono>

template<> neogfx::game::i_sequencer& services::start_service<neogfx::game::i_sequencer>()
{
    static neogfx::game::sequencer sSequencer;
    return sSequencer;
}

namespace neogfx
{
    namespace game
    {
        namespace
        {
            // one tock == one microsecond; replace with the shared timer's resolution
            constexpr sequencer_position TocksPerSecond = 1000000;
        }

        bool sequencer::is_multi_threaded() const
        {
            return iMutex.is<std::recursive_mutex>();
        }

        bool sequencer::is_single_threaded() const
        {
            return iMutex.is<neolib::null_mutex>();
        }

        void sequencer::set_multi_threaded()
        {
            if (!is_multi_threaded())
                iMutex.emplace<std::recursive_mutex>();
        }

        void sequencer::set_single_threaded()
        {
            if (!is_single_threaded())
                iMutex.emplace<neolib::null_mutex>();
        }

        sequencer_track_id sequencer::create_track()
        {
            std::unique_lock lock{ iMutex };

            auto const trackId = static_cast<sequencer_track_id>(iNextTrackCookie++);
            iTracks.emplace(trackId, track_entry{});
            return trackId;
        }

        void sequencer::delete_track(sequencer_track_id aTrack)
        {
            std::unique_lock lock{ iMutex };

            auto existingTrack = iTracks.find(aTrack);
            if (existingTrack == iTracks.end())
                throw track_not_found{};
            for (auto const& clipId : existingTrack->second.clips)
                iClips.erase(clipId);
            iTracks.erase(existingTrack);
        }

        i_sequencer_clip const& sequencer::clip(sequencer_clip_id aClipId) const
        {
            std::unique_lock lock{ iMutex };

            return *entry(aClipId).clip;
        }

        i_sequencer_clip& sequencer::clip(sequencer_clip_id aClipId)
        {
            std::unique_lock lock{ iMutex };

            return *entry(aClipId).clip;
        }

        sequencer_clip_id sequencer::add_clip(i_ref_ptr<i_sequencer_clip> const& aClip, sequencer_track_id aTrack, sequencer_position aStart, sequencer_duration aDuration)
        {
            std::unique_lock lock{ iMutex };

            if (aClip.ptr() == nullptr)
                throw no_clip{};
            auto& theTrack = find_track(aTrack);
            auto const clipId = static_cast<sequencer_clip_id>(iNextClipCookie++);
            auto& newEntry = iClips.emplace(clipId, clip_entry{
                sequencer_clip_ptr{ aClip },
                aTrack,
                std::max<sequencer_position>(aStart, 0),
                std::max<sequencer_duration>(aDuration, 0),
                false }).first->second;
            newEntry.clip->set_id(clipId);
            // keep the track's clips ordered by start position
            auto const insertAt = std::lower_bound(theTrack.clips.begin(), theTrack.clips.end(), newEntry.start,
                [this](sequencer_clip_id aLhs, sequencer_position aRhs) { return entry(aLhs).start < aRhs; });
            theTrack.clips.insert(insertAt, clipId);
            resync_track(theTrack, position());
            return clipId;
        }

        void sequencer::delete_clip(sequencer_clip_id aClipId)
        {
            std::unique_lock lock{ iMutex };

            auto existingClip = iClips.find(aClipId);
            if (existingClip == iClips.end())
                throw clip_not_found{};
            auto& theTrack = find_track(existingClip->second.track);
            theTrack.clips.erase(std::remove(theTrack.clips.begin(), theTrack.clips.end(), aClipId), theTrack.clips.end());
            iClips.erase(existingClip);
            resync_track(theTrack, position());
        }

        bool sequencer::is_playing() const
        {
            std::unique_lock lock{ iMutex };

            return iState == transport_state::Playing;
        }

        sequencer_position sequencer::position() const
        {
            std::unique_lock lock{ iMutex };

            if (iState == transport_state::Playing)
                return iPosition + (now() - iAnchor);
            return iPosition;
        }

        void sequencer::play()
        {
            std::unique_lock lock{ iMutex };

            if (iState == transport_state::Playing)
                return;
            iAnchor = now();
            iState = transport_state::Playing;
        }

        void sequencer::pause()
        {
            std::unique_lock lock{ iMutex };

            if (iState != transport_state::Playing)
                return;
            iPosition = position();
            iAnchor = now();
            iState = transport_state::Paused;
        }

        void sequencer::rewind()
        {
            std::unique_lock lock{ iMutex };

            seek(0);
        }

        void sequencer::seek(sequencer_position aPosition)
        {
            std::unique_lock lock{ iMutex };

            iPosition = std::max<sequencer_position>(aPosition, 0);
            iAnchor = now();
            for (auto& theTrack : iTracks)
                resync_track(theTrack.second, iPosition);
        }

        void sequencer::stop()
        {
            std::unique_lock lock{ iMutex };

            iState = transport_state::Stopped;
            iPosition = 0;
            iAnchor = now();
            for (auto& theTrack : iTracks)
                resync_track(theTrack.second, iPosition);
        }

        optional_sequencer_clip_info sequencer::current_clip(sequencer_track_id aTrack) const
        {
            std::unique_lock lock{ iMutex };

            auto const& theTrack = find_track(aTrack);
            auto const currentPosition = position();
            auto const cursor = find_cursor(theTrack, currentPosition);
            if (cursor == theTrack.clips.size())
                return {};
            auto const& candidate = entry(theTrack.clips[cursor]);
            if (currentPosition < candidate.start)
                return {};
            return sequencer_clip_info{ candidate.clip->id(), currentPosition - candidate.start };
        }

        optional_sequencer_clip_info sequencer::next_clip(sequencer_track_id aTrack) const
        {
            std::unique_lock lock{ iMutex };

            auto const& theTrack = find_track(aTrack);
            auto const currentPosition = position();
            for (auto cursor = find_cursor(theTrack, currentPosition); cursor != theTrack.clips.size(); ++cursor)
            {
                auto const& candidate = entry(theTrack.clips[cursor]);
                if (candidate.start > currentPosition)
                    return sequencer_clip_info{ candidate.clip->id(), 0 };
            }
            return {};
        }

        void sequencer::update()
        {
            std::unique_lock lock{ iMutex };

            auto const currentPosition = position();
            for (auto& theTrack : iTracks)
                update_track(theTrack.second, currentPosition);
        }

        sequencer::transport_state sequencer::state() const
        {
            std::unique_lock lock{ iMutex };

            return iState;
        }

        sequencer_position sequencer::now() const
        {
            std::unique_lock lock{ iMutex };

            static_assert(TocksPerSecond == std::chrono::microseconds::period::den);
            return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        void sequencer::update_track(track_entry& aTrack, sequencer_position aPosition)
        {
            std::unique_lock lock{ iMutex };

            aTrack.active = {};
            while (aTrack.cursor != aTrack.clips.size())
            {
                auto& candidate = entry(aTrack.clips[aTrack.cursor]);
                if (aPosition >= candidate.end())
                {
                    // clip is behind us; land it on its end state exactly once, even if
                    // the frame delta stepped clean over it
                    if (!candidate.finalized)
                    {
                        candidate.clip->advance(candidate.duration);
                        candidate.finalized = true;
                    }
                    ++aTrack.cursor;
                    continue;
                }
                if (aPosition >= candidate.start)
                {
                    candidate.clip->advance(aPosition - candidate.start);
                    aTrack.active = candidate.clip->id();
                }
                break;
            }
        }

        void sequencer::resync_track(track_entry& aTrack, sequencer_position aPosition)
        {
            std::unique_lock lock{ iMutex };

            aTrack.cursor = find_cursor(aTrack, aPosition);
            aTrack.active = {};
            for (std::size_t index = 0; index != aTrack.clips.size(); ++index)
                entry(aTrack.clips[index]).finalized = (index < aTrack.cursor);
        }

        std::size_t sequencer::find_cursor(track_entry const& aTrack, sequencer_position aPosition) const
        {
            std::unique_lock lock{ iMutex };

            // first clip that has not yet ended at aPosition
            auto const cursor = std::lower_bound(aTrack.clips.begin(), aTrack.clips.end(), aPosition,
                [this](sequencer_clip_id aLhs, sequencer_position aRhs) { return entry(aLhs).end() <= aRhs; });
            return static_cast<std::size_t>(std::distance(aTrack.clips.begin(), cursor));
        }

        sequencer::clip_entry const& sequencer::entry(sequencer_clip_id aClipId) const
        {
            std::unique_lock lock{ iMutex };

            auto existingClip = iClips.find(aClipId);
            if (existingClip == iClips.end())
                throw clip_not_found{};
            return existingClip->second;
        }

        sequencer::clip_entry& sequencer::entry(sequencer_clip_id aClipId)
        {
            std::unique_lock lock{ iMutex };

            return const_cast<clip_entry&>(const_cast<sequencer const*>(this)->entry(aClipId));
        }

        sequencer::track_entry const& sequencer::find_track(sequencer_track_id aTrack) const
        {
            std::unique_lock lock{ iMutex };

            auto existingTrack = iTracks.find(aTrack);
            if (existingTrack == iTracks.end())
                throw track_not_found{};
            return existingTrack->second;
        }

        sequencer::track_entry& sequencer::find_track(sequencer_track_id aTrack)
        {
            std::unique_lock lock{ iMutex };

            return const_cast<track_entry&>(const_cast<sequencer const*>(this)->find_track(aTrack));
        }
    }
}
