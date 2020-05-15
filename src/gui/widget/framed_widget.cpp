// framed_widget.cpp
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
#include <neogfx/gui/widget/framed_widget.hpp>

namespace neogfx
{
    framed_widget::framed_widget(frame_style aStyle, dimension aLineWidth) :
        iStyle(aStyle), iLineWidth(aLineWidth)
    {
    }

    framed_widget::framed_widget(i_widget& aParent, frame_style aStyle, dimension aLineWidth) :
        widget(aParent), iStyle(aStyle), iLineWidth(aLineWidth)
    {
    }

    framed_widget::framed_widget(i_layout& aLayout, frame_style aStyle, dimension aLineWidth) :
        widget(aLayout), iStyle(aStyle), iLineWidth(aLineWidth)
    {
    }

    framed_widget::~framed_widget()
    {
    }

    rect framed_widget::client_rect(bool aIncludeMargins) const
    {
        rect cr = widget::client_rect(aIncludeMargins);
        cr.x += effective_frame_width();
        cr.y += effective_frame_width();
        cr.cx -= effective_frame_width() * 2.0;
        cr.cy -= effective_frame_width() * 2.0;
        return cr;
    }

    size framed_widget::minimum_size(const optional_size& aAvailableSpace) const
    {
        size result = widget::minimum_size(aAvailableSpace);
        if (!has_minimum_size())
            result += size{ effective_frame_width() * 2.0 };
        return result;
    }

    size framed_widget::maximum_size(const optional_size& aAvailableSpace) const
    {
        size result = widget::maximum_size(aAvailableSpace);
        if (!has_maximum_size())
        {
            if (result.cx != size::max_dimension())
                result.cx += effective_frame_width() * 2.0;
            if (result.cy != size::max_dimension())
                result.cy += effective_frame_width() * 2.0;
        }
        return result;
    }

    bool framed_widget::transparent_background() const
    {
        return false;
    }

    void framed_widget::paint_non_client(i_graphics_context& aGc) const
    {
        widget::paint_non_client(aGc);
        switch (iStyle)
        {
        case frame_style::NoFrame:
        case frame_style::HiddenFrame:
        default:
            break;
        case frame_style::DottedFrame:
            break;
        case frame_style::DashedFrame:
            break;
        case frame_style::SolidFrame:
        case frame_style::WindowFrame:
            aGc.draw_rect(rect{ point{ 0.0, 0.0 }, non_client_rect().extents() }, pen{ frame_color(), effective_frame_width() });
            break;
        case frame_style::ContainerFrame:
            {
                rect rectBorder{ point{ 0.0, 0.0 }, non_client_rect().extents() };
                rectBorder.deflate(line_width(), line_width());
                aGc.draw_rect(rectBorder, pen(inner_frame_color(), line_width()));
                rectBorder.inflate(line_width(), line_width());
                aGc.draw_rect(rectBorder, pen(frame_color(), line_width()));
            }
            break;
        case frame_style::DoubleFrame:
            break;
        case frame_style::GrooveFrame:
            break;
        case frame_style::RidgeFrame:
            break;
        case frame_style::InsetFrame:
            break;
        case frame_style::OutsetFrame:
            break;
        }
    }

    void framed_widget::paint(i_graphics_context& aGc) const
    {
        widget::paint(aGc);
    }

    void framed_widget::set_frame_style(frame_style aStyle)
    {
        if (iStyle != aStyle)
        {
            iStyle = aStyle;
            if (has_layout_manager())
                layout_manager().layout_items(true);
        }
    }

    bool framed_widget::has_frame_color() const
    {
        return iFrameColor != std::nullopt;
    }

    color framed_widget::frame_color() const
    {
        if (has_frame_color())
            return *iFrameColor;
        else if (iStyle != frame_style::ContainerFrame)
            return background_color().shade(0x60);
        else
            return (has_foreground_color() ? foreground_color() : container_background_color()).darker(0x40);
    }

    void framed_widget::set_frame_color(const optional_color& aFrameColor)
    {
        iFrameColor = aFrameColor;
        update();
    }

    color framed_widget::inner_frame_color() const
    {
        if (iStyle != frame_style::ContainerFrame)
            return frame_color();
        else
            return (has_foreground_color() ? foreground_color() : container_background_color()).lighter(0x40);
    }

    dimension framed_widget::line_width() const
    {
        return units_converter(*this).from_device_units(iLineWidth);
    }

    dimension framed_widget::effective_frame_width() const
    {
        switch (iStyle)
        {
        case frame_style::NoFrame:
        default:
            return 0.0;
        case frame_style::DottedFrame:
        case frame_style::DashedFrame:
        case frame_style::SolidFrame:
        case frame_style::WindowFrame:
            return line_width();
        case frame_style::ContainerFrame:
            return line_width() * 2.0;
        case frame_style::DoubleFrame:
        case frame_style::GrooveFrame:
        case frame_style::RidgeFrame:
            return line_width() * 3.0;
        case frame_style::InsetFrame:
        case frame_style::OutsetFrame:
        case frame_style::HiddenFrame:
            return line_width();
        }
    }
}