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
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/widget.hpp>

namespace neogfx::DesignStudio
{
    class widget_caddy : public widget
    {
    public:
        enum class border_style
        {
            None,
            Drag,
            Edit
        };
    public:
        widget_caddy(i_widget& aParent, const point& aPosition) :
            widget{ aParent },
            iLayout{ *this },
            iBorderStyle{ border_style::None },
            iAnimator{ service<i_async_task>(), [this](neolib::callback_timer& aAnimator) 
            {    
                aAnimator.again();
                if (iBorderStyle != border_style::None)
                    update(); 
            }, 20 }
        {
            move(aPosition);
            iSink = ChildAdded([&](i_widget& aChild)
            {
                iLayout.add(aChild);
                aChild.set_ignore_mouse_events(true);
                aChild.set_ignore_non_client_mouse_events(true);
            });
        }
    public:
        void set_border_style(border_style aStyle)
        {
            iBorderStyle = aStyle;
            update();
        }
    protected:
        void paint(i_graphics_context& aGc) const override
        {
            widget::paint(aGc);
            if (opacity() == 1.0)
            {
                switch (iBorderStyle)
                {
                case border_style::None:
                default:
                    break;
                case border_style::Drag:
                    {
                        auto const cr = client_rect(false);
                        aGc.draw_rect(cr, pen{ color::White.with_alpha(0.75), 2.0 });
                        aGc.line_stipple_on(2.0, 0xCCCC, (7.0 - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 100 % 8));
                        aGc.draw_rect(cr, pen{ color::Black.with_alpha(0.75), 2.0 });
                        aGc.line_stipple_off();
                    }
                    break;
                case border_style::Edit:
                    break;
                }
            }
        }
    private:
        sink iSink;
        vertical_layout iLayout;
        border_style iBorderStyle;
        neolib::callback_timer iAnimator;
    };
}