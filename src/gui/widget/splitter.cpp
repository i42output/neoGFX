// spliter.cpp
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

#include <neogfx/gui/widget/splitter.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/i_skin_manager.hpp>
#include <neogfx/hid/i_surface.hpp>

namespace neogfx
{
    splitter::splitter(splitter_style aStyle) :
        iStyle{ aStyle }
    {
        init();
    }

    splitter::splitter(i_widget& aParent, splitter_style aStyle) : 
        widget{ aParent },
        iStyle{ aStyle }
    {
        init();
    }

    splitter::splitter(i_layout& aLayout, splitter_style aStyle) :
        widget{ aLayout },
        iStyle{ aStyle }
    {
        init();
    }

    splitter::~splitter()
    {
    }

    neogfx::size_policy splitter::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else if ((iStyle & splitter_style::Horizontal) == splitter_style::Horizontal)
            return neogfx::size_policy{size_constraint::Expanding, size_constraint::Minimum};
        else
            return neogfx::size_policy{size_constraint::Minimum, size_constraint::Expanding};
    }

    i_widget& splitter::get_widget_at(const point& aPosition)
    {
        auto s = separator_at(aPosition);
        if (s != std::nullopt)
            return *this;
        return widget::get_widget_at(aPosition);
    }

    void splitter::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left && capturing())
        {
            auto s = separator_at(aPosition);
            if (s != std::nullopt)
            {
                iTracking = s;
                iTrackFrom = aPosition;
                iSizeBeforeTracking = std::make_pair(
                    layout().get_widget_at(iTracking->first).extents(), 
                    layout().get_widget_at(iTracking->second).extents());
                if (has_root())
                    root().window_manager().update_mouse_cursor(root());
            }
        }
    }

    void splitter::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            auto s = separator_at(aPosition);
            if (s != std::nullopt)
            {
                if ((aKeyModifiers & KeyModifier_SHIFT) != KeyModifier_NONE)
                    reset_pane_sizes_requested();
                else
                    reset_pane_sizes_requested(s->first);
                layout_items();
            }
        }
    }

    void splitter::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        if (iTracking != std::nullopt)
        {
            auto& firstWidget = layout().get_widget_at(iTracking->first);
            auto& secondWidget = layout().get_widget_at(iTracking->second);
            bool const resizeBothPanes = (iStyle & splitter_style::ResizeSinglePane) == splitter_style::None || 
                service<i_keyboard>().is_key_pressed(ScanCode_LSHIFT) || service<i_keyboard>().is_key_pressed(ScanCode_RSHIFT);
            if ((iStyle & splitter_style::Horizontal) == splitter_style::Horizontal)
            {
                auto const delta = aPosition.x - iTrackFrom.x;
                if (delta == 0.0)
                    return;
                auto& minimizingWidget = (!resizeBothPanes || delta < 0.0) ? firstWidget : secondWidget;
                auto& maximizingWidget = (!resizeBothPanes || delta > 0.0) ? firstWidget : secondWidget;
                auto const& minimizingWidgetSizeBeforeTracking = (!resizeBothPanes || delta < 0.0) ? iSizeBeforeTracking.first : iSizeBeforeTracking.second;
                auto const& maximizingWidgetSizeBeforeTracking = (!resizeBothPanes || delta > 0.0) ? iSizeBeforeTracking.first : iSizeBeforeTracking.second;
                auto const minSize = minimizingWidget.minimum_size();
                if (&minimizingWidget != &maximizingWidget)
                {
                    auto const adjusted = std::max(minSize.cx, minimizingWidgetSizeBeforeTracking.cx - std::abs(delta));
                    minimizingWidget.set_fixed_size(size{ adjusted, minimizingWidgetSizeBeforeTracking.cy }, false);
                    maximizingWidget.set_fixed_size(size{ maximizingWidgetSizeBeforeTracking.cx + minimizingWidgetSizeBeforeTracking.cx - adjusted, maximizingWidgetSizeBeforeTracking.cy }, false);
                }
                else
                    minimizingWidget.set_fixed_size(size{ std::max(minSize.cx, minimizingWidgetSizeBeforeTracking.cx + delta), minimizingWidgetSizeBeforeTracking.cy }, false);
            }
            else
            {
                auto const delta = aPosition.y - iTrackFrom.y;
                if (delta == 0.0)
                    return;
                auto& minimizingWidget = (!resizeBothPanes || delta < 0.0) ? firstWidget : secondWidget;
                auto& maximizingWidget = (!resizeBothPanes || delta > 0.0) ? firstWidget : secondWidget;
                auto const& minimizingWidgetSizeBeforeTracking = (!resizeBothPanes || delta < 0.0) ? iSizeBeforeTracking.first : iSizeBeforeTracking.second;
                auto const& maximizingWidgetSizeBeforeTracking = (!resizeBothPanes || delta > 0.0) ? iSizeBeforeTracking.first : iSizeBeforeTracking.second;
                auto const minSize = minimizingWidget.minimum_size();
                if (&minimizingWidget != &maximizingWidget)
                {
                    auto const adjusted = std::max(minSize.cy, minimizingWidgetSizeBeforeTracking.cy - std::abs(delta));
                    minimizingWidget.set_fixed_size(size{ minimizingWidgetSizeBeforeTracking.cx, adjusted }, false);
                    maximizingWidget.set_fixed_size(size{ maximizingWidgetSizeBeforeTracking.cx, maximizingWidgetSizeBeforeTracking.cy + minimizingWidgetSizeBeforeTracking.cy - adjusted }, false);
                }
                else
                    minimizingWidget.set_fixed_size(size{ minimizingWidgetSizeBeforeTracking.cx, std::max(minSize.cy, minimizingWidgetSizeBeforeTracking.cy + delta) }, false);
            }
            panes_resized();
        }
    }

    void splitter::mouse_entered(const point& aPosition)
    {
    }

    void splitter::mouse_left()
    {
    }
    
    neogfx::mouse_cursor splitter::mouse_cursor() const
    {
        auto s = separator_at(mouse_position());
        if (s != std::nullopt || iTracking != std::nullopt)
            return (iStyle & splitter_style::Horizontal) == splitter_style::Horizontal ? mouse_system_cursor::SizeWE : mouse_system_cursor::SizeNS;
        else
            return widget::mouse_cursor();
    }

    void splitter::capture_released()
    {
        iTracking = std::nullopt;
    }

    void splitter::panes_resized()
    {
        fix_weightings();
        layout_items();
    }

    void splitter::reset_pane_sizes_requested(const std::optional<std::uint32_t>&)
    {
    }

    void splitter::paint(i_graphics_context& aGc) const
    {
        if ((iStyle & splitter_style::DrawGrip) == splitter_style::DrawGrip)
            service<i_skin_manager>().active_skin().draw_separators(aGc, *this, layout());
    }

    void splitter::init()
    {
        set_padding(neogfx::padding{ 0.0 });
        if ((iStyle & splitter_style::Horizontal) == splitter_style::Horizontal)
            set_layout(make_ref<horizontal_layout>());
        else
            set_layout(make_ref<vertical_layout>());
        layout().set_padding(neogfx::padding{ 0.0 });
    }

    std::optional<splitter::separator_type> splitter::separator_at(const point& aPosition) const
    {
        for (std::uint32_t i = 1u; i < layout().count(); ++i)
            if (separator_rect(layout(), {i - 1u, i}).contains(aPosition))
                    return separator_type{ i - 1u, i };
        return {};
    }
}