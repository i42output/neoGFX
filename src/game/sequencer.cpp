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
#include <neolib/ecs/chrono.hpp>

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
            // one tock == one flick, so that clip positions and ECS step time are the same
            // unit and to_step_time/from_step_time apply to them unchanged
            constexpr sequencer_position TocksPerSecond = neolib::ecs::chrono::flicks::period::den;
        }

        sequencer::sequencer() :
            iTimeline{ std::make_shared<timeline const>() },
            iSequences{ std::make_shared<sequence_set const>() }
        {
            iDefaultSequence = create_sequence();
        }

        sequencer_sequence_id sequencer::create_sequence()
        {
            auto const sequenceId = static_cast<sequencer_sequence_id>(iNextSequenceCookie.fetch_add(1, std::memory_order_relaxed));
            neolib::rcu_update(iSequences, [sequenceId](sequence_set& aSequences)
                {
                    auto const insertAt = std::lower_bound(aSequences.sequences.begin(), aSequences.sequences.end(), sequenceId,
                        [](sequence_entry const& aLhs, sequencer_sequence_id aRhs) { return aLhs.id < aRhs; });
                    aSequences.sequences.insert(insertAt, sequence_entry{ sequenceId, transport{} });
                    ++aSequences.generation;
                });
            return sequenceId;
        }

        void sequencer::delete_sequence(sequencer_sequence_id aSequence)
        {
            if (aSequence == iDefaultSequence)
                throw cannot_delete_default_sequence{};
            if (find_sequence(*iSequences.load(std::memory_order_acquire), aSequence) == npos)
                throw sequence_not_found{};
            // tracks go first: a sequence momentarily without tracks is harmless, a track
            // momentarily without a sequence is not
            neolib::rcu_update(iTimeline, [aSequence](timeline& aTimeline)
                {
                    aTimeline.clips.erase(std::remove_if(aTimeline.clips.begin(), aTimeline.clips.end(),
                        [&aTimeline, aSequence](clip_locator const& aLocator)
                        {
                            auto const trackIndex = find_track(aTimeline, aLocator.track);
                            return trackIndex != npos && aTimeline.tracks[trackIndex].sequence == aSequence;
                        }), aTimeline.clips.end());
                    aTimeline.tracks.erase(std::remove_if(aTimeline.tracks.begin(), aTimeline.tracks.end(),
                        [aSequence](track_entry const& aEntry) { return aEntry.sequence == aSequence; }), aTimeline.tracks.end());
                    ++aTimeline.generation;
                });
            neolib::rcu_update(iSequences, [aSequence](sequence_set& aSequences)
                {
                    auto const sequenceIndex = find_sequence(aSequences, aSequence);
                    if (sequenceIndex == npos)
                        throw sequence_not_found{};
                    aSequences.sequences.erase(std::next(aSequences.sequences.begin(), sequenceIndex));
                    ++aSequences.generation;
                });
        }

        sequencer_sequence_id sequencer::default_sequence() const
        {
            return iDefaultSequence;
        }

        sequencer_track_id sequencer::create_track(sequencer_sequence_id aSequence)
        {
            // the sequence must exist before a track names it; a sequence deleted
            // between this check and publication takes the new track with it
            if (find_sequence(*iSequences.load(std::memory_order_acquire), aSequence) == npos)
                throw sequence_not_found{};
            auto const trackId = static_cast<sequencer_track_id>(iNextTrackCookie.fetch_add(1, std::memory_order_relaxed));
            neolib::rcu_update(iTimeline, [trackId, aSequence](timeline& aTimeline)
                {
                    auto const insertAt = std::lower_bound(aTimeline.tracks.begin(), aTimeline.tracks.end(), trackId,
                        [](track_entry const& aLhs, sequencer_track_id aRhs) { return aLhs.id < aRhs; });
                    aTimeline.tracks.insert(insertAt, track_entry{ trackId, aSequence, std::make_shared<track_clips const>() });
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

        sequencer_sequence_id sequencer::track_sequence(sequencer_track_id aTrack) const
        {
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const trackIndex = find_track(*currentTimeline, aTrack);
            if (trackIndex == npos)
                throw track_not_found{};
            return currentTimeline->tracks[trackIndex].sequence;
        }

        std::size_t sequencer::sequence_count() const
        {
            return iSequences.load(std::memory_order_acquire)->sequences.size();
        }

        sequencer_sequence_id sequencer::sequence_at(std::size_t aIndex) const
        {
            auto const currentSequences = iSequences.load(std::memory_order_acquire);
            if (aIndex >= currentSequences->sequences.size())
                throw sequence_not_found{};
            return currentSequences->sequences[aIndex].id;
        }

        std::size_t sequencer::track_count(sequencer_sequence_id aSequence) const
        {
            if (find_sequence(*iSequences.load(std::memory_order_acquire), aSequence) == npos)
                throw sequence_not_found{};
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            return static_cast<std::size_t>(std::count_if(currentTimeline->tracks.begin(), currentTimeline->tracks.end(),
                [aSequence](track_entry const& aEntry) { return aEntry.sequence == aSequence; }));
        }

        sequencer_track_id sequencer::track_at(sequencer_sequence_id aSequence, std::size_t aIndex) const
        {
            // linear: tracks are stored by id, not grouped by sequence
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            for (auto const& theTrack : currentTimeline->tracks)
                if (theTrack.sequence == aSequence && aIndex-- == 0)
                    return theTrack.id;
            throw track_not_found{};
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

        void sequencer::clear_track(sequencer_track_id aTrack)
        {
            neolib::rcu_update(iTimeline, [aTrack](timeline& aTimeline)
                {
                    auto const trackIndex = find_track(aTimeline, aTrack);
                    if (trackIndex == npos)
                        throw track_not_found{};
                    auto& theTrack = aTimeline.tracks[trackIndex];
                    // already empty: bumping the generation would cost the update thread
                    // a full resync for no change at all
                    if (theTrack.clips->empty())
                        return;
                    // only this track's storage is replaced, so every other track stays
                    // shared with the previous snapshot
                    theTrack.clips = std::make_shared<track_clips const>();
                    aTimeline.clips.erase(std::remove_if(aTimeline.clips.begin(), aTimeline.clips.end(),
                        [aTrack](clip_locator const& aLocator) { return aLocator.track == aTrack; }), aTimeline.clips.end());
                    ++aTimeline.generation;
                });
        }

        bool sequencer::is_playing(sequencer_sequence_id aSequence) const
        {
            return state(aSequence) == transport_state::Playing;
        }

        sequencer_position sequencer::position(sequencer_sequence_id aSequence) const
        {
            auto const currentSequences = iSequences.load(std::memory_order_acquire);
            auto const sequenceIndex = find_sequence(*currentSequences, aSequence);
            if (sequenceIndex == npos)
                throw sequence_not_found{};
            return position_of(currentSequences->sequences[sequenceIndex].playhead, now());
        }

        void sequencer::play(sequencer_sequence_id aSequence)
        {
            modify_transport(aSequence, [this](transport& aTransport)
                {
                    if (aTransport.state == transport_state::Playing)
                        return;
                    aTransport.anchor = now();
                    aTransport.state = transport_state::Playing;
                });
        }

        void sequencer::pause(sequencer_sequence_id aSequence)
        {
            modify_transport(aSequence, [this](transport& aTransport)
                {
                    if (aTransport.state != transport_state::Playing)
                        return;
                    auto const currentTock = now();
                    aTransport.position = position_of(aTransport, currentTock);
                    aTransport.anchor = currentTock;
                    aTransport.state = transport_state::Paused;
                });
        }

        void sequencer::rewind(sequencer_sequence_id aSequence)
        {
            seek(aSequence, 0);
        }

        void sequencer::seek(sequencer_sequence_id aSequence, sequencer_position aPosition)
        {
            modify_transport(aSequence, [this, aPosition](transport& aTransport)
                {
                    aTransport.position = std::max<sequencer_position>(aPosition, 0);
                    aTransport.anchor = now();
                    ++aTransport.seekGeneration;
                });
        }

        void sequencer::stop(sequencer_sequence_id aSequence)
        {
            modify_transport(aSequence, [this](transport& aTransport)
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
            auto const trackPosition = track_position(*currentTimeline, trackIndex);
            if (!trackPosition)
                return {};
            auto const currentPosition = *trackPosition;
            auto const cursor = find_cursor(trackClips, currentPosition);
            if (cursor == trackClips.size())
                return {};
            auto const& candidate = trackClips[cursor];
            if (currentPosition < candidate.start)
                return {};
            return sequencer_clip_info{ candidate.id, currentPosition - candidate.start, candidate.duration };
        }

        optional_sequencer_clip_info sequencer::next_clip(sequencer_track_id aTrack) const
        {
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const trackIndex = find_track(*currentTimeline, aTrack);
            if (trackIndex == npos)
                throw track_not_found{};
            auto const& trackClips = *currentTimeline->tracks[trackIndex].clips;
            auto const trackPosition = track_position(*currentTimeline, trackIndex);
            if (!trackPosition)
                return {};
            auto const currentPosition = *trackPosition;
            for (auto cursor = find_cursor(trackClips, currentPosition); cursor != trackClips.size(); ++cursor)
                if (trackClips[cursor].start > currentPosition)
                    return sequencer_clip_info{ trackClips[cursor].id, 0, trackClips[cursor].duration };
            return {};
        }

        optional_sequencer_clip_info sequencer::clip_info(sequencer_clip_id aClipId) const
        {
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const clipIndex = find_clip(*currentTimeline, aClipId);
            if (clipIndex == npos)
                return {};
            auto const& locator = currentTimeline->clips[clipIndex];
            auto const trackIndex = find_track(*currentTimeline, locator.track);
            if (trackIndex == npos)
                return {};
            auto const trackPosition = track_position(*currentTimeline, trackIndex);
            if (!trackPosition)
                return {};
            auto const currentPosition = *trackPosition;
            if (currentPosition < locator.start)
                return {};
            return sequencer_clip_info{ aClipId,
                std::min(currentPosition - locator.start, locator.duration), locator.duration };
        }

        void sequencer::update()
        {
            // both snapshots are held for the duration of the update so that clips
            // cannot be destroyed underneath us by a concurrent edit
            auto const currentTimeline = iTimeline.load(std::memory_order_acquire);
            auto const currentSequences = iSequences.load(std::memory_order_acquire);
            // one clock reading for the whole update, so that sequences advancing
            // together stay in step with each other
            auto const currentTock = now();
            if (currentTimeline->generation != iSeenGeneration ||
                currentSequences->generation != iSeenSequenceGeneration ||
                iPlayback.size() != currentTimeline->tracks.size())
            {
                resync(*currentTimeline, *currentSequences, currentTock);
                iSeenGeneration = currentTimeline->generation;
                iSeenSequenceGeneration = currentSequences->generation;
            }
            for (std::size_t index = 0; index != currentTimeline->tracks.size(); ++index)
            {
                auto& thePlayback = iPlayback[index];
                // the track's sequence was deleted after this snapshot of the timeline
                // was taken; the track is on its way out with it
                if (thePlayback.sequenceIndex == npos)
                    continue;
                auto const& theTransport = currentSequences->sequences[thePlayback.sequenceIndex].playhead;
                auto const sequencePosition = position_of(theTransport, currentTock);
                // a discontinuity on one sequence resyncs that sequence's tracks only
                if (thePlayback.seenSeekGeneration != theTransport.seekGeneration)
                {
                    thePlayback.cursor = find_cursor(*currentTimeline->tracks[index].clips, sequencePosition);
                    thePlayback.active = {};
                    thePlayback.seenSeekGeneration = theTransport.seekGeneration;
                }
                update_track(*currentTimeline->tracks[index].clips, thePlayback, sequencePosition);
            }
        }

        sequencer::transport_state sequencer::state(sequencer_sequence_id aSequence) const
        {
            auto const currentSequences = iSequences.load(std::memory_order_acquire);
            auto const sequenceIndex = find_sequence(*currentSequences, aSequence);
            if (sequenceIndex == npos)
                throw sequence_not_found{};
            return currentSequences->sequences[sequenceIndex].playhead.state;
        }

        sequencer_position sequencer::now() const
        {
            static_assert(TocksPerSecond == neolib::ecs::chrono::flicks::period::den);
            // measured from first use rather than the clock's own epoch: a flick is 1/705600000s,
            // so converting a full time_since_epoch would overflow the intermediate
            static auto const sEpoch = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<neolib::ecs::chrono::flicks>(
                std::chrono::steady_clock::now() - sEpoch).count();
        }

        sequencer_position sequencer::position_of(transport const& aTransport, sequencer_position aNow)
        {
            if (aTransport.state == transport_state::Playing)
                return aTransport.position + (aNow - aTransport.anchor);
            return aTransport.position;
        }

        std::size_t sequencer::find_sequence(sequence_set const& aSequences, sequencer_sequence_id aSequence)
        {
            auto const existingSequence = std::lower_bound(aSequences.sequences.begin(), aSequences.sequences.end(), aSequence,
                [](sequence_entry const& aLhs, sequencer_sequence_id aRhs) { return aLhs.id < aRhs; });
            if (existingSequence == aSequences.sequences.end() || existingSequence->id != aSequence)
                return npos;
            return static_cast<std::size_t>(std::distance(aSequences.sequences.begin(), existingSequence));
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
                locators.push_back(clip_locator{ aIds[index], aDefinitions[index].track, aDefinitions[index].clip,
                    aDefinitions[index].start, aDefinitions[index].duration });
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

        std::optional<sequencer_position> sequencer::track_position(timeline const& aTimeline, std::size_t aTrackIndex) const
        {
            auto const currentSequences = iSequences.load(std::memory_order_acquire);
            auto const sequenceIndex = find_sequence(*currentSequences, aTimeline.tracks[aTrackIndex].sequence);
            if (sequenceIndex == npos)
                return {};
            return position_of(currentSequences->sequences[sequenceIndex].playhead, now());
        }

        void sequencer::resync(timeline const& aTimeline, sequence_set const& aSequences, sequencer_position aNow)
        {
            iPlayback.clear();
            iPlayback.reserve(aTimeline.tracks.size());
            for (auto const& theTrack : aTimeline.tracks)
            {
                auto const sequenceIndex = find_sequence(aSequences, theTrack.sequence);
                auto const sequencePosition = (sequenceIndex != npos ?
                    position_of(aSequences.sequences[sequenceIndex].playhead, aNow) : sequencer_position{});
                auto const seenSeekGeneration = (sequenceIndex != npos ?
                    aSequences.sequences[sequenceIndex].playhead.seekGeneration : std::uint64_t{});
                iPlayback.push_back(playback{
                    theTrack.id,
                    theTrack.sequence,
                    sequenceIndex,
                    find_cursor(*theTrack.clips, sequencePosition),
                    seenSeekGeneration,
                    {} });
            }
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
