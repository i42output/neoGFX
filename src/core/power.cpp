// power.cpp
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
#include <neogfx/core/power.hpp>

namespace neogfx
{
    power::power() :
        iUpdater{ service<async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            if (is_green_mode_enabled() && std::chrono::steady_clock::now() - iLastActivityTime >= activity_timeout())
                set_active_mode(power_mode::Green);
        }, 1000u },
        iActiveMode{ power_mode::Normal },
        iActivityTimeout{ 5 },
        iGreenModeEnabled{ true },
        iLastActivityTime{ std::chrono::steady_clock::now() }
    {
    }

    power_mode power::active_mode() const
    {
        return iActiveMode;
    }

    void power::register_activity()
    {
        iLastActivityTime = std::chrono::steady_clock::now();
        ActivityRegistered.trigger();
        if (green_mode_active())
            set_active_mode(power_mode::Normal);
    }

    std::chrono::seconds power::activity_timeout() const
    {
        return iActivityTimeout;
    }

    void power::set_activity_timeout(std::chrono::seconds aTimeout)
    {
        iActivityTimeout = aTimeout;
    }

    bool power::is_green_mode_enabled() const
    {
        return iGreenModeEnabled;
    }

    void power::enable_green_mode()
    {
        if (!iGreenModeEnabled)
        {
            iGreenModeEnabled = true;
            GreenModeEnabled.trigger();
            TurboModeDisabled.trigger();
            set_active_mode(power_mode::Normal);
        }
    }

    void power::disable_green_mode()
    {
        if (iGreenModeEnabled)
        {
            iGreenModeEnabled = false;
            GreenModeDisabled.trigger();
            TurboModeEnabled.trigger();
            set_active_mode(power_mode::Turbo);
        }
    }

    void power::set_active_mode(power_mode aMode)
    {
        if (iActiveMode != aMode)
        {
            auto const previousMode = iActiveMode;
            iActiveMode = aMode;
            switch (active_mode())
            {
            case power_mode::Green:
                GreenModeEntered.trigger();
                break;
            case power_mode::Normal:
                break;
            case power_mode::Turbo:
                TurboModeEntered.trigger();
                break;
            }
            switch (previousMode)
            {
            case power_mode::Green:
                GreenModeLeft.trigger();
                break;
            case power_mode::Normal:
                break;
            case power_mode::Turbo:
                TurboModeLeft.trigger();
                break;
            }
        }
    }
}