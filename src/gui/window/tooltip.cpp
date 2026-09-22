// tooltip.cpp
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

#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/window/tooltip.hpp>

namespace neogfx
{
    tooltip::tooltip(i_widget& aOwner, i_string const& aText, window_style aStyle) :
        window{ aOwner, point{}, aStyle, frame_style::SolidFrame, scrollbar_style::None },
        iText{ client_layout(), string{ aText }, text_widget_type::MultiLine }
    {
        client_layout().set_padding(neogfx::padding{ 6.0_dip, 3.0_dip });
    }

    tooltip::~tooltip()
    {
        if (visible())
            last_hidden() = std::chrono::steady_clock::now();
    }

    std::chrono::milliseconds tooltip::show_delay()
    {
        if (last_hidden() != std::nullopt && std::chrono::steady_clock::now() - *last_hidden() < INITIAL_DELAY)
            return RESHOW_DELAY;
        return INITIAL_DELAY;
    }

    i_string const& tooltip::text() const
    {
        return iText.text();
    }

    void tooltip::set_text(i_string const& aText)
    {
        iText.set_text(aText);
        if (visible())
        {
            resize(minimum_size());
            update_position();
        }
    }

    bool tooltip::show(bool aVisible)
    {
        if (aVisible && !visible())
        {
            resize(minimum_size());
            update_position();
        }
        else if (!aVisible && visible())
            last_hidden() = std::chrono::steady_clock::now();
        return window::show(aVisible);
    }

    double tooltip::rendering_priority() const
    {
        if (visible())
            return 1.0;
        return window::rendering_priority();
    }

    size_policy tooltip::size_policy() const
    {
        if (window::has_size_policy())
            return window::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    size tooltip::minimum_size(optional_size const& aAvailableSpace) const
    {
        size result = window::minimum_size(aAvailableSpace);
        rect desktopRect = service<i_surface_manager>().desktop_rect(surface());
        result = result.min(desktopRect.extents());
        return result;
    }

    size tooltip::maximum_size(optional_size const& aAvailableSpace) const
    {
        return minimum_size(aAvailableSpace);
    }

    color tooltip::frame_color() const
    {
        if (has_frame_color())
            return window::frame_color();
        color result = background_color().darker(0x30);
        if (result.similar_intensity(background_color(), 0.05))
            result = result.shaded(0x20);
        return result;
    }

    std::optional<std::chrono::steady_clock::time_point>& tooltip::last_hidden()
    {
        static std::optional<std::chrono::steady_clock::time_point> sLastHidden;
        return sLastHidden;
    }

    void tooltip::update_position()
    {

        auto& wm = service<i_window_manager>();
        rect const desktopRect{ wm.desktop_rect(*this) };
        rect ourRect{ wm.window_rect(*this) };
        point const mousePosition = wm.mouse_position();
        ourRect.position() = mousePosition + point{ 0.0, dpi_scale(20.0) };
        if (ourRect.right() > desktopRect.right())
            ourRect.position().x = desktopRect.right() - ourRect.width();
        if (ourRect.position().x < desktopRect.left())
            ourRect.position().x = desktopRect.left();
        if (ourRect.bottom() > desktopRect.bottom())
            ourRect.position().y = mousePosition.y - ourRect.height();
        if (ourRect.position().y < desktopRect.top())
            ourRect.position().y = desktopRect.top();
        if (ourRect.position() != wm.window_rect(*this).position())
            wm.move_window(*this, ourRect.position());
    }
}
