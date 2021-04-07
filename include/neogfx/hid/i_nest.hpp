// i_nest.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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
    class i_native_window;

    enum class nest_type
    {
        Caddy,
        FullScreen,
        MDI
    };

    class i_nest
    {
    public:
        virtual ~i_nest() = default;
    public:
        virtual i_widget const& widget() const = 0;
        virtual nest_type type() const = 0;
    public:
        virtual bool has(i_native_window const& aNestedWindow) const = 0;
        virtual void add(i_native_window& aNestedWindow) = 0;
        virtual void remove(i_native_window& aNestedWindow) = 0;
    };
}