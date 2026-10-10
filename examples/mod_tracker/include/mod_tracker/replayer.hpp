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

/*
Portions of this file (the replayer's channel and engine state) are derived from OpenMPT
(https://openmpt.org/).

OpenMPT is distributed under the BSD 3-Clause License:

Copyright (c) 2004-2026, OpenMPT Project Developers and Contributors
Copyright (c) 1997-2003, Olivier Lapicque
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the OpenMPT project nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <string>
#include <utility>
#include <vector>

#include <mod_tracker/module.hpp>

namespace mod_tracker
{
    // The replayer works the way trackers always have: a tick engine (speed ticks per row, each tick
    // 2.5 / tempo seconds long) that applies the pattern's notes and effects at tick boundaries and mixes
    // the channels in between, as many frames at a time as the audio device asks for. Every format is
    // played by the same engine: what differs from one tracker to the next is decided by the module's
    // format and behaviours, which follow the trackers (and OpenMPT, where they differ among themselves).
    //
    // A pattern channel plays one note at a time; when a new note starts, the old one can carry on in the
    // background (Impulse Tracker's new note actions) or is faded out quickly to avoid a click. Background
    // voices are channels too, without a pattern channel of their own.
    //
    // Before playback the song is scanned: the engine runs without mixing, which is cheap, to find out
    // where every row falls in time, how long the song is and where it loops back to, and to keep
    // snapshots of the engine along the way so that playback can seek anywhere at once.

    struct scan_cancelled : std::runtime_error { scan_cancelled() : std::runtime_error{ "mod_tracker::scan_cancelled" } {} };

    struct render_options
    {
        std::uint32_t sampleRate = 44100u;
        // 0.0 is mono, 1.0 is the panning as written
        double stereoSeparation = 1.0;
        // a song that never ends or loops is cut off here
        double maximumDuration_s = 30.0 * 60.0;
    };

    struct envelope_state
    {
        std::uint32_t position = 0u;
        bool enabled = false;
        bool filter = false;
    };

    enum channel_flag : std::uint32_t
    {
        CHN_LOOP = 1u << 0u,
        CHN_PINGPONGLOOP = 1u << 1u,
        CHN_SUSTAINLOOP = 1u << 2u,
        CHN_PINGPONGSUSTAIN = 1u << 3u,
        CHN_PINGPONGFLAG = 1u << 4u,        // playing backwards
        CHN_KEYOFF = 1u << 5u,
        CHN_NOTEFADE = 1u << 6u,
        CHN_PORTAMENTO = 1u << 7u,
        CHN_GLISSANDO = 1u << 8u,
        CHN_VIBRATO = 1u << 9u,
        CHN_TREMOLO = 1u << 10u,
        CHN_SURROUND = 1u << 11u,
        CHN_FILTER = 1u << 12u,
        CHN_FASTVOLRAMP = 1u << 13u,
        CHN_MUTE = 1u << 14u,
        CHN_STEREO = 1u << 15u,
        CHN_PANNING = 1u << 16u,            // the sample has a default pan
        // the flags that come from the sample; the rest belong to the channel (surround belongs to both)
        CHN_SAMPLEFLAGS = CHN_LOOP | CHN_PINGPONGLOOP | CHN_SUSTAINLOOP | CHN_PINGPONGSUSTAIN | CHN_STEREO | CHN_PANNING | CHN_PINGPONGFLAG | CHN_SURROUND,
        CHN_CHANNELFLAGS = ~CHN_SAMPLEFLAGS | CHN_SURROUND
    };

    // one channel of the engine: a pattern channel, or a note left playing in the background
    struct channel_state
    {
        // what is playing
        sample const* smp = nullptr;
        instrument const* ins = nullptr;
        double position = 0.0;                  // in frames
        double increment = 0.0;                 // frames per output frame, negative when playing backwards
        std::uint32_t length = 0u;              // where the sample ends (or its loop does); 0 when nothing is playing
        std::uint32_t loopStart = 0u;
        std::uint32_t loopEnd = 0u;
        std::uint32_t flags = 0u;
        std::uint32_t oldFlags = 0u;
        envelope_state volumeEnvelope;
        envelope_state panningEnvelope;
        envelope_state pitchEnvelope;
        std::int32_t fadeOutVolume = 0;         // 0 .. 65536
        // volume and panning
        std::int32_t volume = 0;                // 0 .. 256
        std::int32_t instrumentVolume = 64;     // sample and instrument global volume, 0 .. 64
        std::int32_t channelVolume = 64;        // 0 .. 64
        std::int32_t pan = 128;                 // 0 .. 256
        std::int32_t realPan = 128;
        std::int32_t volumeSwing = 0;
        std::int32_t panSwing = 0;
        std::uint16_t restorePanOnNewNote = 0u;
        std::int32_t realVolume = 0;            // 0 .. 16384, everything applied
        std::int32_t calculatedVolume = 0;
        // pitch
        std::int32_t period = 0;                // Hz, Amiga period or linear period, depending on the module
        std::int32_t portamentoTarget = 0;
        std::uint16_t portamentoSlide = 0u;
        bool portamentoTargetReached = false;
        std::uint32_t c5speed = 8363u;
        std::int32_t finetune = 0;
        std::int32_t transpose = 0;
        std::int32_t cachedPeriod = 0;
        std::int32_t glissandoPeriod = 0;
        // notes and instruments
        std::uint8_t note = NOTE_NONE;
        std::uint8_t newNote = NOTE_NONE;
        std::uint8_t lastNote = NOTE_NONE;
        std::uint8_t newInstrument = 0u;
        std::uint8_t oldInstrument = 0u;
        std::uint16_t swapSample = 0u;
        new_note_action nna = new_note_action::Cut;
        std::uint8_t masterChannel = 0u;        // a background voice: the pattern channel it came from, 1-based
        // effect memory
        std::uint8_t oldPortaUp = 0u;
        std::uint8_t oldPortaDown = 0u;
        std::uint8_t oldFinePortaUpDown = 0u;
        std::uint8_t oldExtraFinePortaUpDown = 0u;
        std::uint8_t oldVolumeSlide = 0u;
        std::uint8_t oldFineVolumeUpDown = 0u;
        std::uint8_t oldVolumeParameter = 0u;
        std::uint8_t oldChannelVolumeSlide = 0u;
        std::uint8_t oldGlobalVolumeSlide = 0u;
        std::uint8_t oldPanSlide = 0u;
        std::uint8_t oldExtendedCommand = 0u;
        std::uint8_t oldTempo = 0u;
        std::uint8_t oldHighOffset = 0u;
        std::uint32_t oldOffset = 0u;
        std::uint32_t previousNoteOffset = 0u;
        std::uint8_t vibratoType = 0u;
        std::uint8_t vibratoSpeed = 0u;
        std::uint8_t vibratoDepth = 0u;
        std::uint8_t vibratoPosition = 0u;
        std::uint8_t tremoloType = 0u;
        std::uint8_t tremoloSpeed = 0u;
        std::uint8_t tremoloDepth = 0u;
        std::uint8_t tremoloPosition = 0u;
        std::uint8_t panbrelloType = 0u;
        std::uint8_t panbrelloSpeed = 0u;
        std::uint8_t panbrelloDepth = 0u;
        std::uint8_t panbrelloPosition = 0u;
        std::int8_t panbrelloOffset = 0;
        std::int8_t panbrelloRandomMemory = 0;
        std::uint8_t retriggerParameter = 0u;
        std::uint8_t retriggerCount = 0u;
        std::uint8_t tremorParameter = 0u;
        std::uint8_t tremorCount = 0u;
        std::uint8_t arpeggio = 0u;
        std::uint8_t arpeggioLastNote = NOTE_NONE;
        effect activeCommand = effect::None;    // arpeggio and tremor carry on from tick to tick
        std::uint32_t patternLoop = 0u;
        std::uint32_t patternLoopCount = 0u;
        std::int32_t autoVibratoDepth = 0;
        std::uint32_t autoVibratoPosition = 0u;
        // resonant filter
        std::uint8_t cutoff = 0x7Fu;
        std::uint8_t resonance = 0u;
        std::uint8_t filterMode = 0u;           // 0 low-pass, 1 high-pass
        std::int16_t cutoffSwing = 0;
        std::int16_t resonanceSwing = 0;
        std::uint16_t restoreCutoffOnNewNote = 0u;
        std::uint16_t restoreResonanceOnNewNote = 0u;
        std::uint8_t activeMacro = 0u;
        float filterA0 = 1.0f;
        float filterB0 = 0.0f;
        float filterB1 = 0.0f;
        float filterHighPass = 0.0f;
        std::array<std::array<float, 2u>, 2u> filterHistory = {};
        // the row
        cell row;
        bool firstTick = false;
        bool triggerNote = false;
        // mixing: the gains now and the gains to ramp to over the tick
        float leftGain = 0.0f;
        float rightGain = 0.0f;
        float newLeftGain = 0.0f;
        float newRightGain = 0.0f;
        float leftRamp = 0.0f;
        float rightRamp = 0.0f;
        std::uint32_t rampFrames = 0u;
        std::uint32_t tickFrames = 0u;
        bool currentSample = false;             // a sample is being mixed
        float peak = 0.0f;

        bool sample_playing() const
        {
            return increment != 0.0;
        }
    };

    // everything the engine needs to carry on from where it is
    struct replayer_state
    {
        std::vector<channel_state> channels;    // the pattern channels, then any background voices
        std::uint32_t orderPosition = 0u;
        std::uint32_t pattern = 0u;
        std::uint32_t row = 0u;
        std::uint32_t nextOrder = 0u;
        std::uint32_t nextRow = 0u;
        std::uint32_t nextPatternStartRow = 0u;
        std::uint32_t speed = 6u;
        double tempo = 125.0;
        std::int32_t globalVolume = 256;
        std::uint32_t tick = 0u;
        std::uint32_t patternDelay = 0u;
        std::uint32_t frameDelay = 0u;
        std::optional<std::uint32_t> breakRow;
        std::optional<std::uint32_t> patternLoopRow;
        std::optional<std::uint32_t> positionJump;
        bool firstTick = false;
        bool breakToRow = false;
        double tickFraction = 0.0;              // fractional frame carried from tick to tick so that rows don't drift
        std::uint64_t tickFramesLeft = 0u;
        bool rowPending = true;                 // the row at orderPosition/row is next and hasn't started
        bool patternLoopJump = false;           // ... and was reached by a pattern loop
        bool ignoreRow = false;                 // ... and is skipped (ProTracker's row delay and pattern break quirk)
        bool ended = false;
        std::optional<std::uint32_t> lastMovedChannel;
        std::uint32_t randomSeed = 0x2F6B3A1Du; // for random waveforms and swing, so that playback is repeatable
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
        // take the tempo and tick timing of another state, keeping everything else as it is
        void restore_timing(replayer_state const& aState);
        bool ended() const;
        void end();
        // frames rendered (or advanced over) by this engine since it was made
        std::uint64_t frames() const;
    public:
        // mix aFrames frames into aOutput (interleaved stereo, added to what is there); with aOutput null the
        // song is advanced without mixing. Pattern channels whose bit is set in aMuted play silently.
        void render(float* aOutput, std::uint64_t aFrames, std::uint64_t aMuted = 0u);
        // the loudest each pattern channel has been since the last call
        float take_peak(std::uint32_t aChannel);
    private:
        // the engine (replayer.cpp)
        void advance_tick();
        bool resolve_row();
        bool start_row();
        std::uint32_t ticks_on_row() const;
        std::uint64_t tick_duration();
        void process_effects();
        bool handle_next_row();
        void instrument_change(std::uint32_t aChannel, std::uint32_t aInstrument, bool aPorta, bool aUpdateVolume, bool aResetEnvelopes = true);
        void note_change(std::uint32_t aChannel, std::uint32_t aNote, bool aPorta, bool aResetEnvelopes, bool aManual = false);
        void apply_instrument_panning(channel_state& aChannel, instrument const* aInstrument, sample const* aSample);
        std::optional<std::uint32_t> background_channel(std::uint32_t aChannel);
        void check_nna(std::uint32_t aChannel, std::uint32_t aInstrument, std::uint32_t aNote, bool aForceCut);
        void key_off(channel_state& aChannel);
        void note_cut(std::uint32_t aChannel, std::uint32_t aTick, bool aCutSample);
        void portamento_up(std::uint32_t aChannel, std::uint8_t aParameter, bool aFineAsRegular);
        void portamento_down(std::uint32_t aChannel, std::uint8_t aParameter, bool aFineAsRegular);
        void fine_portamento_up(channel_state& aChannel, std::uint8_t aParameter);
        void fine_portamento_down(channel_state& aChannel, std::uint8_t aParameter);
        void extra_fine_portamento_up(channel_state& aChannel, std::uint8_t aParameter);
        void extra_fine_portamento_down(channel_state& aChannel, std::uint8_t aParameter);
        bool tone_portamento_shares_memory() const;
        void init_tone_portamento(channel_state& aChannel, std::uint16_t aParameter);
        void tone_portamento(std::uint32_t aChannel, std::uint16_t aParameter);
        std::pair<std::uint16_t, bool> volume_column_tone_portamento(cell const& aCell, std::uint32_t aStartTick) const;
        void vibrato(channel_state& aChannel, std::uint32_t aParameter);
        void fine_vibrato(channel_state& aChannel, std::uint32_t aParameter);
        void panbrello(channel_state& aChannel, std::uint32_t aParameter);
        enum class pan_bits { Four, Six, Eight };
        void panning(channel_state& aChannel, std::uint32_t aParameter, pan_bits aBits);
        void volume_slide(channel_state& aChannel, std::uint8_t aParameter, bool aVolumeColumn = false);
        void panning_slide(channel_state& aChannel, std::uint8_t aParameter, bool aMemory = true);
        void fine_volume_up(channel_state& aChannel, std::uint8_t aParameter, bool aVolumeColumn);
        void fine_volume_down(channel_state& aChannel, std::uint8_t aParameter, bool aVolumeColumn);
        void tremolo(channel_state& aChannel, std::uint32_t aParameter);
        void channel_volume_slide(channel_state& aChannel, std::uint8_t aParameter);
        void extended_mod_commands(std::uint32_t aChannel, std::uint8_t aParameter);
        void extended_s3m_commands(std::uint32_t aChannel, std::uint8_t aParameter);
        void extended_channel_effect(channel_state& aChannel, std::uint32_t aParameter);
        void process_sample_offset(std::uint32_t aChannel);
        void sample_offset(channel_state& aChannel, std::uint32_t aOffset);
        void retrigger_note(std::uint32_t aChannel, std::uint32_t aParameter, std::int32_t aOffset);
        void do_frequency_slide(channel_state& aChannel, std::int32_t& aPeriod, std::int32_t aAmount, bool aTonePortamento = false) const;
        void set_speed(std::uint32_t aParameter);
        void set_tempo(std::uint32_t aParameter);
        void pattern_loop(std::uint32_t aChannel, std::uint8_t aParameter);
        void global_volume_slide(std::uint8_t aParameter, std::uint32_t aChannel);
        std::optional<std::uint32_t> pattern_break(std::uint8_t aParameter) const;
        void update_s3m_effect_memory(channel_state& aChannel, std::uint8_t aParameter) const;
        void process_midi_macro(std::uint32_t aChannel, bool aSmooth, std::string const& aMacro, std::uint8_t aParameter);
        std::uint32_t sample_index(std::uint32_t aNote, std::uint32_t aInstrument) const;
        // pitch (replayer.cpp)
        bool periods_are_frequencies() const;
        bool use_finetune_and_transpose() const;
        std::int32_t period_from_note(std::uint32_t aNote, std::int32_t aFinetune, std::uint32_t aC5Speed) const;
        std::uint32_t note_from_period(std::int32_t aPeriod, std::int32_t aFinetune, std::uint32_t aC5Speed) const;
        static constexpr std::uint32_t FREQUENCY_FRACTION_BITS = 4u;
        std::uint32_t frequency_from_period(std::int32_t aPeriod, std::uint32_t aC5Speed, std::int32_t aPeriodFraction) const;
        // per-tick channel processing and mixing (mixer.cpp)
        void read_note();
        std::int32_t vibrato_delta(std::uint32_t aType, std::uint32_t aPosition);
        void process_tremolo(channel_state& aChannel, std::int32_t& aVolume);
        void process_tremor(channel_state& aChannel, std::int32_t& aVolume);
        bool envelope_processed(channel_state const& aChannel, envelope const& aEnvelope, envelope_state const& aState) const;
        void process_volume_envelope(channel_state& aChannel, std::int32_t& aVolume) const;
        void process_panning_envelope(channel_state& aChannel) const;
        std::int32_t process_pitch_filter_envelope(channel_state& aChannel, std::int32_t& aPeriod) const;
        void increment_envelope_position(channel_state& aChannel, envelope const& aEnvelope, envelope_state& aState, bool aVolume, bool aPanning) const;
        void increment_envelope_positions(channel_state& aChannel) const;
        void process_instrument_fade(channel_state& aChannel, std::int32_t& aVolume) const;
        void process_panbrello(channel_state& aChannel);
        void process_arpeggio(channel_state& aChannel, std::int32_t& aPeriod);
        void process_vibrato(channel_state& aChannel, std::int32_t& aPeriod);
        void process_auto_vibrato(channel_state& aChannel, std::int32_t& aPeriod, std::int32_t& aPeriodFraction);
        std::int32_t setup_channel_filter(channel_state& aChannel, bool aReset, std::int32_t aEnvelopeModifier = 256) const;
        std::int32_t handle_note_change_filter(channel_state& aChannel) const;
        void process_ramping(channel_state& aChannel) const;
        void mix_channel(channel_state& aChannel, float* aOutput, std::uint64_t aFrames, bool aAudible);
        bool fix_position(channel_state& aChannel) const;
        std::uint64_t frames_to_boundary(channel_state const& aChannel) const;
        void stop_channel(channel_state& aChannel) const;
        // -128 .. 127
        std::int32_t random_byte();
    private:
        bool behaves(behaviour aBehaviour) const { return iModule.playBehaviour[aBehaviour]; }
        bool is_mod() const { return iModule.format == module_format::MOD; }
        bool is_s3m() const { return iModule.format == module_format::S3M; }
        bool is_xm() const { return iModule.format == module_format::XM; }
        bool is_it() const { return iModule.is_it(); }
        bool is_dbm() const { return iModule.format == module_format::DBM; }
        std::uint32_t sample_count() const { return static_cast<std::uint32_t>(iModule.samples.size()); }
        std::uint32_t instrument_count() const { return static_cast<std::uint32_t>(iModule.instruments.size()); }
        sample const* sample_slot(std::uint32_t aSample) const;
        instrument const* instrument_slot(std::uint32_t aInstrument) const;
        void reset_envelopes(channel_state& aChannel) const;
        void update_instrument_volume(channel_state& aChannel, sample const* aSample, instrument const* aInstrument) const;
        void set_instrument_pan(channel_state& aChannel, std::int32_t aPan) const;
        void restore_pan_and_filter(channel_state& aChannel) const;
        void process_pitch_pan_separation(std::int32_t& aPan, std::uint32_t aNote, instrument const& aInstrument) const;
        static std::uint32_t sample_flags(sample const& aSample);
    private:
        module_data const& iModule;
        render_options const iOptions;
        float iMasterGain;
        float iSeparation;
        std::uint32_t iRampUpFrames;
        std::uint32_t iRampDownFrames;
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
        std::uint32_t speed;                // ticks per row, once the row's effects have been applied
        double tempo;                       // likewise
    };

    // the song laid out in time, from a scan
    struct timeline
    {
        std::uint32_t sampleRate = 0u;
        std::uint32_t channels = 0u;
        std::uint64_t frameCount = 0u;              // the length of one pass through the song
        std::vector<row_info> rows;                 // in playback order; a row played more than once appears more than once
        std::vector<std::size_t> snapshotRows;      // the rows (indices into rows) that have snapshots, in order
        std::vector<replayer_state> snapshots;      // the engine as each of those rows is about to start
        std::optional<std::size_t> loopRow;         // the row the song jumps back to at its end, if it loops
        std::optional<replayer_state> loopTiming;   // the engine as the loop row was about to start, the first time

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
        // the latest snapshot at or before a row (an index into snapshots)
        std::size_t snapshot_for(std::size_t aRow) const;
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
    // replay routine does; a position that doesn't follow on is a seek, which restores the snapshot before
    // it and advances to the exact frame without mixing. The looping, mute and peak members are atomic so
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
        std::atomic<std::uint64_t> iMuted = 0u;
        std::array<std::atomic<float>, MAX_CHANNELS> iPeaks = {};
    };
}
