// neotif_skin.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/widget/i_tab_container.hpp>
#include <neogfx/gui/widget/neotif_skin.hpp>

namespace neogfx
{
    namespace
    {
        // The 3D look: lit (top/left) and shadowed (bottom/right) edges derived from the face colour
        color top_shadow(color const& aFace)
        {
            return aFace.lighter(0x50);
        }

        color bottom_shadow(color const& aFace)
        {
            return aFace.darker(0x50);
        }

        // (units are pixels)
        dimension bevel_width()
        {
            return std::max(1.0, std::round(static_cast<dimension>(1.0_dip)));
        }

        // A bevel aWidth pixels wide just inside aRect; mitred, so the top right and bottom left
        // corners are split diagonally between the two colours.
        void draw_bevel(i_graphics_context& aGc, rect const& aRect, dimension aWidth, color const& aTopLeft, color const& aBottomRight)
        {
            auto const r = aRect.round();
            for (dimension i = 0.0; i < aWidth; i += 1.0)
            {
                auto const length = r.cx - i * 2.0 - 1.0;
                auto const height = r.cy - i * 2.0 - 1.0;
                if (length <= 0.0 || height <= 0.0)
                    break;
                // top and left
                aGc.fill_rect(rect{ point{ r.x + i, r.y + i }, size{ length, 1.0 } }, aTopLeft);
                aGc.fill_rect(rect{ point{ r.x + i, r.y + i + 1.0 }, size{ 1.0, height } }, aTopLeft);
                // bottom and right
                aGc.fill_rect(rect{ point{ r.x + i + 1.0, r.y + r.cy - i - 1.0 }, size{ length, 1.0 } }, aBottomRight);
                aGc.fill_rect(rect{ point{ r.x + r.cx - i - 1.0, r.y + i }, size{ 1.0, height } }, aBottomRight);
            }
        }

        void draw_raised(i_graphics_context& aGc, rect const& aRect, dimension aWidth, color const& aFace)
        {
            draw_bevel(aGc, aRect, aWidth, top_shadow(aFace), bottom_shadow(aFace));
        }

        void draw_sunken(i_graphics_context& aGc, rect const& aRect, dimension aWidth, color const& aFace)
        {
            draw_bevel(aGc, aRect, aWidth, bottom_shadow(aFace), top_shadow(aFace));
        }

        // A disk with an edge aWidth pixels wide, split diagonally: lit top left and shadowed bottom right if raised, 
        // the other way round if sunken
        void draw_disk(i_graphics_context& aGc, point const& aCenter, dimension aRadius, dimension aWidth, color const& aFace, bool aSunken, scalar aAlpha = 1.0)
        {
            auto const topLeft = (aSunken ? bottom_shadow(aFace) : top_shadow(aFace)).with_combined_alpha(aAlpha);
            auto const bottomRight = (aSunken ? top_shadow(aFace) : bottom_shadow(aFace)).with_combined_alpha(aAlpha);
            aGc.fill_circle(aCenter, aRadius, gradient{
                gradient::color_stop_list{ { 0.0, topLeft }, { 0.5, topLeft }, { 0.5, bottomRight }, { 1.0, bottomRight } },
                gradient_direction::Diagonal });
            aGc.fill_circle(aCenter, aRadius - aWidth, aFace.with_combined_alpha(aAlpha));
        }

        // the colour of a window's decoration (its frame and title bar), active or inactive
        color decoration_color(i_window const& aWindow)
        {
            auto const& palette = service<i_app>().current_style().palette();
            if (aWindow.effectively_enabled() && aWindow.is_effectively_active())
                return palette.color(!aWindow.is_nested() ? color_role::Selection : color_role::AlternateSelection);
            return palette.color(color_role::Theme).shaded(0x1C);
        }

