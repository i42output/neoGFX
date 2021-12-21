// audio_primitives.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <array>
#include <bit>

#pragma once

namespace neogfx
{
    enum class note : std::uint32_t
    {
        A0  = 1,
        Bb0 = 2,
        B0  = 3,
        C1  = 4,
        Db1 = 5,
        D1  = 6,
        Eb1 = 7,
        E1  = 8,
        F1  = 9,
        Gb1 = 10,
        G1  = 11,
        Ab1 = 12,
        A1  = 13,
        Bb1 = 14,
        B1  = 15,
        C2  = 16,
        Db2 = 17,
        D2  = 18,
        Eb2 = 19,
        E2  = 20,
        F2  = 21,
        Gb2 = 22,
        G2  = 23,
        Ab2 = 24,
        A2  = 25,
        Bb2 = 26,
        B2  = 27,
        C3  = 28,
        Db3 = 29,
        D3  = 30,
        Eb3 = 31,
        E3  = 32,
        F3  = 33,
        Gb3 = 34,
        G3  = 35,
        Ab3 = 36,
        A3  = 37,
        Bb3 = 38,
        B3  = 39,
        C4  = 40,
        Db4 = 41,
        D4  = 42,
        Eb4 = 43,
        E4  = 44,
        F4  = 45,
        Gb4 = 46,
        G4  = 47,
        Ab4 = 48,
        A4  = 49,
        Bb4 = 50,
        B4  = 51,
        C5  = 52,
        Db5 = 53,
        D5  = 54,
        Eb5 = 55,
        E5  = 56,
        F5  = 57,
        Gb5 = 58,
        G5  = 59,
        Ab5 = 60,
        A5  = 61,
        Bb5 = 62,
        B5  = 63,
        C6  = 64,
        Db6 = 65,
        D6  = 66,
        Eb6 = 67,
        E6  = 68,
        F6  = 69,
        Gb6 = 70,
        G6  = 71,
        Ab6 = 72,
        A6  = 73,
        Bb6 = 74,
        B6  = 75,
        C7  = 76,
        Db7 = 77,
        D7  = 78,
        Eb7 = 79,
        E7  = 80,
        F7  = 81,
        Gb7 = 82,
        G7  = 83,
        Ab7 = 84,
        A7  = 85,
        Bb7 = 86,
        B7  = 87,
        C8  = 88
    };

    inline float frequency(neogfx::note note)
    {
        return std::pow(2.0f, (static_cast<float>(note) - 49.0f) / 12.0f) * 440.0f;
    }

    template <neogfx::note Note>
    inline float frequency()
    {
        return frequency(Note);
    }

