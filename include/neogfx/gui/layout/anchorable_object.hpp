// anchorable_object.hpp
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
#include <neogfx/core/object.hpp>
#include <neogfx/gui/layout/i_anchors.hpp>

namespace neogfx
{
    template <typename Base>
    class anchorable_object : public object<Base>, public i_anchors
    {
    public:
        const i_anchors& anchors() const override
        {
            return *this;
        }
        i_anchors& anchors() override
        {
            return *this;
        }
        // i_properties
    public:
        void register_anchor(i_anchor_base& aAnchor) override
        {
            iAnchors.emplace(aAnchor.name(), &aAnchor);
        }
        const neogfx::anchor_map& anchor_map() const override
        {
            return iAnchors;
        }
        // state
    private:
        neogfx::anchor_map iAnchors;
    };
}
