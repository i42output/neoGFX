// scrollable_widget.inl
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

#include <neogfx/neogfx.hpp>
#include <neolib/core/scoped.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>

namespace neogfx
{
    template <typename Base>
    scrollable_widget<Base>::~scrollable_widget()
    {
    }

    template <typename Base>
    scrollbar_style scrollable_widget<Base>::scrollbar_style() const
    {
        return iScrollbarStyle;
    }

    template <typename Base>
    point scrollable_widget<Base>::scroll_position() const
    {
        return units_converter{ *this }.from_device_units(point(static_cast<coordinate>(horizontal_scrollbar().position()), static_cast<coordinate>(vertical_scrollbar().position())));
    }

    template <typename Base>
    void scrollable_widget<Base>::scroll_to(i_widget& aChild)
    {
        (void)aChild;
        /* todo */
    }

    template <typename Base>
    void scrollable_widget<Base>::layout_items_started()
    {
        base_type::layout_items_started();
    }

    template <typename Base>
    void scrollable_widget<Base>::layout_items_completed()
    {
        base_type::layout_items_completed();
        if (!as_widget().layout_items_in_progress() && !iIgnoreScrollbarUpdates && !iMovingWidgets)
            update_scrollbar_visibility();
    }

    template <typename Base>
    void scrollable_widget<Base>::resized()
    {
        base_type::resized();
        if (!as_widget().layout_items_in_progress() && !iIgnoreScrollbarUpdates)
            update_scrollbar_visibility();
    }

    template <typename Base>
    rect scrollable_widget<Base>::client_rect(bool aIncludePadding) const
    {
        rect result = base_type::client_rect(aIncludePadding);
        if (vertical_scrollbar().visible())
        {
            if (vertical_scrollbar().style() == scrollbar_style::Normal)
                result.cx -= vertical_scrollbar().width();
            else if (vertical_scrollbar().style() == scrollbar_style::Menu)
            {
                result.y += vertical_scrollbar().width();
                result.cy -= vertical_scrollbar().width() * 2.0;
            }
            else if (vertical_scrollbar().style() == scrollbar_style::Scroller)
                result.cy -= vertical_scrollbar().width() * 2.0;
        }
        if (horizontal_scrollbar().visible())
        {
            if (horizontal_scrollbar().style() == scrollbar_style::Normal)
                result.cy -= horizontal_scrollbar().width();
            else if (vertical_scrollbar().style() == scrollbar_style::Menu)
            {
                result.x += horizontal_scrollbar().width();
                result.cx -= horizontal_scrollbar().width() * 2.0;
            }
            else if (vertical_scrollbar().style() == scrollbar_style::Scroller)
                result.cx -= horizontal_scrollbar().width() * 2.0;
        }
        return result;
    }

    template <typename Base>
    widget_part scrollable_widget<Base>::part(const point& aPosition) const
    {
        if (vertical_scrollbar().visible() && 
            (vertical_scrollbar().element_at(aPosition + as_widget().origin()) != scrollbar_element::None) ||
            (vertical_scrollbar().style() == scrollbar_style::Normal && scrollbar_geometry(vertical_scrollbar()).contains(aPosition + as_widget().origin())))
            return widget_part{ *this, widget_part::VerticalScrollbar };
        else if (horizontal_scrollbar().visible() &&
            (horizontal_scrollbar().element_at(aPosition + as_widget().origin()) != scrollbar_element::None) ||
            (horizontal_scrollbar().style() == scrollbar_style::Normal && scrollbar_geometry(horizontal_scrollbar()).contains(aPosition + as_widget().origin())))
            return widget_part{ *this, widget_part::HorizontalScrollbar };
        else
            return base_type::part(aPosition);
    }

    template <typename Base>
    void scrollable_widget<Base>::paint_non_client_after(i_graphics_context& aGc) const
    {
        base_type::paint_non_client_after(aGc);
        if (vertical_scrollbar().visible() && !vertical_scrollbar().auto_hidden())
            vertical_scrollbar().render(aGc);
        if (horizontal_scrollbar().visible() && !horizontal_scrollbar().auto_hidden())
            horizontal_scrollbar().render(aGc);
        if (vertical_scrollbar().visible() && horizontal_scrollbar().visible() && 
            !vertical_scrollbar().auto_hidden() && !horizontal_scrollbar().auto_hidden() &&
            vertical_scrollbar().style() == horizontal_scrollbar().style() && vertical_scrollbar().style() == scrollbar_style::Normal)
        {
            point const oldOrigin = aGc.origin();
            aGc.set_origin(point{});
            auto const spareSquare = rect{
                    point{ scrollbar_geometry(horizontal_scrollbar()).right(), scrollbar_geometry(vertical_scrollbar()).bottom() },
                    size{ scrollbar_geometry(vertical_scrollbar()).width(), scrollbar_geometry(horizontal_scrollbar()).height() } };
            aGc.fill_rect(spareSquare, scrollbar_color(vertical_scrollbar()));
            aGc.set_origin(oldOrigin);
        }
    }

