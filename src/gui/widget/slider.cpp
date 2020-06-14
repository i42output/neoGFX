// slider.cpp
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
#include <neogfx/gui/widget/slider.hpp>
#include <neogfx/app/i_app.hpp>

namespace neogfx
{
    slider_impl::slider_impl(slider_orientation aOrientation) :
        iOrientation(aOrientation),
        iNormalizedValue{ 0.0 }, 
        iHandlingEvent{ false }
    {
        init();
    }

    slider_impl::slider_impl(i_widget& aParent, slider_orientation aOrientation) :
        widget(aParent),
        iOrientation(aOrientation),
        iNormalizedValue{ 0.0 },
        iHandlingEvent{ false }
    {
        init();
    }

    slider_impl::slider_impl(i_layout& aLayout, slider_orientation aOrientation) :
        widget(aLayout),
        iOrientation(aOrientation),
        iNormalizedValue{ 0.0 },
        iHandlingEvent{ false }
    {
        init();
    }

    slider_impl::~slider_impl()
    {
    }

    void slider_impl::set_bar_color(const optional_color_or_gradient& aBarColor)
    {
        iBarColor = aBarColor;
    }

    size slider_impl::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        return iOrientation == slider_orientation::Horizontal ? size{ 80_dip, 22_dip } : size{ 22_dip, 80_dip };
    }

    void slider_impl::paint(i_graphics_context& aGc) const
    {
        scoped_units su{ *this, units::Pixels };
        rect rectBarBox = bar_box();
        color ink = background_color().shade(0x80);
        aGc.fill_rounded_rect(rectBarBox, 2.0, to_brush(iBarColor ? *iBarColor : ink));
        rectBarBox.inflate(size{ 1.0, 1.0 });
        aGc.fill_rounded_rect(rectBarBox, 2.0, to_brush(iBarColor ? *iBarColor : ink.mid(background_color())));
        rectBarBox.deflate(size{ 1.0, 1.0 });
        point selection = normalized_value_to_position(normalized_value());
        rect selectionRect = rectBarBox;
        if (iOrientation == slider_orientation::Horizontal)
            selectionRect.cx = selection.x - selectionRect.x;
        else
        {
            selectionRect.cy = selectionRect.bottom() - selection.y;
            selectionRect.y = selection.y;
        }
        {
            scoped_scissor ss{ aGc, selectionRect };
            if (normalized_value() > 0.0)
                aGc.fill_rounded_rect(rectBarBox, 2.0, to_brush(iBarColor ? *iBarColor : service<i_app>().current_style().palette().color(color_role::Selection)));
        }
        rect rectIndicator = indicator_box();
        color indicatorColor = foreground_color();
        if (iDragOffset != std::nullopt)
        {
            if (indicatorColor.light(0x40))
                indicatorColor.darken(0x40);
            else
                indicatorColor.lighten(0x40);
        }
        color indicatorBorderColor = indicatorColor.darker(0x40);
        indicatorColor.lighten(0x40);
        aGc.fill_circle(rectIndicator.center(), rectIndicator.width() / 2.0, indicatorBorderColor);
        aGc.fill_circle(rectIndicator.center(), rectIndicator.width() / 2.0 - 1.0, indicatorColor);
    }

    void slider_impl::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            neolib::scoped_flag sf{ iHandlingEvent };
            if (indicator_box().contains(aPosition))
            {
                iDragOffset = aPosition - indicator_box().center();
                if (iOrientation == slider_orientation::Horizontal)
                    set_normalized_value(normalized_value_from_position(point{ aPosition.x - iDragOffset->x, aPosition.y }));
                else
                    set_normalized_value(normalized_value_from_position(point{ aPosition.x, aPosition.y - iDragOffset->y }));
                update();
            }
            else
                set_normalized_value(normalized_value_from_position(aPosition));
        }
    }

    void slider_impl::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            neolib::scoped_flag sf{ iHandlingEvent };
            set_normalized_value(0.5);
        }
    }

    void slider_impl::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        widget::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left)
        {
            iDragOffset = std::nullopt;
            update();
        }
    }

    void slider_impl::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers)
    {
        if (aWheel == mouse_wheel::Vertical)
        {
            neolib::scoped_flag sf{ iHandlingEvent };
            set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() + (aDelta.dy * normalized_step_value()))));
        }
        else
            widget::mouse_wheel_scrolled(aWheel, aPosition, aDelta, aKeyModifiers);
    }

    void slider_impl::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_moved(aPosition, aKeyModifiers);
        if (iDragOffset != std::nullopt)
        {
            neolib::scoped_flag sf{ iHandlingEvent };
            if (iOrientation == slider_orientation::Horizontal)
                set_normalized_value(normalized_value_from_position(point{ aPosition.x - iDragOffset->x, aPosition.y }));
            else
                set_normalized_value(normalized_value_from_position(point{ aPosition.x, aPosition.y - iDragOffset->y }));
        }
    }

    void slider_impl::set_normalized_value(double aValue)
    {
        aValue = std::max(0.0, std::min(1.0, aValue));
        if (iNormalizedValue != aValue)
        {
            iNormalizedValue = aValue;
            update();
        }
    }

    bool slider_impl::handling_event() const
    {
        return iHandlingEvent;
    }

    void slider_impl::init()
    {
        ValueChanged.set_trigger_type(event_trigger_type::Asynchronous);
        ConstraintsChanged.set_trigger_type(event_trigger_type::Asynchronous);

        set_margins(neogfx::margins{});
        if (iOrientation == slider_orientation::Horizontal)
            set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        else
            set_size_policy(neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding });
    }

    rect slider_impl::bar_box() const
    {
        rect result = client_rect(false);
        result.deflate(size{ std::ceil((iOrientation == slider_orientation::Horizontal ? result.height() : result.width()) / 2.5) });
        result.deflate(size{ 1.0, 1.0 });
        return result;
    }

    rect slider_impl::indicator_box() const
    {
        rect result{ normalized_value_to_position(normalized_value()), size{} };
        result.inflate(size{ std::ceil((iOrientation == slider_orientation::Horizontal ? client_rect(false).height() : client_rect(false).width()) / 3.0) });
        return result;
    }

    double slider_impl::normalized_value_from_position(const point& aPosition) const
    {
        if (iOrientation == slider_orientation::Horizontal)
            return std::max(0.0, std::min(aPosition.x - bar_box().x, bar_box().right())) / bar_box().cx;
        else
            return 1.0 - std::max(0.0, std::min(aPosition.y - bar_box().y, bar_box().bottom())) / bar_box().cy;
    }

    point slider_impl::normalized_value_to_position(double aValue) const
    {
        const rect rectBarBox = bar_box();
        if (iOrientation == slider_orientation::Horizontal)
            return point{ rectBarBox.x + rectBarBox.cx * aValue, rectBarBox.center().y };
        else
            return point{ rectBarBox.center().x, rectBarBox.y + rectBarBox.cy * (1.0 - aValue) };
    }
}

