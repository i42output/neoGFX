    // widget_caddy.cpp
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/widget.tpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include "widget_caddy.hpp"

namespace neogfx
{
    template class widget<DesignStudio::i_element_caddy>;
}

namespace neogfx::DesignStudio
{
    widget_caddy::widget_caddy(i_project& aProject, i_element& aElement, i_widget& aParent, const point& aPosition) :
        widget{ aParent },
        iProject{ aProject },
        iElement{ aElement },
        iItem{ aElement.has_layout_item() ? aElement.layout_item() : (aElement.create_layout_item(), aElement.layout_item()) },
        iAnimator{ service<i_async_task>(), [this](neolib::callback_timer& aAnimator) 
        {    
            aAnimator.again();
            if (has_element() && (element().mode() != element_mode::None || element().is_selected() || entered()))
                update(); 
        }, std::chrono::milliseconds{ 20 } }
    {
        set_minimum_size(size{ 96.0_dip, 32.0_dip });
        element().set_caddy(*this);
        bring_to_front();
        move(aPosition);
        iSink = ChildAdded([&](i_widget& aChild)
        {
            aChild.set_ignore_mouse_events(true);
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
        else
            item().set_layout_owner(this);
    }
    
    widget_caddy::~widget_caddy()
    {
        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);
    }

    bool widget_caddy::has_element() const
    {
        return iElement.valid();
    }

    i_element& widget_caddy::element() const
    {
        return *iElement;
    }

    i_layout_item& widget_caddy::item() const
    {
        return *iItem;
    }

    size widget_caddy::minimum_size(optional_size const& aAvailableSpace) const
    {
        size result = item().minimum_size(aAvailableSpace != std::nullopt ? *aAvailableSpace - padding().size() : aAvailableSpace);
        if (result.cx != 0.0)
            result.cx += padding().size().cx;
        if (result.cy != 0.0)
            result.cy += padding().size().cy;
        return result != size{} ? result : widget::minimum_size(aAvailableSpace);
    }

    neogfx::widget_type widget_caddy::widget_type() const
    {
        return widget::widget_type() | neogfx::widget_type::Floating;
    }

    neogfx::padding widget_caddy::padding() const
    {
        return neogfx::padding{ 4.0_dip };
    }

    void widget_caddy::layout_items(bool aDefer)
    {
        widget::layout_items(aDefer);
        if (item().is_widget())
        {
            item().as_widget().move(client_rect(false).top_left());
            item().as_widget().resize(client_rect(false).extents());
        }
    }

    int32_t widget_caddy::layer() const
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

    int32_t widget_caddy::render_layer() const
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

    void widget_caddy::paint(i_graphics_context& aGc) const
    {
        widget::paint(aGc);
        if (item().is_layout())
        {
            auto const r = client_rect(false);
            {
                scoped_opacity so{ aGc, aGc.opacity() * 0.5 };
                scoped_scissor ss{ aGc, r };
                size const iconSize{ std::min<scalar>(r.cx, std::min<scalar>(r.cy, 16.0_dip)), std::min<scalar>(r.cx, std::min<scalar>(r.cy, 16.0_dip)) };
                for (int32_t y = 0; y < r.height() / iconSize.cy; ++y)
                {
                    for (int32_t x = 0; x < r.width() / iconSize.cx; ++x)
                    {
                        if (y % 2 == 1 || x % 3 != y % 3)
                            continue;
                        aGc.draw_texture(rect{ r.top_left() + point{ iconSize } / 4.0 + basic_point<int32_t>{ x, y }.as<scalar>() * iconSize, iconSize }, element().library().element_icon(element().type()));
                    }
                }
            }
            aGc.line_stipple_on(4.0, 0xCCCC);
            aGc.draw_rect(r, pen{ color::PowderBlue.lighter(0x20) });
            aGc.line_stipple_off();
        }
    }

    void widget_caddy::paint_non_client_after(i_graphics_context& aGc) const
    {
        widget::paint_non_client_after(aGc);
        if (opacity() == 1.0)
        {
            auto draw_selected_rect = [&]()
            {
                auto const cr = client_rect(false);
                if (element().is_selected() && element().group() != element_group::Workflow)
                    aGc.fill_rect(cr, service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(0.5));
                aGc.draw_rect(cr, pen{ color::White.with_alpha(0.75), 2.0 });
                aGc.line_stipple_on(2.0, 0xCCCC, (7.0 - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 100 % 8));
                aGc.draw_rect(cr, pen{ color::Black.with_alpha(0.75), 2.0 });
                aGc.line_stipple_off();
            };
            auto draw_resizer_rects = [&]()
            {
                aGc.draw_rect(resizer_part_rect(cardinal::NorthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(resizer_part_rect(cardinal::North, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(resizer_part_rect(cardinal::NorthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(resizer_part_rect(cardinal::East, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(resizer_part_rect(cardinal::SouthEast, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(resizer_part_rect(cardinal::South, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(resizer_part_rect(cardinal::SouthWest, false), color::NavyBlue, color::White.with_alpha(0.75));
                aGc.draw_rect(resizer_part_rect(cardinal::West, false), color::NavyBlue, color::White.with_alpha(0.75));
            };
            switch (element().mode())
            {
            case element_mode::None:
            default:
                if (element().is_selected() || entered())
                    draw_selected_rect();
                if (entered())
                    draw_resizer_rects();
                break;
            case element_mode::Drag:
                draw_selected_rect();
                break;
            case element_mode::Edit:
                draw_selected_rect();
                draw_resizer_rects();
                break;
            }
        }
    }

    focus_policy widget_caddy::focus_policy() const
    {
        return neogfx::focus_policy::StrongFocus;
    }

    void widget_caddy::focus_gained(focus_reason aFocusReason)
    {
        widget::focus_gained(aFocusReason);
        element().set_mode(element_mode::Edit);
        service<i_clipboard>().activate(*this);
        if (element().group() == element_group::Workflow)
            bring_to_front();
    }

    void widget_caddy::focus_lost(focus_reason aFocusReason)
    {
        widget::focus_lost(aFocusReason);
        if (has_element())
            element().set_mode(element_mode::None);
        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);
    }

    bool widget_caddy::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
        if (aScanCode == ScanCode_ESCAPE)
        {
            element().root().select(false, true);
            return true;
        }
        return false;
    }

    bool widget_caddy::ignore_mouse_events(bool aConsiderAncestors) const
    {
        if (element().mode() == element_mode::Drag)
            return true;
        return widget::ignore_mouse_events(aConsiderAncestors);
    }

    void widget_caddy::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            bool const toggleSelect = ((aKeyModifiers & key_modifiers_e::KeyModifier_CTRL) != key_modifiers_e::KeyModifier_NONE);
            auto part = toggleSelect ? cardinal::Center : resize_part_at(aPosition);
            if (!part)
                part = cardinal::Center;
            if (part == cardinal::Center)
                element().select(toggleSelect ? !element().is_selected() : true, !toggleSelect && element().root().selected_child_count() <= 1);
            if (element().is_selected() && part == cardinal::Center)
            {
                element().root().visit([&](i_element& aElement)
                {
                    if (aElement.is_selected() && aElement.has_caddy())
                        aElement.caddy().start_drag(cardinal::Center, aPosition);
                });
            }
            else if (part != cardinal::Center)
                start_drag(*part, aPosition);
        }
    }

    void widget_caddy::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        bool const wasCapturing = capturing();
        widget::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left && wasCapturing)
        {
            if (iDragInfo && !iDragInfo->wasDragged && iDragInfo->part == cardinal::Center)
            {
                if (element().group() == element_group::Workflow && element().has_widget())
                {
                    element().select(false, false);
                    element().widget().set_focus();
                }
            }
            iElement->root().visit([&](i_element& aElement)
            {
                if (aElement.is_selected() && aElement.has_caddy())
                    aElement.caddy().end_drag();
            });
        }
        else if (aButton == mouse_button::Right)
        {
            if (!iElement->is_selected())
                iElement->select(true);
            display_element_context_menu(*this, *iElement);
        }
    }

    void widget_caddy::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_moved(aPosition, aKeyModifiers);
        if (capturing() && iDragInfo)
        {
            bool const ignoreConstraints = ((aKeyModifiers & key_modifiers_e::KeyModifier_SHIFT) != key_modifiers_e::KeyModifier_NONE);
            if (iDragInfo->part == cardinal::Center)
            {
                element().root().visit([&](i_element& aElement)
                {
                    if (aElement.is_selected() && aElement.has_caddy())
                        aElement.caddy().drag( aPosition, ignoreConstraints);
                });
            }
            else
                drag(aPosition, ignoreConstraints);
        }
    }

    void widget_caddy::mouse_entered(const point& aPosition)
    {
        update();
    }

    void widget_caddy::mouse_left()
    {
        update();
    }

    mouse_cursor widget_caddy::mouse_cursor() const
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

    void widget_caddy::start_drag(cardinal aPart, point const& aPosition)
    {
        iDragInfo.emplace(aPart, aPosition - resizer_part_rect(aPart).center());
    }

    void widget_caddy::drag(point const& aPosition, bool aIgnoreConstraints)
    {
        auto const delta = point{ aPosition - resizer_part_rect(iDragInfo->part).center() } - iDragInfo->dragFrom;
        auto r = non_client_rect();
        switch (iDragInfo->part)
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
        auto const minSize = aIgnoreConstraints ? padding().size() : minimum_size();
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
        iDragInfo->wasDragged = true;
    }

    void widget_caddy::end_drag()
    {
        iDragInfo = std::nullopt;
    }

    bool widget_caddy::can_undo() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_redo() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_cut() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_copy() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_paste() const
    {
        // todo
        return false;
    }

    bool widget_caddy::can_delete_selected() const
    {
        bool someSelected = false;
        iProject.root().visit([&](i_element& aElement)
        {
            if (aElement.is_selected())
                someSelected = true;
        });
        return someSelected;
    }

    bool widget_caddy::can_select_all() const
    {
        return true;
    }

    void widget_caddy::undo(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::redo(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::cut(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::copy(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::paste(i_clipboard& aClipboard)
    {
        // todo
    }

    void widget_caddy::delete_selected()
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

    void widget_caddy::select_all()
    {
        iProject.root().visit([&](i_element& aElement)
        {
            if (aElement.has_layout_item())
                aElement.select(true, false);
        });
    }

    std::optional<cardinal> widget_caddy::resize_part_at(point const& aPosition) const
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

    rect widget_caddy::resizer_part_rect(cardinal aPart, bool aForHitTest) const
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
}