        // A bevelled arrow (a triangle) pointing up, down, left or right, centred in aRect: lit on its upper and left edges, 
        // shadowed on its lower and right edges (the other way round if sunken).  Drawn a row (or column) of pixels at a 
        // time so the edges are crisp.
        void draw_arrow(i_graphics_context& aGc, rect const& aRect, scrollbar_element aDirection, bool aVertical, dimension aBevel, color const& aFace, bool aSunken)
        {
            auto const light = aSunken ? bottom_shadow(aFace) : top_shadow(aFace);
            auto const dark = aSunken ? top_shadow(aFace) : bottom_shadow(aFace);
            auto const base = std::floor(std::min(aRect.cx, aRect.cy));
            auto const length = std::round(base * 0.85); // (from base to point)
            if (base < aBevel * 4.0 || length < aBevel * 2.0)
                return;
            bool const pointsBack = aDirection == scrollbar_element::UpButton; // (up or left)
            // across: along the base; along: from base to point
            auto const acrossStart = std::floor(aVertical ? aRect.x + (aRect.cx - base) / 2.0 : aRect.y + (aRect.cy - base) / 2.0);
            auto const alongStart = std::floor(aVertical ? aRect.y + (aRect.cy - length) / 2.0 : aRect.x + (aRect.cx - length) / 2.0);
            auto strip = [&](coordinate aAlong, coordinate aAcross, dimension aExtent, color const& aColor)
            {
                if (aExtent <= 0.0)
                    return;
                aGc.fill_rect(aVertical ? 
                    rect{ point{ aAcross, aAlong }, size{ aExtent, 1.0 } } : 
                    rect{ point{ aAlong, aAcross }, size{ 1.0, aExtent } }, aColor);
            };
            for (dimension i = 0.0; i < length; i += 1.0)
            {
                // i: distance from the point
                auto const half = std::floor((i + 0.5) * (base / 2.0) / length);
                auto const extent = half * 2.0 + 1.0;
                auto const across = acrossStart + std::floor(base / 2.0) - half;
                auto const along = pointsBack ? alongStart + i : alongStart + length - 1.0 - i;
                bool const baseEdge = i >= length - aBevel;
                if (baseEdge)
                {
                    // (the base: shadowed if it is below or right of the face, lit if above or left of it)
                    strip(along, across, extent, pointsBack ? dark : light);
                    continue;
                }
                strip(along, across, extent, aFace);
                auto const edge = std::min(aBevel, std::ceil(extent / 2.0));
                // the upper or left sloping edge is lit and the lower or right one shadowed
                strip(along, across, edge, light);
                strip(along, across + extent - edge, edge, dark);
            }
        }

        // the face colour of a title bar button: that of what it is on (its title bar, or the menu bar a maximized
        // MDI window's title bar is merged into)
        color title_bar_button_face(i_widget const& aButton)
        {
            if (aButton.has_parent())
                return aButton.parent().background_color();
            return aButton.background_color();
        }
    }

    std::string const& neotif_skin::name() const
    {
        static const std::string sName = "Neotif";
        return sName;
    }

    bool neotif_skin::has_fallback() const
    {
        return true;
    }

    i_skin& neotif_skin::fallback()
    {
        return iFallback;
    }

    void neotif_skin::activate()
    {
    }

    void neotif_skin::deactivate()
    {
    }

    size neotif_skin::preferred_size(skin_element aElement, optional_size const& aDesiredSize) const
    {
        return iFallback.preferred_size(aElement, aDesiredSize);
    }

    dimension neotif_skin::push_button_border_width(const i_skinnable_item& aItem, bool aDefaultButton, dimension aBorderWidth) const
    {
        return iFallback.push_button_border_width(aItem, aDefaultButton, aBorderWidth);
    }

    dimension neotif_skin::window_frame_width(const i_skinnable_item& aItem, dimension aFrameWidth) const
    {
        scoped_units su{ aItem.as_widget(), units::Pixels };
        return std::max(aFrameWidth, std::ceil(static_cast<dimension>(6.0_dip)));
    }

