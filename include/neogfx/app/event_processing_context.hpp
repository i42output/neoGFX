// event_processing_context.hpp
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
#include <neogfx/core/async_task.hpp>

#include <neogfx/app/i_event_processing_context.hpp>

namespace neogfx
{
    class event_processing_context : public i_event_processing_context
    {
    public:
        struct currently_idle : std::logic_error { currently_idle() : std::logic_error("neogfx::event_processing_context::currently_idle") {} };
    public:
        event_processing_context(const std::string& aName = std::string{});
        event_processing_context(i_async_task& aParent, const std::string& aName = std::string{});
    public:
        const std::string& name() const override;
    public:
        bool process_events() override;
    private:
        std::string iName;
    };
}