// nest.hpp
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

#include <neogfx/gui/widget/i_nest.hpp>

namespace neogfx
{
    class nest : public i_nest
    {
    public:
        nest(i_widget& aWidget);
    public:
        std::size_t nested_window_count() const override;
        const i_nested_window& nested_window(std::size_t aIndex) const override;
        i_nested_window& nested_window(std::size_t aIndex) override;
        void add(i_nested_window& aWindow) override;
        void remove(i_nested_window& aWindow) override;
    public:
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
    private:
        i_widget& iWidget;
        std::vector<i_nested_window*> iNestedWindows;
    };
}