// i_spacer.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/i_enum.hpp>
#include <neogfx/gui/layout/i_layout_item.hpp>

namespace neogfx
{
    class i_widget;
    class i_layout;

    enum class expansion_policy : uint32_t
    {
        ExpandHorizontally  = 0x0001,
        ExpandVertically    = 0x0002
    };

    inline constexpr expansion_policy operator|(expansion_policy aLhs, expansion_policy aRhs)
    {
        return static_cast<expansion_policy>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr expansion_policy operator&(expansion_policy aLhs, expansion_policy aRhs)
    {
        return static_cast<expansion_policy>(static_cast<uint32_t>(aLhs)& static_cast<uint32_t>(aRhs));
    }
}

template <>
const neolib::enum_enumerators_t<neogfx::expansion_policy> neolib::enum_enumerators_v<neogfx::expansion_policy>
{
    declare_enum_string(neogfx::expansion_policy, ExpandHorizontally)
    declare_enum_string(neogfx::expansion_policy, ExpandVertically)
};

namespace neogfx
{
    class i_spacer : public i_layout_item
    {
    public:
        typedef i_spacer abstract_type;
        typedef std::optional<size> optional_weight;
    public:
        virtual ~i_spacer() = default;
    public:
        virtual neogfx::expansion_policy expansion_policy() const = 0;
        virtual void set_expansion_policy(neogfx::expansion_policy aExpansionPolicy) = 0;
    };
}