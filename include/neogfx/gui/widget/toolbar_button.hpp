// toolbar_button.hpp
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
#include <neogfx/app/i_action.hpp>
#include "push_button.hpp"

namespace neogfx
{
    class toolbar_button : public push_button
    {
    public:
        toolbar_button(i_action& aAction);
        toolbar_button(std::shared_ptr<i_action> aAction);
        toolbar_button(i_widget& aParent, i_action& aAction);
        toolbar_button(i_widget& aParent, std::shared_ptr<i_action> aAction);
        toolbar_button(i_layout& aLayout, i_action& aAction);
        toolbar_button(i_layout& aLayout, std::shared_ptr<i_action> aAction);
        ~toolbar_button();
    protected:
        virtual void layout_items_completed();
    public:
        const i_action& action() const;
        i_action& action();
    public:
        virtual neogfx::size_policy size_policy() const;
        virtual size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const;
        virtual size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const;
    public:
        virtual void paint(i_graphics_context& aGc) const;
    public:
        virtual color base_color() const;
    public:
        virtual neogfx::focus_policy focus_policy() const;
    public:
        virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
        virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
    protected:
        virtual void handle_clicked();
    private:
        void init();
        void update_state();
    private:
        std::shared_ptr<i_action> iAction;
        sink iSink;
    };
}