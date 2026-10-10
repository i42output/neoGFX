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

#include <filesystem>
#include <format>

#include <neogfx/app/file_dialog.hpp>
#include <neogfx/gui/window/i_window.hpp>

#include <mod_tracker/tracker.hpp>

namespace mod_tracker
{
    namespace
    {
        std::string to_time(double aSeconds)
        {
            auto const seconds = static_cast<std::uint32_t>(aSeconds);
            return std::format("{:02}:{:02}", seconds / 60u, seconds % 60u);
        }
    }

    tracker::tracker(ng::i_layout& aLayout) :
        ng::widget<>{ aLayout },
        iLayout{ *this },
        iToolbar{ iLayout },
        iOpen{ iToolbar, "Open..." },
        iPlay{ iToolbar, "Play" },
        iPause{ iToolbar, "Pause" },
        iStop{ iToolbar, "Stop" },
        iPrevious{ iToolbar, "<< Previous" },
        iNext{ iToolbar, "Next >>" },
        iLoop{ iToolbar, "Loop" },
        iToolbarSpacer{ iToolbar },
        iStatus{ iToolbar },
        iViews{ iLayout },
        iPatternView{ iViews, iPlayer },
        iSampleView{ iViews, iPlayer },
        iSpectra{ iLayout },
        iLeftSpectrum{ iSpectra, iPlayer, 0u, "Left" },
        iRightSpectrum{ iSpectra, iPlayer, 1u, "Right" },
        iInfoBar{ iLayout },
        iPosition{ iInfoBar },
        iInfoSpacer{ iInfoBar },
        iTime{ iInfoBar },
        // the sequencer is pumped from here: one thread (the GUI thread), once per frame
        iFrameTimer{ *this, [this](ng::widget_timer& aTimer) { aTimer.again(); frame(); }, std::chrono::milliseconds{ 16 } }
    {
        set_size_policy(ng::size_constraint::Expanding);
        iLayout.set_padding(ng::padding{ 4.0 });
        iLayout.set_spacing(ng::size{ 4.0 });
        iToolbar.set_spacing(ng::size{ 4.0 });
        iViews.set_spacing(ng::size{ 2.0 });
        iSpectra.set_spacing(ng::size{ 2.0 });

        iLoop.set_checked(iPlayer.looping());

        iOpen.Clicked([this]() { open(); });
        iPlay.Clicked([this]() { iPlayer.play(); iPatternView.set_focus(); });
        iPause.Clicked([this]() { iPlayer.pause(); iPatternView.set_focus(); });
        iStop.Clicked([this]() { iPlayer.stop(); iPatternView.set_focus(); });
        iPrevious.Clicked([this]()
            {
                if (iPlayer.loaded())
                    iPlayer.seek_order(static_cast<std::int32_t>(iPlayer.song().rows[iPlayer.current_row()].orderPosition) - 1);
            });
        iNext.Clicked([this]()
            {
                if (iPlayer.loaded())
                    iPlayer.seek_order(static_cast<std::int32_t>(iPlayer.song().rows[iPlayer.current_row()].orderPosition) + 1);
            });
        iLoop.Checked([this]() { iPlayer.set_looping(true); });
        iLoop.Unchecked([this]() { iPlayer.set_looping(false); });

        update_controls();
    }

    tracker::~tracker()
    {
        // stop the frame timer before the player it pumps is destroyed; a load still running is
        // abandoned, as in open(), rather than waited for
        iFrameTimer.cancel();
        if (iLoadThread.joinable())
        {
            iLoadThread.request_stop();
            iLoadThread.detach();
        }
    }

    void tracker::open()
    {
        auto const paths = ng::open_file_dialog(*this, ng::file_dialog_spec{ "Open Module", {}, module_file_patterns(), "Modules (MOD, S3M, XM, IT, MPTM, DBM, MO3)" });
        if (paths && !paths->empty())
            open(paths->front());
    }

