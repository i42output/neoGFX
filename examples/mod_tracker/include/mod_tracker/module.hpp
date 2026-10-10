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
Portions of this file (the module model's play behaviours and instrument properties) are derived
from OpenMPT (https://openmpt.org/).

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
#include <bitset>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace mod_tracker
{
    // One model for every format the tracker plays: ProTracker-family MODs, Scream Tracker 3 (S3M),
    // FastTracker 2 (XM), Impulse Tracker (IT), OpenMPT (MPTM), DigiBooster Pro (DBM) and MO3 (which
    // wraps one of the others with compressed samples). Each loader translates its format into this
    // model: notes, a volume column and an effect column in every cell, samples (as floating point
    // audio) and, for the formats that have them, instruments with envelopes. What each format does
    // differently on playback is recorded too (the format itself, its flags and a set of behaviours),
    // because the same effect often behaves differently from one tracker to the next.

    constexpr std::uint32_t MAX_CHANNELS = 64u;

    struct module_load_error : std::runtime_error
    {
        module_load_error(std::string const& aReason) : std::runtime_error{ "mod_tracker: " + aReason } {}
    };

    enum class module_format : std::uint8_t
    {
        MOD,
        S3M,
        XM,
        IT,
        MPTM,
        DBM
    };

    // notes: 1 is C-0 and 61 (C-5) plays a sample at its middle C frequency; above NOTE_MAX are the
    // special notes
    constexpr std::uint8_t NOTE_NONE = 0u;
    constexpr std::uint8_t NOTE_MIN = 1u;
    constexpr std::uint8_t NOTE_MAX = 128u;
    constexpr std::uint8_t NOTE_MIDDLE_C = 61u;
    constexpr std::uint8_t NOTE_FADE = 253u;
    constexpr std::uint8_t NOTE_CUT = 254u;
    constexpr std::uint8_t NOTE_OFF = 255u;

    inline bool is_note(std::uint8_t aNote)
    {
        return aNote >= NOTE_MIN && aNote <= NOTE_MAX;
    }

    enum class volume_command : std::uint8_t
    {
        None,
        Volume,             // 0 .. 64
        Panning,            // 0 .. 64
        VolumeSlideUp,
        VolumeSlideDown,
        FineVolumeUp,
        FineVolumeDown,
        VibratoSpeed,
        VibratoDepth,
        PanSlideLeft,
        PanSlideRight,
        TonePortamento,
        PortamentoUp,
        PortamentoDown,
        Offset
    };

    enum class effect : std::uint8_t
    {
        None,
        Arpeggio,
        PortamentoUp,
        PortamentoDown,
        TonePortamento,
        Vibrato,
        TonePortamentoVolumeSlide,
        VibratoVolumeSlide,
        Tremolo,
        Panning,                    // 8-bit panning
        Offset,
        VolumeSlide,
        PositionJump,
        Volume,
        PatternBreak,               // the parameter is the row, in decimal
        Retrigger,
        Speed,
        Tempo,
        Tremor,
        ExtendedMod,                // MOD/XM Exy
        ExtendedS3M,                // S3M/IT Sxy
        ChannelVolume,
        ChannelVolumeSlide,
        GlobalVolume,
        GlobalVolumeSlide,
        KeyOff,
        FineVibrato,
        Panbrello,
        ExtraFinePortamento,        // XM X1y / X2y
        PanningSlide,
        SetEnvelopePosition,
        Midi,                       // IT Zxx (filter macros)
        SmoothMidi,
        DelayCut,
        ExtendedParameter,
        Dummy
    };

    struct cell
    {
        std::uint8_t note = NOTE_NONE;
        std::uint8_t instrument = 0u;               // 1-based instrument (or sample) number, 0 for none
        volume_command volumeCommand = volume_command::None;
        std::uint8_t volume = 0u;
        effect command = effect::None;
        std::uint8_t parameter = 0u;

        bool is_tone_portamento() const
        {
            return command == effect::TonePortamento || command == effect::TonePortamentoVolumeSlide || volumeCommand == volume_command::TonePortamento;
        }
    };

    struct pattern
    {
        std::uint32_t rows = 64u;
        std::vector<cell> cells;                    // rows x channels
    };

    enum class loop_type : std::uint8_t
    {
        None,
        Forward,
        PingPong
    };

    enum class vibrato_type : std::uint8_t
    {
        Sine,
        Square,
        RampUp,
        RampDown,
        Random
    };

    struct sample
    {
        std::string name;
        std::vector<float> data;                    // -1.0 .. 1.0, interleaved if stereo
        std::uint32_t length = 0u;                  // in frames
        bool stereo = false;
        loop_type loop = loop_type::None;
        std::uint32_t loopStart = 0u;
        std::uint32_t loopEnd = 0u;
        loop_type sustain = loop_type::None;        // IT sustain loop
        std::uint32_t sustainStart = 0u;
        std::uint32_t sustainEnd = 0u;
        std::uint32_t c5speed = 8363u;              // frequency at middle C (S3M, IT)
        std::int32_t finetune = 0;                  // -128 .. 127 (MOD, XM)
        std::int32_t relativeNote = 0;              // XM transpose
        std::uint16_t volume = 256u;                // 0 .. 256
        std::uint16_t globalVolume = 64u;           // 0 .. 64
        std::optional<std::uint16_t> pan;           // 0 .. 256
        vibrato_type vibratoType = vibrato_type::Sine;
        std::uint8_t vibratoSweep = 0u;
        std::uint8_t vibratoDepth = 0u;
        std::uint8_t vibratoRate = 0u;

        bool has_data() const
        {
            return length != 0u && !data.empty();
        }
        // make the loops consistent with the length
        void sanitise();
    };

    struct envelope_point
    {
        std::uint16_t tick = 0u;
        std::uint8_t value = 0u;                    // 0 .. 64
    };

    struct envelope
    {
        std::vector<envelope_point> points;
        bool enabled = false;
        bool loop = false;
        bool sustain = false;
        bool carry = false;
        bool filter = false;                        // pitch envelope used as a filter envelope
        std::uint8_t loopStart = 0u;
        std::uint8_t loopEnd = 0u;
        std::uint8_t sustainStart = 0u;
        std::uint8_t sustainEnd = 0u;

        // the envelope's value at a tick, scaled to 0 .. aRangeOut
        std::int32_t value_at(std::int32_t aPosition, std::int32_t aRangeOut, std::int32_t aRangeIn = 64) const;
        void sanitise(std::uint8_t aMaximumValue = 64u);
    };

    enum class new_note_action : std::uint8_t
    {
        Cut,
        Continue,
        NoteOff,
        NoteFade
    };

    enum class duplicate_check : std::uint8_t
    {
        None,
        Note,
        Sample,
        Instrument
    };

    enum class duplicate_action : std::uint8_t
    {
        Cut,
        NoteOff,
        NoteFade
    };

    struct instrument
    {
        std::string name;
        std::array<std::uint16_t, NOTE_MAX> keyboard = {};    // the sample (1-based, 0 for none) each note plays
        std::array<std::uint8_t, NOTE_MAX> noteMap = {};      // the note each note plays as
        envelope volumeEnvelope;
        envelope panningEnvelope;
        envelope pitchEnvelope;
        std::uint32_t fadeOut = 0u;
        std::uint16_t globalVolume = 64u;                     // 0 .. 64
        std::optional<std::uint16_t> pan;                     // 0 .. 256
        new_note_action nna = new_note_action::Cut;
        duplicate_check dct = duplicate_check::None;
        duplicate_action dca = duplicate_action::Cut;
        std::int8_t pitchPanSeparation = 0;
        std::uint8_t pitchPanCenter = NOTE_MIDDLE_C - NOTE_MIN;
        std::uint8_t volumeSwing = 0u;                        // 0 .. 100
        std::uint8_t panSwing = 0u;                           // 0 .. 64
        std::optional<std::uint8_t> cutoff;                   // 0 .. 127
        std::optional<std::uint8_t> resonance;                // 0 .. 127
        std::optional<std::uint8_t> filterMode;               // 0 low-pass, 1 high-pass (OpenMPT)
        std::uint16_t volumeRampUp = 0u;                      // OpenMPT: the ramp up for new notes, in 10 microseconds (0 for the default)

        instrument()
        {
            for (std::size_t note = 0u; note < noteMap.size(); ++note)
                noteMap[note] = static_cast<std::uint8_t>(note + NOTE_MIN);
        }
    };

    struct channel_settings
    {
        std::uint16_t pan = 128u;                   // 0 .. 256
        std::uint8_t volume = 64u;                  // 0 .. 64
        bool surround = false;
        bool muted = false;                         // muted in the file
    };

    // The ways in which trackers differ when playing the same effects. Each format sets the behaviours
    // of the tracker it comes from (and some loaders adjust them for the program that saved the file).
    enum class behaviour : std::uint8_t
    {
        CompatiblePlay, PeriodsAreHertz, TempoClamp, PerChannelGlobalVolSlide, PanOverride, RowDelayWithNoteDelay, SlidesAtSpeed1,
        ITInstrWithoutNote, ITVolColFinePortamento, ITArpeggio, ITOutOfRangeDelay, ITPortaMemoryShare, ITPatternLoopTargetReset,
        ITFT2PatternLoop, ITPingPongNoReset, ITEnvelopeReset, ITClearOldNoteAfterCut, ITVibratoTremoloPanbrello, ITTremor,
        ITRetrigger, ITMultiSampleBehaviour, ITPortaTargetReached, ITPatternLoopBreak, ITOffset, ITSwingBehaviour, ITNNAReset,
        ITSCxStopsSample, ITEnvelopePositionHandling, ITPortamentoInstrument, ITPingPongMode, ITRealNoteMapping,
        ITHighOffsetNoRetrig, ITFilterBehaviour, ITNoSurroundPan, ITShortSampleRetrig, ITPortaNoNote, ITFT2DontResetNoteOffOnPorta,
        ITVolColMemory, ITPortamentoSwapResetsPos, ITEmptyNoteMapSlot, ITFirstTickHandling, ITSampleAndHoldPanbrello,
        ITClearPortaTarget, ITPanbrelloHold, ITPanningReset, ITPatternLoopWithJumps, ITPatternLoopWithJumpsOld, ITInstrWithNoteOff,
        ITMultiSampleInstrumentNumber, ITInstrWithNoteOffOldEffects, ITDoNotOverrideChannelPan, ITDCTBehaviour,
        ITPitchPanSeparation, ITResetFilterOnPortaSmpChange, ITInitialNoteMemory, ITNoSustainOnPortamento,
        ITEmptyNoteMapSlotIgnoreCell, ITOffsetWithInstrNumber, ITDoublePortamentoSlides, ITCarryAfterNoteOff, ITNoteCutWithPorta,
        ITVolColNoSlidePropagation, ITStoppedFilterEnvAtStart, ITCompatGxxCarryPortaWithIns,
        FT2Arpeggio, FT2Retrigger, FT2VolColVibrato, FT2PortaNoNote, FT2KeyOff, FT2PanSlide, FT2ST3OffsetOutOfRange,
        FT2RestrictXCommand, FT2RetrigWithNoteDelay, FT2SetPanEnvPos, FT2PortaIgnoreInstr, FT2VolColMemory, FT2LoopE60Restart,
        FT2ProcessSilentChannels, FT2ReloadSampleSettings, FT2PortaDelay, FT2Transpose, FT2PatternLoopWithJumps,
        FT2PortaTargetNoReset, FT2EnvelopeEscape, FT2Tremor, FT2OutOfRangeDelay, FT2Periods, FT2PanWithDelayedNoteOff,
        FT2VolColDelay, FT2FinetunePrecision, FT2NoteOffFlags, FT2MODTremoloRampWaveform, FT2PortaUpDownMemory,
        FT2PanSustainRelease, FT2NoteDelayWithoutInstr, FT2PortaResetDirection, FT2AutoVibratoAbortSweep,
        FT2OffsetMemoryRequiresNote,
        ST3NoMutedChannels, ST3PortaSampleChange, ST3EffectMemory, ST3VibratoMemory, ST3PortaAfterArpeggio,
        ST3OffsetWithoutInstrument, ST3RetrigAfterNoteCut, ST3SampleSwap, ST3LimitPeriod, ApplyUpperPeriodLimit,
        S3MIgnoreCombinedFineSlides,
        MODVBlankTiming, MODOneShotLoops, MODIgnorePanning, MODSampleSwap, MODOutOfRangeNoteDelay, MODTempoOnSecondTick,
        MPTOldSwingBehaviour, ImprecisePingPongLoops, FT2VolumeRamping,
        Count
    };

    class behaviours
    {
    public:
        bool operator[](behaviour aBehaviour) const
        {
            return iBits[static_cast<std::size_t>(aBehaviour)];
        }
        void set(behaviour aBehaviour, bool aValue = true)
        {
            iBits.set(static_cast<std::size_t>(aBehaviour), aValue);
        }
        void reset(behaviour aBehaviour)
        {
            iBits.reset(static_cast<std::size_t>(aBehaviour));
        }
        void reset()
        {
            iBits.reset();
        }
    private:
        std::bitset<static_cast<std::size_t>(behaviour::Count)> iBits;
    };

    // the behaviours of the tracker each format comes from
    behaviours default_behaviours(module_format aFormat);

    enum class tempo_mode : std::uint8_t
    {
        Classic,                // a tick lasts 2.5 / tempo seconds
        Alternative,            // tempo is ticks per second
        Modern                  // tempo is beats per minute
    };

    // how loud samples are mixed, and the pan law (OpenMPT's mix levels, which depend on the program that saved the module)
    enum class mix_levels : std::uint8_t
    {
        Original,               // ModPlug Tracker
        v1_17RC1,
        v1_17RC2,
        v1_17RC3,
        Compatible,             // linear panning
        CompatibleFT2           // FastTracker 2's square root panning
    };

    struct module_data
    {
        module_format format = module_format::MOD;
        std::string formatName;                     // "ProTracker", "Impulse Tracker", ..., for display
        std::string container;                      // "MO3" for a module unpacked from one
        std::string tracker;                        // the program that saved it, as far as can be told
        std::string title;
        std::string signature;                      // MOD: the four-character signature (empty for Soundtracker)
        std::uint32_t channels = 4u;
        std::vector<channel_settings> channelSettings;
        std::vector<sample> samples;                // sample n is samples[n - 1]
        std::vector<instrument> instruments;        // instrument n is instruments[n - 1]; empty in sample mode
        std::vector<std::uint16_t> orders;          // pattern numbers, with ORDER_SKIP and ORDER_END markers
        std::uint16_t restart = 0u;                 // the order position played after the last one
        std::vector<pattern> patterns;
        // initial settings
        std::uint32_t speed = 6u;
        double tempo = 125.0;
        std::uint32_t globalVolume = 256u;          // 0 .. 256
        std::uint32_t samplePreAmp = 48u;           // 0 .. 128
        tempo_mode tempoMode = tempo_mode::Classic;
        std::uint32_t rowsPerBeat = 4u;
        mix_levels mixLevels = mix_levels::Compatible;
        // playback flags
        bool linearSlides = false;
        bool itOldEffects = false;
        bool itCompatibleGxx = false;
        bool extendedFilterRange = false;
        bool fastVolumeSlides = false;              // S3M (ST3.00)
        bool amigaLimits = false;
        bool s3mOldVibrato = false;
        bool protrackerMode = false;
        behaviours playBehaviour;
        std::uint32_t openmptVersion = 0u;          // the version of (Open)ModPlug Tracker that saved it, 0xMMmmrrbb
        std::int32_t minimumPeriod = 1;
        std::int32_t maximumPeriod = 0x7FFFFFFF;
        // IT MIDI macros: the parametered macro for each SFx and the fixed macros for Z80 .. ZFF
        std::array<std::string, 16u> parameteredMacros;
        std::array<std::string, 128u> fixedMacros;

        static constexpr std::uint16_t ORDER_SKIP = 0xFFFEu;   // "+++"
        static constexpr std::uint16_t ORDER_END = 0xFFFFu;    // "---"

        bool instrument_mode() const
        {
            return !instruments.empty();
        }
        bool is_it() const
        {
            return format == module_format::IT || format == module_format::MPTM;
        }
        cell const& at(std::uint32_t aPattern, std::uint32_t aRow, std::uint32_t aChannel) const
        {
            return patterns[aPattern].cells[aRow * channels + aChannel];
        }
        cell& at(std::uint32_t aPattern, std::uint32_t aRow, std::uint32_t aChannel)
        {
            return patterns[aPattern].cells[aRow * channels + aChannel];
        }
        bool valid_pattern(std::uint16_t aPattern) const
        {
            return aPattern < patterns.size();
        }
        void reset_macros();
    };

    // load a module of any supported format, telling which from its contents (and, for a MOD file that
    // has no signature, from it being plausible as a 15-sample Soundtracker module)
    module_data load_module(std::span<std::byte const> aData);
    module_data load_module(std::string const& aPath);

    // the file name patterns of the supported formats, for the file dialog
    std::vector<std::string> const& module_file_patterns();

    // tracker-style text for one cell, in the conventions of the module's own tracker
    std::string note_text(module_data const& aModule, cell const& aCell);
    std::string instrument_text(cell const& aCell);
    std::string volume_text(module_data const& aModule, cell const& aCell);
    std::string effect_text(module_data const& aModule, cell const& aCell);
    bool has_volume_column(module_data const& aModule);
}