    enum class instrument : std::uint32_t
    {
        AcousticGrandPiano  = 1,
        BrightAcousticPiano = 2,
        ElectricGrandPiano  = 3,
        HonkytonkPiano      = 4,
        ElectricPiano1      = 5,
        ElectricPiano2      = 6,
        Harpsichord         = 7,
        Clavi               = 8,  
        Celesta             = 9,
        Glockenspiel        = 10,
        MusicBox            = 11,
        Vibraphone          = 12,
        Marimba             = 13,
        Xylophone           = 14,
        TubularBells        = 15,
        Dulcimer            = 16,
        DrawbarOrgan        = 17,
        PercussiveOrgan     = 18,
        RockOrgan           = 19,
        ChurchOrgan         = 20,
        ReedOrgan           = 21,
        Accordion           = 22,
        Harmonica           = 23,
        TangoAccordion      = 24,
        AcousticGuitarNylon = 25,
        AcousticGuitarSteel = 26,
        ElectricGuitarJazz  = 27,
        ElectricGuitarClean = 28,
        ElectricGuitarMuted = 29,
        OverdrivenGuitar    = 30,
        DistortionGuitar    = 31,
        Guitarharmonics     = 32,
        AcousticBass        = 33,
        ElectricBassFinger  = 34,
        ElectricBassPick    = 35,
        FretlessBass        = 36,
        SlapBass1           = 37,
        SlapBass2           = 38,
        SynthBass1          = 39,
        SynthBass2          = 40,
        Violin              = 41,
        Viola               = 42,
        Cello               = 43,
        Contrabass          = 44,
        TremoloStrings      = 45,
        PizzicatoStrings    = 46,
        OrchestralHarp      = 47,
        Timpani             = 48,
        StringEnsemble1     = 49,
        StringEnsemble2     = 50,
        SynthStrings1       = 51,
        SynthStrings2       = 52,
        ChoirAahs           = 53,
        VoiceOohs           = 54,
        SynthVoice          = 55,
        OrchestraHit        = 56,
        Trumpet             = 57,
        Trombone            = 58,
        Tuba                = 59,
        MutedTrumpet        = 60,
        FrenchHorn          = 61,
        BrassSection        = 62,
        SynthBrass1         = 63,
        SynthBrass2         = 64,
        SopranoSax          = 65,
        AltoSax             = 66,
        TenorSax            = 67,
        BaritoneSax         = 68,
        Oboe                = 69,
        EnglishHorn         = 70,
        Bassoon             = 71,
        Clarinet            = 72,
        Piccolo             = 73,
        Flute               = 74,
        Recorder            = 75,
        PanFlute            = 76,
        BlownBottle         = 77,
        Shakuhachi          = 78,
        Whistle             = 79,
        Ocarina             = 80,
        Lead1               = 81,
        Lead2               = 82,
        Lead3               = 83,
        Lead4               = 84,
        Lead5               = 85,
        Lead6               = 86,
        Lead7               = 87,
        Lead8               = 88,
        Pad1                = 89,
        Pad2                = 90,
        Pad3                = 91,
        Pad4                = 92,
        Pad5                = 93,
        Pad6                = 94,
        Pad7                = 95,
        Pad8                = 96,
        FX1                 = 97,
        FX2                 = 98,
        FX3                 = 99,
        FX4                 = 100,
        FX5                 = 101,
        FX6                 = 102,
        FX7                 = 103,
        FX8                 = 104,
        Sitar               = 105,
        Banjo               = 106,
        Shamisen            = 107,
        Koto                = 108,
        Kalimba             = 109,
        Bagpipe             = 110,
        Fiddle              = 111,
        Shanai              = 112,
        TinkleBell          = 113,
        Agogo               = 114,
        SteelDrums          = 115,
        Woodblock           = 116,
        TaikoDrum           = 117,
        MelodicTom          = 118,
        SynthDrum           = 119,
        ReverseCymbal       = 120,
        GuitarFretNoise     = 121,
        BreathNoise         = 122,
        Seashore            = 123,
        BirdTweet           = 124,
        TelephoneRing       = 125,
        Helicopter          = 126,
        Applause            = 127,
        Gunshot             = 128
    };

    enum class percussion_instrument
    {
        AcousticBassDrum    = 35,
        BassDrum1           = 36,
        SideStick           = 37,
        AcousticSnare       = 38,
        HandClap            = 39,
        ElectricSnare       = 40,
        LowFloorTom         = 41,
        ClosedHiHat         = 42,
        HighFloorTom        = 43,
        PedalHiHat          = 44,
        LowTom              = 45,
        OpenHiHat           = 46,
        LowMidTom           = 47,
        HiMidTom            = 48,
        CrashCymbal1        = 49,
        HighTom             = 50,
        RideCymbal1         = 51,
        ChineseCymbal       = 52,
        RideBell            = 53,
        Tambourine          = 54,
        SplashCymbal        = 55,
        Cowbell             = 56,
        CrashCymbal2        = 57,
        Vibraslap           = 58,
        RideCymbal2         = 59,
        HiBongo             = 60,
        LowBongo            = 61,
        MuteHiConga         = 62,
        OpenHiConga         = 63,
        LowConga            = 64,
        HighTimbale         = 65,
        LowTimbale          = 66,
        HighAgogo           = 67,
        LowAgogo            = 68,
        Cabasa              = 69,
        Maracas             = 70,
        ShortWhistle        = 71,
        LongWhistle         = 72,
        ShortGuiro          = 73,
        LongGuiro           = 74,
        Claves              = 75,
        HiWoodBlock         = 76,
        LowWoodBlock        = 77,
        MuteCuica           = 78,
        OpenCuica           = 79,
        MuteTriangle        = 80,
        OpenTriangle        = 81
    };

