// i_terminal.hpp
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
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    class i_terminal : public i_widget
    {
    public:
        typedef basic_size<std::int32_t> size_type;
        typedef basic_point<std::int32_t> point_type;
        typedef size_type::dimension_type dimension_type;
        typedef point_type::coordinate_type coordinate_type;
    public:
        declare_event(input, i_string const&)
        declare_event(terminal_resized, size_type)
    public:
        virtual void set_text_attributes(optional_text_attributes const& aTextAttributes) = 0;
    public:
        virtual size_type terminal_size() const = 0;
    public:
        virtual void output(i_string const& aOutput) = 0;
    };
}