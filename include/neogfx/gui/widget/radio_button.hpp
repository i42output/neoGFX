// radio_button.hpp
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
#include "button.hpp"

namespace neogfx
{
    extern template class widget<i_radio_button>;
    extern template class button<i_radio_button>;

    class radio_button : public button<i_radio_button>
    {
        meta_object(button<i_radio_button>)
    public:
        define_event(On, on)
        define_event(Off, off)
    private:
        class disc : public widget<>
        {
        public:
            disc(radio_button& aParent);
        public:
            size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
            size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        };
    public:
        radio_button(std::string const& aText = std::string());
        radio_button(i_widget& aParent, std::string const& aText = std::string());
        radio_button(i_layout& aLayout, std::string const& aText = std::string());
    protected:
        neogfx::object_type object_type() const override;
    public:
        bool is_on() const override;
        bool is_off() const override;
        void set_on() override;
    public:
        const i_radio_button* next_button() const override;
        i_radio_button* next_button() override;
        bool any_siblings_on() const override;
    public:
        neogfx::size_policy size_policy() const override;
    public:
        rect element_rect(skin_element aElement) const override;
    protected:
        void paint(i_graphics_context& aGc) const override;
    protected:
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
    protected:
        void handle_clicked() override;
    protected:
        bool can_toggle() const override;
        bool set_checked_state(const std::optional<bool>& aCheckedState) override;
    private:
        disc iDisc;
    };
}