    inline std::string to_string(neogfx::instrument instrument)
    {
        static std::unordered_map<neogfx::instrument, std::string> const sMap = 
        {
            { neogfx::instrument::AcousticGrandPiano,   "Acoustic Grand Piano" },
            { neogfx::instrument::BrightAcousticPiano,  "Bright Acoustic Piano" },
            { neogfx::instrument::ElectricGrandPiano,   "Electric Grand Piano" },
            { neogfx::instrument::HonkytonkPiano,       "Honky - tonk Piano" },
            { neogfx::instrument::ElectricPiano1,       "Electric Piano 1" },
            { neogfx::instrument::ElectricPiano2,       "Electric Piano 2" },
            { neogfx::instrument::Harpsichord,          "Harpsichord" },
            { neogfx::instrument::Clavi,                "Clavi" },
            { neogfx::instrument::Celesta,              "Celesta" },
            { neogfx::instrument::Glockenspiel,         "Glockenspiel" },
            { neogfx::instrument::MusicBox,             "Music Box" },
            { neogfx::instrument::Vibraphone,           "Vibraphone" },
            { neogfx::instrument::Marimba,              "Marimba" },
            { neogfx::instrument::Xylophone,            "Xylophone" },
            { neogfx::instrument::TubularBells,         "Tubular Bells" },
            { neogfx::instrument::Dulcimer,             "Dulcimer" },
            { neogfx::instrument::DrawbarOrgan,         "Drawbar Organ" },
            { neogfx::instrument::PercussiveOrgan,      "Percussive Organ" },
            { neogfx::instrument::RockOrgan,            "Rock Organ" },
            { neogfx::instrument::ChurchOrgan,          "Church Organ" },
            { neogfx::instrument::ReedOrgan,            "Reed Organ" },
            { neogfx::instrument::Accordion,            "Accordion" },
            { neogfx::instrument::Harmonica,            "Harmonica" },
            { neogfx::instrument::TangoAccordion,       "Tango Accordion" },
            { neogfx::instrument::AcousticGuitarNylon,       "Acoustic Guitar(nylon)" },
            { neogfx::instrument::AcousticGuitarSteel,       "Acoustic Guitar(steel)" },
            { neogfx::instrument::ElectricGuitarJazz,       "Electric Guitar(jazz)" },
            { neogfx::instrument::ElectricGuitarClean,       "Electric Guitar(clean)" },
            { neogfx::instrument::ElectricGuitarMuted,       "Electric Guitar(muted)" },
            { neogfx::instrument::OverdrivenGuitar,     "Overdriven Guitar" },
            { neogfx::instrument::DistortionGuitar,     "Distortion Guitar" },
            { neogfx::instrument::Guitarharmonics,      "Guitar harmonics" },
            { neogfx::instrument::AcousticBass,         "Acoustic Bass" },
            { neogfx::instrument::ElectricBassFinger,   "Electric Bass(finger)" },
            { neogfx::instrument::ElectricBassPick,     "Electric Bass(pick)" },
            { neogfx::instrument::FretlessBass,         "Fretless Bass" },
            { neogfx::instrument::SlapBass1,            "Slap Bass 1" },
            { neogfx::instrument::SlapBass2,            "Slap Bass 2" },
            { neogfx::instrument::SynthBass1,           "Synth Bass 1" },
            { neogfx::instrument::SynthBass2,           "Synth Bass 2" },
            { neogfx::instrument::Violin,               "Violin" },
            { neogfx::instrument::Viola,                "Viola" },
            { neogfx::instrument::Cello,                "Cello" },
            { neogfx::instrument::Contrabass,           "Contrabass" },
            { neogfx::instrument::TremoloStrings,       "Tremolo Strings" },
            { neogfx::instrument::PizzicatoStrings,     "Pizzicato Strings" },
            { neogfx::instrument::OrchestralHarp,       "Orchestral Harp" },
            { neogfx::instrument::Timpani,              "Timpani" },
            { neogfx::instrument::StringEnsemble1,      "String Ensemble 1" },
            { neogfx::instrument::StringEnsemble2,      "String Ensemble 2" },
            { neogfx::instrument::SynthStrings1,        "SynthStrings 1" },
            { neogfx::instrument::SynthStrings2,        "SynthStrings 2" },
            { neogfx::instrument::ChoirAahs,            "Choir Aahs" },
            { neogfx::instrument::VoiceOohs,            "Voice Oohs" },
            { neogfx::instrument::SynthVoice,           "Synth Voice" },
            { neogfx::instrument::OrchestraHit,         "Orchestra Hit" },
            { neogfx::instrument::Trumpet,              "Trumpet" },
            { neogfx::instrument::Trombone,             "Trombone" },
            { neogfx::instrument::Tuba,                 "Tuba" },
            { neogfx::instrument::MutedTrumpet,         "Muted Trumpet" },
            { neogfx::instrument::FrenchHorn,           "French Horn" },
            { neogfx::instrument::BrassSection,         "Brass Section" },
            { neogfx::instrument::SynthBrass1,          "SynthBrass 1" },
            { neogfx::instrument::SynthBrass2,          "SynthBrass 2" },
            { neogfx::instrument::SopranoSax,           "Soprano Sax" },
            { neogfx::instrument::AltoSax,              "Alto Sax" },
            { neogfx::instrument::TenorSax,             "Tenor Sax" },
            { neogfx::instrument::BaritoneSax,          "Baritone Sax" },
            { neogfx::instrument::Oboe,                 "Oboe" },
            { neogfx::instrument::EnglishHorn,          "English Horn" },
            { neogfx::instrument::Bassoon,              "Bassoon" },
            { neogfx::instrument::Clarinet,             "Clarinet" },
            { neogfx::instrument::Piccolo,              "Piccolo" },
            { neogfx::instrument::Flute,                "Flute" },
            { neogfx::instrument::Recorder,             "Recorder" },
            { neogfx::instrument::PanFlute,             "Pan Flute" },
            { neogfx::instrument::BlownBottle,          "Blown Bottle" },
            { neogfx::instrument::Shakuhachi,           "Shakuhachi" },
            { neogfx::instrument::Whistle,              "Whistle" },
            { neogfx::instrument::Ocarina,              "Ocarina" },
            { neogfx::instrument::Lead1,                "Lead 1 (square)" },
            { neogfx::instrument::Lead2,                "Lead 2 (sawtooth)" },
            { neogfx::instrument::Lead3,                "Lead 3 (calliope)" },
            { neogfx::instrument::Lead4,                "Lead 4 (chiff)" },
            { neogfx::instrument::Lead5,                "Lead 5 (charang)" },
            { neogfx::instrument::Lead6,                "Lead 6 (voice)" },
            { neogfx::instrument::Lead7,                "Lead 7 (fifths)" },
            { neogfx::instrument::Lead8,                "Lead 8 (bass + lead)" },
            { neogfx::instrument::Pad1,                 "Pad 1 (new age)" },
            { neogfx::instrument::Pad2,                 "Pad 2 (warm)" },
            { neogfx::instrument::Pad3,                 "Pad 3 (polysynth)" },
            { neogfx::instrument::Pad4,                 "Pad 4 (choir)" },
            { neogfx::instrument::Pad5,                 "Pad 5 (bowed)" },
            { neogfx::instrument::Pad6,                 "Pad 6 (metallic)" },
            { neogfx::instrument::Pad7,                 "Pad 7 (halo)" },
            { neogfx::instrument::Pad8,                 "Pad 8 (sweep)" },
            { neogfx::instrument::FX1,                  "FX 1 (rain)" },
            { neogfx::instrument::FX2,                  "FX 2 (soundtrack)" },
            { neogfx::instrument::FX3,                  "FX 3 (crystal)" },
            { neogfx::instrument::FX4,                  "FX 4 (atmosphere)" },
            { neogfx::instrument::FX5,                  "FX 5 (brightness)" },
            { neogfx::instrument::FX6,                  "FX 6 (goblins)" },
            { neogfx::instrument::FX7,                  "FX 7 (echoes)" },
            { neogfx::instrument::FX8,                  "FX 8 (sci - fi)" },
            { neogfx::instrument::Sitar,                "Sitar" },
            { neogfx::instrument::Banjo,                "Banjo" },
            { neogfx::instrument::Shamisen,             "Shamisen" },
            { neogfx::instrument::Koto,                 "Koto" },
            { neogfx::instrument::Kalimba,              "Kalimba" },
            { neogfx::instrument::Bagpipe,              "Bag pipe" },
            { neogfx::instrument::Fiddle,               "Fiddle" },
            { neogfx::instrument::Shanai,               "Shanai" },
            { neogfx::instrument::TinkleBell,           "Tinkle Bell" },
            { neogfx::instrument::Agogo,                "Agogo" },
            { neogfx::instrument::SteelDrums,           "Steel Drums" },
            { neogfx::instrument::Woodblock,            "Woodblock" },
            { neogfx::instrument::TaikoDrum,            "Taiko Drum" },
            { neogfx::instrument::MelodicTom,           "Melodic Tom" },
            { neogfx::instrument::SynthDrum,            "Synth Drum" },
            { neogfx::instrument::ReverseCymbal,        "Reverse Cymbal" },
            { neogfx::instrument::GuitarFretNoise,      "Guitar Fret Noise" },
            { neogfx::instrument::BreathNoise,          "Breath Noise" },
            { neogfx::instrument::Seashore,             "Seashore" },
            { neogfx::instrument::BirdTweet,            "Bird Tweet" },
            { neogfx::instrument::TelephoneRing,        "Telephone Ring" },
            { neogfx::instrument::Helicopter,           "Helicopter" },
            { neogfx::instrument::Applause,             "Applause" },
            { neogfx::instrument::Gunshot,              "Gunshot" }
        };

        return sMap.at(instrument);
    }  
     
