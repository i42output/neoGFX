// dialog_button_box.hpp
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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx
{
    enum class standard_button : uint32_t
    {
        Ok = 0x00000001,
        Cancel = 0x00000002,
        Close = 0x00000004,
        Discard = 0x00000008,
        Apply = 0x00000010,
        Reset = 0x00000020,
        RestoreDefaults = 0x00000040,
        Yes = 0x00000080,
        No = 0x00000100,
        YesToAll = 0x00000200,
        NoToAll = 0x00000400,
        Abort = 0x00000800,
        Retry = 0x00001000,
        Ignore = 0x00002000,
        Open = 0x00004000,
        Save = 0x00008000,
        SaveAll = 0x00010000,
        Help = 0x00020000,
        Custom1 = 0x10000000,
        Custom2 = 0x20000000,
        Custom3 = 0x40000000,
        Custom4 = 0x80000000
    };

    inline constexpr standard_button operator|(standard_button aLhs, standard_button aRhs)
    {
        return static_cast<standard_button>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr standard_button operator&(standard_button aLhs, standard_button aRhs)
    {
        return static_cast<standard_button>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    enum class button_role
    {
        Invalid,
        Accept,
        Reject,
        Destructive,
        Action,
        Apply,
        Reset,
        Yes,
        No,
        Help
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::standard_button> neolib::enum_enumerators_v<neogfx::standard_button>
{
    declare_enum_string(neogfx::standard_button, Ok)
    declare_enum_string(neogfx::standard_button, Cancel)
    declare_enum_string(neogfx::standard_button, Close)
    declare_enum_string(neogfx::standard_button, Discard)
    declare_enum_string(neogfx::standard_button, Apply)
    declare_enum_string(neogfx::standard_button, Reset)
    declare_enum_string(neogfx::standard_button, RestoreDefaults)
    declare_enum_string(neogfx::standard_button, Yes)
    declare_enum_string(neogfx::standard_button, No)
    declare_enum_string(neogfx::standard_button, YesToAll)
    declare_enum_string(neogfx::standard_button, NoToAll)
    declare_enum_string(neogfx::standard_button, Abort)
    declare_enum_string(neogfx::standard_button, Retry)
    declare_enum_string(neogfx::standard_button, Ignore)
    declare_enum_string(neogfx::standard_button, Open)
    declare_enum_string(neogfx::standard_button, Save)
    declare_enum_string(neogfx::standard_button, SaveAll)
    declare_enum_string(neogfx::standard_button, Help)
    declare_enum_string(neogfx::standard_button, Custom1)
    declare_enum_string(neogfx::standard_button, Custom2)
    declare_enum_string(neogfx::standard_button, Custom3)
    declare_enum_string(neogfx::standard_button, Custom4)
};

template <>
const neolib::enum_enumerators_t<neogfx::button_role> neolib::enum_enumerators_v<neogfx::button_role>
{
    declare_enum_string(neogfx::button_role, Invalid)
    declare_enum_string(neogfx::button_role, Accept)
    declare_enum_string(neogfx::button_role, Reject)
    declare_enum_string(neogfx::button_role, Destructive)
    declare_enum_string(neogfx::button_role, Action)
    declare_enum_string(neogfx::button_role, Apply)
    declare_enum_string(neogfx::button_role, Reset)
    declare_enum_string(neogfx::button_role, Yes)
    declare_enum_string(neogfx::button_role, No)
    declare_enum_string(neogfx::button_role, Help)
};

namespace neogfx
{
    class dialog_button_box : public widget
    {
    public:
        define_event(Accepted, accepted)
        define_event(Rejected, rejected)
    public:
        define_event(Clicked, clicked, standard_button)
    public:
        typedef std::pair<button_role, std::string> button_details;
    private:
        typedef std::pair<standard_button, button_role> button_key;
        struct button_sorter
        {
            bool operator()(const button_key& aLhs, const button_key& aRhs) const;
        };
        typedef std::multimap<button_key, std::unique_ptr<push_button>, button_sorter> button_list;
    public:
        struct button_not_found : std::logic_error { button_not_found() : std::logic_error("neogfx::dialog_button_box::button_not_found") {} };
    public:
        dialog_button_box(i_widget& aParent);
        dialog_button_box(i_layout& aLayout);
        ~dialog_button_box();
    public:
        standard_button button_with_role(button_role aButtonRole) const;
        button_role role_of_button(standard_button aStandardButton);
        void enable_role(button_role aButtonRole);
        void disable_role(button_role aButtonRole);
        push_button& button(standard_button aStandardButton) const;
        void add_button(standard_button aStandardButton);
        void add_button(standard_button aStandardButton, button_role aButtonRole, const std::string& aButtonText);
        void add_buttons(standard_button aStandardButtons);
        void set_default_button(standard_button aButton);
        void clear();
        i_layout& option_layout();
    public:
        static bool has_reject_role(standard_button aStandardButtons);
        static button_details standard_button_details(standard_button aStandardButton);
    private:
        void init();
        bool can_reject() const;
    private:
        static bool similar_role(button_role aButtonRole1, button_role aButtonRole2);
    private:
        sink iSink;
        horizontal_layout iLayout;
        horizontal_layout iOptionLayout;
        horizontal_spacer iSpacer;
        horizontal_layout iStandardButtonLayout;
        button_list iButtons;
        std::optional<standard_button> iDefaultButton;
    };
}