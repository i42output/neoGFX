// i_button.hpp
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
#include <neolib/i_enum.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>

namespace neogfx
{
    enum class button_checkable
    {
        NotCheckable,
        BiState,
        TriState
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::button_checkable> neolib::enum_enumerators_v<neogfx::button_checkable>
{
    declare_enum_string(neogfx::button_checkable, NotCheckable)
    declare_enum_string(neogfx::button_checkable, BiState)
    declare_enum_string(neogfx::button_checkable, TriState)
};

namespace neogfx
{
    typedef std::optional<bool> button_checked_state;

    struct not_tri_state_checkable : public std::logic_error { not_tri_state_checkable() : std::logic_error("neogfx::not_tri_state_checkable") {} };

    class i_button : public virtual i_skinnable_item
    {
    public:
        declare_event(pressed)
        declare_event(clicked)
        declare_event(double_clicked)
        declare_event(right_clicked)
        declare_event(released)
    public:
        declare_event(toggled)
        declare_event(checked)
        declare_event(unchecked)
        declare_event(indeterminate)
    public:
        virtual button_checkable checkable() const = 0;
        virtual void set_checkable(button_checkable aCheckable = button_checkable::BiState) = 0;
        virtual bool is_checked() const = 0;
        virtual bool is_unchecked() const = 0;
        virtual bool is_indeterminate() const = 0;
        virtual void check() = 0;
        virtual void uncheck() = 0;
        virtual void set_indeterminate() = 0;
        virtual void set_checked(bool aChecked) = 0;
        virtual void toggle() = 0;
    };
}