    void neotif_skin::adjust_title_bar_appearance(const i_skinnable_item& aItem, title_bar_appearance& aAppearance) const
    {
        auto const& titleBar = aItem.as_widget();
        scoped_units su{ titleBar, units::Pixels };

        // the icon (window menu) and buttons are squares the height of the title bar; the title is centred in what is left
        auto const bevel = bevel_width();
        auto const margin = std::ceil(static_cast<dimension>(2.0_dip));
        auto const contentHeight = std::max(aAppearance.iconSize.cy, aAppearance.titleHeight);
        auto const height = std::ceil(contentHeight + (bevel + margin) * 2.0);
        auto const top = std::floor((height - contentHeight) / 2.0);
        auto const left = std::floor((height - aAppearance.iconSize.cx) / 2.0);
        aAppearance.padding = neogfx::padding{ left, top, bevel + margin, height - contentHeight - top };
        aAppearance.spacing = size{ (height - left - aAppearance.iconSize.cx) * 2.0, 0.0 };
        aAppearance.buttonSize = size{ height, height };
        aAppearance.titleAlignment = alignment::Center | alignment::VCenter;
        aAppearance.backgroundColor = decoration_color(titleBar.root());
        aAppearance.buttonImages = false;
    }

    void neotif_skin::draw_push_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance) const
    {
        // (rounded buttons don't have bevels)
        if (aAppearance.borderRadii.has_value())
        {
            iFallback.draw_push_button(aGc, aItem, aButton, aAppearance);
            return;
        }
        auto const& button = aItem.as_widget();
        scoped_units su{ button, units::Pixels };
        auto faceRect = button.client_rect().round();
        bool const pressed = button.capturing() && button.entered();
        bool const sunken = pressed || aButton.is_checked();
        switch (aButton.style())
        {
        case push_button_style::TitleBar:
            {
                auto const face = aAppearance.customized ? aAppearance.faceColor : title_bar_button_face(button);
                aGc.fill_rect(faceRect, face);
                if (pressed)
                    draw_sunken(aGc, faceRect, bevel_width(), face);
                else
                    draw_raised(aGc, faceRect, bevel_width(), face);
            }
            return;
        case push_button_style::Tab:
            {
                // a raised bevel (the window frame's) around the whole button; the button's bounding rect 
                // runs on past the side facing the tab page, so that side is clipped away leaving the tab open to its 
                // page (see draw_tab_page_join)
                auto const tabRect = button.client_rect().combined(aAppearance.boundingRect).round();
                auto const& face = aAppearance.faceColor;
                aGc.fill_rect(tabRect, face);
                draw_raised(aGc, tabRect, bevel_width(), face);
            }
            break;
        case push_button_style::Toolbar:
            // flat until it is pointed at, pressed or checked
            if (button.effectively_enabled() && (button.entered() || button.capturing() || aButton.is_checked()))
            {
                auto const face = button.effective_background_color();
                if (sunken)
                    draw_sunken(aGc, faceRect, bevel_width(), face);
                else
                    draw_raised(aGc, faceRect, bevel_width(), face);
            }
            break;
        default:
            {
                // as thin as the window frame's; a default button's is 2dip
                auto const bevel = !aAppearance.defaultButton ? bevel_width() : std::max(1.0, std::round(static_cast<dimension>(2.0_dip)));
                auto const& face = aAppearance.faceColor;
                aGc.fill_rect(faceRect, face);
                if (sunken)
                    draw_sunken(aGc, faceRect, bevel, face);
                else
                    draw_raised(aGc, faceRect, bevel, face);
            }
            break;
        }
        if (button.has_focus())
        {
            rect focusRect = faceRect;
            auto const focusInset = (aAppearance.defaultButton ? std::max(1.0, std::round(static_cast<dimension>(2.0_dip))) : bevel_width()) + 1.0;
            focusRect.deflate(focusInset, focusInset);
            aGc.draw_focus_rect(focusRect);
        }
    }

    void neotif_skin::push_button_text_color(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance, i_optional<color>& aTextColor) const
    {
        iFallback.push_button_text_color(aItem, aButton, aAppearance, aTextColor);
    }

    void neotif_skin::draw_scrollbar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_scrollbar& aScrollbar) const
    {
        // (menu and scroller style scrollbars are drawn by the default skin)
        if (aScrollbar.type() != scrollbar_style::Normal)
        {
            iFallback.draw_scrollbar(aGc, aItem, aScrollbar);
            return;
        }
        if (!aScrollbar.visible() || aScrollbar.auto_hidden())
            return;
        scoped_units su{ aItem.as_widget(), units::Pixels };
        // a sunken trough with bevelled arrow buttons at its ends and a raised thumb
        bool const vertical = aScrollbar.orientation() == scrollbar_orientation::Vertical;
        auto const bevel = bevel_width();
        auto const face = aScrollbar.container().scrollbar_color(aScrollbar);
        auto const trough = face.darker(0x20);
        auto const troughRect = aItem.element_rect(skin_element::Scrollbar).round();
        aGc.fill_rect(troughRect, trough);
        draw_sunken(aGc, troughRect, bevel, face);
        auto const inset = bevel + std::ceil(static_cast<dimension>(1.0_dip));
        auto const deflate_across = [&](rect aRect)
        {
            return vertical ? aRect.deflated(inset, 0.0) : aRect.deflated(0.0, inset);
        };
        draw_arrow(aGc, deflate_across(aItem.element_rect(skin_element::ScrollbarUpArrow).round()), scrollbar_element::UpButton, vertical, bevel, face,
            aScrollbar.clicked_element() == scrollbar_element::UpButton);
        draw_arrow(aGc, deflate_across(aItem.element_rect(skin_element::ScrollbarDownArrow).round()), scrollbar_element::DownButton, vertical, bevel, face,
            aScrollbar.clicked_element() == scrollbar_element::DownButton);
        bool const noThumb = (aScrollbar.maximum() - aScrollbar.minimum() <= aScrollbar.page());
        if (noThumb)
            return;
        auto const thumbRect = deflate_across(aItem.element_rect(skin_element::ScrollbarThumb).round());
        if (thumbRect.cx <= bevel * 2.0 || thumbRect.cy <= bevel * 2.0)
            return;
        aGc.fill_rect(thumbRect, face);
        draw_raised(aGc, thumbRect, bevel, face);
    }

    void neotif_skin::draw_check_box(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const
    {
        // a sunken box with the window frame's bevel
        auto const& widget = aItem.as_widget();
        scoped_units su{ widget, units::Pixels };
        auto const boxRect = aItem.element_rect(skin_element::CheckBox).round();
        auto const alpha = widget.effectively_enabled() ? 1.0 : 0.25;
        auto const bevel = bevel_width();
        auto const face = widget.base_color();
        bool const checked = aCheckedState != std::nullopt && *aCheckedState == true;
        bool const indeterminate = aCheckedState == std::nullopt;
        aGc.fill_rect(boxRect, face.with_combined_alpha(alpha));
        draw_bevel(aGc, boxRect, bevel, bottom_shadow(face).with_combined_alpha(alpha), top_shadow(face).with_combined_alpha(alpha));
        auto const markRect = boxRect.deflated(bevel + std::ceil(static_cast<dimension>(2.0_dip)), bevel + std::ceil(static_cast<dimension>(2.0_dip)));
        auto const mark = service<i_app>().current_style().palette().color(color_role::PrimaryAccent).with_combined_alpha(alpha);
        if (checked)
        {
            scoped_snap_to_pixel snap{ aGc, false };
            aGc.draw_line(markRect.top_left(), markRect.bottom_right(), pen{ mark, static_cast<dimension>(2.0_dip), false });
            aGc.draw_line(markRect.bottom_left(), markRect.top_right(), pen{ mark, static_cast<dimension>(2.0_dip), false });
        }
        else if (indeterminate)
            aGc.fill_rect(markRect, mark);
    }

    void neotif_skin::draw_radio_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const
    {
        // a sunken disk with the window frame's bevel
        auto const& widget = aItem.as_widget();
        scoped_units su{ widget, units::Pixels };
        auto const discRect = aItem.element_rect(skin_element::RadioButton);
        auto const radius = discRect.width() / 2.0;
        auto const bevel = bevel_width();
        if (radius <= bevel)
            return;
        auto const alpha = widget.effectively_enabled() ? 1.0 : 0.25;
        bool const on = aCheckedState != std::nullopt && *aCheckedState == true;
        draw_disk(aGc, discRect.center(), radius, bevel, widget.base_color(), true, alpha);
        auto const dotRadius = radius - bevel - static_cast<dimension>(3.0_dip);
        if (on && dotRadius > 0.0)
            aGc.fill_circle(discRect.center(), dotRadius, service<i_app>().current_style().palette().color(color_role::PrimaryAccent).with_combined_alpha(alpha));
    }

    void neotif_skin::draw_tree_expander(i_graphics_context& aGc, const i_skinnable_item& aItem, bool aExpandedState) const
    {
        iFallback.draw_tree_expander(aGc, aItem, aExpandedState);
    }

    void neotif_skin::draw_slider(i_graphics_context& aGc, const i_skinnable_item& aItem, slider_orientation aOrientation, double aNormalizedValue, bool aDragging, i_optional<color_or_gradient::abstract_type> const& aBarColor) const
    {
        iFallback.draw_slider(aGc, aItem, aOrientation, aNormalizedValue, aDragging, aBarColor);

        // the default skin's track with a 3D disk over its thumb: lit top left, shadowed bottom right (the other way 
        // round while being dragged)
        auto const& widget = aItem.as_widget();
        scoped_units su{ widget, units::Pixels };
        auto const thumb = aItem.element_rect(skin_element::SliderThumb);
        auto const radius = thumb.width() / 2.0;
        auto const bevel = bevel_width();
        if (radius <= bevel)
            return;
        draw_disk(aGc, thumb.center(), radius, bevel, widget.base_color().lighter(0x40), aDragging);
    }

    void neotif_skin::draw_progress_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_progress_bar& aProgressBar) const
    {
        iFallback.draw_progress_bar(aGc, aItem, aProgressBar);
    }

    void neotif_skin::draw_separators(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_layout& aLayout) const
    {
        iFallback.draw_separators(aGc, aItem, aLayout);
    }

    void neotif_skin::draw_window_frame(i_graphics_context& aGc, const i_skinnable_item& aItem, const rect& aFrameRect, dimension aFrameWidth, const color& aFrameColor) const
    {
        auto const& window = aItem.as_widget().root();
        auto const outer = aFrameRect.round();
        auto const width = std::round(aFrameWidth);
        if (width <= 0.0)
            return;
        auto const bevel = bevel_width();

        // the frame's face
        aGc.fill_rect(rect{ outer.top_left(), size{ outer.cx, width } }, aFrameColor);
        aGc.fill_rect(rect{ point{ outer.x, outer.y + outer.cy - width }, size{ outer.cx, width } }, aFrameColor);
        aGc.fill_rect(rect{ point{ outer.x, outer.y + width }, size{ width, outer.cy - width * 2.0 } }, aFrameColor);
        aGc.fill_rect(rect{ point{ outer.x + outer.cx - width, outer.y + width }, size{ width, outer.cy - width * 2.0 } }, aFrameColor);

        // raised at its outer edge and sunken at its inner edge
        draw_raised(aGc, outer, bevel, aFrameColor);
        if (width > bevel * 2.0)
            draw_sunken(aGc, outer.deflated(width - bevel, width - bevel), bevel, aFrameColor);

        // the corners of a resizable window's frame are resize handles, marked off from the sides by grooves
        if ((window.style() & window_style::Resize) != window_style::Resize || width < bevel * 3.0)
            return;
        // the grooves line up with the ends of the title bar's icon and last button, and with its bottom edge (a title bar 
        // section's last bevel and the next one's first bevel); without a title bar the corners are a fixed size
        coordinate x1, x2, y1;
        auto const titleBar = window.widget_part_rect(widget_part::TitleBar).round();
        if (titleBar.cx > titleBar.cy * 2.0 && titleBar.cy > 0.0)
        {
            x1 = titleBar.x + titleBar.cy - bevel;
            x2 = titleBar.x + titleBar.cx - titleBar.cy - bevel;
            y1 = titleBar.y + titleBar.cy - bevel;
        }
        else
        {
            auto const corner = std::ceil(width + static_cast<dimension>(20.0_dip));
            x1 = outer.x + corner;
            x2 = outer.x + outer.cx - corner - bevel;
            y1 = outer.y + corner;
        }
        auto const y2 = outer.y + outer.cy - (y1 - outer.y) - bevel * 2.0; // (mirroring y1)
        if (x2 <= x1 + bevel * 2.0 || y2 <= y1 + bevel * 2.0)
            return;
        auto const dark = bottom_shadow(aFrameColor);
        auto const light = top_shadow(aFrameColor);
        auto const across = width - bevel * 2.0;
        auto groove_x = [&](coordinate x, coordinate y)
        {
            aGc.fill_rect(rect{ point{ x, y }, size{ bevel, across } }, dark);
            aGc.fill_rect(rect{ point{ x + bevel, y }, size{ bevel, across } }, light);
        };
        auto groove_y = [&](coordinate x, coordinate y)
        {
            aGc.fill_rect(rect{ point{ x, y }, size{ across, bevel } }, dark);
            aGc.fill_rect(rect{ point{ x, y + bevel }, size{ across, bevel } }, light);
        };
        auto const left = outer.x + bevel;
        auto const top = outer.y + bevel;
        auto const right = outer.x + outer.cx - width + bevel;
        auto const bottom = outer.y + outer.cy - width + bevel;
        groove_x(x1, top);
        groove_x(x2, top);
        groove_x(x1, bottom);
        groove_x(x2, bottom);
        groove_y(left, y1);
        groove_y(left, y2);
        groove_y(right, y1);
        groove_y(right, y2);
    }

    void neotif_skin::draw_container_frame(i_graphics_context& aGc, const i_skinnable_item& aItem, const rect& aFrameRect, dimension aFrameWidth, const color&, const color&) const
    {
        // raised, as a tab is, at the thickness of a tab's bevel
        auto const& widget = aItem.as_widget();
        scoped_units su{ widget, units::Pixels };
        auto const outer = aFrameRect.round();
        auto const width = std::round(aFrameWidth);
        if (width <= 0.0)
            return;
        auto const face = widget.background_color();
        aGc.fill_rect(rect{ outer.top_left(), size{ outer.cx, width } }, face);
        aGc.fill_rect(rect{ point{ outer.x, outer.y + outer.cy - width }, size{ outer.cx, width } }, face);
        aGc.fill_rect(rect{ point{ outer.x, outer.y + width }, size{ width, outer.cy - width * 2.0 } }, face);
        aGc.fill_rect(rect{ point{ outer.x + outer.cx - width, outer.y + width }, size{ width, outer.cy - width * 2.0 } }, face);
        draw_raised(aGc, outer, std::min(bevel_width(), width), face);
    }

    void neotif_skin::draw_tab_page_join(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_tab_page& aPage, dimension aFrameWidth, const color&) const
    {
        // the frame is opened up along the selected tab, whose sides carry on through it
        auto const& page = aItem.as_widget();
        scoped_units su{ page, units::Pixels };
        auto const& tab = aPage.tab();
        auto const pageRect = page.to_client_coordinates(page.non_client_rect()).round();
        auto const tabRect = page.to_client_coordinates(tab.as_widget().non_client_rect()).round();
        auto const across = std::round(aFrameWidth);
        if (across <= 0.0)
            return;
        auto const bevel = std::min(bevel_width(), across);
        auto const face = page.background_color();
        auto const light = top_shadow(face);
        auto const dark = bottom_shadow(face);
        auto const alignment = tab.container().tab_container_style() & tab_container_style::TabAlignmentMask;
        if (alignment == tab_container_style::TabAlignmentTop || alignment == tab_container_style::TabAlignmentBottom)
        {
            // (clear of the page's corners)
            auto const y = alignment == tab_container_style::TabAlignmentTop ? pageRect.y : pageRect.y + pageRect.cy - across;
            auto const x1 = std::max(tabRect.x, pageRect.x + across);
            auto const x2 = std::min(tabRect.x + tabRect.cx, pageRect.x + pageRect.cx - across);
            if (x2 - x1 <= bevel * 2.0)
                return;
            aGc.fill_rect(rect{ point{ x1, y }, size{ x2 - x1, across } }, face);
            if (x1 == tabRect.x)
                aGc.fill_rect(rect{ point{ x1, y }, size{ bevel, across } }, light);
            if (x2 == tabRect.x + tabRect.cx)
                aGc.fill_rect(rect{ point{ x2 - bevel, y }, size{ bevel, across } }, dark);
        }
        else
        {
            auto const x = alignment == tab_container_style::TabAlignmentLeft ? pageRect.x : pageRect.x + pageRect.cx - across;
            auto const y1 = std::max(tabRect.y, pageRect.y + across);
            auto const y2 = std::min(tabRect.y + tabRect.cy, pageRect.y + pageRect.cy - across);
            if (y2 - y1 <= bevel * 2.0)
                return;
            aGc.fill_rect(rect{ point{ x, y1 }, size{ across, y2 - y1 } }, face);
            if (y1 == tabRect.y)
                aGc.fill_rect(rect{ point{ x, y1 }, size{ across, bevel } }, light);
            if (y2 == tabRect.y + tabRect.cy)
                aGc.fill_rect(rect{ point{ x, y2 - bevel }, size{ across, bevel } }, dark);
        }
    }

    void neotif_skin::draw_title_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const title_bar_appearance&) const
    {
        auto const& titleBar = aItem.as_widget();
        scoped_units su{ titleBar, units::Pixels };
        auto const face = titleBar.background_color();
        auto const iconRect = aItem.element_rect(skin_element::TitleBarIcon);
        if (iconRect.cx > 0.0)
            draw_raised(aGc, iconRect, bevel_width(), face);
        auto const titleRect = aItem.element_rect(skin_element::TitleBarTitle);
        if (titleRect.cx > 0.0)
            draw_raised(aGc, titleRect, bevel_width(), face);
    }

    void neotif_skin::draw_title_bar_button(i_graphics_context& aGc, const i_skinnable_item& aItem, widget_part_e aButton, const title_bar_appearance&) const
    {
        auto const& button = aItem.as_widget();
        scoped_units su{ button, units::Pixels };
        auto const face = title_bar_button_face(button);
        auto const faceRect = button.client_rect().round();
        auto const bevel = bevel_width();
        bool const pressed = button.capturing() && button.entered();
        auto const alpha = button.effectively_enabled() ? 1.0 : 0.5;
        auto const light = top_shadow(face).with_combined_alpha(alpha);
        auto const dark = bottom_shadow(face).with_combined_alpha(alpha);
        auto const glyph = [&](scalar aProportion)
        {
            auto const extent = std::max(bevel * 2.0 + 1.0, std::floor(std::min(faceRect.cx, faceRect.cy) * aProportion));
            return rect{
                point{ std::floor(faceRect.x + (faceRect.cx - extent) / 2.0), std::floor(faceRect.y + (faceRect.cy - extent) / 2.0) },
                size{ extent, extent } };
        };
        switch (aButton)
        {
        case widget_part::MinimizeButton:
            // a small raised square
            draw_bevel(aGc, glyph(0.2), bevel, pressed ? dark : light, pressed ? light : dark);
            break;
        case widget_part::MaximizeButton:
            // a large raised square
            draw_bevel(aGc, glyph(0.55), bevel, pressed ? dark : light, pressed ? light : dark);
            break;
        case widget_part::RestoreButton:
            // the large square, sunken (the window is maximized)
            draw_bevel(aGc, glyph(0.55), bevel, pressed ? light : dark, pressed ? dark : light);
            break;
        case widget_part::CloseButton:
            {
                // an engraved cross
                auto const cross = glyph(0.4);
                scoped_snap_to_pixel snap{ aGc, false };
                auto const offset = point{ bevel, bevel };
                auto const thickness = bevel * 2.0;
                aGc.draw_line(cross.top_left() + offset, cross.bottom_right() + offset, pen{ pressed ? dark : light, thickness });
                aGc.draw_line(cross.bottom_left() + offset, cross.top_right() + offset, pen{ pressed ? dark : light, thickness });
                aGc.draw_line(cross.top_left(), cross.bottom_right(), pen{ pressed ? light : dark, thickness });
                aGc.draw_line(cross.bottom_left(), cross.top_right(), pen{ pressed ? light : dark, thickness });
            }
            break;
        default:
            break;
        }
    }
}
