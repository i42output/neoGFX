// i_layout_item_cache.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/layout/i_layout_item.hpp>

namespace neogfx
{
    enum class layout_item_disposition
    {
        Unknown,
        Weighted,
        Unweighted,
        TooSmall,
        FixedSize
    };

    class i_layout_item_cache : public i_layout_item
    {
        friend class layout;
    public:
        virtual const i_layout_item& subject() const = 0;
        virtual i_layout_item& subject() = 0;
        virtual i_ref_ptr<i_layout_item>& subject_ptr() = 0;
    public:
        virtual layout_item_disposition& cached_disposition() const = 0;
    };
}
