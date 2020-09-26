// toolbar_button.cpp
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
#include <neogfx/gui/widget/toolbar_button.hpp>

namespace neogfx
{
    toolbar_button::toolbar_button(i_action& aAction) : 
        push_button{ aAction.button_text(), push_button_style::Toolbar }, iAction{ std::shared_ptr<i_action>{}, &aAction }
    {
        init();
    }

    toolbar_button::toolbar_button(std::shared_ptr<i_action> aAction) :
        push_button{ aAction->button_text(), push_button_style::Toolbar }, iAction{ aAction }
    {
        init();
    }

    toolbar_button::toolbar_button(i_widget& aParent, i_action& aAction) :
        push_button{ aParent, aAction.button_text(), push_button_style::Toolbar }, iAction{ std::shared_ptr<i_action>{}, &aAction }
    {
        init();
    }

    toolbar_button::toolbar_button(i_widget& aParent, std::shared_ptr<i_action> aAction) :
        push_button{ aParent, aAction->button_text(), push_button_style::Toolbar }, iAction{ aAction }
    {
        init();
    }

    toolbar_button::toolbar_button(i_layout& aLayout, i_action& aAction) :
        push_button{ aLayout, aAction.button_text(), push_button_style::Toolbar }, iAction{ std::shared_ptr<i_action>{}, &aAction }
    {
        init();
    }

    toolbar_button::toolbar_button(i_layout& aLayout, std::shared_ptr<i_action> aAction) :
        push_button{ aLayout, aAction->button_text(), push_button_style::Toolbar }, iAction{ aAction }
    {
        init();
    }

    toolbar_button::~toolbar_button()
    {
    }
    
    void toolbar_button::layout_items_completed()
    {
        push_button::layout_items_completed();
        if (capturing())
        {
            point pt = label().position();
            pt += point{ 1.0, 0.0 };
            label().move(pt);
        }
    }

    const i_action& toolbar_button::action() const
    {
        return *iAction;
    }

    i_action& toolbar_button::action()
    {
        return *iAction;
    }

    neogfx::size_policy toolbar_button::size_policy() const
    {
        if (push_button::has_size_policy() || !action().is_separator())
            return push_button::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
    }

    size toolbar_button::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (push_button::has_minimum_size() || !action().is_separator())
            return push_button::minimum_size(aAvailableSpace);
        return units_converter(*this).from_device_units(size{ 2.0, 2.0 });
    }    

    size toolbar_button::maximum_size(const optional_size& aAvailableSpace) const
    {
        if (push_button::has_maximum_size() || !action().is_separator())
            return push_button::maximum_size(aAvailableSpace);
        return size::max_size();
    }

    void toolbar_button::paint(i_graphics_context& aGc) const
    {
        if (!action().is_separator())
            push_button::paint(aGc);
        else
        {
            scoped_units su(*this, units::Pixels);
            rect line = client_rect();
            line.deflate(0, std::floor(client_rect().height() / 6.0));
            line.cx = 1.0;
            color ink = (has_base_color() ? base_color() : service<i_app>().current_style().palette().color(color_role::Base));
            aGc.fill_rect(line, ink.darker(0x40));
            ++line.x;
            aGc.fill_rect(line, ink.lighter(0x40));
        }
    }

    color toolbar_button::base_color() const
    {
        if (push_button::has_base_color())
            return push_button::base_color();
        return color{};
    }

    focus_policy toolbar_button::focus_policy() const
    {
        return neogfx::focus_policy::NoFocus;
    }

    void toolbar_button::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        push_button::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        layout_items(false);
    }

    void toolbar_button::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        push_button::mouse_button_released(aButton, aPosition);
        layout_items(false);
    }

    void toolbar_button::handle_clicked()
    {
        push_button::handle_clicked();
        if (action().is_enabled() && !action().is_separator())
        {
            action().triggered().async_trigger();
            if (action().is_checkable())
            {
                if (is_checked())
                    action().check();
                else
                    action().uncheck();
            }
        }
    }

    void toolbar_button::init()
    {
        iSink += action().changed([this]() { update_state(); });
        update_state();
        iSink += action().enabled([this]() { enable(); });
        iSink += action().disabled([this]() { disable(); });
        enable(action().is_enabled());
        auto update_checked = [this]()
        {
            if (is_checked())
            {
                action().check();
                image_widget().set_image(action().checked_image());
            }
            else
            {
                action().uncheck();
                image_widget().set_image(action().image());
            }
        };
        iSink += Checked(update_checked);
        iSink += Unchecked(update_checked);
        iSink += action().checked([this]() {set_checked(true); });
        iSink += action().unchecked([this]() {set_checked(false); });
        set_checked(action().is_checked());
    }

    void toolbar_button::update_state()
    {
        if (action().is_checkable())
            set_checkable();
        label().set_placement(label_placement::ImageVertical);
        set_text(action().button_text());
        set_image(action().image());
        image_widget().set_aspect_ratio(aspect_ratio::KeepExpanding);
    }
}