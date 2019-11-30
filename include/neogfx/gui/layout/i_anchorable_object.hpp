// i_anchorable_object.hpp
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
#include <neolib/map.hpp>
#include <neolib/string.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gui/layout/i_anchor.hpp>

namespace neogfx
{
    struct anchor_not_found : std::runtime_error { anchor_not_found(const std::string& aAnchor) : std::runtime_error{ "neoGFX: Anchor '" + aAnchor + "' not found." } {} };

    class i_anchorable_object : public i_object
    {
        // types
    public:
        typedef i_anchorable_object abstract_type;
        typedef neolib::i_map<i_string, i_anchor*> anchor_map_type;
        // operations
    public:
        virtual void anchor_to(i_anchorable_object& aRhs, const i_string& aLhsAnchor, anchor_constraint_function aLhsFunction, const i_string& aRhsAnchor, anchor_constraint_function aRhsFunction) = 0;
        // state
    public:
        virtual const anchor_map_type& anchors() const = 0;
        virtual anchor_map_type& anchors() = 0;
        // helpers
    public:
        void anchor_to(i_anchorable_object& aRhs, const std::string& aLhsAnchor, anchor_constraint_function aLhsFunction, const std::string& aRhsAnchor, anchor_constraint_function aRhsFunction)
        {
            return anchor_to(aRhs, string{ aLhsAnchor }, aLhsFunction, string{ aRhsAnchor }, aRhsFunction);
        }
    };

    inline void layout_as_same_size(i_anchorable_object& aFirst, i_anchorable_object& aSecond)
    {
        aFirst.anchor_to(aSecond, string{ "MinimumSize" }, anchor_constraint_function::Max, string{ "MinimumSize" }, anchor_constraint_function::Max);
    }
}
