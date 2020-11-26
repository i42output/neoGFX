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
    class widget_caddy : public widget<>
    {
    public:
        enum class mode
        {
            None,
            Drag,
            Edit
        };
    public:
        widget_caddy(i_widget& aParent, const point& aPosition) :
            widget{ aParent },
            iMode{ mode::None },
            iAnimator{ service<i_async_task>(), [this](neolib::callback_timer& aAnimator) 
            {    
                aAnimator.again();
                if (iMode != mode::None)
                    update(); 
            }, 20 }
        {
            bring_to_front();
            move(aPosition);
            iSink = ChildAdded([&](i_widget& aChild)
            {
                aChild.set_ignore_mouse_events(true);
                aChild.set_ignore_non_client_mouse_events(true);
            });
        }
    public:
        void set_widget(i_ref_ptr<i_widget> const& aWidget)
        {
            add(aWidget);
            iWidget = aWidget;
            layout_items();
        }
        void set_mode(mode aMode)
        {
            iMode = aMode;
            update();
        }
    public:
        size minimum_size(optional_size const& aAvailableSpace = {}) const override
        {
            size result = widget::minimum_size(aAvailableSpace);
            if (iWidget)
            {
                result = iWidget->minimum_size(aAvailableSpace != std::nullopt ? *aAvailableSpace - padding().size() : aAvailableSpace);
                if (result.cx != 0.0)
                    result.cx += padding().size().cx;
                if (result.cy != 0.0)
                    result.cy += padding().size().cy;
            }
            return result;
        }
    protected:
        neogfx::padding padding() const override
        {
            return neogfx::padding{ 4.0_dip };
        }
    protected:
        void layout_items(bool aDefer = false) override
        {
            widget::layout_items(aDefer);
            if (iWidget)
            {
                iWidget->move(client_rect(false).top_left());
                iWidget->resize(client_rect(false).extents());
            }
        }
    protected:
        int32_t layer() const override
        {
            switch (iMode)
            {
            case mode::None:
            default:
                return 0;
            case mode::Drag:
                return -1;
            case mode::Edit:
                return 1;
            }
        }
    protected:
        int32_t render_layer() const override
        {
            switch (iMode)
            {
            case mode::None:
            default:
                return 0;
            case mode::Drag:
            case mode::Edit:
                return 1;
            }
        }
        void paint_non_client_after(i_graphics_context& aGc) const override
        {
            widget::paint_non_client_after(aGc);
            if (opacity() == 1.0)
            {
                switch (iMode)
                {
                case mode::None:
                default:
                    break;
                case mode::Drag:
                    {
                        auto const cr = client_rect(false);
                        aGc.draw_rect(cr, pen{ color::White.with_alpha(0.75), 2.0 });
                        aGc.line_stipple_on(2.0, 0xCCCC, (7.0 - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 100 % 8));
                        aGc.draw_rect(cr, pen{ color::Black.with_alpha(0.75), 2.0 });
                        aGc.line_stipple_off();
                    }
                    break;
                case mode::Edit:
                    {
                        auto const cr = client_rect(false);
                        aGc.draw_rect(cr, pen{ color::White.with_alpha(0.75), 2.0 });
                        aGc.line_stipple_on(2.0, 0xCCCC, (7.0 - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 100 % 8));
                        aGc.draw_rect(cr, pen{ color::Black.with_alpha(0.75), 2.0 });
                        aGc.line_stipple_off();
                        aGc.draw_rect(resizer_part_rect(cardinal::NorthWest), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::North), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::NorthEast), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::East), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::SouthEast), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::South), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::SouthWest), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::West), color::NavyBlue, color::White.with_alpha(0.75));
                    }
                    break;
                }
            }
        }
    protected:
        neogfx::focus_policy focus_policy() const override
        {
            return neogfx::focus_policy::StrongFocus;
        }
        void focus_gained(focus_reason aFocusReason) override
        {
            widget::focus_gained(aFocusReason);
            set_mode(mode::Edit);
        }
        void focus_lost(focus_reason aFocusReason) override
        {
            widget::focus_lost(aFocusReason);
            set_mode(mode::None);
        }
    protected:
        bool ignore_mouse_events() const override
        {
            if (iMode == mode::Drag)
                return true;
            return widget::ignore_mouse_events();
        }
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override
        {
            widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
            if (aButton == mouse_button::Left)
            {
                auto update_resizer = [&](cardinal aCardinal) -> bool
                {
                    if (resizer_part_rect(aCardinal).contains(aPosition))
                    {
                        iResizerDrag.emplace(std::make_pair(aCardinal, aPosition - resizer_part_rect(aCardinal).center()));
                        return true;
                    }
                    return false;
                };
                if (!update_resizer(cardinal::NorthWest))
                    if (!update_resizer(cardinal::North))
                        if (!update_resizer(cardinal::NorthEast))
                            if (!update_resizer(cardinal::West))
                                if (!update_resizer(cardinal::Center))
                                    if (!update_resizer(cardinal::East))
                                        if (!update_resizer(cardinal::SouthWest))
                                            if (!update_resizer(cardinal::South))
                                                update_resizer(cardinal::SouthEast);
            }
        }
        void mouse_button_released(mouse_button aButton, const point& aPosition) override
        {
            widget::mouse_button_released(aButton, aPosition);
            iResizerDrag = {};
        }
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override
        {
            widget::mouse_moved(aPosition, aKeyModifiers);
            if (capturing() && iResizerDrag)
            {
                auto delta = point{ aPosition - resizer_part_rect(iResizerDrag->first).center() } - iResizerDrag->second;
                switch (iResizerDrag->first)
                {
                case cardinal::NorthWest:
                    delta = delta.min(non_client_rect().bottom_right().to_delta());
                    move(position() + delta);
                    resize((extents() - size{ delta }).max(size{}));
                    break;
                case cardinal::North:
                    delta.dx = 0.0;
                    delta.dy = std::min(delta.dy, non_client_rect().bottom());
                    move(position() + delta);
                    resize((extents() - size{ delta }).max(size{}));
                    break;
                case cardinal::NorthEast:
                    move(position() + size{ 0.0, delta.dy });
                    resize((extents() - size{ delta }).max(size{}));
                    break;
                case cardinal::West:
                    break;
                case cardinal::Center:
                    move(position() + delta);
                    break;
                case cardinal::East:
                    break;
                case cardinal::SouthWest:
                    break;
                case cardinal::South:
                    break;
                case cardinal::SouthEast:
                    break;
                }
            }
        }
        neogfx::mouse_cursor mouse_cursor() const override
        {
            point mousePos = root().mouse_position() - origin();
            if (resizer_part_rect(cardinal::NorthWest).contains(mousePos) || resizer_part_rect(cardinal::SouthEast).contains(mousePos))
                return mouse_system_cursor::SizeNWSE;
            else if (resizer_part_rect(cardinal::NorthEast).contains(mousePos) || resizer_part_rect(cardinal::SouthWest).contains(mousePos))
                return mouse_system_cursor::SizeNESW;
            else if (resizer_part_rect(cardinal::North).contains(mousePos) || resizer_part_rect(cardinal::South).contains(mousePos))
                return mouse_system_cursor::SizeNS;
            else if (resizer_part_rect(cardinal::West).contains(mousePos) || resizer_part_rect(cardinal::East).contains(mousePos))
                return mouse_system_cursor::SizeWE;
            else if (resizer_part_rect(cardinal::Center).contains(mousePos))
                return mouse_system_cursor::SizeAll;
            return widget::mouse_cursor();
        }
    private:
        rect resizer_part_rect(cardinal aPart) const
        {
            auto const pw = padding().left * 2.0;
            auto const cr = client_rect(false).inflated(pw / 2.0);
            switch (aPart)
            {
            case cardinal::NorthWest:
                return rect{ cr.top_left(), size{ pw } };
            case cardinal::North:
                return rect{ point{ cr.center().x - pw / 2.0, cr.top() }, size{ pw } };
            case cardinal::NorthEast:
                return rect{ point{ cr.right() - pw, cr.top() }, size{ pw } };
            case cardinal::West:
                return rect{ point{ cr.left(), cr.center().y - pw / 2.0 }, size{ pw } };
            case cardinal::Center:
                return cr.deflated(size{ pw });
            case cardinal::East:
                return rect{ point{ cr.right() - pw, cr.center().y - pw / 2.0 }, size{ pw } };
            case cardinal::SouthWest:
                return rect{ point{ cr.left(), cr.bottom() - pw }, size{ pw } };
            case cardinal::South:
                return rect{ point{ cr.center().x - pw / 2.0, cr.bottom() - pw }, size{ pw } };
            case cardinal::SouthEast:
                return rect{ point{ cr.right() - pw, cr.bottom() - pw }, size{ pw } };
            default:
                return cr;
            }
        }
    private:
        sink iSink;
        mode iMode;
        weak_ref_ptr<i_widget> iWidget;
        neolib::callback_timer iAnimator;
        std::optional<std::pair<cardinal, point>> iResizerDrag;
    };
}