    inline std::string to_string(neogfx::percussion_instrument instrument)
    {
        static std::unordered_map<percussion_instrument, std::string> const sMap =
        {
            { neogfx::percussion_instrument::AcousticBassDrum   , "Acoustic Bass Drum" },
            { neogfx::percussion_instrument::BassDrum1          , "Bass Drum 1" },
            { neogfx::percussion_instrument::SideStick          , "Side Stick" },
            { neogfx::percussion_instrument::AcousticSnare      , "Acoustic Snare" },
            { neogfx::percussion_instrument::HandClap           , "Hand Clap" },
            { neogfx::percussion_instrument::ElectricSnare      , "Electric Snare" },
            { neogfx::percussion_instrument::LowFloorTom        , "Low Floor Tom" },
            { neogfx::percussion_instrument::ClosedHiHat        , "Closed Hi Hat" },
            { neogfx::percussion_instrument::HighFloorTom       , "High Floor Tom" },
            { neogfx::percussion_instrument::PedalHiHat         , "Pedal Hi - Hat" },
            { neogfx::percussion_instrument::LowTom             , "Low Tom" },
            { neogfx::percussion_instrument::OpenHiHat          , "Open Hi - Hat" },
            { neogfx::percussion_instrument::LowMidTom          , "Low - Mid Tom" },
            { neogfx::percussion_instrument::HiMidTom           , "Hi - Mid Tom" },
            { neogfx::percussion_instrument::CrashCymbal1       , "Crash Cymbal 1" },
            { neogfx::percussion_instrument::HighTom            , "High Tom" },
            { neogfx::percussion_instrument::RideCymbal1        , "Ride Cymbal 1" },
            { neogfx::percussion_instrument::ChineseCymbal      , "Chinese Cymbal" },
            { neogfx::percussion_instrument::RideBell           , "Ride Bell" },
            { neogfx::percussion_instrument::Tambourine         , "Tambourine" },
            { neogfx::percussion_instrument::SplashCymbal       , "Splash Cymbal" },
            { neogfx::percussion_instrument::Cowbell            , "Cowbell" },
            { neogfx::percussion_instrument::CrashCymbal2       , "Crash Cymbal 2" },
            { neogfx::percussion_instrument::Vibraslap          , "Vibraslap" },
            { neogfx::percussion_instrument::RideCymbal2        , "Ride Cymbal 2" },
            { neogfx::percussion_instrument::HiBongo            , "Hi Bongo" },
            { neogfx::percussion_instrument::LowBongo           , "Low Bongo" },
            { neogfx::percussion_instrument::MuteHiConga        , "Mute Hi Conga" },
            { neogfx::percussion_instrument::OpenHiConga        , "Open Hi Conga" },
            { neogfx::percussion_instrument::LowConga           , "Low Conga" },
            { neogfx::percussion_instrument::HighTimbale        , "High Timbale" },
            { neogfx::percussion_instrument::LowTimbale         , "Low Timbale" },
            { neogfx::percussion_instrument::HighAgogo          , "High Agogo" },
            { neogfx::percussion_instrument::LowAgogo           , "Low Agogo" },
            { neogfx::percussion_instrument::Cabasa             , "Cabasa" },
            { neogfx::percussion_instrument::Maracas            , "Maracas" },
            { neogfx::percussion_instrument::ShortWhistle       , "Short Whistle" },
            { neogfx::percussion_instrument::LongWhistle        , "Long Whistle" },
            { neogfx::percussion_instrument::ShortGuiro         , "Short Guiro" },
            { neogfx::percussion_instrument::LongGuiro          , "Long Guiro" },
            { neogfx::percussion_instrument::Claves             , "Claves" },
            { neogfx::percussion_instrument::HiWoodBlock        , "Hi Wood Block" },
            { neogfx::percussion_instrument::LowWoodBlock       , "Low Wood Block" },
            { neogfx::percussion_instrument::MuteCuica          , "Mute Cuica" },
            { neogfx::percussion_instrument::OpenCuica          , "Open Cuica" },
            { neogfx::percussion_instrument::MuteTriangle       , "Mute Triangle" },
            { neogfx::percussion_instrument::OpenTriangle       , "Open Triangle" }
        };

        return sMap.at(instrument);
    }

