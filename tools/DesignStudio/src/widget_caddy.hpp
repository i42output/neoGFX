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
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>

namespace neogfx::DesignStudio
{
    class widget_caddy : public widget<>, private i_clipboard_sink
    {
    public:
        widget_caddy(i_project& aProject, i_element& aElement, i_widget& aParent, const point& aPosition) :
            widget{ aParent },
            iProject{ aProject },
            iElement{ aElement },
            iItem{ aElement.layout_item() },
            iAnimator{ service<i_async_task>(), [this](neolib::callback_timer& aAnimator) 
            {    
                aAnimator.again();
                if (has_element() && (element().mode() != element_mode::None || element().is_selected()))
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
            iSink += element().mode_changed([&]()
            {
                update();
            });
            iSink += element().selection_changed([&]()
            {
                update();
            });
            iSink += iProject.element_removed([&](i_element& aElement)
            {
                if (&aElement == iElement)
                    parent().remove(*this);
            });
            if (item().is_widget())
                add(item().as_widget());
        }
        ~widget_caddy()
        {
            if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
                service<i_clipboard>().deactivate(*this);
        }
    public:
        bool has_element() const
        {
            return iElement.valid();
        }
        i_element& element() const
        {
            return *iElement;
        }
        i_layout_item& item() const
        {
            return *iItem;
        }
    public:
        size minimum_size(optional_size const& aAvailableSpace = {}) const override
        {
            size result = widget::minimum_size(aAvailableSpace);
            result = item().minimum_size(aAvailableSpace != std::nullopt ? *aAvailableSpace - padding().size() : aAvailableSpace);
            if (result.cx != 0.0)
                result.cx += padding().size().cx;
            if (result.cy != 0.0)
                result.cy += padding().size().cy;
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
            if (item().is_widget())
            {
                item().as_widget().move(client_rect(false).top_left());
                item().as_widget().resize(client_rect(false).extents());
            }
        }
    protected:
        int32_t layer() const override
        {
            switch (element().mode())
            {
            case element_mode::None:
            default:
                return 0;
            case element_mode::Drag:
                return -1;
            case element_mode::Edit:
                return 1;
            }
        }
    protected:
        int32_t render_layer() const override
        {
            switch (element().mode())
            {
            case element_mode::None:
            default:
                return 0;
            case element_mode::Drag:
            case element_mode::Edit:
                return 1;
            }
        }
        void paint_non_client_after(i_graphics_context& aGc) const override
        {
            widget::paint_non_client_after(aGc);
            if (opacity() == 1.0)
            {
                auto draw_selected_rect = [&]()
                {
                    auto const cr = client_rect(false);
                    if (element().is_selected())
                        aGc.fill_rect(cr, service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(0.5));
                    aGc.draw_rect(cr, pen{ color::White.with_alpha(0.75), 2.0 });
                    aGc.line_stipple_on(2.0, 0xCCCC, (7.0 - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 100 % 8));
                    aGc.draw_rect(cr, pen{ color::Black.with_alpha(0.75), 2.0 });
                    aGc.line_stipple_off();
                };
                switch (element().mode())
                {
                case element_mode::None:
                default:
                    if (element().is_selected())
                        draw_selected_rect();
                    break;
                case element_mode::Drag:
                    draw_selected_rect();
                    break;
                case element_mode::Edit:
                    draw_selected_rect();
                    aGc.draw_rect(resizer_part_rect(cardinal::NorthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                    aGc.draw_rect(resizer_part_rect(cardinal::North, false), color::NavyBlue, color::White.with_alpha(0.75));
                    aGc.draw_rect(resizer_part_rect(cardinal::NorthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                    aGc.draw_rect(resizer_part_rect(cardinal::East, false), color::NavyBlue, color::White.with_alpha(0.75));
                    aGc.draw_rect(resizer_part_rect(cardinal::SouthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                    aGc.draw_rect(resizer_part_rect(cardinal::South, false), color::NavyBlue, color::White.with_alpha(0.75));
                    aGc.draw_rect(resizer_part_rect(cardinal::SouthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                    aGc.draw_rect(resizer_part_rect(cardinal::West, false), color::NavyBlue, color::White.with_alpha(0.75));
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
            element().set_mode(element_mode::Edit);
            service<i_clipboard>().activate(*this);
        }
        void focus_lost(focus_reason aFocusReason) override
        {
            widget::focus_lost(aFocusReason);
            element().set_mode(element_mode::None);
            if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
                service<i_clipboard>().deactivate(*this);
        }
    protected:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override
        {
            widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            if (aScanCode == ScanCode_ESCAPE)
            {
                element().root().select(false, true);
                return true;
            }
            return false;
        }
    protected:
        bool ignore_mouse_events() const override
        {
            if (element().mode() == element_mode::Drag)
                return true;
            return widget::ignore_mouse_events();
        }
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override
        {
            widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
            if (aButton == mouse_button::Left)
            {
                bool const toggleSelect = ((aKeyModifiers & key_modifiers_e::KeyModifier_CTRL) != key_modifiers_e::KeyModifier_NONE);
                auto part = toggleSelect ? cardinal::Center : resize_part_at(aPosition);
                if (!part)
                    part = cardinal::Center;
                element().select(toggleSelect ? !element().is_selected() : true, !toggleSelect);
                if (element().is_selected())
                    iResizerDrag.emplace(std::make_pair(*part, aPosition - resizer_part_rect(*part).center()));
            }
        }
        void mouse_button_released(mouse_button aButton, const point& aPosition) override
        {
            widget::mouse_button_released(aButton, aPosition);
            iResizerDrag = {};
            if (aButton == mouse_button::Right)
            {
                if (!iElement->is_selected())
                    iElement->select(true);
                context_menu menu{ *this, root().mouse_position() + root().window_position() };
                action actionSendToBack{ "Send To Back"_t };
                action actionBringToFont{ "Bring To Front"_t };
                auto& actionCut = service<i_app>().action_cut();
                auto& actionCopy = service<i_app>().action_copy();
                auto& actionPaste = service<i_app>().action_paste();
                auto& actionDelete = service<i_app>().action_delete();
                auto& actionSelectAll = service<i_app>().action_select_all();
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
            auto const part = (service<i_keyboard>().modifiers() & key_modifiers_e::KeyModifier_CTRL) == key_modifiers_e::KeyModifier_NONE ?
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
    protected:
        bool can_undo() const override
        {
            // todo
            return false;
        }
        bool can_redo() const override
        {
            // todo
            return false;
        }
        bool can_cut() const override
        {
            // todo
            return false;
        }
        bool can_copy() const override
        {
            // todo
            return false;
        }
        bool can_paste() const override
        {
            // todo
            return false;
        }
        bool can_delete_selected() const override
        {
            bool someSelected = false;
            iProject.root().visit([&](i_element& aElement)
            {
                if (aElement.is_selected())
                    someSelected = true;
            });
            return someSelected;
        }
        bool can_select_all() const override
        {
            return true;
        }
        void undo(i_clipboard& aClipboard) override
        {
            // todo
        }
        void redo(i_clipboard& aClipboard) override
        {
            // todo
        }
        void cut(i_clipboard& aClipboard) override
        {
            // todo
        }
        void copy(i_clipboard& aClipboard) override
        {
            // todo
        }
        void paste(i_clipboard& aClipboard) override
        {
            // todo
        }
        void delete_selected() override
        {
            thread_local std::vector<weak_ref_ptr<i_element>> tToDelete;
            iProject.root().visit([&](i_element& aElement)
            {
                if (aElement.is_selected())
                    tToDelete.push_back(aElement);
            });
            for (auto& e : tToDelete)
            {
                if (e.valid())
                    iProject.remove_element(*e);
            }
            tToDelete.clear();
        }
        void select_all() override
        {
            iProject.root().visit([&](i_element& aElement)
            {
                if (aElement.has_layout_item())
                    aElement.select(true, false);
            });
        }
    private:
        sink iSink;
        i_project& iProject;
        weak_ref_ptr<i_element> iElement;
        weak_ref_ptr<i_layout_item> iItem;
        neolib::callback_timer iAnimator;
        std::optional<std::pair<cardinal, point>> iResizerDrag;
    };
}