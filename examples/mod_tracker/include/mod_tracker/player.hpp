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

#pragma once

#include <mod_tracker/mod_tracker.hpp>

#include <deque>
#include <memory>
#include <optional>
#include <vector>

#include <neogfx/audio/i_audio.hpp>
#include <neogfx/game/i_sequencer.hpp>

#include <mod_tracker/module.hpp>
#include <mod_tracker/replayer.hpp>
#include <mod_tracker/module_bitstream.hpp>

namespace mod_tracker
{
    // Sequencer clip payload for one visit to an order position: the rows of the song's timeline that
    // the clip spans. The sequencer says which of these clips is under the playhead, and how far into
    // it, which is all the pattern display needs to know.
    class pattern_clip
    {
    public:
        pattern_clip(std::size_t aFirstRow, std::size_t aRowCount) :
            iFirstRow{ aFirstRow }, iRowCount{ aRowCount }
        {
        }
    public:
        void advance(ng::game::sequencer_offset)
        {
        }
    public:
        std::size_t first_row() const
        {
            return iFirstRow;
        }
        std::size_t row_count() const
        {
            return iRowCount;
        }
    private:
        std::size_t iFirstRow;
        std::size_t iRowCount;
    };

    // Plays a module in real time through the neoGFX audio framework, under the control of the neoGFX
    // sequencer. The module is a bitstream (module_bitstream) that the audio device pulls from on its own
    // thread, mixing as it goes, as a tracker's replay routine does. The song gets a sequence of its own
    // with two tracks:
    //
    //   audio track    one open-ended ng::audio_playback clip, which starts the device pulling from the
    //                  module when the playhead enters it and restarts it at the right position when the
    //                  playhead jumps (a seek); the module mixes from wherever it is asked to start
    //   pattern track  one pattern_clip per order position visited, laid out from the song's timeline and
    //                  extended a loop at a time while the song loops, which the display reads back to find
    //                  the row under the playhead
    //
    // The sequence's transport is the clock that the display and the start, stop and seek of playback all
    // follow; within playback the audio device's own clock governs the mixing, so the audio is sample
    // accurate whatever the GUI is doing. update() pumps the sequencer and must be called once per frame
    // from the GUI thread, which is the one thread allowed to pump it in this application.
    class player
    {
    public:
        enum class state
        {
            Empty,
            Stopped,
            Paused,
            Playing
        };
    public:
        player();
        ~player();
        player(player const&) = delete;
        player& operator=(player const&) = delete;
    public:
        // the rate songs should be scanned and mixed at: the audio device's
        ng::audio_sample_rate sample_rate() const;
    public:
        bool loaded() const;
        void load(std::shared_ptr<module_data const> aModule, std::shared_ptr<timeline const> aTimeline, render_options const& aOptions);
        void unload();
        module_data const& song_module() const;
        timeline const& song() const;
    public:
        state current_state() const;
        bool looping() const;
        void set_looping(bool aLooping);
        void play();
        void pause();
        void stop();
        // seek to the start of a row of the timeline (an index into song().rows)
        void seek(std::size_t aRow);
        // seek to the next time the song reaches an order position, or failing that the first time it does
        void seek_order(std::int32_t aOrderPosition);
        void seek_relative(std::int32_t aRows);
    public:
        bool muted(std::uint32_t aChannel) const;
        void set_muted(std::uint32_t aChannel, bool aMuted);
        // the loudest a channel has been since the last call, 0.0 .. 1.0
        float take_level(std::uint32_t aChannel);
    public:
        void update();
        // the row under the playhead (an index into song().rows)
        std::size_t current_row() const;
        // the playhead's position within the song, and how many times it has looped
        std::uint64_t current_frame() const;
        std::uint64_t current_pass() const;
    private:
        std::uint64_t position() const;
        ng::game::sequencer_position to_position(std::uint64_t aFrame) const;
        std::uint64_t to_frame(ng::game::sequencer_position aPosition) const;
        void add_pattern_clips(std::size_t aFirstRow, std::uint64_t aStart);
        void remove_loop_clips(std::uint64_t aBefore);
        void stop_audio();
    private:
        ng::i_audio_device& iDevice;
        ng::game::i_sequencer& iSequencer;
        ng::game::sequencer_sequence_id iSequence;
        std::optional<ng::game::sequencer_track_id> iAudioTrack;
        std::optional<ng::game::sequencer_track_id> iPatternTrack;
        ng::game::sequencer_clip_ptr iAudioClip;
        ng::ref_ptr<module_bitstream> iAudio;
        std::shared_ptr<module_data const> iModule;
        std::shared_ptr<timeline const> iTimeline;
        // the pattern clips laid out for each further pass of a looping song; the first pass's clips stay for
        // as long as the song is loaded, as seeks are made to positions in it
        struct loop_pass
        {
            std::uint64_t end;
            std::vector<ng::game::sequencer_clip_id> clips;
        };
        std::deque<loop_pass> iLoopPasses;
        std::uint64_t iClipsEnd = 0u;
        std::uint64_t iLastPosition = 0u;
        bool iPaused = false;
        bool iLooping = true;
    };
}
