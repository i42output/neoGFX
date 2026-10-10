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

#include <cstddef>
#include <cstdint>
#include <array>
#include <atomic>
#include <functional>
#include <optional>
#include <stdexcept>
#include <stop_token>
#include <vector>

#include <mod_tracker/module.hpp>

namespace mod_tracker
{
    // The replayer works the way trackers always have: a tick engine (speed ticks per row, each tick
    // 2.5 / tempo seconds long) that applies the pattern's notes and effects at tick boundaries and mixes
    // the channels in between, as many frames at a time as the audio device asks for.
    //
    // Before playback the song is scanned: the engine runs without mixing, which is cheap, to find out
    // where every row falls in time, how long the song is and where it loops back to, and to keep a
    // snapshot of the engine at the start of every row so that playback can seek anywhere at once.

    constexpr std::uint32_t MAX_CHANNELS = 32u;

    struct scan_cancelled : std::runtime_error { scan_cancelled() : std::runtime_error{ "mod_tracker::scan_cancelled" } {} };

    struct render_options
    {
        std::uint32_t sampleRate = 44100u;
        // 0.0 is mono, 1.0 is the hard left/right split of an Amiga
        double stereoSeparation = 0.6;
        // a song that never ends or loops is cut off here
        double maximumDuration_s = 30.0 * 60.0;
    };

    struct channel_state
    {
        // what is playing, and what the next note will play: a sample number without a note changes the
        // volume straight away but the sample only when a note is next triggered
        sample const* instrument = nullptr;
        sample const* selected = nullptr;
        double position = 0.0;
        bool playing = false;
        // what it sounds like
        std::int32_t period = 0;
        std::int32_t volume = 0;
        std::int32_t finetune = 0;
        double pan = 0.5;
        // per-tick output, after arpeggio, vibrato and tremolo
        std::int32_t outputPeriod = 0;
        std::int32_t outputVolume = 0;
        // effect state
        cell current;
        std::int32_t portaTarget = 0;
        std::uint8_t portaSpeed = 0u;
        std::uint8_t vibratoSpeed = 0u;
        std::uint8_t vibratoDepth = 0u;
        std::uint8_t vibratoPosition = 0u;
        std::uint8_t vibratoWaveform = 0u;
        std::uint8_t tremoloSpeed = 0u;
        std::uint8_t tremoloDepth = 0u;
        std::uint8_t tremoloPosition = 0u;
        std::uint8_t tremoloWaveform = 0u;
        std::uint32_t sampleOffset = 0u;
        std::uint32_t loopRow = 0u;
        std::uint32_t loopCount = 0u;
        // mixing
        float gainLeft = 0.0f;
        float gainRight = 0.0f;
    };

    // everything the engine needs to carry on from where it is
    struct replayer_state
    {
        std::vector<channel_state> channels;
        std::uint32_t orderPosition = 0u;
        std::uint32_t row = 0u;
        std::uint8_t speed = 6u;
        std::uint8_t tempo = 125u;
        double tickFraction = 0.0;          // fractional frame carried from tick to tick so that rows don't drift
        std::uint32_t tick = 0u;
        std::uint32_t ticksInRow = 0u;
        std::uint64_t tickFramesLeft = 0u;
        std::uint32_t patternDelay = 0u;
        std::optional<std::uint32_t> positionJump;
        std::optional<std::uint32_t> patternBreak;
        std::optional<std::uint32_t> patternLoopTarget;
        bool rowPending = true;             // the row at orderPosition/row is next and hasn't started
        bool patternLoopJump = false;       // ... and was reached by a pattern loop
        bool songEnd = false;               // F00
        bool ended = false;
    };

    class replayer
    {
    public:
        // called as each row is about to start; returning false ends the song there
        using row_handler = std::function<bool(replayer&)>;
    public:
        replayer(module_data const& aModule, render_options const& aOptions);
    public:
        void set_row_handler(row_handler aHandler);
        replayer_state const& state() const;
        void restore(replayer_state const& aState);
        // take the tempo and tick timing (and song position) of another state, keeping the channels as they are
        void restore_timing(replayer_state const& aState);
        bool ended() const;
        void end();
        // frames rendered (or advanced over) by this engine since it was made
        std::uint64_t frames() const;
    public:
        // mix aFrames frames into aOutput (interleaved stereo, added to what is there); with aOutput null the
        // song is advanced without mixing. Channels whose bit is set in aMuted play silently.
        void render(float* aOutput, std::uint64_t aFrames, std::uint32_t aMuted = 0u);
        // the loudest each channel has been since the last call
        float take_peak(std::uint32_t aChannel);
    private:
        void advance_tick();
        void start_row();
        void run_tick(std::uint32_t aTick);
        void update_output(std::uint32_t aTick, bool aContinuous);
        bool next_row();
        void mix_channel(std::uint32_t aIndex, float* aOutput, std::uint64_t aFrames, bool aMuted);
        std::int32_t target_period(channel_state const& aChannel, cell const& aCell) const;
        void trigger(channel_state& aChannel, std::int32_t aPeriod);
        void retrigger(channel_state& aChannel);
        void tone_portamento(channel_state& aChannel);
        static void volume_slide(channel_state& aChannel, std::uint8_t aUp, std::uint8_t aDown);
    private:
        module_data const& iModule;
        render_options const iOptions;
        std::int32_t const iMinPeriod;
        std::int32_t const iMaxPeriod;
        float const iMasterGain;
        float const iSmoothing;
        replayer_state iState;
        row_handler iRowHandler;
        std::vector<float> iPeaks;
        std::uint64_t iFrames = 0u;
    };

