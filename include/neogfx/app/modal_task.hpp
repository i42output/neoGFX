// modal_task.hpp
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
#include <neogfx/app/event_processing_context.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/window/i_window.hpp>

namespace neogfx
{
    template <typename T, typename... TaskArgs>
    inline T modal_task(i_widget& aParent, std::string const& aTaskName, TaskArgs&&... aTaskArgs)
    {
        aParent.root().modal_enable(false);
        auto result = std::async(std::forward<TaskArgs>(aTaskArgs)...);
        event_processing_context epc{ aTaskName };
        while (result.wait_for(std::chrono::seconds{ 0 }) != std::future_status::ready)
            epc.process_events();
        aParent.root().modal_enable(true);
        return result.get();
    }
}