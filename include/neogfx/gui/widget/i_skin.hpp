// i_skin.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/optional.hpp>

#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gfx/path.hpp>
#include <neogfx/gfx/pen.hpp>
#include <neogfx/gfx/gradient.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>
#include <neogfx/gui/widget/i_scrollbar.hpp>
#include <neogfx/gui/widget/i_button.hpp>
#include <neogfx/gui/widget/i_push_button.hpp>
#include <neogfx/gui/widget/i_progress_bar.hpp>
#include <neogfx/gui/widget/slider_bits.hpp>

namespace neogfx
{
    // How a push button wants to look: computed by the button (so that subclasses can customize it) 
    // and drawn by the active skin.  Units are pixels.
    struct push_button_appearance
    {
        rect boundingRect;
        neogfx::path outline;
        dimension borderWidth = 0.0;
        color faceColor;
        color outerBorderColor;
        color innerBorderColor;
        std::optional<line_style> lineStyle;
        std::optional<std::pair<vec4, vec4>> borderRadii; // (x radii, y radii)
        bool spotColor = false;
        bool defaultButton = false;
        bool customized = false; // face, border or shape set explicitly (property, palette, style sheet or subclass)
    };

    class i_skin
    {
    public:
        struct no_fallback : std::logic_error { no_fallback() : std::logic_error{ "neogfx::i_skin::no_fallback" } {} };
    public:
        virtual ~i_skin() = default;
    public:
        virtual std::string const& name() const = 0;
        virtual bool has_fallback() const = 0;
        virtual i_skin& fallback() = 0;
    public:
        virtual void activate() = 0;
        virtual void deactivate() = 0;
    public:
        virtual size preferred_size(skin_element aElement, optional_size const& aDesiredSize = {}) const = 0;
        virtual dimension push_button_border_width(const i_skinnable_item& aItem, bool aDefaultButton, dimension aBorderWidth) const = 0;
    public:
        virtual void draw_push_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance) const = 0;
        virtual void push_button_text_color(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance, i_optional<color>& aTextColor) const = 0; ///< (no value: the button's usual text color)
        virtual void draw_scrollbar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_scrollbar& aScrollbar) const = 0;
        virtual void draw_check_box(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const = 0;
        virtual void draw_radio_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const = 0;
        virtual void draw_tree_expander(i_graphics_context& aGc, const i_skinnable_item& aItem, bool aExpandedState) const = 0;
        virtual void draw_slider(i_graphics_context& aGc, const i_skinnable_item& aItem, slider_orientation aOrientation, double aNormalizedValue, bool aDragging, i_optional<color_or_gradient::abstract_type> const& aBarColor) const = 0; ///< (track and thumb: the item's SliderTrack and SliderThumb element rects)
        virtual void draw_progress_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_progress_bar& aProgressBar) const = 0;
        virtual void draw_separators(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_layout& aLayout) const = 0;
        // helpers
    public:
        neolib::optional<color> push_button_text_color(const i_skinnable_item& aItem, const i_push_button& aButton, const push_button_appearance& aAppearance) const
        {
            neolib::optional<color> result;
            push_button_text_color(aItem, aButton, aAppearance, result);
            return result;
        }
    };

    typedef std::pair<std::uint32_t, std::uint32_t> separator_index;

    inline rect separator_rect(const i_layout& aLayout, separator_index aSeparator)
    {
        rect r1(aLayout.item_at(aSeparator.first).position(), aLayout.item_at(aSeparator.first).extents());
        rect r2(aLayout.item_at(aSeparator.second).position(), aLayout.item_at(aSeparator.second).extents());
        if (aLayout.direction() == layout_direction::Horizontal)
        {
            rect r3(point(r1.right(), r1.top()), size(r2.left() - r1.right(), r1.height()));
            rect r4 = r3;
            r4.x -= r3.width();
            r4.cx *= 3.0;
            return r4;
        }
        else
        {
            rect r3(point(r1.left(), r1.bottom()), size(r2.width(), r2.top() - r1.bottom()));
            rect r4 = r3;
            r4.y -= r3.height();
            r4.cy *= 3.0;
            return r4;
        }
    }
}