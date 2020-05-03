// power.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <chrono>
#include <neolib/timer.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/i_power.hpp>

namespace neogfx
{
    class power : public i_power
    {
    public:
        define_declared_event(ActivityRegistered, activity_registered)
        define_declared_event(GreenModeEnabled, green_mode_enabled)
        define_declared_event(GreenModeDisabled, green_mode_disabled)
        define_declared_event(GreenModeEntered, green_mode_entered)
        define_declared_event(GreenModeLeft, green_mode_left)
        define_declared_event(TurboModeEnabled, turbo_mode_enabled)
        define_declared_event(TurboModeDisabled, turbo_mode_disabled)
        define_declared_event(TurboModeEntered, turbo_mode_entered)
        define_declared_event(TurboModeLeft, turbo_mode_left)
    public:
        power();
    public:
        void register_activity() override;
        std::chrono::seconds activity_timeout() const override;
        void set_activity_timeout(std::chrono::seconds aTimeout) override;
        bool green_mode_active() const override;
        bool is_green_mode_enabled() const override;
        void enable_green_mode() override;
        void disable_green_mode() override;
    private:
        neolib::callback_timer iUpdater;
        std::chrono::seconds iActivityTimeout;
        bool iGreenModeEnabled;
        bool iGreenModeActive;
        std::optional<std::chrono::steady_clock::time_point> iLastActivityTime;
    };
}