// scrollable_widget.ipp
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
    template <Widget Base>
    inline scrollable_widget<Base>::~scrollable_widget()
    {
        service<i_scrollbar_container_updater>().unqueue(*this);
    }

    template <Widget Base>
    inline scrollbar_style scrollable_widget<Base>::scrollbar_style() const
    {
        return iScrollbarStyle;
    }

    template <Widget Base>
    inline point scrollable_widget<Base>::scroll_position() const
    {
        return units_converter{ *this }.from_device_units(point(static_cast<coordinate>(horizontal_scrollbar().position()), static_cast<coordinate>(vertical_scrollbar().position())));
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::scroll_to(i_widget& aChild)
    {
        (void)aChild;
        /* todo */
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::layout_items_started()
    {
        base_type::layout_items_started();
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::layout_items_completed()
    {
        base_type::layout_items_completed();
        if (!base_type::as_widget().layout_items_in_progress() && !iIgnoreScrollbarUpdates && !iMovingWidgets)
        {
            scoped_property_transition_suppression sts1{ iVerticalScrollbar.Position };
            scoped_property_transition_suppression sts2{ iHorizontalScrollbar.Position };
            update_scrollbar_visibility();
        }
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::resized()
    {
        base_type::resized();
        if (!base_type::as_widget().layout_items_in_progress() && !iIgnoreScrollbarUpdates)
        {
            scoped_property_transition_suppression sts1{ iVerticalScrollbar.Position };
            scoped_property_transition_suppression sts2{ iHorizontalScrollbar.Position };
            update_scrollbar_visibility();
        }
    }

    template <Widget Base>
    inline rect scrollable_widget<Base>::client_rect(bool aExtendIntoPadding) const
    {
        rect result = base_type::client_rect(aExtendIntoPadding);
        if (vertical_scrollbar().visible())
        {
            if (vertical_scrollbar().type() == scrollbar_style::Normal)
                result.cx -= vertical_scrollbar().width();
            else if (vertical_scrollbar().type() == scrollbar_style::Menu)
            {
                result.y += vertical_scrollbar().width();
                result.cy -= vertical_scrollbar().width() * 2.0;
            }
            else if (vertical_scrollbar().type() == scrollbar_style::Scroller)
                result.cy -= vertical_scrollbar().width() * 2.0;
        }
        if (horizontal_scrollbar().visible())
        {
            if (horizontal_scrollbar().type() == scrollbar_style::Normal)
                result.cy -= horizontal_scrollbar().width();
            else if (vertical_scrollbar().type() == scrollbar_style::Menu)
            {
                result.x += horizontal_scrollbar().width();
                result.cx -= horizontal_scrollbar().width() * 2.0;
            }
            else if (vertical_scrollbar().type() == scrollbar_style::Scroller)
                result.cx -= horizontal_scrollbar().width() * 2.0;
        }
        return result;
    }

    template <Widget Base>
    inline view scrollable_widget<Base>::view(bool aExtendIntoPadding) const
    {
        neogfx::view result = base_type::view(aExtendIntoPadding);
        if (!aExtendIntoPadding)
            result.set_viewport(scroll_page());
        result.set_view(scroll_area());
        return result;
    }

    template <Widget Base>
    inline widget_part scrollable_widget<Base>::part(const point& aPosition) const
    {
        if (vertical_scrollbar().visible() &&
            ((vertical_scrollbar().element_at(aPosition) != scrollbar_element::None) ||
            (vertical_scrollbar().type() == scrollbar_style::Normal && scrollbar_geometry(vertical_scrollbar()).contains(aPosition))))
            return widget_part{ *this, widget_part::VerticalScrollbar };
        else if (horizontal_scrollbar().visible() &&
            ((horizontal_scrollbar().element_at(aPosition) != scrollbar_element::None) ||
            (horizontal_scrollbar().type() == scrollbar_style::Normal && scrollbar_geometry(horizontal_scrollbar()).contains(aPosition))))
            return widget_part{ *this, widget_part::HorizontalScrollbar };
        else
            return base_type::part(aPosition);
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::paint_non_client_after(i_graphics_context& aGc) const
    {
        base_type::paint_non_client_after(aGc);
        if (vertical_scrollbar().visible() && !vertical_scrollbar().auto_hidden())
            vertical_scrollbar().render(aGc);
        if (horizontal_scrollbar().visible() && !horizontal_scrollbar().auto_hidden())
            horizontal_scrollbar().render(aGc);
        if (vertical_scrollbar().visible() && horizontal_scrollbar().visible() && 
            !vertical_scrollbar().auto_hidden() && !horizontal_scrollbar().auto_hidden() &&
            vertical_scrollbar().type() == horizontal_scrollbar().type() && vertical_scrollbar().type() == scrollbar_style::Normal)
        {
            auto const spareSquare = rect{
                    point{ scrollbar_geometry(horizontal_scrollbar()).right(), scrollbar_geometry(vertical_scrollbar()).bottom() },
                    size{ scrollbar_geometry(vertical_scrollbar()).width(), scrollbar_geometry(horizontal_scrollbar()).height() } };
            aGc.fill_rect(spareSquare, scrollbar_color(vertical_scrollbar()));
        }
    }

    template <Widget Base>
    inline bool scrollable_widget<Base>::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers)
    {
        scoped_property_transition_suppression sts1{ iVerticalScrollbar.Position };
        scoped_property_transition_suppression sts2{ iHorizontalScrollbar.Position };
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
            handledVertical = vertical_scrollbar().set_position(
                vertical_scrollbar().position() + 
                std::min(std::max(((verticalSense == mouse_wheel::Vertical ? aDelta.dy : aDelta.dx) >= 0.0 ? -maxSteps : maxSteps) * vertical_scrollbar().step(), -maxDeltaY), maxDeltaY));
        if ((aWheel & horizontalSense) != mouse_wheel::None && horizontal_scrollbar().visible())
            handledHorizontal = horizontal_scrollbar().set_position(
                horizontal_scrollbar().position() + 
                std::min(std::max(((horizontalSense == mouse_wheel::Horizontal ? aDelta.dx : aDelta.dx) >= 0.0 ? -maxSteps : maxSteps) * horizontal_scrollbar().step(), -maxDeltaX), maxDeltaX));
        mouse_wheel passOn = static_cast<mouse_wheel>(
            aWheel & ((handledVertical ? ~verticalSense : verticalSense) | (handledHorizontal ? ~horizontalSense : horizontalSense)));
        if (passOn != mouse_wheel::None)
            return base_type::mouse_wheel_scrolled(passOn, aPosition, aDelta, aKeyModifiers);
        else
            return true;
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
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
                base_type::as_widget().set_capture();
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
                    if (vertical_scrollbar().visible() && vertical_scrollbar().element_at(aPosition) != scrollbar_element::None)
                    {
                        base_type::as_widget().update(true);
                        vertical_scrollbar().click_element(vertical_scrollbar().element_at(aPosition));
                    }
                    else if (horizontal_scrollbar().visible() && horizontal_scrollbar().element_at(aPosition) != scrollbar_element::None)
                    {
                        base_type::as_widget().update(true);
                        horizontal_scrollbar().click_element(horizontal_scrollbar().element_at(aPosition));
                    }
                }
            }
        }
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            if (vertical_scrollbar().clicked_element() == scrollbar_element::None && horizontal_scrollbar().clicked_element() == scrollbar_element::None)
            {
                if (vertical_scrollbar().visible() && vertical_scrollbar().element_at(aPosition) != scrollbar_element::None)
                {
                    base_type::as_widget().update(true);
                    vertical_scrollbar().click_element(vertical_scrollbar().element_at(aPosition));
                }
                else if (horizontal_scrollbar().visible() && horizontal_scrollbar().element_at(aPosition) != scrollbar_element::None)
                {
                    base_type::as_widget().update(true);
                    horizontal_scrollbar().click_element(horizontal_scrollbar().element_at(aPosition));
                }
            }
        }
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        base_type::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left)
        {
            if (vertical_scrollbar().clicked_element() != scrollbar_element::None)
            {
                base_type::as_widget().update(true);
                vertical_scrollbar().unclick_element();
            }
            else if (horizontal_scrollbar().clicked_element() != scrollbar_element::None)
            {
                base_type::as_widget().update(true);
                horizontal_scrollbar().unclick_element();
            }
        }
        else if (aButton == mouse_button::Middle)
        {
            vertical_scrollbar().untrack();
            horizontal_scrollbar().untrack();
        }
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_moved(aPosition, aKeyModifiers);
        vertical_scrollbar().update(aPosition);
        horizontal_scrollbar().update(aPosition);
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::mouse_entered(const point& aPosition)
    {
        base_type::mouse_entered(aPosition);
        vertical_scrollbar().update();
        horizontal_scrollbar().update();
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::mouse_left()
    {
        base_type::mouse_left();
        vertical_scrollbar().update();
        horizontal_scrollbar().update();
    }

    template <Widget Base>
    inline bool scrollable_widget<Base>::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        bool handled = true;
        switch (aScanCode)
        {
        case ScanCode_LEFT:
            horizontal_scrollbar().set_position(horizontal_scrollbar().Position.effective_value() - horizontal_scrollbar().step());
            break;
        case ScanCode_RIGHT:
            horizontal_scrollbar().set_position(horizontal_scrollbar().Position.effective_value() + horizontal_scrollbar().step());
            break;
        case ScanCode_UP:
            vertical_scrollbar().set_position(vertical_scrollbar().Position.effective_value() - vertical_scrollbar().step());
            break;
        case ScanCode_DOWN:
            vertical_scrollbar().set_position(vertical_scrollbar().Position.effective_value() + vertical_scrollbar().step());
            break;
        case ScanCode_PAGEUP:
            vertical_scrollbar().set_position(vertical_scrollbar().Position.effective_value() - vertical_scrollbar().page());
            break;
        case ScanCode_PAGEDOWN:
            vertical_scrollbar().set_position(vertical_scrollbar().Position.effective_value() + vertical_scrollbar().page());
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

    template <Widget Base>
    inline const scrollbar& scrollable_widget<Base>::vertical_scrollbar() const
    {
        return iVerticalScrollbar;
    }

    template <Widget Base>
    inline scrollbar& scrollable_widget<Base>::vertical_scrollbar()
    {
        return iVerticalScrollbar;
    }

    template <Widget Base>
    inline const scrollbar& scrollable_widget<Base>::horizontal_scrollbar() const
    {
        return iHorizontalScrollbar;
    }

    template <Widget Base>
    inline scrollbar& scrollable_widget<Base>::horizontal_scrollbar()
    {
        return iHorizontalScrollbar;
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::init_scrollbars()
    {
        if (base_type::device_metrics_available())
        {
            vertical_scrollbar().set_step(std::ceil(1.0_cm));
            horizontal_scrollbar().set_step(std::ceil(1.0_cm));
        }
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::init()
    {
        auto async_update = [&]()
        {
            if (!iScrollbarUpdater && !iSuppressScrollbarVisibilityUpdates)
                iScrollbarUpdater.emplace(*this, [this](widget_timer&)
                {
                    update_scrollbar_visibility();
                }, std::chrono::seconds{});
        };
        iSink += base_type::ChildAdded([&, async_update](i_widget& aWidget)
        {
            async_update();
        });
        iSink += base_type::ChildRemoved([&, async_update](i_widget& aWidget)
        {
            async_update();
        });
        init_scrollbars();
    }

    template <Widget Base>
    inline scrolling_disposition scrollable_widget<Base>::scrolling_disposition() const
    {
        return neogfx::scrolling_disposition::ScrollChildWidgetVertically | neogfx::scrolling_disposition::ScrollChildWidgetHorizontally;
    }

    template <Widget Base>
    inline scrolling_disposition scrollable_widget<Base>::scrolling_disposition(const i_widget&) const
    {
        return neogfx::scrolling_disposition::ScrollChildWidgetVertically | neogfx::scrolling_disposition::ScrollChildWidgetHorizontally;
    }

    template <Widget Base>
    inline rect scrollable_widget<Base>::scroll_area() const
    {
        scoped_units su{ *this, units::Pixels };
        
        auto const cr = client_rect(false);
        
        rect result = cr;

        if (iUpdatingScrollbarVisibility)
        {
            auto& self = base_type::as_widget();

            optional_point min;
            optional_point max;

            for (auto& c : self.children())
            {
                if (c->hidden() || c->extents().cx == 0.0 || c->extents().cy == 0.0)
                    continue;
                if (!min)
                    min.emplace(std::numeric_limits<scalar>::infinity(), std::numeric_limits<scalar>::infinity());
                if (!max)
                    max.emplace(-std::numeric_limits<scalar>::infinity(), -std::numeric_limits<scalar>::infinity());
                point const childTopLeftPos{ point{ units_converter{ *c }.to_device_units(c->position()) } };
                point const childBottomRightPos{ point{ units_converter{ *c }.to_device_units(c->position() + units_converter{ *c }.to_device_units(c->extents())) - point{ 1.0, 1.0 } } };
                if ((scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
                {
                    min->x = std::min(min->x, childTopLeftPos.x);
                    max->x = std::max(max->x, childBottomRightPos.x);
                }
                if ((scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
                {
                    min->y = std::min(min->y, childTopLeftPos.y);
                    max->y = std::max(max->y, childBottomRightPos.y);
                }
            }

            if (!min)
                min = cr.top_left();
            if (!max)
                max = cr.top_left();

            if (self.has_layout())
            {
                auto const& ourLayout = self.layout();
                if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
                    max->x += ourLayout.internal_spacing().right;
                if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
                    max->y += ourLayout.internal_spacing().bottom;
            }
      
            if (max->x > cr.right() - 1.0 - self.internal_spacing().right)
                max->x += self.internal_spacing().right;
            if (max->y > cr.bottom() - 1.0 - self.internal_spacing().bottom)
                max->y += self.internal_spacing().bottom;

            min = min->min(cr.top_left());
            max = max->max(cr.bottom_right()) - point{ 1.0, 1.0 };
            
            result = rect{ *min, *max };
        }
        else
        {
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
        }

        return to_units(*this, su.saved_units(), result);
    }
    template <Widget Base>
    inline rect scrollable_widget<Base>::scroll_page() const
    {
        return client_rect();
    }

    template <Widget Base>
    inline rect scrollable_widget<Base>::scrollbar_geometry(const i_scrollbar& aScrollbar) const
    {
        auto const sbrect = client_rect();
        switch (aScrollbar.orientation())
        {
        case scrollbar_orientation::Vertical:
            if (aScrollbar.type() == scrollbar_style::Normal)
                return rect{ sbrect.top_right(), size{ aScrollbar.width(), sbrect.cy } };
            else if (aScrollbar.type() == scrollbar_style::Scroller)
                return rect{ sbrect.bottom_left(),  size{ sbrect.width(), aScrollbar.width() * 2.0  } };
            else if (aScrollbar.type() == scrollbar_style::Menu)
                return sbrect + point{ 0.0, -aScrollbar.width() } + size{ 0.0, aScrollbar.width() * 2.0 };
            return rect{};
        case scrollbar_orientation::Horizontal:
            if (aScrollbar.type() == scrollbar_style::Normal)
                return rect{ sbrect.bottom_left(), size{ sbrect.cx , aScrollbar.width() } };
            else if (aScrollbar.type() == scrollbar_style::Scroller)
                return rect{ sbrect.top_right(),  size{ aScrollbar.width() * 2.0, sbrect.height() } };
            else if (aScrollbar.type() == scrollbar_style::Menu)
                return sbrect + point{ -aScrollbar.width(), 0.0 } + size{ aScrollbar.width() * 2.0, 0.0 };
            return rect{};
        default:
            return rect{};
        }
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::scrollbar_updated(const i_scrollbar& aScrollbar, i_scrollbar::update_reason_e)
    {
        if (!iIgnoreScrollbarUpdates)
        {
            point scrollPosition = scroll_position();
            if (iOldScrollPosition != scrollPosition)
            {
                neolib::scoped_flag sf{ iMovingWidgets };
                for (auto& c : base_type::as_widget().children())
                {
                    point delta = -(scrollPosition - iOldScrollPosition);
                    if (aScrollbar.orientation() == scrollbar_orientation::Horizontal || (scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::DontScrollChildWidget)
                        delta.y = 0.0;
                    if (aScrollbar.orientation() == scrollbar_orientation::Vertical || (scrolling_disposition(*c) & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::DontScrollChildWidget)
                        delta.x = 0.0;
                    c->move(c->position() + delta);
                }
                if (aScrollbar.orientation() == scrollbar_orientation::Vertical)
                {
                    iOldScrollPosition.y = scrollPosition.y;
                }
                else if (aScrollbar.orientation() == scrollbar_orientation::Horizontal)
                {
                    iOldScrollPosition.x = scrollPosition.x;
                }
            }
        }
        base_type::as_widget().update(true);
    }

    template <Widget Base>
    inline color scrollable_widget<Base>::scrollbar_color(const i_scrollbar&) const
    {
        return base_type::as_widget().palette_color(color_role::Foreground);
    }

    template <Widget Base>
    inline i_widget const& scrollable_widget<Base>::as_widget() const
    {
        return *this;
    }

    template <Widget Base>
    i_widget& scrollable_widget<Base>::as_widget()
    {
        return *this;
    }

    template <Widget Base>
    inline bool scrollable_widget<Base>::use_scrollbar_container_updater() const
    {
        return true;
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::update_scrollbar_visibility()
    {
        if (!base_type::device_metrics_available())
            return;

        if (iUpdatingScrollbarVisibility)
            return;

#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "widget:layout_items: update_scrollbar_visibility..."  << endl;
#endif

        neolib::scoped_flag sf{ iUpdatingScrollbarVisibility };

        scoped_units su{ *this, units::Pixels };

        if (use_scrollbar_container_updater())
        {
            auto& updater = service<i_scrollbar_container_updater>();
            if (!updater.processing())
            {
                neolib::scoped_counter<uint32_t> sc(iIgnoreScrollbarUpdates);
                update_scrollbar_visibility(UsvStageInit);
                updater.queue(*this);
                return;
            }
            else if (&updater.current() != this)
            {
                return;
            }
        }

        iScrollbarUpdater = {};
        if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetVertically) == neogfx::scrolling_disposition::ScrollChildWidgetVertically)
            vertical_scrollbar().lock(0.0);
        if ((scrolling_disposition() & neogfx::scrolling_disposition::ScrollChildWidgetHorizontally) == neogfx::scrolling_disposition::ScrollChildWidgetHorizontally)
            horizontal_scrollbar().lock(0.0);
        {
            neolib::scoped_counter<uint32_t> sc(iIgnoreScrollbarUpdates);
            update_scrollbar_visibility(UsvStageInit);
            auto const& cr = base_type::as_widget().client_rect();
            if (cr.cx > vertical_scrollbar().width() &&
                cr.cy > horizontal_scrollbar().width())
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

        scrollbar_updated(vertical_scrollbar(), i_scrollbar::update_reason_e::Updated);
        scrollbar_updated(horizontal_scrollbar(), i_scrollbar::update_reason_e::Updated);
        
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
        {
            auto const scrollArea = scroll_area();
            auto const scrollPage = scroll_page();
            service<debug::logger>() << neolib::logger::severity::Debug << "widget:layout_items: update_scrollbar_visibility: scroll_area: " << scrollArea << ", scroll_page: " << scrollPage << endl;
        }
#endif
    }

    template <Widget Base>
    inline bool scrollable_widget<Base>::update_scrollbar_visibility(usv_stage_e aStage)
    {
        bool updatePage = false; 

        switch (aStage)
        {
        case UsvStageInit:
            if (vertical_scrollbar().visible())
            {
                vertical_scrollbar().hide();
                updatePage = true;
            }
            if (horizontal_scrollbar().visible())
            {
                horizontal_scrollbar().hide();
                updatePage = true;
            }
            break;
        case UsvStageCheckVertical1:
        case UsvStageCheckVertical2:
            if (scroll_area().cy > scroll_page().cy)
            {
                if (!vertical_scrollbar().visible())
                {
                    vertical_scrollbar().show();
                    updatePage = true;
                }
            }
            break;
        case UsvStageCheckHorizontal:
            if (scroll_area().cx > scroll_page().cx)
            {
                if (!horizontal_scrollbar().visible())
                {
                    horizontal_scrollbar().show();
                    updatePage = true;
                }
                break;
            }
            break;
        case UsvStageDone:
            {
                auto const& sa = units_converter{ *this }.to_device_units(scroll_area());
                auto const& sp = units_converter{ *this }.to_device_units(scroll_page());
                neolib::scoped_counter<uint32_t> sc(iIgnoreScrollbarUpdates);
                vertical_scrollbar().set_minimum(sa.top());
                vertical_scrollbar().set_maximum(sa.bottom());
                vertical_scrollbar().set_page(sp.cy);
                horizontal_scrollbar().set_minimum(sa.left());
                horizontal_scrollbar().set_maximum(sa.right());
                horizontal_scrollbar().set_page(sp.cx);
                iOldScrollPosition = scroll_position();
            }
            break;
        default:
            break;
        }

        if (updatePage)
            scroll_page_updated();

        return updatePage;
    }

    template <Widget Base>
    inline void scrollable_widget<Base>::scroll_page_updated()
    {
        base_type::as_widget().layout_items();
    }
}