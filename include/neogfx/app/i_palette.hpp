// i_palette.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/gfx/color.hpp>

namespace neogfx
{
    class i_widget;

    enum color_role : uint32_t
    {
        Theme,
        Background,
        Foreground,
        Base,
        AlternateBase,
        Text,
        Selection,
        AlternateSelection,
        SelectedText,
        Focus,
        Hover,
        PrimaryAccent,
        SecondaryAccent,
        Void
    };

    class i_palette
    {
        friend class palette;
    public:
        declare_event(changed)
    public:
        struct no_proxy : std::logic_error { no_proxy() : std::logic_error{ "neogfx::i_palette::no_proxy" } {} };
    public:
        virtual ~i_palette() = default;
    public:
        virtual bool operator==(const i_palette& aOther) const = 0;
        virtual bool operator!=(const i_palette& aOther) const = 0;
    public:
        virtual bool has_color(color_role aRole) const = 0;
        virtual neogfx::color color(color_role aRole) const = 0;
        virtual const optional_color& maybe_color(color_role aRole) const = 0;
        virtual void set_color(color_role aRole, const optional_color& aColor) = 0;
    public:
        virtual neogfx::color default_text_color_for_widget(const i_widget& aWidget) const = 0;
    public:
        virtual bool has_proxy() const = 0;
        virtual const i_palette& proxy() const = 0;
    protected:
        virtual const i_palette* proxy_ptr() const = 0;
    };
}