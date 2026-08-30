// sequencer.hpp
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

#pragma once

#include <neogfx/game/i_sequencer.hpp>

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <vector>

namespace neogfx
{
    namespace game
    {
        // Threading model
        // ---------------
        // The timeline (tracks and clips) and the transport are each held as an
        // immutable snapshot behind an atomic shared_ptr. Mutating operations build a
        // new snapshot and publish it with a compare-exchange loop, so any number of
        // threads may edit or query concurrently without a mutex, and readers never
        // block. Playback bookkeeping (the per-track cursor) is private to the thread
        // that calls update(), which must be one thread only.
        //
        // Storage
        // -------
        // Everything is a sorted vector: clips per track ordered by start position, the
        // track list and the clip index ordered by id. Each track's clip list is itself
        // shared, so editing one track leaves every other track's storage untouched and
        // shared with the previous snapshot. Reads are binary searches over contiguous
        // memory and never allocate; allocation happens only when publishing an edit.
        class sequencer : public i_sequencer
        {
        public:
            struct track_not_found : std::logic_error { track_not_found() : std::logic_error{ "neogfx::game::sequencer::track_not_found" } {} };
            struct clip_not_found : std::logic_error { clip_not_found() : std::logic_error{ "neogfx::game::sequencer::clip_not_found" } {} };
            struct no_clip : std::logic_error { no_clip() : std::logic_error{ "neogfx::game::sequencer::no_clip" } {} };
        public:
            enum class transport_state : std::uint32_t
            {
                Stopped,
                Paused,
                Playing
            };
            // for batched edits; see add_clips
            struct clip_definition
            {
                sequencer_clip_ptr clip;
                sequencer_track_id track;
                sequencer_position start;
                sequencer_duration duration;
            };
            using clip_definitions = std::vector<clip_definition>;
            using clip_ids = std::vector<sequencer_clip_id>;
        private:
            static constexpr std::size_t npos = static_cast<std::size_t>(-1);
            struct clip_entry
            {
                sequencer_clip_id id;
                sequencer_clip_ptr clip;
                sequencer_position start;
                sequencer_duration duration;

                sequencer_position end() const { return start + duration; }
            };
            // ordered by start position
            using track_clips = std::vector<clip_entry>;
            using track_clips_ptr = std::shared_ptr<track_clips const>;
            struct track_entry
            {
                sequencer_track_id id;
                track_clips_ptr clips;
            };
            // ordered by track id
            using track_list = std::vector<track_entry>;
            struct clip_locator
            {
                sequencer_clip_id id;
                sequencer_track_id track;
                sequencer_clip_ptr clip;
            };
            // ordered by clip id
            using clip_index = std::vector<clip_locator>;
            struct timeline
            {
                track_list tracks;
                clip_index clips;
                // bumped on every edit so that update() knows to resync
                std::uint64_t generation = 0;
            };
            using timeline_ptr = std::shared_ptr<timeline const>;
            struct transport
            {
                transport_state state = transport_state::Stopped;
                // position as of the last transport change
                sequencer_position position = 0;
                // clock reading at that transport change
                sequencer_position anchor = 0;
                // bumped on every discontinuity so that update() knows to resync
                std::uint64_t seekGeneration = 0;
            };
            using transport_ptr = std::shared_ptr<transport const>;
            // pending additions for one track, ordered by start position; prepared once
            // before publishing so that a lost compare-exchange does not redo the work
            struct track_group
            {
                sequencer_track_id track;
                track_clips entries;
            };
            using track_groups = std::vector<track_group>;
            // update thread only; kept parallel to timeline::tracks
            struct playback
            {
                sequencer_track_id track;
                // index of the first clip not yet passed; clips before it are done
                std::size_t cursor = 0;
                optional_sequencer_clip_id active;
            };
            using playback_list = std::vector<playback>;
        public:
            sequencer();
            ~sequencer() = default;
        public:
            sequencer_track_id create_track() final;
            void delete_track(sequencer_track_id aTrack) final;
        public:
            i_sequencer_clip const& clip(sequencer_clip_id aClipId) const final;
            i_sequencer_clip& clip(sequencer_clip_id aClipId) final;
        public:
            sequencer_clip_id add_clip(i_ref_ptr<i_sequencer_clip> const& aClip, sequencer_track_id aTrack, sequencer_position aStart, sequencer_duration aDuration) final;
            void delete_clip(sequencer_clip_id aClipId) final;
        public:
            bool is_playing() const final;
            sequencer_position position() const final;
        public:
            void play() final;
            void pause() final;
            void rewind() final;
            void seek(sequencer_position aPosition) final;
            void stop() final;
        public:
            optional_sequencer_clip_info current_clip(sequencer_track_id aTrack) const final;
            optional_sequencer_clip_info next_clip(sequencer_track_id aTrack) const final;
        public:
            // add many clips as one edit; strongly preferred to repeated add_clip when
            // populating a timeline, which would copy the affected track once per clip
            clip_ids add_clips(clip_definitions const& aClips);
        public:
            // must be pumped by exactly one thread, once per frame or once per tock
            void update();
            transport_state state() const;
            // prefer this to clip(): the reference returned by clip() is only valid for
            // as long as the caller holds a reference of its own
            sequencer_clip_ptr clip_ptr(sequencer_clip_id aClipId) const;
        protected:
            // seam for the shared tock counter; the default is a steady_clock reading in tocks
            virtual sequencer_position now() const;
        private:
            static sequencer_position position_of(transport const& aTransport, sequencer_position aNow);
            static std::size_t find_track(timeline const& aTimeline, sequencer_track_id aTrack);
            static std::size_t find_clip(timeline const& aTimeline, sequencer_clip_id aClipId);
            static std::size_t find_cursor(track_clips const& aClips, sequencer_position aPosition);
            static track_groups group_clips(clip_definitions const& aDefinitions, clip_ids const& aIds);
            static clip_index locate_clips(clip_definitions const& aDefinitions, clip_ids const& aIds);
            static void insert_clips(timeline& aTimeline, track_groups const& aGroups, clip_index const& aLocators);
            void resync(timeline const& aTimeline, sequencer_position aPosition);
            void update_track(track_clips const& aClips, playback& aPlayback, sequencer_position aPosition);
        private:
            std::atomic<neolib::cookie> iNextTrackCookie = 0;
            std::atomic<neolib::cookie> iNextClipCookie = 0;
            std::atomic<timeline_ptr> iTimeline;
            std::atomic<transport_ptr> iTransport;
        private:
            // update thread only; deliberately unsynchronised
            playback_list iPlayback;
            std::uint64_t iSeenGeneration = 0;
            std::uint64_t iSeenSeekGeneration = 0;
        };
    }
}
