// sequencer.hpp
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

#pragma once

#include <neogfx/game/i_sequencer.hpp>

#include <map>
#include <stdexcept>
#include <vector>
#include <mutex>

#include <neolib/core/mutex.hpp>

namespace neogfx
{
    namespace game
    {
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
        private:
            struct clip_entry
            {
                sequencer_clip_ptr clip;
                sequencer_track_id track;
                sequencer_position start;
                sequencer_duration duration;
                bool finalized;

                sequencer_position end() const { return start + duration; }
            };
            struct track_entry
            {
                // clip ids ordered by start position
                std::vector<sequencer_clip_id> clips;
                // index of the first clip not yet passed
                std::size_t cursor = 0;
                optional_sequencer_clip_id active;
            };
            using clip_map = std::map<sequencer_clip_id, clip_entry>;
            using track_map = std::map<sequencer_track_id, track_entry>;
        public:
            sequencer() = default;
            ~sequencer() = default;
        public:
            bool is_multi_threaded() const final;
            bool is_single_threaded() const final;
            void set_multi_threaded() final;
            void set_single_threaded() final;
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
            // must be pumped once per frame (or once per tock) by the owner; see notes
            void update();
            transport_state state() const;
        protected:
            // seam for the shared tock counter; the default is a steady_clock reading in tocks
            virtual sequencer_position now() const;
        private:
            void update_track(track_entry& aTrack, sequencer_position aPosition);
            void resync_track(track_entry& aTrack, sequencer_position aPosition);
            std::size_t find_cursor(track_entry const& aTrack, sequencer_position aPosition) const;
            clip_entry const& entry(sequencer_clip_id aClipId) const;
            clip_entry& entry(sequencer_clip_id aClipId);
            track_entry const& find_track(sequencer_track_id aTrack) const;
            track_entry& find_track(sequencer_track_id aTrack);
        private:
            mutable neolib::multi_mutex<std::recursive_mutex, neolib::null_mutex> iMutex;
            neolib::cookie iNextTrackCookie = 0;
            neolib::cookie iNextClipCookie = 0;
            clip_map iClips;
            track_map iTracks;
            transport_state iState = transport_state::Stopped;
            // position as of the last transport change
            sequencer_position iPosition = 0;
            // clock reading at that transport change
            sequencer_position iAnchor = 0;
        };
    }
}
