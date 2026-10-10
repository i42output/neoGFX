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

#include <complex>
#include <string>
#include <vector>

#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gfx/text/font.hpp>

#include <mod_tracker/player.hpp>

namespace mod_tracker
{
    // A real-time spectrum analyser for one channel of the output (0 = left, 1 = right): the latest window
    // of what is being heard is analysed once per frame and shown as bars on a logarithmic frequency scale,
    // in decibels, with peak hold.
    class spectrum_view : public ng::widget<>
    {
    public:
        spectrum_view(ng::i_layout& aLayout, player& aPlayer, std::uint32_t aChannel, std::string const& aTitle);
    public:
        // called once per frame, after the player has been updated
        void refresh();
    protected:
        void paint(ng::i_graphics_context& aGc) const override;
    private:
        void analyse();
        double x_of(double aFrequency, double aLeft, double aWidth) const;
    private:
        player& iPlayer;
        std::uint32_t const iChannel;
        std::string const iTitle;
        ng::font iFont;
        double const iSampleRate;
        double const iTopFrequency;
        // the analysis: the window function, the FFT's bit reversal and twiddle factors, and its buffers
        std::vector<float> iWindow;
        std::vector<std::uint32_t> iBitReverse;
        std::vector<std::complex<float>> iTwiddles;
        std::vector<float> iSamples;
        std::vector<std::complex<float>> iSpectrum;
        std::vector<float> iMagnitudes;
        // per band: the level just measured, the level shown (0.0 .. 1.0) and the peak held, with how many
        // more frames it is held for
        std::vector<float> iMeasured;
        std::vector<float> iLevels;
        std::vector<float> iPeaks;
        std::vector<std::uint32_t> iPeakHold;
    };
}
