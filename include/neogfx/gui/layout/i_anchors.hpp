// i_anchors.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <map>

namespace neogfx
{
    class i_anchor_base;

    typedef std::map<std::string, i_anchor_base*> anchor_map;

    class i_anchors
    {
    public:
        virtual void register_anchor(i_anchor_base& aAnchor) = 0;
        virtual const anchor_map& anchor_map() const = 0;
        // helpers
    public:
        uint32_t count() const
        {
            return static_cast<uint32_t>(anchor_map().size());
        }
        std::string name(uint32_t aIndex) const
        {
            return std::next(anchor_map().begin(), aIndex)->first;
        }
        const i_anchor_base& anchor(uint32_t aIndex) const
        {
            return *std::next(anchor_map().begin(), aIndex)->second;
        }
        i_anchor_base& anchor(uint32_t aIndex)
        {
            return *std::next(anchor_map().begin(), aIndex)->second;
        }
    };
}
