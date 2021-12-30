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

#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/audio/audio_waveform.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include "app.hpp"

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

    auto& playbackDevice = ng::service<ng::i_audio>().create_playback_device(
        ng::audio_data_format{ ng::audio_sample_format::F32, 2u, 48000u });
    playbackDevice.start();

    float const amplitude = 0.25f;
    ng::audio_waveform waveform{ playbackDevice, amplitude };
    float const componentAmplitude = 0.333f;
    waveform.create_oscillator(ng::frequency<ng::note::C4>(), componentAmplitude);
    waveform.create_oscillator(ng::frequency<ng::note::C3>(), componentAmplitude); // 1 octave below middle C
    waveform.create_oscillator(ng::frequency<ng::note::C5>(), componentAmplitude); // 1 octave above middle C

    playbackDevice.play(waveform, std::chrono::seconds(10));

    try
    {
        return app.exec();
    }
    catch (std::exception& e)
    {
        app.halt();
        ng::service<ng::debug::logger>() << "neoGFX Design Studio: terminating with exception: " << e.what() << ng::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        app.halt();
        ng::service<ng::debug::logger>() << "neoGFX Design Studio: terminating with unknown exception" << ng::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
        std::exit(EXIT_FAILURE);
    }
}

