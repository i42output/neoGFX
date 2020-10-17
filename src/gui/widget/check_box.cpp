// check_box.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/i_skin_manager.hpp>
#include <neogfx/gui/widget/check_box.hpp>

namespace neogfx
{
    check_box::box::box(check_box& aParent)
    {
        aParent.layout().add_at(0, *this);
        set_padding(neogfx::padding{});
        set_ignore_mouse_events(true);
    }

    size check_box::box::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        dimension const length = units_converter(*this).from_device_units(static_cast<const check_box&>(parent()).text_widget().font().height() * (2.0 / 3.0));
        return ceil_rasterized(service<i_skin_manager>().active_skin().preferred_size(skin_element::CheckBox, size{ length }));
    }

    size check_box::box::maximum_size(optional_size const& aAvailableSpace) const
    {
        return minimum_size(aAvailableSpace);
    }
        
    check_box::check_box(std::string const& aText, button_checkable aCheckable) :
        button(aText), iBox(*this)
    {
        set_checkable(aCheckable);
        set_padding(neogfx::padding(0.0));
        layout().set_padding(neogfx::padding(0.0));
        layout().add_spacer();
        label().text_widget().set_alignment(alignment::Left | alignment::VCenter);
    }

    check_box::check_box(i_widget& aParent, std::string const& aText, button_checkable aCheckable) :
        button(aParent, aText), iBox(*this)
    {
        set_checkable(aCheckable);
        set_padding(neogfx::padding(0.0));
        layout().set_padding(neogfx::padding(0.0));
        layout().add_spacer();
        label().text_widget().set_alignment(alignment::Left | alignment::VCenter);
    }

    check_box::check_box(i_layout& aLayout, std::string const& aText, button_checkable aCheckable) :
        button(aLayout, aText), iBox(*this)
    {
        set_checkable(aCheckable);
        set_padding(neogfx::padding(0.0));
        layout().set_padding(neogfx::padding(0.0));
        layout().add_spacer();
        label().text_widget().set_alignment(alignment::Left | alignment::VCenter);
    }

    size_policy check_box::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    rect check_box::element_rect(skin_element aElement) const
    {
        switch (aElement)
        {
        case skin_element::CheckBox:
            return iBox.client_rect() + iBox.position();
        default:
            return button::element_rect(aElement);
        }
    }

    void check_box::paint(i_graphics_context& aGc) const
    {
        if (has_focus())
        {
            rect const focusRect = label().client_rect() + label().position();
            aGc.draw_focus_rect(focusRect);
        }
        service<i_skin_manager>().active_skin().draw_check_box(aGc, *this, checked_state());
    }

    void check_box::mouse_entered(const point& aPosition)
    {
        button::mouse_entered(aPosition);
        update();
    }

    void check_box::mouse_left()
    {
        button::mouse_left();
        update();
    }
}