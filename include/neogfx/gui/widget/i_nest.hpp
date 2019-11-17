// i_nest.hpp
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

namespace neogfx
{
    class i_widget;
    class i_nested_window;

    class i_nest
    {
    public:
        virtual std::size_t nested_window_count() const = 0;
        virtual const i_nested_window& nested_window(std::size_t aIndex) const = 0;
        virtual i_nested_window& nested_window(std::size_t aIndex) = 0;
        virtual void add(i_nested_window& aWindow) = 0;
        virtual void remove(i_nested_window& aWindow) = 0;
    public:
        virtual const i_widget& as_widget() const = 0;    
        virtual i_widget& as_widget() = 0;
    };
}