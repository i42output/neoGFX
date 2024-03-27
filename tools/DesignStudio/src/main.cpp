// main.cpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

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

#include <chrono>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/audio/audio_waveform.hpp>
#include <neogfx/audio/audio_instrument.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include "app.hpp"

using namespace std::chrono_literals;
using namespace neogfx::string_literals;

int main(int argc, char* argv[])
{
    neolib::application_info appInfo
    {
        argc, argv,
        "neoGFX Design Studio",
        "i42 Software",
        neolib::version{ 1, 0, 0, 0, "pre-release" },
        "Copyright (c) 2020 Leigh Johnston",
        {}, {}, {}, ".nel"
    };

    std::cout << "------ " << appInfo.name() << " ------" << std::endl;
    std::cout << appInfo.copyright() << std::endl << std::endl;

    ds::app app{ appInfo };

    ng::service<ng::debug::logger>().set_filter_severity(neolib::logger::severity::Error);

#if 0
    auto& playbackDevice = ng::service<ng::i_audio>().create_playback_device(
        ng::audio_data_format{ ng::audio_sample_format::F32, 2u, 48000u });
    playbackDevice.start();

    float const amplitude = 0.25f;

    ng::audio_waveform waveform{ playbackDevice, amplitude };
    float const componentAmplitude = 0.1f;
    waveform.create_oscillator(ng::frequency<ng::note::C4>(), componentAmplitude);
    waveform.create_oscillator(ng::frequency<ng::note::C5>(), componentAmplitude);
    waveform.create_oscillator(ng::frequency<ng::note::C8>(), componentAmplitude);

    ng::audio_instrument churchOrgan{ playbackDevice, ng::instrument::ChurchOrgan, amplitude };
    churchOrgan.set_envelope(ng::adsr_envelope{ 0.1f, 0.0f, 1.0f, 0.1f });
    churchOrgan.play_note(5s, ng::note::G4, 6s);
    churchOrgan.play_note(6s, ng::note::A4, 5s);
    churchOrgan.play_note(7s, ng::note::F4, 4s);
    churchOrgan.play_note(8s, ng::note::F3, 3s);
    churchOrgan.play_note(9s, ng::note::C4, 2s);

    playbackDevice.play(waveform, 15s);
    playbackDevice.play(churchOrgan, 15s);
#endif

    try
    {
        return app.exec();
    }
    catch (std::exception& e)
    {
        app.thread().halt();
        ng::service<ng::debug::logger>() << "neoGFX Design Studio: terminating with exception: " << e.what() << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        app.thread().halt();
        ng::service<ng::debug::logger>() << "neoGFX Design Studio: terminating with unknown exception" << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
        std::exit(EXIT_FAILURE);
    }
}

