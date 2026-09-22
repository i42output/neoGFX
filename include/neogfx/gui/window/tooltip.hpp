// tooltip.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <chrono>
#include <optional>

#include <neogfx/gui/widget/text_widget.hpp>
#include "window.hpp"

namespace neogfx
{
    class tooltip : public window
    {
        meta_object(window)
    public:
        static const window_style DEFAULT_STYLE =
            window_style::Weak |
            window_style::NoDecoration |
            window_style::NoActivate |
            window_style::InitiallyHidden |
            window_style::InitiallyRenderable |
            window_style::DropShadow |
            window_style::Popup |
            window_style::Tool;
    public:
        // Delays mirror the Windows tooltip control defaults (TTM_SETDELAYTIME):
        // initial = double-click time, autopop = 10 x initial, reshow = initial / 5.
        static constexpr std::chrono::milliseconds INITIAL_DELAY{ 500 };
        static constexpr std::chrono::milliseconds AUTOPOP_DELAY{ 5000 };
        static constexpr std::chrono::milliseconds RESHOW_DELAY{ 100 };
    public:
        tooltip(i_widget& aOwner, i_string const& aText, window_style aStyle = DEFAULT_STYLE);
        ~tooltip();
    public:
        // Delay a tool should wait before showing its tip: RESHOW_DELAY if another tip was
        // visible a moment ago (the pointer is moving from tool to tool), else INITIAL_DELAY.
        static std::chrono::milliseconds show_delay();
    public:
        i_string const& text() const;
        void set_text(i_string const& aText);
    public:
        using window::show;
        bool show(bool aVisible) override;
    public:
        double rendering_priority() const override;
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        color frame_color() const override;
    private:
        void update_position();
        static std::optional<std::chrono::steady_clock::time_point>& last_hidden();
    private:
        text_widget iText;
    };
}