    template <typename Base>
    void scrollable_widget<Base>::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers)
    {
        bool handledVertical = false;
        bool handledHorizontal = false;
        mouse_wheel verticalSense = mouse_wheel::Vertical;
        mouse_wheel horizontalSense = mouse_wheel::Horizontal;
        if (service<i_keyboard>().is_key_pressed(ScanCode_LSHIFT) || service<i_keyboard>().is_key_pressed(ScanCode_RSHIFT))
            std::swap(verticalSense, horizontalSense);
        auto const maxSteps = 6.0; // todo: configurable
        auto const maxDeltaY = std::max(vertical_scrollbar().page() - vertical_scrollbar().step(), vertical_scrollbar().step());
        auto const maxDeltaX = std::max(horizontal_scrollbar().page() - horizontal_scrollbar().step(), horizontal_scrollbar().step());
        if ((aWheel & verticalSense) != mouse_wheel::None && vertical_scrollbar().visible())
            handledVertical = vertical_scrollbar().set_position(vertical_scrollbar().position() + std::min(std::max(((verticalSense == mouse_wheel::Vertical ? aDelta.dy : aDelta.dx) >= 0.0 ? -maxSteps : maxSteps) * vertical_scrollbar().step(), -maxDeltaY), maxDeltaY));
        if ((aWheel & horizontalSense) != mouse_wheel::None && horizontal_scrollbar().visible())
            handledHorizontal = horizontal_scrollbar().set_position(horizontal_scrollbar().position() + std::min(std::max(((horizontalSense == mouse_wheel::Horizontal ? aDelta.dx : aDelta.dx) >= 0.0 ? -maxSteps : maxSteps) * horizontal_scrollbar().step(), -maxDeltaX), maxDeltaX));
        mouse_wheel passOn = static_cast<mouse_wheel>(
            aWheel & ((handledVertical ? ~verticalSense : verticalSense) | (handledHorizontal ? ~horizontalSense : horizontalSense)));
        if (passOn != mouse_wheel::None)
            base_type::mouse_wheel_scrolled(passOn, aPosition, aDelta, aKeyModifiers);
    }

    template <typename Base>
    void scrollable_widget<Base>::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        if (aButton == mouse_button::Middle)
        {
            bool handled = false;
            if (vertical_scrollbar().visible())
            {
                vertical_scrollbar().track();
                handled = true;
            }
            if (horizontal_scrollbar().visible())
            {
                horizontal_scrollbar().track();
                handled = true;
            }
            if (handled)
                as_widget().set_capture();
            else
                base_type::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        }
        else
        {
            base_type::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
            if (aButton == mouse_button::Left)
            {
                if (vertical_scrollbar().clicked_element() == scrollbar_element::None && horizontal_scrollbar().clicked_element() == scrollbar_element::None)
                {
                    if (vertical_scrollbar().visible() && vertical_scrollbar().element_at(aPosition + as_widget().origin()) != scrollbar_element::None)
                    {
                        as_widget().update(true);
                        vertical_scrollbar().click_element(vertical_scrollbar().element_at(aPosition + as_widget().origin()));
                    }
                    else if (horizontal_scrollbar().visible() && horizontal_scrollbar().element_at(aPosition + as_widget().origin()) != scrollbar_element::None)
                    {
                        as_widget().update(true);
                        horizontal_scrollbar().click_element(horizontal_scrollbar().element_at(aPosition + as_widget().origin()));
                    }
                }
            }
        }
    }

    template <typename Base>
    void scrollable_widget<Base>::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            if (vertical_scrollbar().clicked_element() == scrollbar_element::None && horizontal_scrollbar().clicked_element() == scrollbar_element::None)
            {
                if (vertical_scrollbar().visible() && vertical_scrollbar().element_at(aPosition + as_widget().origin()) != scrollbar_element::None)
                {
                    as_widget().update(true);
                    vertical_scrollbar().click_element(vertical_scrollbar().element_at(aPosition + as_widget().origin()));
                }
                else if (horizontal_scrollbar().visible() && horizontal_scrollbar().element_at(aPosition + as_widget().origin()) != scrollbar_element::None)
                {
                    as_widget().update(true);
                    horizontal_scrollbar().click_element(horizontal_scrollbar().element_at(aPosition + as_widget().origin()));
                }
            }
        }
    }

    template <typename Base>
    void scrollable_widget<Base>::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        base_type::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left)
        {
            if (vertical_scrollbar().clicked_element() != scrollbar_element::None)
            {
                as_widget().update(true);
                vertical_scrollbar().unclick_element();
            }
            else if (horizontal_scrollbar().clicked_element() != scrollbar_element::None)
            {
                as_widget().update(true);
                horizontal_scrollbar().unclick_element();
            }
        }
        else if (aButton == mouse_button::Middle)
        {
            vertical_scrollbar().untrack();
            horizontal_scrollbar().untrack();
        }
    }

    template <typename Base>
    void scrollable_widget<Base>::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_moved(aPosition, aKeyModifiers);
        vertical_scrollbar().update(aPosition + as_widget().origin());
        horizontal_scrollbar().update(aPosition + as_widget().origin());
    }

    template <typename Base>
    void scrollable_widget<Base>::mouse_entered(const point& aPosition)
    {
        base_type::mouse_entered(aPosition);
        vertical_scrollbar().update();
        horizontal_scrollbar().update();
    }

    template <typename Base>
    void scrollable_widget<Base>::mouse_left()
    {
        base_type::mouse_left();
        vertical_scrollbar().update();
        horizontal_scrollbar().update();
    }

    template <typename Base>
    bool scrollable_widget<Base>::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        bool handled = true;
        switch (aScanCode)
        {
        case ScanCode_LEFT:
            horizontal_scrollbar().set_position(horizontal_scrollbar().position() - horizontal_scrollbar().step());
            break;
        case ScanCode_RIGHT:
            horizontal_scrollbar().set_position(horizontal_scrollbar().position() + horizontal_scrollbar().step());
            break;
        case ScanCode_UP:
            vertical_scrollbar().set_position(vertical_scrollbar().position() - vertical_scrollbar().step());
            break;
        case ScanCode_DOWN:
            vertical_scrollbar().set_position(vertical_scrollbar().position() + vertical_scrollbar().step());
            break;
        case ScanCode_PAGEUP:
            vertical_scrollbar().set_position(vertical_scrollbar().position() - vertical_scrollbar().page());
            break;
        case ScanCode_PAGEDOWN:
            vertical_scrollbar().set_position(vertical_scrollbar().position() + vertical_scrollbar().page());
            break;
        case ScanCode_HOME:
            if (horizontal_scrollbar().visible() && !(aKeyModifiers & KeyModifier_CTRL))
                horizontal_scrollbar().set_position(horizontal_scrollbar().minimum());
            else
                vertical_scrollbar().set_position(vertical_scrollbar().minimum());
            break;
        case ScanCode_END:
            if (horizontal_scrollbar().visible() && !(aKeyModifiers & KeyModifier_CTRL))
                horizontal_scrollbar().set_position(horizontal_scrollbar().maximum());
            else
                vertical_scrollbar().set_position(vertical_scrollbar().maximum());
            break;
        default:
            handled = base_type::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            break;
        }
        return handled;
    }

    template <typename Base>
    const i_scrollbar& scrollable_widget<Base>::vertical_scrollbar() const
    {
        return iVerticalScrollbar;
    }

    template <typename Base>
    i_scrollbar& scrollable_widget<Base>::vertical_scrollbar()
    {
        return iVerticalScrollbar;
    }

    template <typename Base>
    const i_scrollbar& scrollable_widget<Base>::horizontal_scrollbar() const
    {
        return iHorizontalScrollbar;
    }

    template <typename Base>
    i_scrollbar& scrollable_widget<Base>::horizontal_scrollbar()
    {
        return iHorizontalScrollbar;
    }

    template <typename Base>
    void scrollable_widget<Base>::init_scrollbars()
    {
        if (base_type::device_metrics_available())
        {
            vertical_scrollbar().set_step(std::ceil(1.0_cm));
            horizontal_scrollbar().set_step(std::ceil(1.0_cm));
        }
    }

    template <typename Base>
    void scrollable_widget<Base>::init()
    {
        iSink = base_type::ChildAdded([&](i_widget& aWidget)
        {
            if (!iScrollbarUpdater)
                iScrollbarUpdater.emplace(service<i_async_task>(), [this](neolib::callback_timer&) { update_scrollbar_visibility(); }, 0);
        });
        iSink += base_type::ChildRemoved([&](i_widget& aWidget)
        {
            if (!iScrollbarUpdater)
                iScrollbarUpdater.emplace(service<i_async_task>(), [this](neolib::callback_timer&) { update_scrollbar_visibility(); }, 0);
        });
        init_scrollbars();
    }


    template <typename Base>
    scrolling_disposition scrollable_widget<Base>::scrolling_disposition() const
    {
        return neogfx::scrolling_disposition::ScrollChildWidgetVertically | neogfx::scrolling_disposition::ScrollChildWidgetHorizontally;
    }

    template <typename Base>
    scrolling_disposition scrollable_widget<Base>::scrolling_disposition(const i_widget&) const
    {
        return neogfx::scrolling_disposition::ScrollChildWidgetVertically | neogfx::scrolling_disposition::ScrollChildWidgetHorizontally;
    }

    template <typename Base>
    rect scrollable_widget<Base>::scroll_area() const
    {
        rect result = client_rect();
        if (horizontal_scrollbar().visible())
        {
            result.x = horizontal_scrollbar().minimum();
            result.cx = horizontal_scrollbar().maximum() - result.x;
        }
        if (vertical_scrollbar().visible())
        {
            result.y = vertical_scrollbar().minimum();
            result.cy = vertical_scrollbar().maximum() - result.y;
        }
        return result;
    }

    template <typename Base>
    rect scrollable_widget<Base>::scrollbar_geometry(const i_scrollbar& aScrollbar) const
    {
        auto const sbrect = as_widget().to_window_coordinates(as_widget().client_rect());
        switch (aScrollbar.type())
        {
        case scrollbar_type::Vertical:
            if (aScrollbar.style() == scrollbar_style::Normal)
                return rect{ sbrect.top_right(), size{ aScrollbar.width(), sbrect.cy } };
            else if (aScrollbar.style() == scrollbar_style::Scroller)
                return rect{ sbrect.bottom_left(),  size{ sbrect.width(), aScrollbar.width() * 2.0  } };
            else // scrollbar_style::Menu
                return sbrect + point{ 0.0, -aScrollbar.width() } + size{ 0.0, aScrollbar.width() * 2.0 };
        case scrollbar_type::Horizontal: 
            if (aScrollbar.style() == scrollbar_style::Normal)
                return rect{ sbrect.bottom_left(), size{ sbrect.cx , aScrollbar.width() } };
            else if (aScrollbar.style() == scrollbar_style::Scroller)
                return rect{ sbrect.top_right(),  size{ aScrollbar.width() * 2.0, sbrect.height() } };
            else // scrollbar_style::Menu
                return sbrect + point{ -aScrollbar.width(), 0.0 } + size{ aScrollbar.width() * 2.0, 0.0 };
        default:
            return rect{};
        }
    }

    template <typename Base>
    void scrollable_widget<Base>::scrollbar_updated(const i_scrollbar& aScrollbar, i_scrollbar::update_reason_e)
    {
        if (!iIgnoreScrollbarUpdates)
        {
            point scrollPosition = scroll_position();
            if (iOldScrollPosition != scrollPosition)
            {
                neolib::scoped_flag sf{ iMovingWidgets };
                for (auto& c : as_widget().children())
                {
                    point delta = -(scrollPosition - iOldScrollPosition);
                    if (aScrollbar.type() == scrollbar_type::Horizontal || (scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::DontScrollChildWidget)
                        delta.y = 0.0;
                    if (aScrollbar.type() == scrollbar_type::Vertical || (scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::DontScrollChildWidget)
                        delta.x = 0.0;
                    c->move(c->position() + delta);
                }
                if (aScrollbar.type() == scrollbar_type::Vertical)
                {
                    iOldScrollPosition.y = scrollPosition.y;
                }
                else if (aScrollbar.type() == scrollbar_type::Horizontal)
                {
                    iOldScrollPosition.x = scrollPosition.x;
                }
            }
        }
        as_widget().update(true);
    }

    template <typename Base>
    color scrollable_widget<Base>::scrollbar_color(const i_scrollbar&) const
    {
        return as_widget().palette_color(color_role::Foreground);
    }

    template <typename Base>
    const i_widget& scrollable_widget<Base>::as_widget() const
    {
        return *this;
    }

    template <typename Base>
    i_widget& scrollable_widget<Base>::as_widget()
    {
        return *this;
    }

    template <typename Base>
    void scrollable_widget<Base>::update_scrollbar_visibility()
    {
        if (!base_type::device_metrics_available())
            return;
        if (iUpdatingScrollbarVisibility)
            return;
        neolib::scoped_flag sf{ iUpdatingScrollbarVisibility };
        iScrollbarUpdater = {};
        if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
            vertical_scrollbar().lock(0.0);
        if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
            horizontal_scrollbar().lock(0.0);
        {
            neolib::scoped_counter<uint32_t> sc(iIgnoreScrollbarUpdates);
            update_scrollbar_visibility(UsvStageInit);
            if (as_widget().client_rect().cx > vertical_scrollbar().width() &&
                as_widget().client_rect().cy > horizontal_scrollbar().width())
            {
                update_scrollbar_visibility(UsvStageCheckVertical1);
                update_scrollbar_visibility(UsvStageCheckHorizontal);
                update_scrollbar_visibility(UsvStageCheckVertical2);
            }
            update_scrollbar_visibility(UsvStageDone);
        }
        if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
            vertical_scrollbar().unlock();
        if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
            horizontal_scrollbar().unlock();
    }

    template <typename Base>
    void scrollable_widget<Base>::update_scrollbar_visibility(usv_stage_e aStage)
    {
        if ((scrolling_disposition() & neogfx::scrolling_disposition::DontConsiderChildWidgets) == neogfx::scrolling_disposition::DontConsiderChildWidgets)
            return;
        switch (aStage)
        {
        case UsvStageInit:
            if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
                vertical_scrollbar().hide();
            if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
                horizontal_scrollbar().hide();
            as_widget().layout_items();
            break;
        case UsvStageCheckVertical1:
        case UsvStageCheckVertical2:
            if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
            {
                auto const& cr = as_widget().client_rect();
                for (auto& child : as_widget().children())
                {
                    auto const& childPos = child->position();
                    auto const& childExtents = child->extents();
                    if (child->hidden() || childExtents.cx == 0.0 || childExtents.cy == 0.0)
                        continue;
                    if ((scrolling_disposition(*child) & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::DontScrollChildWidget)
                        continue;
                    if (childPos.y < cr.top() || childPos.y + childExtents.cy > cr.bottom())
                    {
                        vertical_scrollbar().show();
                        as_widget().layout_items();
                        break;
                    }
                }
            }
            break;
        case UsvStageCheckHorizontal:
            if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
            {
                auto const& cr = as_widget().client_rect();
                for (auto& child : as_widget().children())
                {
                    auto const& childPos = child->position();
                    auto const& childExtents = child->extents();
                    if (child->hidden() || childExtents.cx == 0.0 || childExtents.cy == 0.0)
                        continue;
                    if ((scrolling_disposition(*child) & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::DontScrollChildWidget)
                        continue;
                    if (childPos.x < cr.left() || childPos.x + childExtents.cx > cr.right())
                    {
                        horizontal_scrollbar().show();
                        as_widget().layout_items();
                        break;
                    }
                }
            }
            break;
        case UsvStageDone:
            {
                point min{ std::numeric_limits<scalar>::max(), std::numeric_limits<scalar>::max() };
                point max{ std::numeric_limits<scalar>::min(), std::numeric_limits<scalar>::min() };
                for (auto& c : as_widget().children())
                {
                    if (c->hidden() || c->extents().cx == 0.0 || c->extents().cy == 0.0)
                        continue;
                    point const childTopLeftPos{ point{ units_converter{ *c }.to_device_units(c->position()) } };
                    point const childBottomRightPos{ point{ units_converter{ *c }.to_device_units(c->position() + c->extents()) - point{ 1.0, 1.0 } } };
                    if ((scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
                    {
                        min.x = std::min(min.x, childTopLeftPos.x);
                        max.x = std::max(max.x, childBottomRightPos.x);
                    }
                    if ((scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
                    {
                        min.y = std::min(min.y, childTopLeftPos.y);
                        max.y = std::max(max.y, childBottomRightPos.y);
                    }
                }
                min = min.min({});
                if (as_widget().has_layout())
                {
                    if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
                    {
                        min.x += as_widget().layout().padding().right;
                        max.x += as_widget().layout().padding().right;
                    }
                    if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
                    {
                        min.y += as_widget().layout().padding().bottom;
                        max.y += as_widget().layout().padding().bottom;
                    }
                }
                if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
                {
                    vertical_scrollbar().set_minimum(min.y);
                    vertical_scrollbar().set_maximum(max.y);
                    vertical_scrollbar().set_page(units_converter{ *this }.to_device_units(as_widget().client_rect()).height());
                }
                if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
                {
                    horizontal_scrollbar().set_minimum(min.x);
                    horizontal_scrollbar().set_maximum(max.x);
                    horizontal_scrollbar().set_page(units_converter{ *this }.to_device_units(as_widget().client_rect()).width());
                }
            }
            break;
        default:
            break;
        }
    }
}