    struct row_info
    {
        std::uint32_t orderPosition;
        std::uint32_t pattern;
        std::uint32_t row;
        std::uint64_t frame;                // first frame of the row
        std::uint8_t speed;                 // ticks per row, once the row's effects have been applied
        std::uint8_t tempo;                 // BPM, likewise
    };

    // the song laid out in time, from a scan
    struct timeline
    {
        std::uint32_t sampleRate = 0u;
        std::uint32_t channels = 0u;
        std::uint64_t frameCount = 0u;              // the length of one pass through the song
        std::vector<row_info> rows;                 // in playback order; a row played more than once appears more than once
        std::vector<replayer_state> snapshots;      // the engine as each row is about to start
        std::optional<std::size_t> loopRow;         // the row the song jumps back to at its end, if it loops

        double duration_s() const
        {
            return sampleRate != 0u ? static_cast<double>(frameCount) / sampleRate : 0.0;
        }
        std::uint64_t loop_length() const
        {
            return loopRow ? frameCount - rows[*loopRow].frame : 0u;
        }
        // the row being played at aFrame, a frame within one pass
        std::size_t row_at(std::uint64_t aFrame) const;
        // Playback runs on through the loop indefinitely, so a playback position (frames since the start
        // of the song) may be some way past the end of one pass; this is the frame within the pass that it
        // corresponds to, or nothing if the song has ended by then.
        std::optional<std::uint64_t> song_frame(std::uint64_t aPosition, bool aLooping) const;
        // the playback position at which the pass that aPosition is in ends
        std::uint64_t pass_end(std::uint64_t aPosition) const;
    };

    // aProgress is written (0.0 .. 1.0) as the scan proceeds; scan_cancelled is thrown if aStop is requested
    timeline scan(module_data const& aModule, render_options const& aOptions, std::stop_token aStop, std::atomic<float>& aProgress);

    // Real-time playback of a scanned song: render() is called from the audio thread with the playback
    // position wanted and mixes from there. Consecutive calls carry on from one another, as a tracker's
    // replay routine does; a position that doesn't follow on is a seek, which restores the snapshot for its
    // row and advances to the exact frame without mixing. The looping, mute and peak members are atomic so
    // that the GUI thread can use them while the audio thread renders.
    class stream
    {
    public:
        stream(module_data const& aModule, timeline const& aTimeline, render_options const& aOptions);
    public:
        void render(std::uint64_t aPosition, float* aOutput, std::uint64_t aFrames);
    public:
        bool looping() const;
        void set_looping(bool aLooping);
        bool muted(std::uint32_t aChannel) const;
        void set_muted(std::uint32_t aChannel, bool aMuted);
        // a position close to where playback already is counts as carrying on (playback being restarted
        // after a pause, say), not as a seek; this makes the next render start from aPosition, whatever
        // position it is asked for, so that a seek lands on exactly the frame intended
        void force_seek(std::uint64_t aPosition);
        // the loudest each channel has been since the last call
        float take_peak(std::uint32_t aChannel);
    private:
        void seek(std::uint64_t aPosition);
        bool row_starting();
    private:
        timeline const& iTimeline;
        replayer iReplayer;
        std::uint64_t const iContinuityTolerance;
        // where the device's playback cursor will be next if it carries on, and where the song will be: the
        // two drift apart by a few milliseconds when playback restarts close to (rather than exactly at)
        // where it left off, and the song position is the one that counts
        std::uint64_t iNextCursor = 0u;
        std::uint64_t iNextPosition = 0u;
        std::size_t iNextRow = 0u;
        std::uint64_t iSeenSeekRequest = 0u;
        std::atomic<std::uint64_t> iSeekRequest = 0u;
        std::atomic<std::uint64_t> iSeekTarget = 0u;
        std::atomic<bool> iLooping = true;
        std::atomic<std::uint32_t> iMuted = 0u;
        std::array<std::atomic<float>, MAX_CHANNELS> iPeaks = {};
    };
}
