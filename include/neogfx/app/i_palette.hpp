// i_palette.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/event.hpp>
#include <neogfx/core/color.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    enum color_role : uint32_t
    {
        Theme,
        Background,
        Foreground,
        Text,
        Selection,
        Hover,
        WidgetDetailPrimary,
        WidgetDetailSecondary
    };

    class i_palette
    {
    public:
        declare_event(changed)
    public:
        virtual ~i_palette() = default;
    public:
        virtual bool has_color(color_role aRole) const = 0;
        virtual neogfx::color color(color_role aRole) const = 0;
        virtual const optional_color& maybe_color(color_role aRole) const = 0;
        virtual void set_color(color_role aRole, const optional_color& aColor) = 0;
        // helpers
    public:
        neogfx::color text_color_for_widget(const i_widget& aWidget) const
        {
            optional_color textColor;
            const i_widget* w = nullptr;
            do
            {
                if (w == nullptr)
                    w = &aWidget;
                else
                    w = &w->parent();
                if (w->has_background_color())
                {
                    textColor = w->background_color().brightness() >= 0.509 ? color::Black : color::White;
                    break;
                }
                else if (w->has_foreground_color())
                {
                    textColor = w->foreground_color().brightness() >= 0.509 ? color::Black : color::White;
                    break;
                }
            } while (w->has_parent());
            auto defaultTextColor = color(color_role::Text);
            if (textColor == std::nullopt || textColor->similar_intensity(defaultTextColor))
                textColor = defaultTextColor;
            return *textColor;
        }
    };
}