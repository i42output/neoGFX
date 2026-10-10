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

#include <mod_tracker/mod_tracker.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

#include <neogfx/gfx/i_graphics_context.hpp>

#include <mod_tracker/spectrum_view.hpp>

namespace mod_tracker
{
    namespace
    {
        ng::color const BACKGROUND{ 14, 17, 22 };
        ng::color const HEADER_BACKGROUND{ 28, 32, 42 };
        ng::color const HEADER_TEXT{ 150, 158, 175 };
        ng::color const GRID{ 30, 35, 46 };
        ng::color const SCALE_TEXT{ 90, 98, 115 };
        ng::color const BAR_LOW{ 40, 110, 200 };
        ng::color const BAR_MID{ 80, 200, 160 };
        ng::color const BAR_HIGH{ 255, 196, 92 };
        ng::color const PEAK{ 220, 225, 235 };

        // 4096 frames is 93 ms at 44.1 kHz, which resolves 10.8 Hz: enough to separate the bass notes
        constexpr std::uint32_t FFT_SIZE = 4096u;
        constexpr std::uint32_t BAND_COUNT = 48u;
        constexpr double BOTTOM_FREQUENCY = 20.0;
        constexpr double TOP_FREQUENCY = 20000.0;
        // the level scale, in dBFS (a full scale sine wave reads 0 dB); like most analysers the display is
        // tilted up 3 dB per octave about 1 kHz, so that music, whose energy falls with frequency, reads flat
        constexpr double FLOOR_DB = -72.0;
        constexpr double TILT_DB_PER_OCTAVE = 3.0;
        constexpr double TILT_PIVOT = 1000.0;
        // a bar jumps up to what is measured and falls back at this much of the scale per frame; a peak is
        // held for this many frames and then falls
        constexpr float LEVEL_FALL = 0.025f;
        constexpr std::uint32_t PEAK_HOLD_FRAMES = 40u;
        constexpr float PEAK_FALL = 0.01f;

        ng::color bar_color(double aLevel)
        {
            return aLevel < 0.6 ? ng::mix(BAR_LOW, BAR_MID, aLevel / 0.6) : ng::mix(BAR_MID, BAR_HIGH, (aLevel - 0.6) / 0.4);
        }
    }

