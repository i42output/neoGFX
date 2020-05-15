// i_power.hpp
/*
  neogfx C++ App/Game Engine
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
#include <neogfx/core/i_event.hpp>

namespace neogfx
{
    enum class power_mode
    {
        Green,
        Normal,
        Turbo
    };

    class i_power
    {
    public:
        declare_event(activity_registered)
        declare_event(green_mode_enabled)
        declare_event(green_mode_disabled)
        declare_event(green_mode_entered)
        declare_event(green_mode_left)
        declare_event(turbo_mode_enabled)
        declare_event(turbo_mode_disabled)
        declare_event(turbo_mode_entered)
        declare_event(turbo_mode_left)
    public:
        virtual ~i_power() = default;
    public:
        virtual power_mode active_mode() const = 0;
    public:
        virtual void register_activity() = 0;
        virtual std::chrono::seconds activity_timeout() const = 0;
        virtual void set_activity_timeout(std::chrono::seconds aTimeout) = 0;
    public:
        virtual bool is_green_mode_enabled() const = 0;
        virtual void enable_green_mode() = 0;
        virtual void disable_green_mode() = 0;
    public:
        bool green_mode_active() const
        {
            return active_mode() == power_mode::Green;
        }
        bool turbo_mode_active() const
        {
            return active_mode() == power_mode::Turbo;
        }
        bool is_turbo_mode_enabled() const
        {
            return !is_green_mode_enabled();
        }
        void enable_turbo_mode()
        {
            disable_green_mode();
        }
        void disable_turbo_mode()
        {
            enable_green_mode();
        }
    };
}