    enum class audio_channel : std::uint64_t
    {
        None                               = 0x0000000000000000ULL,
        Mono                               = 0x0000000000000001ULL,
        FrontLeft                          = 0x0000000000000002ULL,
        FrontRight                         = 0x0000000000000004ULL,
        FrontCenter                        = 0x0000000000000008ULL,
        Lfe                                = 0x0000000000000010ULL,
        BackLeft                           = 0x0000000000000020ULL,
        BackRight                          = 0x0000000000000040ULL,
        FrontLeftCenter                    = 0x0000000000000080ULL,
        FrontRightCenter                   = 0x0000000000000100ULL,
        BackCenter                         = 0x0000000000000200ULL,
        SideLeft                           = 0x0000000000000400ULL,
        SideRight                          = 0x0000000000000800ULL,
        TopCenter                          = 0x0000000000001000ULL,
        TopFrontLeft                       = 0x0000000000002000ULL,
        TopFrontCenter                     = 0x0000000000004000ULL,
        TopFrontRight                      = 0x0000000000008000ULL,
        TopBackLeft                        = 0x0000000000010000ULL,
        TopBackCenter                      = 0x0000000000020000ULL,
        TopBackRight                       = 0x0000000000040000ULL,
        Aux0                               = 0x0000000000080000ULL,
        Aux1                               = 0x0000000000100000ULL,
        Aux2                               = 0x0000000000200000ULL,
        Aux3                               = 0x0000000000400000ULL,
        Aux4                               = 0x0000000000800000ULL,
        Aux5                               = 0x0000000001000000ULL,
        Aux6                               = 0x0000000002000000ULL,
        Aux7                               = 0x0000000004000000ULL,
        Aux8                               = 0x0000000008000000ULL,
        Aux9                               = 0x0000000010000000ULL,
        Aux10                              = 0x0000000020000000ULL,
        Aux11                              = 0x0000000040000000ULL,
        Aux12                              = 0x0000000080000000ULL,
        Aux13                              = 0x0000000100000000ULL,
        Aux14                              = 0x0000000200000000ULL,
        Aux15                              = 0x0000000400000000ULL,
        Aux16                              = 0x0000000800000000ULL,
        Aux17                              = 0x0000001000000000ULL,
        Aux18                              = 0x0000002000000000ULL,
        Aux19                              = 0x0000004000000000ULL,
        Aux20                              = 0x0000008000000000ULL,
        Aux21                              = 0x0000010000000000ULL,
        Aux22                              = 0x0000020000000000ULL,
        Aux23                              = 0x0000040000000000ULL,
        Aux24                              = 0x0000080000000000ULL,
        Aux25                              = 0x0000100000000000ULL,
        Aux26                              = 0x0000200000000000ULL,
        Aux27                              = 0x0000400000000000ULL,
        Aux28                              = 0x0000800000000000ULL,
        Aux29                              = 0x0001000000000000ULL,
        Aux30                              = 0x0002000000000000ULL,
        Aux31                              = 0x0004000000000000ULL,
                                           
