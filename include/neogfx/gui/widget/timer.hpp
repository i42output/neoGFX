// timer.hpp
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

#include <neolib/task/timer.hpp>

#include <neogfx/core/async_task.hpp>

namespace neogfx
{
    class i_widget;

    class widget_timer : public neolib::callback_timer
    {
    public:
        widget_timer(i_widget& aWidget, std::function<void(widget_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait = true);
        template <typename Context>
        widget_timer(i_widget& aWidget, const Context& aContext, std::function<void(widget_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait = true) :
            widget_timer{ aWidget, dynamic_cast<const i_lifetime&>(aContext), aCallback, aDuration_s, aInitialWait } {}
        widget_timer(i_widget& aWidget, const i_lifetime& aContext, std::function<void(widget_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait = true);
    public:
        std::optional<destroyed_flag> iContextDestroyed;
    };
}