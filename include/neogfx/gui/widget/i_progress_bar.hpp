// i_progress_bar.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2022 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{   
    enum class progress_bar_style : std::uint32_t
    {
        Default = 0
    };

    class i_progress_bar
    {
    public:
        virtual progress_bar_style style() const = 0;
        virtual void set_style(progress_bar_style aStyle) = 0;
        virtual i_string const& text() const = 0;
        virtual void set_text(i_string const& aText) = 0;
    public:
        virtual scalar value() const = 0;
        virtual void set_value(scalar aValue) = 0;
        virtual scalar minimum() const = 0;
        virtual void set_minimum(scalar aMinimum) = 0;
        virtual scalar maximum() const = 0;
        virtual void set_maximum(scalar aMaximum) = 0;
    public:
        virtual rect bar_rect() const = 0;
        virtual i_string const& value_as_text() const = 0;
    };
}