        Left                               = FrontLeft,
        Right                              = FrontRight
    };

    inline audio_channel operator&(audio_channel lhs, audio_channel rhs)
    {
        return static_cast<audio_channel>(static_cast<std::uint64_t>(lhs) & static_cast<std::uint64_t>(rhs));
    }

    inline audio_channel operator|(audio_channel lhs, audio_channel rhs)
    {
        return static_cast<audio_channel>(static_cast<std::uint64_t>(lhs) | static_cast<std::uint64_t>(rhs));
    }

    inline audio_channel operator^(audio_channel lhs, audio_channel rhs)
    {
        return static_cast<audio_channel>(static_cast<std::uint64_t>(lhs) ^ static_cast<std::uint64_t>(rhs));
    }

    inline audio_channel operator~(audio_channel lhs)
    {
        return static_cast<audio_channel>(~static_cast<std::uint64_t>(lhs));
    }

    inline std::uint64_t channel_count(audio_channel channels)
    {
        return static_cast<std::uint64_t>(std::popcount(static_cast<std::uint64_t>(channels)));
    }

    enum class audio_stream_format : std::uint32_t
    {
        Unknown = 0,

        Pcm     = 1
    };

    enum class audio_sample_format : std::uint32_t
    {
        Unknown = 0,

        U8      = 1,
        S16     = 2,
        S24     = 3,
        S32     = 4,
        F32     = 5
    };

    typedef std::uint64_t audio_sample_rate;

	enum class audio_standard_sample_rate : std::uint64_t
	{
        Rate48000   = 48000,
        Rate44100   = 44100,

        Rate32000   = 32000,
        Rate24000   = 24000,
        Rate22050   = 22050,

        Rate88200   = 88200,
        Rate96000   = 96000,
        Rate176400  = 176400,
        Rate192000  = 192000,

        Rate16000   = 16000,
        Rate11025   = 11250,
        Rate8000    = 8000,

        Rate352800  = 352800,
        Rate384000  = 384000,
	};

    struct audio_data_format
    {
        typedef audio_data_format abstract_type;

        audio_sample_format sampleFormat;
        std::uint32_t channels;
        audio_sample_rate sampleRate;
    };

    typedef std::uint64_t audio_sample_index;
    typedef std::uint64_t audio_sample_count;

    typedef std::uint64_t audio_frame_index;
    typedef std::uint64_t audio_frame_count;

    template <typename SampleType, std::size_t Channels>
    using audio_frame = std::array<SampleType, Channels>;

    struct adsr_envelope
    {
        float attack;
        float decay;
        float sustain;
        float release;
    };
}