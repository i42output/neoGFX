// dock.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/vector.hpp>
#include <neogfx/gui/widget/framed_widget.hpp>
#include <neogfx/gui/widget/i_dock.hpp>

namespace neogfx
{
    class dock : public framed_widget, public i_dock
    {
    public:
        using item = ref_ptr<i_dockable>;
        using item_list = neolib::vector<item>;
    private:
        using abstract_item = abstract_t<item>;
    public:
        dock(i_widget& aParent, dock_area aArea);
        dock(i_layout& aLayout, dock_area aArea);
    public:
        dock_area area() const override;
        void set_area(dock_area aArea) override;
    public:
        const item_list& items() const override;
    protected:
        color frame_color() const override;
    private:
        void add(const abstract_item& aItem) override;
        void remove(const abstract_item& aItem) override;
    private:
        void update_layout();
    private:
        dock_area iArea;
        item_list iItems;
    };
}