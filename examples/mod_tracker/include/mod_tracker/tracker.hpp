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

#include <atomic>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/widget/check_box.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>

#include <mod_tracker/module.hpp>
#include <mod_tracker/replayer.hpp>
#include <mod_tracker/player.hpp>
#include <mod_tracker/pattern_view.hpp>
#include <mod_tracker/sample_view.hpp>

namespace mod_tracker
{
    class tracker : public ng::widget<>
    {
    private:
        // loading and scanning a module reads a file and runs through the whole song, so it happens on a
        // thread of its own; the GUI thread polls for the result once per frame and never waits for it
        struct load_job
        {
            std::string path;
            render_options options;
            std::atomic<float> progress = 0.0f;
            std::atomic<bool> done = false;
            std::shared_ptr<module_data const> loaded;
            std::shared_ptr<timeline const> scanned;
            std::exception_ptr error;
        };
    public:
        tracker(ng::i_layout& aLayout);
        ~tracker();
    public:
        void open();
        void open(std::string const& aPath);
    private:
        void frame();
        void collect_load();
        void update_controls();
        static void set_text(ng::label& aLabel, std::string const& aText);
    private:
        player iPlayer;
        ng::vertical_layout iLayout;
        ng::horizontal_layout iToolbar;
        ng::push_button iOpen;
        ng::push_button iPlay;
        ng::push_button iPause;
        ng::push_button iStop;
        ng::push_button iPrevious;
        ng::push_button iNext;
        ng::check_box iLoop;
        ng::horizontal_spacer iToolbarSpacer;
        ng::label iStatus;
        ng::horizontal_layout iViews;
        pattern_view iPatternView;
        sample_view iSampleView;
        ng::horizontal_layout iInfoBar;
        ng::label iPosition;
        ng::horizontal_spacer iInfoSpacer;
        ng::label iTime;
        std::string iStatusText;
        std::shared_ptr<load_job> iJob;
        std::jthread iLoadThread;
        ng::widget_timer iFrameTimer;
    };
}
