// script_widget.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2021 Leigh Johnston
  
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
#include <boost/format.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/terminal.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>

namespace neogfx::DesignStudio
{
    class console_widget : public window
    {
        typedef window base_type;
    public:
        console_widget(i_element& aElement, i_widget& aParent, window_style aStyle) :
            window{ aParent, window_placement{ rect{ point{}, size{ 480.0_dip, 480.0_dip } } }, aStyle | window_style::SizeToContents },
            iTerminal{ client_layout() }
        {
            title_bar().set_icon(aElement.library().element_icon(aElement.type()));
            title_bar().set_title(""_s);
            //create_status_bar<neogfx::status_bar>(
            //    neogfx::status_bar::style::DisplayMessage | neogfx::status_bar::style::DisplaySizeGrip | neogfx::status_bar::style::BackgroundAsWindowBorder );
        }
        ~console_widget()
        {
        }
    private:
        terminal iTerminal;
        sink iSink;
    };

    template <>
    struct element_traits<console_widget> : element_traits<i_element>
    {
        static constexpr bool needsCaddy = false;
    };
}
