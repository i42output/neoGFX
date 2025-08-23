// anchorable.hpp
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

#include <neogfx/gui/layout/i_anchorable.hpp>

namespace neogfx
{
    template <typename Base>
    class anchorable : public Base
    {
        typedef Base base_type;
        // types
    public:
        using typename base_type::abstract_type;
        typedef neolib::unordered_flat_map<string, i_anchor*> anchor_map_type;
        // operations
    public:
        ~anchorable()
        {
        }
    public:
        using base_type::anchor_to;
        i_anchor& anchor_to(i_anchorable& aRhs, const i_string& aLhsAnchor, anchor_constraint_function aLhsFunction, const i_string& aRhsAnchor, anchor_constraint_function aRhsFunction) override
        {
            auto lhsAnchor = anchors().find(aLhsAnchor);
            auto rhsAnchor = aRhs.anchors().find(aRhsAnchor);
            if (lhsAnchor == anchors().end())
                throw anchor_not_found(aLhsAnchor);
            if (rhsAnchor == aRhs.anchors().end())
                throw anchor_not_found(aRhsAnchor);
            rhsAnchor->second()->constrain(*lhsAnchor->second(), aLhsFunction, aRhsFunction);
            return *lhsAnchor->second();
        }
        // state
    public:
        const anchor_map_type& anchors() const override
        {
            return iAnchors;
        }
        anchor_map_type& anchors() override
        {
            return iAnchors;
        }
        // state
    private:
        anchor_map_type iAnchors;
    };
}