    void tracker::open(std::string const& aPath)
    {
        // a load already in progress is abandoned rather than waited for: it stops at the next row of its
        // own accord, and owns everything it touches, so it can be left to finish in the background
        if (iLoadThread.joinable())
        {
            iLoadThread.request_stop();
            iLoadThread.detach();
        }
        auto job = std::make_shared<load_job>();
        job->path = aPath;
        job->options.sampleRate = static_cast<std::uint32_t>(iPlayer.sample_rate());
        iJob = job;
        iStatusText.clear();
        iLoadThread = std::jthread{ [job](std::stop_token aStop)
            {
                try
                {
                    auto loaded = std::make_shared<module_data const>(mod_tracker::load_module(job->path));
                    job->scanned = std::make_shared<timeline const>(mod_tracker::scan(*loaded, job->options, aStop, job->progress));
                    job->loaded = std::move(loaded);
                }
                catch (scan_cancelled const&)
                {
                }
                catch (...)
                {
                    job->error = std::current_exception();
                }
                job->done.store(true, std::memory_order_release);
            } };
        update_controls();
    }

    void tracker::frame()
    {
        collect_load();
        iPlayer.update();
        iPatternView.refresh();
        iSampleView.refresh();
        iLeftSpectrum.refresh();
        iRightSpectrum.refresh();
        update_controls();
    }

    void tracker::collect_load()
    {
        if (iJob == nullptr || !iJob->done.load(std::memory_order_acquire))
            return;
        auto const job = std::move(iJob);
        iJob = nullptr;
        // the job has finished, so this doesn't wait (beyond the thread's last few instructions)
        if (iLoadThread.joinable())
            iLoadThread.join();
        if (job->error)
        {
            try
            {
                std::rethrow_exception(job->error);
            }
            catch (std::exception const& e)
            {
                iStatusText = e.what();
            }
            catch (...)
            {
                iStatusText = "Unknown error loading module";
            }
            return;
        }
        if (job->loaded == nullptr || job->scanned == nullptr)
            return;
        iPlayer.load(job->loaded, job->scanned, job->options);
        iStatusText = std::filesystem::path{ job->path }.filename().string();
        auto const& title = iPlayer.song_module().title;
        root().set_title_text(ng::string{ "neoGFX MOD Tracker - " + (title.empty() ? iStatusText : title) });
        iPatternView.set_focus();
        iPlayer.play();
    }

    void tracker::update_controls()
    {
        auto const state = iPlayer.current_state();
        auto const enable = [](ng::i_widget& aWidget, bool aEnable)
            {
                if (aWidget.enabled() != aEnable)
                    aWidget.enable(aEnable);
            };
        enable(iPlay, state == player::state::Stopped || state == player::state::Paused);
        enable(iPause, state == player::state::Playing);
        enable(iStop, state == player::state::Playing || state == player::state::Paused);
        enable(iPrevious, state != player::state::Empty);
        enable(iNext, state != player::state::Empty);

        if (iJob != nullptr)
            set_text(iStatus, std::format("Loading {}... {:.0f}%",
                std::filesystem::path{ iJob->path }.filename().string(), iJob->progress.load() * 100.0f));
        else
            set_text(iStatus, iStatusText);

        if (state == player::state::Empty)
        {
            set_text(iPosition, "");
            set_text(iTime, "");
            return;
        }
        auto const& song = iPlayer.song();
        auto const& row = song.rows[iPlayer.current_row()];
        set_text(iPosition, std::format("Position {:02}/{:02}   Pattern {:02}   Row {:02}   Speed {}   Tempo {:g}",
            row.orderPosition, iPlayer.song_module().orders.size() - 1u, row.pattern, row.row, row.speed, row.tempo));
        auto const pass = iPlayer.current_pass();
        set_text(iTime, to_time(static_cast<double>(iPlayer.current_frame()) / song.sampleRate) + " / " + to_time(song.duration_s()) +
            (pass != 0u ? std::format("  (loop {})", pass) : song.loopRow ? std::string{ "  (loops)" } : std::string{}));
    }

    void tracker::set_text(ng::label& aLabel, std::string const& aText)
    {
        // only touch the label when the text changes: setting it relays out the toolbar
        if (aLabel.text().to_std_string() != aText)
            aLabel.set_text(ng::string{ aText });
    }
}
