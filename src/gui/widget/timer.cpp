// timer.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/window/i_window.hpp>

namespace neogfx
{
    widget_timer::widget_timer(i_widget& aWidget, std::function<void(widget_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait) :
        callback_timer{
            service<i_async_task>(),
            [this, &aWidget, aCallback](callback_timer& aTimer)
            {
                if (iContextDestroyed == std::nullopt && aWidget.has_root())
                    iContextDestroyed = aWidget.root().surface();
                if (iContextDestroyed == std::nullopt || !*iContextDestroyed)
                    aCallback(*this);
            }, aDuration_s, aInitialWait }
    {
    }
    widget_timer::widget_timer(i_widget& aWidget, const i_lifetime& aContext, std::function<void(widget_timer&)> aCallback, const duration_type& aDuration_s, bool aInitialWait) :
        callback_timer{
            service<i_async_task>(),
            aContext,
            [this, &aWidget, aCallback](callback_timer& aTimer)
            {
                aCallback(*this);
            }, aDuration_s, aInitialWait }
    {
    }
}