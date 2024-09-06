// framed_widget.ipp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/framed_widget.hpp>

namespace neogfx
{
    template <Widget Base>
    inline neogfx::border framed_widget<Base>::border() const
    {
        return neogfx::border{ effective_frame_width(), effective_frame_width(), effective_frame_width(), effective_frame_width() };
    }

    template <Widget Base>
    inline void framed_widget<Base>::paint_non_client(i_graphics_context& aGc) const
    {
#ifdef NEOGFX_DEBUG
        if (debug::renderItem == this)
        {
            aGc.flush();
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::paint_non_client(), frame_color: " << frame_color() << std::endl;
        }
#endif // NEOGFX_DEBUG

        if (!has_frame_radius())
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
            if (!has_frame_radius())
                aGc.draw_rect(rect{ point{ 0.0, 0.0 }, base_type::as_widget().non_client_rect().extents() }, pen{ frame_color(), effective_frame_width(), false });
            else
                aGc.draw_rounded_rect(rect{ point{ 0.0, 0.0 }, base_type::as_widget().non_client_rect().extents() }, frame_radius(), pen{ frame_color(), effective_frame_width() },
                    base_type::as_widget().has_background_color() || !base_type::as_widget().background_is_transparent() ?
                        brush{ base_type::as_widget().background_color().with_combined_alpha(base_type::as_widget().has_background_opacity() ? base_type::as_widget().background_opacity() : 1.0) } : brush{});
            break;
        case frame_style::ContainerFrame:
            {
                rect rectBorder{ point{ 0.0, 0.0 }, base_type::as_widget().non_client_rect().extents() };
                rectBorder.deflate(line_width(), line_width());
                aGc.draw_rect(rectBorder, pen{ inner_frame_color(), line_width(), false });
                rectBorder.inflate(line_width(), line_width());
                aGc.draw_rect(rectBorder, pen{ frame_color(), line_width(), false });
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

    template <Widget Base>
    inline void framed_widget<Base>::set_frame_style(frame_style aStyle)
    {
        if (iStyle != aStyle)
        {
            iStyle = aStyle;
            base_type::as_widget().update_layout();
        }
    }

    template <Widget Base>
    inline bool framed_widget<Base>::has_frame_color() const
    {
        return iFrameColor != std::nullopt;
    }

    template <Widget Base>
    inline color framed_widget<Base>::frame_color() const
    {
        if (has_frame_color())
            return *iFrameColor;
        else
            return base_type::as_widget().background_color().shaded(0x20);
    }

    template <Widget Base>
    inline void framed_widget<Base>::set_frame_color(const optional_color& aFrameColor)
    {
        iFrameColor = aFrameColor;
        base_type::as_widget().update(true);
    }

    template <Widget Base>
    inline color framed_widget<Base>::inner_frame_color() const
    {
        if (iStyle != frame_style::ContainerFrame)
            return frame_color();
        else
            return (base_type::as_widget().has_base_color() ? base_type::as_widget().base_color() : base_type::as_widget().container_background_color()).lighter(0x40);
    }

    template <Widget Base>
    inline bool framed_widget<Base>::has_frame_radius() const
    {
        return iFrameRadius != std::nullopt;
    }

    template <Widget Base>
    inline vec4 framed_widget<Base>::frame_radius() const
    {
        if (has_frame_radius())
            return *iFrameRadius;
        else
            return vec4{};
    }

    template <Widget Base>
    inline void framed_widget<Base>::set_frame_radius(const optional_vec4& aFrameRadius)
    {
        iFrameRadius = aFrameRadius;
        base_type::as_widget().update(true);
    }

    template <Widget Base>
    inline dimension framed_widget<Base>::line_width() const
    {
        return units_converter{ *this }.from_device_units(iLineWidth);
    }

    template <Widget Base>
    inline dimension framed_widget<Base>::effective_frame_width() const
    {
        switch (iStyle)
        {
        case frame_style::NoFrame:
        default:
            return 0.0;
        case frame_style::DottedFrame:
        case frame_style::DashedFrame:
        case frame_style::SolidFrame:
            return line_width();
        case frame_style::WindowFrame:
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