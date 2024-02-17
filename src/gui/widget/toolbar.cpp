// toolbar.cpp
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

#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/layout/flow_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/toolbar.hpp>

namespace neogfx
{
    namespace
    {
        std::unique_ptr<i_layout> make_toolbar_layout(toolbar& aToolbar, toolbar_style aStyle)
        {
            if ((aStyle & toolbar_style::Flow) == toolbar_style::Flow)
            {
                if ((aStyle & toolbar_style::Horizontal) == toolbar_style::Horizontal)
                    return std::make_unique<flow_layout>(aToolbar, flow_layout_direction::Horizontal);
                else
                    return std::make_unique<flow_layout>(aToolbar, flow_layout_direction::Vertical);
            }
            else
            {
                if ((aStyle & toolbar_style::Horizontal) == toolbar_style::Horizontal)
                    return std::make_unique<horizontal_layout>(aToolbar);
                else
                    return std::make_unique<vertical_layout>(aToolbar);
            }
        }
    }

    toolbar::toolbar(toolbar_style aStyle) :
        iStyle{ aStyle }, 
        iLayout{ make_toolbar_layout(*this, aStyle) }
    {
    }

    toolbar::toolbar(i_widget& aParent, toolbar_style aStyle) :
        widget{ aParent }, 
        iStyle{ aStyle }, 
        iLayout{ make_toolbar_layout(*this, aStyle) }
    {
    }

    toolbar::toolbar(i_layout& aLayout, toolbar_style aStyle) :
        widget{ aLayout }, 
        iStyle{ aStyle }, 
        iLayout{ make_toolbar_layout(*this, aStyle) }
    {
    }

    toolbar_style toolbar::style() const
    {
        return iStyle;
    }

    size_policy toolbar::size_policy() const
    {
        if (widget::has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    size toolbar::button_image_extents() const
    {
        if (iButtonImageExtents != std::nullopt)
            return *iButtonImageExtents;
        return size{ 32.0_dip, 32.0_dip };
    }

    void toolbar::set_button_image_extents(optional_size const& aExtents)
    {
        iButtonImageExtents = aExtents;
        for (auto& button : iButtons)
            button->set_image_extents(button_image_extents());
    }

    uint32_t toolbar::button_count() const
    {
        return static_cast<uint32_t>(iButtons.size());
    }

    const toolbar_button& toolbar::button(button_index aButtonIndex) const
    {
        if (aButtonIndex >= button_count())
            throw bad_button_index();
        return *iButtons[aButtonIndex];
    }

    toolbar_button& toolbar::button(button_index aButtonIndex)
    {
        return const_cast<toolbar_button&>(to_const(*this).button(aButtonIndex));
    }

    i_action& toolbar::add_action(i_action& aAction)
    {
        insert_action(button_count(), aAction);
        return aAction;
    }

    i_action& toolbar::add_action(i_ref_ptr<i_action> const& aAction)
    {
        insert_action(button_count(), aAction);
        return *aAction;
    }

    void toolbar::add_separator()
    {
        insert_separator(button_count());
    }

    i_action& toolbar::insert_action(button_index aButtonIndex, i_action& aAction)
    {
        if (aButtonIndex > iButtons.size())
            throw bad_button_index();
        iButtons.insert(iButtons.begin() + aButtonIndex, std::make_unique<toolbar_button>(layout(), aAction));
        iButtons[aButtonIndex]->set_image_extents(button_image_extents());
        return aAction;
    }

    i_action& toolbar::insert_action(button_index aButtonIndex, i_ref_ptr<i_action> const& aAction)
    {
        if (aButtonIndex > iButtons.size())
            throw bad_button_index();
        iButtons.insert(iButtons.begin() + aButtonIndex, std::make_unique<toolbar_button>(layout(), aAction));
        iButtons[aButtonIndex]->set_image_extents(button_image_extents());
        return *aAction;
    }

    void toolbar::insert_separator(button_index aButtonIndex)
    {
        if (aButtonIndex > iButtons.size())
            throw bad_button_index();
        iButtons.insert(iButtons.begin() + aButtonIndex, std::make_unique<toolbar_button>(layout(), iSeparator));
    }
}