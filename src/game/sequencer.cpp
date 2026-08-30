// sequencer.cpp
/*
  neolib C++ App/Game Engine
  Copyright (c)  2026 Leigh Johnston.  All Rights Reserved.

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
#include <iterator>
#include <utility>

#include <neolib/core/rcu.hpp>

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

        sequencer::sequencer() :
            iTimeline{ std::make_shared<timeline const>() },
            iTransport{ std::make_shared<transport const>() }
        {}

        sequencer_track_id sequencer::create_track()
        {
            auto const trackId = static_cast<sequencer_track_id>(iNextTrackCookie.fetch_add(1, std::memory_order_relaxed));
            neolib::rcu_update(iTimeline, [trackId](timeline& aTimeline)
                {
                    auto const insertAt = std::lower_bound(aTimeline.tracks.begin(), aTimeline.tracks.end(), trackId,
                        [](track_entry const& aLhs, sequencer_track_id aRhs) { return aLhs.id < aRhs; });
                    aTimeline.tracks.insert(insertAt, track_entry{ trackId, std::make_shared<track_clips const>() });
                    ++aTimeline.generation;
                });
            return trackId;
        }

        void sequencer::delete_track(sequencer_track_id aTrack)
        {
            neolib::rcu_update(iTimeline, [aTrack](timeline& aTimeline)
                {
                    auto const trackIndex = find_track(aTimeline, aTrack);
                    if (trackIndex == npos)
                        throw track_not_found{};
                    aTimeline.tracks.erase(std::next(aTimeline.tracks.begin(), trackIndex));
                    aTimeline.clips.erase(std::remove_if(aTimeline.clips.begin(), aTimeline.clips.end(),
                        [aTrack](clip_locator const& aLocator) { return aLocator.track == aTrack; }), aTimeline.clips.end());
                    ++aTimeline.generation;
                });
        }

        i_sequencer_clip const& sequencer::clip(sequencer_clip_id aClipId) const
        {
            return *clip_ptr(aClipId);
        }

        i_sequencer_clip& sequencer::clip(sequencer_clip_id aClipId)
        {
            return *clip_ptr(aClipId);
        }

        sequencer_clip_ptr sequencer::clip_ptr(sequencer_clip_id aClipId) const
        {
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const clipIndex = find_clip(*currentTimeline, aClipId);
            if (clipIndex == npos)
                throw clip_not_found{};
            return currentTimeline->clips[clipIndex].clip;
        }

        sequencer_clip_id sequencer::add_clip(i_ref_ptr<i_sequencer_clip> const& aClip, sequencer_track_id aTrack, sequencer_position aStart, sequencer_duration aDuration)
        {
            if (aClip.ptr() == nullptr)
                throw no_clip{};
            return add_clips({ clip_definition{ sequencer_clip_ptr{ aClip }, aTrack, aStart, aDuration } }).front();
        }

        sequencer::clip_ids sequencer::add_clips(clip_definitions const& aClips)
        {
            clip_ids newIds;
            newIds.reserve(aClips.size());
            for (auto const& definition : aClips)
            {
                if (definition.clip.ptr() == nullptr)
                    throw no_clip{};
                newIds.push_back(static_cast<sequencer_clip_id>(iNextClipCookie.fetch_add(1, std::memory_order_relaxed)));
            }
            // ids are set before publication so that no reader sees an unidentified clip
            for (std::size_t index = 0; index != aClips.size(); ++index)
                aClips[index].clip->set_id(newIds[index]);
            // all sorting and grouping happens here, once, outside the retry loop
            auto const groups = group_clips(aClips, newIds);
            auto const locators = locate_clips(aClips, newIds);
            neolib::rcu_update(iTimeline, [&groups, &locators](timeline& aTimeline)
                {
                    insert_clips(aTimeline, groups, locators);
                    ++aTimeline.generation;
                });
            return newIds;
        }

        void sequencer::delete_clip(sequencer_clip_id aClipId)
        {
            neolib::rcu_update(iTimeline, [aClipId](timeline& aTimeline)
                {
                    auto const clipIndex = find_clip(aTimeline, aClipId);
                    if (clipIndex == npos)
                        throw clip_not_found{};
                    auto const trackIndex = find_track(aTimeline, aTimeline.clips[clipIndex].track);
                    if (trackIndex != npos)
                    {
                        auto& theTrack = aTimeline.tracks[trackIndex];
                        // copy-on-write: only this track's storage is duplicated
                        auto updatedClips = std::make_shared<track_clips>(*theTrack.clips);
                        updatedClips->erase(std::remove_if(updatedClips->begin(), updatedClips->end(),
                            [aClipId](clip_entry const& aEntry) { return aEntry.id == aClipId; }), updatedClips->end());
                        theTrack.clips = std::move(updatedClips);
                    }
                    aTimeline.clips.erase(std::next(aTimeline.clips.begin(), clipIndex));
                    ++aTimeline.generation;
                });
        }

        bool sequencer::is_playing() const
        {
            return iTransport.load(std::memory_order_acquire)->state == transport_state::Playing;
        }

        sequencer_position sequencer::position() const
        {
            return position_of(*iTransport.load(std::memory_order_acquire), now());
        }

        void sequencer::play()
        {
            neolib::rcu_update(iTransport, [this](transport& aTransport)
                {
                    if (aTransport.state == transport_state::Playing)
                        return;
                    aTransport.anchor = now();
                    aTransport.state = transport_state::Playing;
                });
        }

        void sequencer::pause()
        {
            neolib::rcu_update(iTransport, [this](transport& aTransport)
                {
                    if (aTransport.state != transport_state::Playing)
                        return;
                    auto const currentTock = now();
                    aTransport.position = position_of(aTransport, currentTock);
                    aTransport.anchor = currentTock;
                    aTransport.state = transport_state::Paused;
                });
        }

        void sequencer::rewind()
        {
            seek(0);
        }

        void sequencer::seek(sequencer_position aPosition)
        {
            neolib::rcu_update(iTransport, [this, aPosition](transport& aTransport)
                {
                    aTransport.position = std::max<sequencer_position>(aPosition, 0);
                    aTransport.anchor = now();
                    ++aTransport.seekGeneration;
                });
        }

        void sequencer::stop()
        {
            neolib::rcu_update(iTransport, [this](transport& aTransport)
                {
                    aTransport.state = transport_state::Stopped;
                    aTransport.position = 0;
                    aTransport.anchor = now();
                    ++aTransport.seekGeneration;
                });
        }

        optional_sequencer_clip_info sequencer::current_clip(sequencer_track_id aTrack) const
        {
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const trackIndex = find_track(*currentTimeline, aTrack);
            if (trackIndex == npos)
                throw track_not_found{};
            auto const& trackClips = *currentTimeline->tracks[trackIndex].clips;
            auto const currentPosition = position();
            auto const cursor = find_cursor(trackClips, currentPosition);
            if (cursor == trackClips.size())
                return {};
            auto const& candidate = trackClips[cursor];
            if (currentPosition < candidate.start)
                return {};
            return sequencer_clip_info{ candidate.id, currentPosition - candidate.start };
        }

        optional_sequencer_clip_info sequencer::next_clip(sequencer_track_id aTrack) const
        {
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const trackIndex = find_track(*currentTimeline, aTrack);
            if (trackIndex == npos)
                throw track_not_found{};
            auto const& trackClips = *currentTimeline->tracks[trackIndex].clips;
            auto const currentPosition = position();
            for (auto cursor = find_cursor(trackClips, currentPosition); cursor != trackClips.size(); ++cursor)
                if (trackClips[cursor].start > currentPosition)
                    return sequencer_clip_info{ trackClips[cursor].id, 0 };
            return {};
        }

        void sequencer::update()
        {
            // both snapshots are held for the duration of the update so that clips
            // cannot be destroyed underneath us by a concurrent edit
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const currentTransport = iTransport.load(std::memory_order_acquire);
            auto const currentPosition = position_of(*currentTransport, now());
            if (currentTimeline->generation != iSeenGeneration ||
                currentTransport->seekGeneration != iSeenSeekGeneration ||
                iPlayback.size() != currentTimeline->tracks.size())
            {
                resync(*currentTimeline, currentPosition);
                iSeenGeneration = currentTimeline->generation;
                iSeenSeekGeneration = currentTransport->seekGeneration;
            }
            for (std::size_t index = 0; index != currentTimeline->tracks.size(); ++index)
                update_track(*currentTimeline->tracks[index].clips, iPlayback[index], currentPosition);
        }

        sequencer::transport_state sequencer::state() const
        {
            return iTransport.load(std::memory_order_acquire)->state;
        }

        sequencer_position sequencer::now() const
        {
            static_assert(TocksPerSecond == std::chrono::microseconds::period::den);
            return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        sequencer_position sequencer::position_of(transport const& aTransport, sequencer_position aNow)
        {
            if (aTransport.state == transport_state::Playing)
                return aTransport.position + (aNow - aTransport.anchor);
            return aTransport.position;
        }

        std::size_t sequencer::find_track(timeline const& aTimeline, sequencer_track_id aTrack)
        {
            auto const existingTrack = std::lower_bound(aTimeline.tracks.begin(), aTimeline.tracks.end(), aTrack,
                [](track_entry const& aLhs, sequencer_track_id aRhs) { return aLhs.id < aRhs; });
            if (existingTrack == aTimeline.tracks.end() || existingTrack->id != aTrack)
                return npos;
            return static_cast<std::size_t>(std::distance(aTimeline.tracks.begin(), existingTrack));
        }

        std::size_t sequencer::find_clip(timeline const& aTimeline, sequencer_clip_id aClipId)
        {
            auto const existingClip = std::lower_bound(aTimeline.clips.begin(), aTimeline.clips.end(), aClipId,
                [](clip_locator const& aLhs, sequencer_clip_id aRhs) { return aLhs.id < aRhs; });
            if (existingClip == aTimeline.clips.end() || existingClip->id != aClipId)
                return npos;
            return static_cast<std::size_t>(std::distance(aTimeline.clips.begin(), existingClip));
        }

        std::size_t sequencer::find_cursor(track_clips const& aClips, sequencer_position aPosition)
        {
            // first clip that has not yet ended at aPosition
            auto const cursor = std::lower_bound(aClips.begin(), aClips.end(), aPosition,
                [](clip_entry const& aLhs, sequencer_position aRhs) { return aLhs.end() <= aRhs; });
            return static_cast<std::size_t>(std::distance(aClips.begin(), cursor));
        }

        sequencer::track_groups sequencer::group_clips(clip_definitions const& aDefinitions, clip_ids const& aIds)
        {
            // one group per distinct track, so that each affected track's storage is
            // copied once regardless of how many clips land on it; a batch touches few
            // tracks, so the linear search for the group is cheaper than sorting
            track_groups groups;
            for (std::size_t index = 0; index != aDefinitions.size(); ++index)
            {
                auto const& definition = aDefinitions[index];
                auto existingGroup = std::find_if(groups.begin(), groups.end(),
                    [&definition](track_group const& aGroup) { return aGroup.track == definition.track; });
                if (existingGroup == groups.end())
                {
                    groups.push_back(track_group{ definition.track, {} });
                    existingGroup = std::prev(groups.end());
                }
                existingGroup->entries.push_back(clip_entry{
                    aIds[index],
                    definition.clip,
                    std::max<sequencer_position>(definition.start, 0),
                    std::max<sequencer_duration>(definition.duration, 0) });
            }
            for (auto& theGroup : groups)
                std::stable_sort(theGroup.entries.begin(), theGroup.entries.end(),
                    [](clip_entry const& aLhs, clip_entry const& aRhs) { return aLhs.start < aRhs.start; });
            return groups;
        }

        sequencer::clip_index sequencer::locate_clips(clip_definitions const& aDefinitions, clip_ids const& aIds)
        {
            // ids were allocated in ascending order within this batch, so building the
            // locators in definition order already yields the ordering the index needs
            clip_index locators;
            locators.reserve(aDefinitions.size());
            for (std::size_t index = 0; index != aDefinitions.size(); ++index)
                locators.push_back(clip_locator{ aIds[index], aDefinitions[index].track, aDefinitions[index].clip });
            return locators;
        }

        void sequencer::insert_clips(timeline& aTimeline, track_groups const& aGroups, clip_index const& aLocators)
        {
            for (auto const& theGroup : aGroups)
            {
                auto const trackIndex = find_track(aTimeline, theGroup.track);
                if (trackIndex == npos)
                    throw track_not_found{};
                auto& theTrack = aTimeline.tracks[trackIndex];
                auto const& existingClips = *theTrack.clips;
                // copy-on-write: only this track's storage is duplicated, and the merge
                // target is sized up front so nothing reallocates part way through
                auto updatedClips = std::make_shared<track_clips>();
                updatedClips->reserve(existingClips.size() + theGroup.entries.size());
                std::merge(existingClips.begin(), existingClips.end(), theGroup.entries.begin(), theGroup.entries.end(),
                    std::back_inserter(*updatedClips),
                    [](clip_entry const& aLhs, clip_entry const& aRhs) { return aLhs.start < aRhs.start; });
                theTrack.clips = std::move(updatedClips);
            }
            clip_index updatedIndex;
            updatedIndex.reserve(aTimeline.clips.size() + aLocators.size());
            std::merge(aTimeline.clips.begin(), aTimeline.clips.end(), aLocators.begin(), aLocators.end(),
                std::back_inserter(updatedIndex),
                [](clip_locator const& aLhs, clip_locator const& aRhs) { return aLhs.id < aRhs.id; });
            aTimeline.clips = std::move(updatedIndex);
        }

        void sequencer::resync(timeline const& aTimeline, sequencer_position aPosition)
        {
            iPlayback.clear();
            iPlayback.reserve(aTimeline.tracks.size());
            for (auto const& theTrack : aTimeline.tracks)
                iPlayback.push_back(playback{ theTrack.id, find_cursor(*theTrack.clips, aPosition), {} });
        }

        void sequencer::update_track(track_clips const& aClips, playback& aPlayback, sequencer_position aPosition)
        {
            aPlayback.active = {};
            aPlayback.cursor = std::min(aPlayback.cursor, aClips.size());
            while (aPlayback.cursor != aClips.size())
            {
                auto const& candidate = aClips[aPlayback.cursor];
                if (aPosition >= candidate.end())
                {
                    // clip is behind us; land it on its end state exactly once, even if
                    // the frame delta stepped clean over it
                    candidate.clip->advance(candidate.duration);
                    ++aPlayback.cursor;
                    continue;
                }
                if (aPosition >= candidate.start)
                {
                    candidate.clip->advance(aPosition - candidate.start);
                    aPlayback.active = candidate.id;
                }
                break;
            }
        }
    }
}
