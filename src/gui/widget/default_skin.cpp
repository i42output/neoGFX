// default_skin.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/default_skin.hpp>

namespace neogfx
{
    const std::string& default_skin::name() const
    {
        static const std::string sName = "Default";
        return sName;
    }

    bool default_skin::has_fallback() const
    {
        return false;
    }

    i_skin& default_skin::fallback()
    {
        throw no_fallback();
    }

    void default_skin::activate()
    {
    }

    void default_skin::deactivate()
    {
    }

    void default_skin::draw_check_box(i_graphics_context& aGraphicsContext, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const
    {
        auto const& widget = aItem.as_widget();
        scoped_units su{ widget, units::Pixels };
        rect boxRect = aItem.draw_rect();
        auto enabledAlphaCoefficient = widget.effectively_enabled() ? 1.0 : 0.25;
        color hoverColor = service<i_app>().current_style().palette().hover_color().same_lightness_as(
            widget.background_color().dark() ?
                widget.background_color().lighter(0x20) :
                widget.background_color().darker(0x20));
        if (widget.capturing())
            widget.background_color().dark() ? hoverColor.lighten(0x20) : hoverColor.darken(0x20);
        color fillColor = widget.enabled() && aItem.click_rect().contains(widget.root().mouse_position() - widget.origin()) ?
            hoverColor : widget.background_color();
        aGraphicsContext.fill_rect(boxRect, fillColor.with_combined_alpha(enabledAlphaCoefficient));
        color borderColor1 = widget.container_background_color().mid(widget.container_background_color().mid(widget.background_color()));
        if (borderColor1.similar_intensity(widget.container_background_color(), 0.03125))
            borderColor1.dark() ? borderColor1.lighten(0x40) : borderColor1.darken(0x40);
        aGraphicsContext.draw_rect(boxRect, pen{ borderColor1.with_combined_alpha(enabledAlphaCoefficient), 1.0 });
        boxRect.deflate(1.0, 1.0);
        aGraphicsContext.draw_rect(boxRect, pen{ borderColor1.mid(widget.background_color()).with_combined_alpha(enabledAlphaCoefficient), 1.0 });
        boxRect.deflate(2.0, 2.0);
        if (aCheckedState != std::nullopt && *aCheckedState == true)
        {
            scoped_snap_to_pixel snap{ aGraphicsContext, false };
            /* todo: draw tick image eye candy */
            aGraphicsContext.draw_line(boxRect.top_left(), boxRect.bottom_right(), pen(service<i_app>().current_style().palette().widget_detail_primary_color().with_combined_alpha(enabledAlphaCoefficient), 2.0));
            aGraphicsContext.draw_line(boxRect.bottom_left(), boxRect.top_right(), pen(service<i_app>().current_style().palette().widget_detail_primary_color().with_combined_alpha(enabledAlphaCoefficient), 2.0));
        }
        else if (aCheckedState == std::nullopt)
            aGraphicsContext.fill_rect(boxRect, service<i_app>().current_style().palette().widget_detail_primary_color().with_combined_alpha(enabledAlphaCoefficient));
    }
}