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
            if (!green_mode_active() && iLastActivityTime && std::chrono::steady_clock::now() - *iLastActivityTime >= activity_timeout())
            {
                iLastActivityTime = {};
                iGreenModeActive = true;
                GreenModeEntered.trigger();
            }
        }, 1000u },
        iActivityTimeout{ 5 },
        iGreenModeEnabled{ true },
        iGreenModeActive{ false }
    {
    }

    void power::register_activity()
    {
        iLastActivityTime = std::chrono::steady_clock::now();
        ActivityRegistered.trigger();
        if (green_mode_active())
        {
            iGreenModeActive = false;
            GreenModeLeft.trigger();
        }
    }

    std::chrono::seconds power::activity_timeout() const
    {
        return iActivityTimeout;
    }

    void power::set_activity_timeout(std::chrono::seconds aTimeout)
    {
        iActivityTimeout = aTimeout;
    }

    bool power::green_mode_active() const
    {
        return iGreenModeActive;
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
            TurboModeLeft.trigger();
        }
    }

    void power::disable_green_mode()
    {
        if (iGreenModeEnabled)
        {
            iGreenModeEnabled = false;
            GreenModeDisabled.trigger();
            TurboModeEnabled.trigger();
            if (green_mode_active())
            {
                iGreenModeActive = false;
                GreenModeLeft.trigger();
                TurboModeEntered.trigger();
            }
        }
    }
}