    spectrum_view::spectrum_view(ng::i_layout& aLayout, player& aPlayer, std::uint32_t aChannel, std::string const& aTitle) :
        ng::widget<>{ aLayout },
        iPlayer{ aPlayer },
        iChannel{ aChannel },
        iTitle{ aTitle },
        iFont{ "Consolas", ng::font_style::Normal, 9.0 },
        iSampleRate{ static_cast<double>(aPlayer.sample_rate()) },
        iTopFrequency{ std::min(TOP_FREQUENCY, iSampleRate / 2.0) },
        iWindow(FFT_SIZE),
        iBitReverse(FFT_SIZE),
        iTwiddles(FFT_SIZE / 2u),
        iSamples(FFT_SIZE),
        iSpectrum(FFT_SIZE),
        iMagnitudes(FFT_SIZE / 2u + 1u),
        iMeasured(BAND_COUNT),
        iLevels(BAND_COUNT),
        iPeaks(BAND_COUNT),
        iPeakHold(BAND_COUNT)
    {
        set_size_policy(ng::size_policy{ ng::size_constraint::Expanding, ng::size_constraint::Minimum });
        set_minimum_size(ng::size{ 320.0, 150.0 });

        // a Hann window, scaled so that a full scale sine wave measures 1.0
        double sum = 0.0;
        for (std::uint32_t index = 0u; index < FFT_SIZE; ++index)
            sum += 0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * index / FFT_SIZE);
        for (std::uint32_t index = 0u; index < FFT_SIZE; ++index)
            iWindow[index] = static_cast<float>((1.0 - std::cos(2.0 * std::numbers::pi * index / FFT_SIZE)) / sum);
        std::uint32_t bits = 0u;
        while ((1u << bits) < FFT_SIZE)
            ++bits;
        for (std::uint32_t index = 0u; index < FFT_SIZE; ++index)
        {
            std::uint32_t reversed = 0u;
            for (std::uint32_t bit = 0u; bit < bits; ++bit)
                if ((index & (1u << bit)) != 0u)
                    reversed |= 1u << (bits - 1u - bit);
            iBitReverse[index] = reversed;
        }
        for (std::uint32_t index = 0u; index < FFT_SIZE / 2u; ++index)
            iTwiddles[index] = std::polar(1.0f, static_cast<float>(-2.0 * std::numbers::pi * index / FFT_SIZE));
    }

    void spectrum_view::refresh()
    {
        if (iPlayer.output_samples(iChannel, iSamples.data(), FFT_SIZE))
            analyse();
        else
            std::fill(iMeasured.begin(), iMeasured.end(), 0.0f);
        bool changed = false;
        for (std::uint32_t band = 0u; band < BAND_COUNT; ++band)
        {
            auto const level = std::max(iMeasured[band], iLevels[band] - LEVEL_FALL);
            auto const shown = level < 0.002f ? 0.0f : level;
            if (shown != iLevels[band])
            {
                iLevels[band] = shown;
                changed = true;
            }
            auto peak = iPeaks[band];
            if (shown >= peak)
            {
                peak = shown;
                iPeakHold[band] = PEAK_HOLD_FRAMES;
            }
            else if (iPeakHold[band] > 0u)
                --iPeakHold[band];
            else
                peak = std::max(shown, peak - PEAK_FALL);
            if (peak != iPeaks[band])
            {
                iPeaks[band] = peak;
                changed = true;
            }
        }
        if (changed)
            update();
    }

    void spectrum_view::analyse()
    {
        // an in-place radix-2 FFT of the windowed samples
        for (std::uint32_t index = 0u; index < FFT_SIZE; ++index)
            iSpectrum[iBitReverse[index]] = std::complex<float>{ iSamples[index] * iWindow[index], 0.0f };
        for (std::uint32_t length = 2u; length <= FFT_SIZE; length <<= 1u)
        {
            auto const half = length / 2u;
            auto const step = FFT_SIZE / length;
            for (std::uint32_t first = 0u; first < FFT_SIZE; first += length)
                for (std::uint32_t index = 0u; index < half; ++index)
                {
                    auto const u = iSpectrum[first + index];
                    auto const v = iSpectrum[first + index + half] * iTwiddles[index * step];
                    iSpectrum[first + index] = u + v;
                    iSpectrum[first + index + half] = u - v;
                }
        }
        for (std::uint32_t bin = 0u; bin <= FFT_SIZE / 2u; ++bin)
            iMagnitudes[bin] = std::abs(iSpectrum[bin]);

        // each band takes the strongest bin within it; a band narrower than a bin (in the bass) reads the
        // spectrum at its centre instead
        auto const binWidth = iSampleRate / FFT_SIZE;
        auto const ratio = iTopFrequency / BOTTOM_FREQUENCY;
        for (std::uint32_t band = 0u; band < BAND_COUNT; ++band)
        {
            auto const low = BOTTOM_FREQUENCY * std::pow(ratio, static_cast<double>(band) / BAND_COUNT);
            auto const high = BOTTOM_FREQUENCY * std::pow(ratio, static_cast<double>(band + 1u) / BAND_COUNT);
            auto const centre = std::sqrt(low * high);
            auto const first = static_cast<std::uint32_t>(std::ceil(low / binWidth));
            auto const last = std::min(static_cast<std::uint32_t>(std::ceil(high / binWidth)), FFT_SIZE / 2u + 1u);
            double magnitude = 0.0;
            if (first < last)
                for (auto bin = first; bin < last; ++bin)
                    magnitude = std::max(magnitude, static_cast<double>(iMagnitudes[bin]));
            else
            {
                auto const position = centre / binWidth;
                auto const below = std::min(static_cast<std::uint32_t>(position), FFT_SIZE / 2u - 1u);
                auto const fraction = position - below;
                magnitude = iMagnitudes[below] * (1.0 - fraction) + iMagnitudes[below + 1u] * fraction;
            }
            auto const db = 20.0 * std::log10(std::max(magnitude, 1e-9)) + TILT_DB_PER_OCTAVE * std::log2(centre / TILT_PIVOT);
            iMeasured[band] = static_cast<float>(std::clamp((db - FLOOR_DB) / -FLOOR_DB, 0.0, 1.0));
        }
    }

    double spectrum_view::x_of(double aFrequency, double aLeft, double aWidth) const
    {
        return aLeft + aWidth * std::log(aFrequency / BOTTOM_FREQUENCY) / std::log(iTopFrequency / BOTTOM_FREQUENCY);
    }

    void spectrum_view::paint(ng::i_graphics_context& aGc) const
    {
        auto const clientRect = client_rect();
        aGc.fill_rect(clientRect, BACKGROUND);

        auto const charWidth = aGc.text_extent(ng::string{ "0" }, iFont).cx;
        auto const lineHeight = std::ceil(iFont.height() * 1.25);
        auto const textOffset = std::floor((lineHeight - iFont.height()) / 2.0);
        auto draw = [&](ng::point const& aPosition, std::string const& aText, ng::color const& aColor)
            {
                aGc.draw_text(aPosition + ng::point{ 0.0, textOffset }, ng::string{ aText }, iFont, ng::text_format{ aColor });
            };

        ng::rect const headerRect{ clientRect.top_left(), ng::size{ clientRect.width(), lineHeight } };
        aGc.fill_rect(headerRect, HEADER_BACKGROUND);
        draw(headerRect.top_left() + ng::point{ charWidth, 0.0 }, iTitle, HEADER_TEXT);

        // the plot, with the dB scale to its left and the frequency scale below it
        auto const left = std::floor(clientRect.x + charWidth * 4.5);
        auto const top = std::floor(headerRect.bottom() + lineHeight / 2.0);
        auto const right = std::floor(clientRect.x + clientRect.width() - charWidth);
        auto const bottom = std::floor(clientRect.bottom() - lineHeight);
        if (right - left < BAND_COUNT || bottom - top < 8.0)
            return;
        ng::rect const plot{ ng::point{ left, top }, ng::size{ right - left, bottom - top } };

        for (double db = 0.0; db >= FLOOR_DB; db -= 12.0)
        {
            auto const y = std::floor(plot.y + plot.height() * db / FLOOR_DB);
            aGc.fill_rect(ng::rect{ ng::point{ plot.x, y }, ng::size{ plot.width(), 1.0 } }, GRID);
            auto const label = std::to_string(static_cast<int>(db));
            draw(ng::point{ plot.x - charWidth * (label.size() + 0.5), y - lineHeight / 2.0 }, label, SCALE_TEXT);
        }
        struct mark { double frequency; char const* label; };
        for (auto const& m : { mark{ 50.0, "50" }, mark{ 100.0, "100" }, mark{ 200.0, "200" }, mark{ 500.0, "500" },
            mark{ 1000.0, "1k" }, mark{ 2000.0, "2k" }, mark{ 5000.0, "5k" }, mark{ 10000.0, "10k" }, mark{ 20000.0, "20k" } })
        {
            if (m.frequency > iTopFrequency)
                break;
            auto const x = std::floor(x_of(m.frequency, plot.x, plot.width()));
            aGc.fill_rect(ng::rect{ ng::point{ x, plot.y }, ng::size{ 1.0, plot.height() } }, GRID);
            std::string const label{ m.label };
            auto const labelX = std::min(x - charWidth * label.size() / 2.0, plot.x + plot.width() - charWidth * label.size());
            draw(ng::point{ labelX, plot.bottom() }, label, SCALE_TEXT);
        }

        // the bars, and their peaks
        auto const bandWidth = plot.width() / BAND_COUNT;
        auto const gap = std::max(1.0, std::floor(bandWidth * 0.2));
        for (std::uint32_t band = 0u; band < BAND_COUNT; ++band)
        {
            auto const x = std::floor(plot.x + band * bandWidth);
            auto const width = std::floor(plot.x + (band + 1u) * bandWidth) - x - gap;
            auto const level = static_cast<double>(iLevels[band]);
            if (level > 0.0)
            {
                auto const height = std::round(plot.height() * level);
                aGc.fill_rect(ng::rect{ ng::point{ x, plot.bottom() - height }, ng::size{ width, height } }, bar_color(level));
            }
            auto const peak = static_cast<double>(iPeaks[band]);
            if (peak > 0.0)
            {
                auto const y = std::round(plot.bottom() - plot.height() * peak);
                aGc.fill_rect(ng::rect{ ng::point{ x, std::min(y, plot.bottom() - 2.0) }, ng::size{ width, 2.0 } }, PEAK);
            }
        }
    }
}
