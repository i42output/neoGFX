// nest.hpp
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

#include <set>

#include <neogfx/core/event.hpp>
#include <neogfx/hid/i_nest.hpp>

namespace neogfx
{
    class nest : public i_nest
    {
    public:
        nest(i_widget& aWidget, nest_type aType);
        ~nest();
    public:
        i_widget& widget() const override;
        nest_type type() const override;
    public:
        bool has(i_native_window const& aNestedWindow) const override;
        void add(i_native_window& aNestedWindow) override;
        void remove(i_native_window& aNestedWindow) override;
    private:
        i_widget& iWidget;
        nest_type iType;
        std::set<i_native_window const*> iNestedWindows;
        sink iSink;
    };
}