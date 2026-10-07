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
#include <neogfx/gui/dialog/dialog_button_box_bits.hpp>

namespace neogfx
{
    class dialog_button_box : public widget<>
    {
        meta_object(widget<>)
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
        void add_button(standard_button aStandardButton, button_role aButtonRole, std::string const& aButtonText);
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