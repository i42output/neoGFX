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
#include <neogfx/app/action.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>

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
        widget_caddy(i_project& aProject, i_widget& aParent, const point& aPosition) :
            widget{ aParent },
            iProject{ aProject },
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
        ~widget_caddy()
        {
        }
    public:
        void set_item(i_ref_ptr<i_layout_item> const& aItem)
        {
            if (aItem->is_widget())
            {
                add(aItem->as_widget());
                iItem = aItem;
            }
            layout_items();
        }
        void set_element(i_element& aElement)
        {
            iElement = aElement;
            set_item(aElement.layout_item());
            iSink += aElement.selection_changed([&]()
            {
                set_mode(aElement.is_selected() ? mode::Edit : mode::None);
            });
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
            if (iItem)
            {
                result = iItem->minimum_size(aAvailableSpace != std::nullopt ? *aAvailableSpace - padding().size() : aAvailableSpace);
                if (result.cx != 0.0)
                    result.cx += padding().size().cx;
                if (result.cy != 0.0)
                    result.cy += padding().size().cy;
            }
            return result;
        }
    protected:
        neogfx::widget_type widget_type() const override
        {
            return widget::widget_type() | neogfx::widget_type::Floating;
        }
        neogfx::padding padding() const override
        {
            return neogfx::padding{ 4.0_dip };
        }
    protected:
        void layout_items(bool aDefer = false) override
        {
            widget::layout_items(aDefer);
            if (iItem->is_widget())
            {
                iItem->as_widget().move(client_rect(false).top_left());
                iItem->as_widget().resize(client_rect(false).extents());
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
                        aGc.draw_rect(resizer_part_rect(cardinal::NorthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::North, false), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::NorthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::East, false), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::SouthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::South, false), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::SouthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                        aGc.draw_rect(resizer_part_rect(cardinal::West, false), color::NavyBlue, color::White.with_alpha(0.75));
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
            if (iElement)
                iElement->select();
        }
        void focus_lost(focus_reason aFocusReason) override
        {
            widget::focus_lost(aFocusReason);
            set_mode(mode::None);
            if (iElement)
                iElement->select(false);
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
                auto part = (aKeyModifiers & key_modifiers_e::KeyModifier_CTRL) != key_modifiers_e::KeyModifier_CTRL ? resize_part_at(aPosition) : cardinal::Center;
                if (!part)
                    part = cardinal::Center;
                iResizerDrag.emplace(std::make_pair(*part, aPosition - resizer_part_rect(*part).center()));
            }
        }
        void mouse_button_released(mouse_button aButton, const point& aPosition) override
        {
            widget::mouse_button_released(aButton, aPosition);
            iResizerDrag = {};
            if (aButton == mouse_button::Right)
            {
                context_menu menu{ *this, root().mouse_position() + root().window_position() };
                action actionSendToBack{ "Send To Back"_t };
                action actionBringToFont{ "Bring To Front"_t };
                action actionCut{ "Cut"_t, ":/neogfx/resources/icons/cut.png" };
                action actionCopy{ "Copy"_t, ":/neogfx/resources/icons/copy.png" };
                action actionPaste{ "Paste"_t, ":/neogfx/resources/icons/paste.png" };
                action actionDelete{ "Delete"_t };
                action actionSelectAll{ "Select All"_t };
                actionCut.set_shortcut("Ctrl+X");
                actionCopy.set_shortcut("Ctrl+C");
                actionPaste.set_shortcut("Ctrl+V");
                actionDelete.set_shortcut("Del");
                actionSelectAll.set_shortcut("Ctrl+A");
                if (&*parent().children().back() == this)
                    actionSendToBack.disable();
                if (&*parent().children().front() == this)
                    actionBringToFont.disable();
                actionSendToBack.triggered([&]()
                {
                    send_to_back();
                    parent().children().front()->set_focus();
                });
                actionBringToFont.triggered([&]()
                {
                    bring_to_front();
                });
                actionDelete.triggered([&]()
                {
                    if (iElement)
                        iProject.remove_element(*iElement);
                    parent().remove(*this);
                });
                menu.menu().add_action(actionSendToBack);
                menu.menu().add_action(actionBringToFont);
                menu.menu().add_separator();
                menu.menu().add_action(actionCut);
                menu.menu().add_action(actionCopy);
                menu.menu().add_action(actionPaste);
                menu.menu().add_action(actionDelete);
                menu.menu().add_separator();
                menu.menu().add_action(actionSelectAll);
                menu.exec();
            }
        }
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override
        {
            widget::mouse_moved(aPosition, aKeyModifiers);
            if (capturing() && iResizerDrag)
            {
                bool const ignoreConstraints = ((aKeyModifiers & key_modifiers_e::KeyModifier_SHIFT) != key_modifiers_e::KeyModifier_NONE);
                auto const delta = point{ aPosition - resizer_part_rect(iResizerDrag->first).center() } - iResizerDrag->second;
                auto r = non_client_rect();
                switch (iResizerDrag->first)
                {
                case cardinal::NorthWest:
                    r.x += delta.dx;
                    r.y += delta.dy;
                    r.cx -= delta.dx;
                    r.cy -= delta.dy;
                    break;
                case cardinal::NorthEast:
                    r.y += delta.dy;
                    r.cx += delta.dx;
                    r.cy -= delta.dy;
                    break;
                case cardinal::North:
                    r.y += delta.dy;
                    r.cy -= delta.dy;
                    break;
                case cardinal::West:
                    r.x += delta.dx;
                    r.cx -= delta.dx;
                    break;
                case cardinal::Center:
                    r.x += delta.dx;
                    r.y += delta.dy;
                    break;
                case cardinal::East:
                    r.cx += delta.dx;
                    break;
                case cardinal::SouthWest:
                    r.x += delta.dx;
                    r.cx -= delta.dx;
                    r.cy += delta.dy;
                    break;
                case cardinal::SouthEast:
                    r.cx += delta.dx;
                    r.cy += delta.dy;
                    break;
                case cardinal::South:
                    r.cy += delta.dy;
                    break;
                }
                auto const minSize = ignoreConstraints ? padding().size() : minimum_size();
                if (r.cx < minSize.cx)
                {
                    r.cx = minSize.cx;
                    if (non_client_rect().x < r.x)
                        r.x = non_client_rect().right() - r.cx;
                }
                if (r.cy < minSize.cy)
                {
                    r.cy = minSize.cy;
                    if (non_client_rect().y < r.y)
                        r.y = non_client_rect().bottom() - r.cy;
                }
                move(r.top_left() - parent().origin());
                resize(r.extents());
            }
        }
        neogfx::mouse_cursor mouse_cursor() const override
        {
            auto part = !service<i_keyboard>().is_key_pressed(scan_code_e::ScanCode_LCTRL) && !service<i_keyboard>().is_key_pressed(scan_code_e::ScanCode_LCTRL) ?
                resize_part_at(root().mouse_position() - origin()) : cardinal::Center;
            if (part)
                switch (*part)
                {
                case cardinal::NorthWest:
                case cardinal::SouthEast:
                    return mouse_system_cursor::SizeNWSE;
                case cardinal::NorthEast:
                case cardinal::SouthWest:
                    return mouse_system_cursor::SizeNESW;
                case cardinal::North:
                case cardinal::South:
                    return mouse_system_cursor::SizeNS;
                case cardinal::West:
                case cardinal::East:
                    return mouse_system_cursor::SizeWE;
                case cardinal::Center:
                    return mouse_system_cursor::SizeAll;
                };
            return widget::mouse_cursor();
        }
    private:
        std::optional<cardinal> resize_part_at(point const& aPosition) const
        {
            if (resizer_part_rect(cardinal::NorthWest).contains(aPosition))
                return cardinal::NorthWest;
            else if (resizer_part_rect(cardinal::SouthEast).contains(aPosition))
                return cardinal::SouthEast;
            else if (resizer_part_rect(cardinal::NorthEast).contains(aPosition))
                return cardinal::NorthEast;
            else if (resizer_part_rect(cardinal::SouthWest).contains(aPosition))
                return cardinal::SouthWest;
            else if (resizer_part_rect(cardinal::North).contains(aPosition))
                return cardinal::North;
            else if (resizer_part_rect(cardinal::South).contains(aPosition))
                return cardinal::South;
            else if (resizer_part_rect(cardinal::West).contains(aPosition))
                return cardinal::West;
            else if (resizer_part_rect(cardinal::East).contains(aPosition))
                return cardinal::East;
            else if (resizer_part_rect(cardinal::Center).contains(aPosition))
                return cardinal::Center;
            else
                return {};
        }
        rect resizer_part_rect(cardinal aPart, bool aForHitTest = true) const
        {
            auto const pw = padding().left * 2.0;
            auto const cr = client_rect(false).inflated(pw / 2.0);
            rect result;
            switch (aPart)
            {
            case cardinal::NorthWest:
                result = rect{ cr.top_left(), size{ pw } };
                break;
            case cardinal::North:
                result = rect{ point{ cr.center().x - pw / 2.0, cr.top() }, size{ pw } };
                break;
            case cardinal::NorthEast:
                result = rect{ point{ cr.right() - pw, cr.top() }, size{ pw } };
                break;
            case cardinal::West:
                result = rect{ point{ cr.left(), cr.center().y - pw / 2.0 }, size{ pw } };
                break;
            case cardinal::Center:
                result = cr.deflated(size{ pw });
                break;
            case cardinal::East:
                result = rect{ point{ cr.right() - pw, cr.center().y - pw / 2.0 }, size{ pw } };
                break;
            case cardinal::SouthWest:
                result = rect{ point{ cr.left(), cr.bottom() - pw }, size{ pw } };
                break;
            case cardinal::South:
                result = rect{ point{ cr.center().x - pw / 2.0, cr.bottom() - pw }, size{ pw } };
                break;
            case cardinal::SouthEast:
                result = rect{ point{ cr.right() - pw, cr.bottom() - pw }, size{ pw } };
                break;
            default:
                result = cr;
                break;
            }
            if (aForHitTest && aPart != cardinal::Center)
                result.inflate(result.extents() / 2.0);
            return result;
        }
    private:
        sink iSink;
        i_project& iProject;
        mode iMode;
        weak_ref_ptr<i_element> iElement;
        weak_ref_ptr<i_layout_item> iItem;
        neolib::callback_timer iAnimator;
        std::optional<std::pair<cardinal, point>> iResizerDrag;
    };
}