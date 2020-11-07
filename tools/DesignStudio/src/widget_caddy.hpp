// widget_caddy.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/gui/widget/widget.hpp>

namespace neogfx::DesignStudio
{
    class widget_caddy : public widget
    {
    public:
        widget_caddy(i_widget& aParent) :
            widget{ aParent }
        {
            iSink = ChildAdded([&](i_widget& aChild)
            {
                aChild.set_ignore_mouse_events(true);
                aChild.set_ignore_non_client_mouse_events(true);
            });
        }
    private:
        sink iSink;
    };
}