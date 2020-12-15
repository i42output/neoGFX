// i_help.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/event.hpp>

namespace neogfx
{
    enum class help_type
    {
        Action
    };

    class i_help_source
    {
    public:
        virtual ~i_help_source() = default;
    public:
        virtual bool help_active() const = 0;
        virtual neogfx::help_type help_type() const = 0;
        virtual i_string const& help_text() const = 0;
    };

    class i_help
    {
    public:
        struct help_not_active : std::logic_error { help_not_active() : std::logic_error("neogfx::i_help::help_not_active") {} };
        struct invalid_help_source : std::logic_error { invalid_help_source() : std::logic_error("neogfx::i_help::invalid_help_source") {} };
    public:
        declare_event(help_activated, const i_help_source&)
        declare_event(help_deactivated, const i_help_source&)
    public:
        virtual ~i_help() = default;
    public:
        virtual bool help_active() const = 0;
        virtual const i_help_source& active_help() const = 0;
    public:
        virtual void activate(const i_help_source& aSource) = 0;
        virtual void deactivate(const i_help_source& aSource) = 0;
    };
}