// framed_widget.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include "widget.hpp"

namespace neogfx
{
    enum class frame_style
    {
        NoFrame,
        DottedFrame,
        DashedFrame,
        SolidFrame,
        ContainerFrame,
        DoubleFrame,
        GrooveFrame,
        RidgeFrame,
        InsetFrame,
        OutsetFrame,
        HiddenFrame,
        WindowFrame
    };

    template <typename Base = widget>
    class framed_widget : public Base
    {
        typedef framed_widget<Base> self_type;
        typedef Base base_type;
    public:
        framed_widget(frame_style aStyle = frame_style::SolidFrame, dimension aLineWidth = 1.0) :
            base_type{}, iStyle(aStyle), iLineWidth(aLineWidth)
        {
        }
        framed_widget(i_widget& aParent, frame_style aStyle = frame_style::SolidFrame, dimension aLineWidth = 1.0) :
            base_type(aParent), iStyle(aStyle), iLineWidth(aLineWidth)
        {
        }
        framed_widget(i_layout& aLayout, frame_style aStyle = frame_style::SolidFrame, dimension aLineWidth = 1.0) :
            base_type(aLayout), iStyle(aStyle), iLineWidth(aLineWidth)
        {
        }
    public:
        rect client_rect(bool aIncludePadding = true) const override
        {
            rect cr = base_type::client_rect(aIncludePadding);
            auto const effectiveFrameWidth = effective_frame_width();
            cr.x += effectiveFrameWidth;
            cr.y += effectiveFrameWidth;
            cr.cx -= effectiveFrameWidth * 2.0;
            cr.cy -= effectiveFrameWidth * 2.0;
            return cr;
        }
    public:
        size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override
        {
            size result = base_type::minimum_size(aAvailableSpace);
            if (!base_type::has_minimum_size())
                result += size{ effective_frame_width() * 2.0 };
            return result;
        }
        size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override
        {
            size result = base_type::maximum_size(aAvailableSpace);
            if (!base_type::has_maximum_size())
            {
                if (result.cx != size::max_dimension())
                    result.cx += effective_frame_width() * 2.0;
                if (result.cy != size::max_dimension())
                    result.cy += effective_frame_width() * 2.0;
            }
            return result;
        }
    public:
        bool transparent_background() const override
        {
            return false;
        }
        void paint_non_client(i_graphics_context& aGc) const override
        {
            base_type::paint_non_client(aGc);
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
                aGc.draw_rect(rect{ point{ 0.0, 0.0 }, base_type::non_client_rect().extents() }, pen{ frame_color(), effective_frame_width() });
                break;
            case frame_style::ContainerFrame:
            {
                rect rectBorder{ point{ 0.0, 0.0 }, base_type::non_client_rect().extents() };
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
    public:
        void set_frame_style(frame_style aStyle)
        {
            if (iStyle != aStyle)
            {
                iStyle = aStyle;
                if (base_type::has_layout_manager())
                    base_type::layout_manager().layout_items(true);
            }
        }
    public:
        virtual bool has_frame_color() const
        {
            return iFrameColor != std::nullopt;
        }
        virtual color frame_color() const
        {
            if (has_frame_color())
                return *iFrameColor;
            else if (iStyle != frame_style::ContainerFrame)
                return base_type::background_color().shade(0x60);
            else
                return (base_type::has_foreground_color() ? base_type::foreground_color() : base_type::container_background_color()).darker(0x40);
        }
        virtual void set_frame_color(const optional_color& aFrameColor = optional_color{})
        {
            iFrameColor = aFrameColor;
            base_type::update();
        }
        virtual color inner_frame_color() const
        {
            if (iStyle != frame_style::ContainerFrame)
                return frame_color();
            else
                return (base_type::has_foreground_color() ? base_type::foreground_color() : base_type::container_background_color()).lighter(0x40);
        }
    public:
        dimension line_width() const
        {
            return units_converter(*this).from_device_units(iLineWidth);
        }
        dimension effective_frame_width() const
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
    private:
        frame_style iStyle;
        dimension iLineWidth;
        optional_color iFrameColor;
    };
}