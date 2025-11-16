// menu_item_widget.hpp
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

#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/i_menu_item_widget.hpp>
#include <neogfx/gui/widget/i_menu_item.hpp>

namespace neogfx
{
    class popup_menu;

    class menu_item_widget : public widget<i_menu_item_widget>
    {
        meta_object(widget<i_menu_item_widget>)
    public:
        menu_item_widget(i_menu& aMenu, i_menu_item& aMenuItem);
        menu_item_widget(i_widget& aParent, i_menu& aMenu, i_menu_item& aMenuItem);
        menu_item_widget(i_layout& aLayout, i_menu& aMenu, i_menu_item& aMenuItem);
        ~menu_item_widget();
    public:
        i_menu& menu() const override;
        i_menu_item& menu_item() const override;
    public:
        image_widget& item_icon() override; // todo: i_image_widget
        text_widget& item_text() override; // todo: i_text_widget
        void set_icon_size(size const& aIconSize) override;
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        void paint_non_client(i_graphics_context& aGc) const override;
        void paint(i_graphics_context& aGc) const override;
    public:
        color palette_color(color_role aColorRole) const override;
    public:
        bool can_capture() const override;
    public:
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override;
    protected:
        std::string mnemonic() const override;
        void mnemonic_execute() override;
        i_widget& mnemonic_widget() override;
    protected:
        bool help_active() const override;
        neogfx::help_type help_type() const override;
        i_string const& help_text() const override;
    public:
        point sub_menu_position() const;
    private:
        void init();
        virtual void select_item(bool aOpenAnySubMenu = false);
        void close_menu();
    private:
        sink iSink;
        dimension iGap = 9.0;
        size iIconSize = size{ 16.0, 16.0 };
        i_menu& iMenu;
        i_menu_item& iMenuItem;
        horizontal_layout iLayout;
        image_widget iIcon;
        text_widget iText;
        horizontal_spacer iSpacer;
        text_widget iShortcutText;
        std::optional<std::unique_ptr<widget_timer>> iSubMenuOpener;
        mutable std::optional<std::pair<color, texture>> iSubMenuArrow;
    };
}