// default_skin.cpp
/*
  neogfx C++ App/Game Engine
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

#include <neogfx/neogfx.hpp>

#include <neolib/task/thread.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/default_skin.hpp>

namespace neogfx
{
    inline color default_border_color(i_widget const& aWidget)
    {
        auto const result = aWidget.container_background_color().mid(aWidget.palette_color(color_role::Base));
        if (result.similar_intensity(aWidget.container_background_color(), 0.05))
            return result.shaded(0x20);
        return result;
    }

    std::string const& default_skin::name() const
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

    size default_skin::preferred_size(skin_element aElement, optional_size const& aDesiredSize) const
    {
        switch (aElement)
        {
        case skin_element::CheckBox:
            return (aDesiredSize == std::nullopt ? size{ 3.5_mm, 3.5_mm } : size{ 3.5_mm, 3.5_mm }.max(*aDesiredSize)).ceil();
        case skin_element::RadioButton:
            return (aDesiredSize == std::nullopt ? size{ 3.5_mm, 3.5_mm } : size{ 3.5_mm, 3.5_mm }.max(*aDesiredSize)).ceil();
        case skin_element::ProgressBar:
            return (aDesiredSize == std::nullopt ? size{ 20.0_mm, 3.5_mm } : size{ 20.0_mm, 3.5_mm }.max(*aDesiredSize)).ceil();
        default:
            return {};
        }
    }

    void default_skin::draw_scrollbar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_scrollbar& aScrollbar) const
    {
        if (aScrollbar.auto_hidden())
            return;
        scoped_units su{ aItem.as_widget(), units::Pixels };
        rect const g = aItem.element_rect(skin_element::Scrollbar);
        auto const width = aScrollbar.width();
        color scrollbarColor = aScrollbar.container().scrollbar_color(aScrollbar).shaded(0x30);
        color backgroundColor = scrollbarColor.unshaded(0x30);
        bool const noThumb = (aScrollbar.maximum() - aScrollbar.minimum() <= aScrollbar.page());
        if (aScrollbar.type() == scrollbar_style::Normal)
            aGc.fill_rect(g, backgroundColor);
        else if (aScrollbar.type() == scrollbar_style::Menu)
        {
            auto g1 = g;
            auto g2 = g;
            if (aScrollbar.orientation() == scrollbar_orientation::Vertical)
            {
                g1.cy = width;
                g2.y = g2.y + g2.cy - width;
                g2.cy = g1.cy;
            }
            else
            {
                g1.cx = width;
                g2.x = g2.x + g2.cx - width;
                g2.cx = g1.cx;
            }
            aGc.fill_rect(g1, backgroundColor);
            aGc.fill_rect(g2, backgroundColor);
        }
        else if (aScrollbar.type() == scrollbar_style::Scroller)
        {
            auto g1 = g;
            auto g2 = g;
            if (aScrollbar.orientation() == scrollbar_orientation::Vertical)
            {
                g1.y = g1.y + g1.cy - width * 2.0;
                g1.cy = width;
                g2.y = g2.y + g2.cy - width;
                g2.cy = g1.cy;
            }
            else
            {
                g1.x = g1.x + g1.cx - width * 2.0;
                g1.cx = width;
                g2.x = g2.x + g2.cx - width;
                g2.cx = g1.cx;
            }
            aGc.fill_rect(g1, backgroundColor);
            aGc.fill_rect(g2, backgroundColor);
            if (aScrollbar.position() != aScrollbar.minimum())
            {
                auto fadeRect = g;
                if (aScrollbar.orientation() == scrollbar_orientation::Vertical)
                    fadeRect.cy = width;
                else
                    fadeRect.cx = width;
                aGc.fill_rect(fadeRect, gradient
                    {
                        gradient::color_stop_list{ { 0.0, backgroundColor }, { 1.0, backgroundColor } },
                        gradient::alpha_stop_list{ { 0.0, 0xFF_u8 }, { 1.0, 0x00_u8 } },
                        aScrollbar.orientation() == scrollbar_orientation::Vertical ? gradient_direction::Vertical : gradient_direction::Horizontal
                    });
            }
            if (aScrollbar.position() != aScrollbar.maximum() - aScrollbar.page())
            {
                auto fadeRect = g;
                if (aScrollbar.orientation() == scrollbar_orientation::Vertical)
                {
                    fadeRect.y = fadeRect.y + fadeRect.cy - width * 3.0;
                    fadeRect.cy = width;
                }
                else
                {
                    fadeRect.x = fadeRect.x + fadeRect.cx - width * 3.0;
                    fadeRect.cx = width;
                }
                aGc.fill_rect(fadeRect, gradient
                    {
                        gradient::color_stop_list{ { 0.0, backgroundColor }, { 1.0, backgroundColor } },
                        gradient::alpha_stop_list{ { 0.0, 0x00_u8 },{ 1.0, 0xFF_u8 } },
                        aScrollbar.orientation() == scrollbar_orientation::Vertical ? gradient_direction::Vertical : gradient_direction::Horizontal
                    });
            }
        }

        const dimension padding = 3.0_dip;
        rect rectUpButton = aItem.element_rect(skin_element::ScrollbarUpArrow).deflate(padding, padding);
        rect rectDownButton = aItem.element_rect(skin_element::ScrollbarDownArrow).deflate(padding, padding);
        if (aScrollbar.orientation() == scrollbar_orientation::Vertical)
        {
            coordinate x = std::floor(rectUpButton.center().x);
            coordinate w = 1.0;
            for (coordinate y = 0.0; y < rectUpButton.height(); ++y)
            {
                aGc.fill_rect(rect{ point{ x, std::floor(y + rectUpButton.top()) }, size{ w, 1.0 } },
                    scrollbarColor.shaded(noThumb || aScrollbar.position() == aScrollbar.minimum() ? 0x00 : aScrollbar.clicked_element()== scrollbar_element::UpButton ? 0x60 : aScrollbar.hovering_element() == scrollbar_element::UpButton ? 0x30 : 0x00) );
                aGc.fill_rect(rect{ point{ x, std::floor(rectDownButton.bottom() - y) }, size{ w, 1.0 } },
                    scrollbarColor.shaded(noThumb || aScrollbar.position() == aScrollbar.maximum() - aScrollbar.page() ? 0x00 : aScrollbar.clicked_element() == scrollbar_element::DownButton ? 0x60 : aScrollbar.hovering_element() == scrollbar_element::DownButton ? 0x30 : 0x00));
                x -= 1.0;
                w += 2.0;
            }
        }
        else
        {
            coordinate y = std::floor(rectUpButton.center().y);
            coordinate h = 1.0;
            for (coordinate x = 0.0; x < rectUpButton.width(); ++x)
            {
                aGc.fill_rect(rect{ point{ std::floor(x + rectUpButton.left()), y }, size{ 1.0, h } },
                    scrollbarColor.shaded(noThumb || aScrollbar.position() == aScrollbar.minimum() ? 0x00 : aScrollbar.clicked_element() == scrollbar_element::UpButton ? 0x60 : aScrollbar.hovering_element() == scrollbar_element::UpButton ? 0x30 : 0x00));
                aGc.fill_rect(rect{ point{ std::floor(rectDownButton.right() - x), y }, size{ 1.0, h } },
                    scrollbarColor.shaded(noThumb || aScrollbar.position() == aScrollbar.maximum() - aScrollbar.page() ? 0x00 : aScrollbar.clicked_element() == scrollbar_element::DownButton ? 0x60 : aScrollbar.hovering_element() == scrollbar_element::DownButton ? 0x30 : 0x00));
                y -= 1.0;
                h += 2.0;
            }
        }
        if (aScrollbar.type() == scrollbar_style::Normal && !noThumb)
            aGc.fill_rect(aItem.element_rect(skin_element::ScrollbarThumb).deflate(aScrollbar.orientation() == scrollbar_orientation::Vertical ? padding : 0.0, aScrollbar.orientation() == scrollbar_orientation::Vertical ? 0.0 : padding),
                scrollbarColor.shaded(aScrollbar.clicked_element() == scrollbar_element::Thumb ? 0x60 : aScrollbar.hovering_element() == scrollbar_element::Thumb ? 0x30 : 0x00));;
    }

    void default_skin::draw_check_box(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const
    {
        auto const& widget = aItem.as_widget();
        scoped_units su{ widget, units::Pixels };
        rect boxRect = aItem.element_rect(skin_element::CheckBox);
        auto enabledAlphaCoefficient = widget.effectively_enabled() ? 1.0 : 0.25;
        color hoverColor = service<i_app>().current_style().palette().color(color_role::Hover).same_lightness_as(widget.base_color().shaded(0x20));
        if (widget.capturing() && widget.capture_position() && aItem.element_rect(skin_element::ClickableArea).contains(*widget.capture_position()))
            widget.base_color().dark() ? hoverColor.lighten(0x20) : hoverColor.darken(0x20);
        color const fillColor = widget.enabled() && !widget.ignore_mouse_events() && aItem.element_rect(skin_element::ClickableArea).contains(widget.capture_position() ? *widget.capture_position() : widget.mouse_position()) ?
            hoverColor : widget.base_color();
        aGc.fill_rect(boxRect, fillColor.with_combined_alpha(enabledAlphaCoefficient));
        color borderColor = default_border_color(widget);
        aGc.draw_rect(boxRect, pen{ borderColor.with_combined_alpha(enabledAlphaCoefficient), 1.0 });
        boxRect.deflate(3.0_dip, 3.0_dip);
        if (aCheckedState != std::nullopt && *aCheckedState == true)
        {
            scoped_snap_to_pixel snap{ aGc, false };
            /* todo: draw tick image eye candy */
            aGc.draw_line(boxRect.top_left(), boxRect.bottom_right(), pen(service<i_app>().current_style().palette().color(color_role::PrimaryAccent).with_combined_alpha(enabledAlphaCoefficient), 2.0_dip));
            aGc.draw_line(boxRect.bottom_left(), boxRect.top_right(), pen(service<i_app>().current_style().palette().color(color_role::PrimaryAccent).with_combined_alpha(enabledAlphaCoefficient), 2.0_dip));
        }
        else if (aCheckedState == std::nullopt)
            aGc.fill_rect(boxRect, service<i_app>().current_style().palette().color(color_role::PrimaryAccent).with_combined_alpha(enabledAlphaCoefficient));
    }

    void default_skin::draw_radio_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const
    {
        auto const& widget = aItem.as_widget();
        scoped_units su{ widget, units::Pixels };
        rect discRect = aItem.element_rect(skin_element::RadioButton);
        auto enabledAlphaCoefficient = widget.effectively_enabled() ? 1.0 : 0.25;
        color borderColor = default_border_color(widget);
        aGc.fill_circle(discRect.center(), discRect.width() / 2.0, borderColor.with_combined_alpha(enabledAlphaCoefficient));
        discRect.deflate(1.0, 1.0);
        color hoverColor = service<i_app>().current_style().palette().color(color_role::Hover).same_lightness_as(widget.base_color().shaded(0x20));
        if (widget.capturing())
            widget.base_color().dark() ? hoverColor.lighten(0x20) : hoverColor.darken(0x20);
        color fillColor = widget.effectively_enabled() && !widget.ignore_mouse_events() && aItem.element_rect(skin_element::ClickableArea).contains(widget.mouse_position()) ?
            hoverColor : widget.base_color();
        aGc.fill_circle(discRect.center(), discRect.width() / 2.0, fillColor.with_combined_alpha(enabledAlphaCoefficient));
        discRect.deflate(3.0_dip, 3.0_dip);
        if (aCheckedState != std::nullopt && *aCheckedState == true)
            aGc.fill_circle(discRect.center(), discRect.width() / 2.0, service<i_app>().current_style().palette().color(color_role::PrimaryAccent).with_combined_alpha(enabledAlphaCoefficient));
    }

    void default_skin::draw_tree_expander(i_graphics_context& aGc, const i_skinnable_item& aItem, bool aExpandedState) const
    {
        auto const expanderRect = aItem.element_rect(skin_element::TreeExpander);
        auto const expanderColor = aItem.is_widget() && aItem.as_widget().has_base_color() ? aItem.as_widget().base_color() : service<i_app>().current_style().palette().color(color_role::Text);
        thread_local neogfx::game::mesh mesh{ {}, neogfx::vertices_2d{ 3, neogfx::vec2{} }, neogfx::game::default_faces(3) };
        auto const d1 = 3.0_dip;
        auto const d2 = 1.0_dip;
        if (!aExpandedState)
            mesh.vertices = { vec3{ -d1 + d2, -d1 - d2 }, vec3{ d1 - d2, 0.0 }, vec3{ -d1 + d2, d1 + d2 } };
        else
            mesh.vertices = { vec3{ -d1, d1 }, vec3{ d1, -d1 }, vec3{ d1, d1 } };
        if (!aExpandedState)
            aGc.draw_shape(mesh, expanderRect.center().to_vec3(), expanderColor);
        else
            aGc.fill_shape(mesh, expanderRect.center().to_vec3(), expanderColor);
    }

    void default_skin::draw_progress_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_progress_bar& aProgressBar) const
    {
        auto const& barRect = aProgressBar.bar_rect();
        auto const color1 = aItem.as_widget().background_color().shaded(0x0B);
        aGc.draw_rounded_rect(barRect, 5.0_dip, pen{ color1.shaded(0x10), 2.0_dip }, color1);
        auto const doneRect = barRect.deflated(delta{ 2.0_dip }).with_cx(barRect.deflated(delta{ 2.0_dip }).cx * ((aProgressBar.value() - aProgressBar.minimum()) / (aProgressBar.maximum() - aProgressBar.minimum())));
        aGc.fill_rounded_rect(doneRect, 5.0_dip, gradient{ gradient::color_stop_list{ { 0.0, color::LightGreen }, { 0.5, color::LightGreen }, { 1.0, color::DarkGreen } }, gradient_direction::Vertical });
        scalar frame = static_cast<scalar>(neolib::thread::program_elapsed_ms() % 1000) / 1000.0;
        auto const w = 0.75_cm;
        auto const h = barRect.cy / 1.5;
        auto const frameRect = rect{ point{ barRect.x - w + (barRect.cx + w * 2.0) * frame, barRect.y - h * 0.25 }, size{ w, h } };
        scoped_scissor ss{ aGc, doneRect }; // todo: replace with a stencil
        scoped_filter<blur_filter> filter{ aGc, blur_filter{ frameRect.inflated(size{ w, h }), 2.0, blurring_algorithm::Gaussian, std::min(h * 2.0, 15.0), 4.0} };
        filter.front_buffer().fill_ellipse(frameRect.center(), w * 0.5, h * 0.5, color::LightGreen.with_alpha(0.2));
    }

    void default_skin::draw_separators(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_layout& aLayout) const
    {
        auto ink1 = (aItem.as_widget().has_base_color() ? aItem.as_widget().base_color() : service<i_app>().current_style().palette().color(color_role::Base));
        ink1 = ink1.shaded(0x60);
        auto ink2 = ink1.darker(0x30);
        for (std::uint32_t i = 1u; i < aLayout.count(); ++i) 
        {
            if (aLayout.item_at(i - 1u).is_layout() && (!aLayout.item_at(i - 1u).as_layout().enabled() || aLayout.item_at(i - 1u).as_layout().count() == 0))
                continue;
            if (aLayout.item_at(i).is_layout() && (!aLayout.item_at(i).as_layout().enabled() || aLayout.item_at(i).as_layout().count() == 0))
                continue;
            auto const sepRect = separator_rect(aLayout, { i - 1u, i });
            if (aLayout.direction() == layout_direction::Horizontal)
            {
                rect const gripRect{ (sepRect.center() - point{ 1.0_dip, 8.5_dip }).ceil(), size{ 2.0_dip, 17.0_dip } };
                for (auto s = gripRect.top_left(); s.y < gripRect.bottom_left().y; s += point{ 0.0, 3.0_dip, })
                {
                    aGc.fill_rect(rect{ s, size{ 2.0_dip, 2.0_dip } }, ink1);
                    aGc.fill_rect(rect{ s, size{ 1.0_dip, 1.0_dip } }, ink2);
                }
            }
            else
            {
                rect const gripRect{ (sepRect.center() - point{ 8.5_dip, 1.0_dip }).ceil(), size{ 17.0_dip, 2.0_dip } };
                for (auto s = gripRect.top_left(); s.x < gripRect.top_right().x; s += point{ 3.0_dip, 0.0 })
                {
                    aGc.fill_rect(rect{ s, size{ 2.0_dip, 2.0_dip } }, ink1);
                    aGc.fill_rect(rect{ s, size{ 1.0_dip, 1.0_dip } }, ink2);
                }
            }
        }
    }
}