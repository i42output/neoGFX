// toolbar.hpp
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

#include <neogfx/app/action.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/toolbar_button.hpp>

namespace neogfx
{
    enum class toolbar_style : std::uint32_t
    {
        Horizontal  = 0x00000001,
        Vertical    = 0x00000002,

        Flow        = 0x00000010,
        Collapsible = 0x00000020, // todo

        Default     = Horizontal | Flow
    };

    inline constexpr toolbar_style operator|(toolbar_style aLhs, toolbar_style aRhs)
    {
        return static_cast<toolbar_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr toolbar_style operator&(toolbar_style aLhs, toolbar_style aRhs)
    {
        return static_cast<toolbar_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    class toolbar : public widget<>, public i_action_container
    {
        meta_object(widget<>)
    private:
        typedef std::unique_ptr<toolbar_button> button_pointer;
        typedef std::vector<button_pointer> button_list;
    public:
        typedef std::uint32_t button_index;
    public:
        struct bad_button_index : std::logic_error { bad_button_index() : std::logic_error("neogfx::toolbar::bad_button_index") {} };
    public:
        toolbar(toolbar_style aStyle = toolbar_style::Default);
        toolbar(i_widget& aParent, toolbar_style aStyle = toolbar_style::Default);
        toolbar(i_layout& aLayout, toolbar_style aStyle = toolbar_style::Default);
    public:
        toolbar_style style() const;
    public:
        neogfx::size_policy size_policy() const override;
    public:
        virtual size button_image_extents() const;
        virtual void set_button_image_extents(optional_size const& aExtents);
        virtual std::uint32_t button_count() const;
        virtual const toolbar_button& button(button_index aButtonIndex) const;
        virtual toolbar_button& button(button_index aButtonIndex);
        i_action& add_action(i_action& aAction) override;
        i_action& add_action(i_ref_ptr<i_action> const& aAction) override;
        virtual void add_separator();
        virtual i_action& insert_action(button_index aButtonIndex, i_action& aAction);
        virtual i_action& insert_action(button_index aButtonIndex, i_ref_ptr<i_action> const& aAction);
        virtual void insert_separator(button_index aButtonIndex);
    private:
        toolbar_style iStyle;
        std::unique_ptr<i_layout> iLayout;
        button_list iButtons;
        action iSeparator;
        optional_size iButtonImageExtents;
    };
}