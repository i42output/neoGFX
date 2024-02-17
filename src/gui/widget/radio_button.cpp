// radio_button.cpp
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
#include <neogfx/gui/widget/radio_button.hpp>

namespace neogfx
{
    radio_button::disc::disc(radio_button& aParent)
    {
        aParent.layout().add_at(0, *this);
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    size radio_button::disc::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        dimension const length = units_converter{ *this }.from_device_units(static_cast<const radio_button&>(parent()).text_widget().font().height()* (2.0 / 3.0));
        return ceil_rasterized(service<i_skin_manager>().active_skin().preferred_size(skin_element::RadioButton, size{ length }));
    }

    size radio_button::disc::maximum_size(optional_size const& aAvailableSpace) const
    {
        return minimum_size(aAvailableSpace);
    }
        
    radio_button::radio_button(std::string const& aText) :
        base_type{ aText }, iDisc{ *this }
    {
        set_checkable(button_checkable::BiState);
        set_padding(neogfx::padding{ 0.0 });
        layout().set_padding(neogfx::padding{ 0.0 });
        layout().add_spacer();
        if (!any_siblings_on())
            set_on();
        text_widget().set_alignment(alignment::Left | alignment::VCenter);
    }

    radio_button::radio_button(i_widget& aParent, std::string const& aText) :
        base_type{ aParent, aText }, iDisc{ *this }
    {
        set_checkable(button_checkable::BiState);
        set_padding(neogfx::padding{ 0.0 });
        layout().set_padding(neogfx::padding{ 0.0 });
        layout().add_spacer();
        if (!any_siblings_on())
            set_on();
        text_widget().set_alignment(alignment::Left | alignment::VCenter);
    }

    radio_button::radio_button(i_layout& aLayout, std::string const& aText) :
        base_type{ aLayout, aText }, iDisc{ *this }
    {
        set_checkable(button_checkable::BiState);
        set_padding(neogfx::padding{ 0.0 });
        layout().set_padding(neogfx::padding{ 0.0 });
        layout().add_spacer();
        if (!any_siblings_on())
            set_on();
        text_widget().set_alignment(alignment::Left | alignment::VCenter);
    }

    neogfx::object_type radio_button::object_type() const
    {
        return object_type::RadioButton;
    }

    bool radio_button::is_on() const
    {
        return is_checked();
    }

    bool radio_button::is_off() const
    {
        return !is_checked();
    }

    void radio_button::set_on()
    {
        check();
    }

    size_policy radio_button::size_policy() const
    {
        if (has_size_policy())
            return base_type::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };

    }

    rect radio_button::element_rect(skin_element aElement) const
    {
        switch (aElement)
        {
        case skin_element::RadioButton:
            return iDisc.client_rect() + iDisc.position();
        default:
            return base_type::element_rect(aElement);
        }
    }

    void radio_button::paint(i_graphics_context& aGc) const
    {
        if (has_focus())
        {
            rect focusRect = label().client_rect() + label().position();
            aGc.draw_focus_rect(focusRect);
        }
        service<i_skin_manager>().active_skin().draw_radio_button(aGc, *this, checked_state());
    }

    void radio_button::mouse_entered(const point& aPosition)
    {
        base_type::mouse_entered(aPosition);
        update();
    }

    void radio_button::mouse_left()
    {
        base_type::mouse_left();
        update();
    }

    void radio_button::handle_clicked()
    {
        base_type::handle_clicked();
        set_on();
    }

    const i_radio_button* radio_button::next_button() const
    {
        const i_widget* candidate = &after();
        while (candidate != this)
        {
            if ((candidate->object_type() & neogfx::object_type::MASK_RESERVED_SPECIFIC) == neogfx::object_type::RadioButton && is_sibling_of(*candidate))
                return static_cast<const i_radio_button*>(candidate);
            candidate = &candidate->after();
        }
        return this;
    }

    i_radio_button* radio_button::next_button()
    {
        return const_cast<i_radio_button*>(to_const(*this).next_button());
    }

    bool radio_button::can_toggle() const
    {
        return false;
    }

    bool radio_button::set_checked_state(const std::optional<bool>& aCheckedState)
    {
        if (checked_state() == aCheckedState)
            return false;
        if (aCheckedState != std::nullopt && *aCheckedState)
            for (i_radio_button* nextRadioButton = next_button(); nextRadioButton != this; nextRadioButton = nextRadioButton->next_button())
                nextRadioButton->uncheck();
        button::set_checked_state(aCheckedState);
        if (is_on())
            On.trigger();
        else if (is_off())
            Off.trigger();
        return true;
    }

    bool radio_button::any_siblings_on() const
    {
        for (const i_radio_button* nextRadioButton = next_button(); nextRadioButton != this; nextRadioButton = nextRadioButton->next_button())
            if (nextRadioButton->is_on())
                return true;
        return false;
    }
}