// check_box.hpp
/*
neogfx C++ GUI Library
Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/gui/widget/button.hpp>
#include <neogfx/gui/layout/spacer.hpp>

namespace neogfx
{
    class check_box : public button
    {
    private:
        class box : public widget
        {
        public:
            box(check_box& aParent);
        public:
            size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
            size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
        };
    public:
        check_box(const std::string& aText = std::string(), button_checkable aCheckable = button_checkable::BiState);
        check_box(i_widget& aParent, const std::string& aText = std::string(), button_checkable aCheckable = button_checkable::BiState);
        check_box(i_layout& aLayout, const std::string& aText = std::string(), button_checkable aCheckable = button_checkable::BiState);
    public:
        neogfx::size_policy size_policy() const override;
    public:
        rect element_rect(skin_element aElement) const override;
    protected:
        void paint(i_graphics_context& aGc) const override;
    protected:
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
    private:
        box iBox;
    };
}