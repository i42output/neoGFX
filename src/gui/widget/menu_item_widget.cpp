// menu_item_widget.cpp
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
#include <neogfx/gui/widget/menu_item_widget.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/gui/window/popup_menu.hpp>
#include <neogfx/app/i_app.hpp>

namespace neogfx
{
    menu_item_widget::menu_item_widget(i_menu& aMenu, i_menu_item& aMenuItem) :
        widget{ aMenu.as_widget() }, iMenu { aMenu }, iMenuItem{ aMenuItem }, iLayout{ *this }, iIcon{ iLayout, texture{}, aspect_ratio::Keep }, iText{ iLayout }, iSpacer{ iLayout }, iShortcutText{ iLayout }
    {
        init();
    }

    menu_item_widget::menu_item_widget(i_widget& aParent, i_menu& aMenu, i_menu_item& aMenuItem) :
        widget{ aParent }, iMenu{ aMenu }, iMenuItem{ aMenuItem }, iLayout{ *this }, iIcon{ iLayout, texture{}, aspect_ratio::Keep }, iText{ iLayout }, iSpacer{ iLayout }, iShortcutText{ iLayout }
    {
        init();
    }

    menu_item_widget::menu_item_widget(i_layout& aLayout, i_menu& aMenu, i_menu_item& aMenuItem) :
        widget{ aLayout }, iMenu{ aMenu }, iMenuItem{ aMenuItem }, iLayout{ *this }, iIcon{ iLayout, texture{}, aspect_ratio::Keep }, iText{ iLayout }, iSpacer{ iLayout }, iShortcutText{ iLayout }
    {
        init();
    }

    menu_item_widget::~menu_item_widget()
    {
        service<i_app>().remove_mnemonic(*this);
        iSubMenuOpener.reset();
    }

    i_menu& menu_item_widget::menu() const
    {
        return iMenu;
    }

    i_menu_item& menu_item_widget::menu_item() const
    {
        return iMenuItem;
    }

    neogfx::size_policy menu_item_widget::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return neogfx::size_policy{ menu().type() == menu_type::Popup ? size_constraint::Expanding : size_constraint::Minimum, size_constraint::Minimum };
    }

    size menu_item_widget::minimum_size(optional_size const&) const
    {
        size result = widget::minimum_size();
        if (menu_item().type() == menu_item_type::Action && menu_item().action().is_separator())
            result.cy = units_converter(*this).from_device_units(3.0_dip);
        return result;
    }

    void menu_item_widget::paint_non_client(i_graphics_context& aGc) const
    {
        if (menu().has_selected_item() && menu().selected_item() == (menu().find(menu_item())))
        {
            bool openSubMenu = (menu_item().type() == menu_item_type::SubMenu && menu_item().sub_menu().is_open());
            color fillColor = background_color().dark() ? color::Black : color::White;
            if (openSubMenu && menu().type() == menu_type::MenuBar)
            {
                if (fillColor.similar_intensity(service<i_app>().current_style().palette().color(color_role::Theme), 0.05))
                    fillColor = service<i_app>().current_style().palette().color(color_role::Selection);
            }
            else if (fillColor.similar_intensity(background_color(), 0.05))
                fillColor = fillColor.shaded(0x20);
            fillColor.set_alpha(0xC0);
            aGc.fill_rect(client_rect(), fillColor);
        }
    }

    void menu_item_widget::paint(i_graphics_context& aGc) const
    {
        if (menu_item().type() != menu_item_type::Action || !menu_item().action().is_separator())
        {
            widget::paint(aGc);
            if (menu_item().type() == menu_item_type::SubMenu && menu().type() == menu_type::Popup)
            {
                bool openSubMenu = (menu_item().type() == menu_item_type::SubMenu && menu_item().sub_menu().is_open());
                color ink = openSubMenu ? service<i_app>().current_style().palette().color(color_role::Selection)
                    : background_color().shaded(0x80);
                if (iSubMenuArrow == std::nullopt || iSubMenuArrow->first != ink)
                {
                    const char* sArrowImagePattern
                    {
                        "[6,9]"
                        "{0,paper}"
                        "{1,ink}"

                        "000000"
                        "010000"
                        "011000"
                        "011100"
                        "011110"
                        "011100"
                        "011000"
                        "010000"
                        "000000"
                    };
                    const char* sArrowHighDpiImagePattern
                    {
                        "[12,18]"
                        "{0,paper}"
                        "{1,ink}"

                        "000000000000"
                        "001000000000"
                        "001100000000"
                        "001110000000"
                        "001111000000"
                        "001111100000"
                        "001111110000"
                        "001111111000"
                        "001111111100"
                        "001111111100"
                        "001111111000"
                        "001111110000"
                        "001111100000"
                        "001111000000"
                        "001110000000"
                        "001100000000"
                        "001000000000"
                        "000000000000"
                    };
                    iSubMenuArrow.emplace(ink, !high_dpi() ? 
                        image{ "neogfx::menu_item_widget::sArrowImagePattern::" + ink.to_string(), sArrowImagePattern, { { "paper", color{} },{ "ink", ink } } } : 
                        image{ "neogfx::menu_item_widget::sArrowHighDpiImagePattern::" + ink.to_string(), sArrowHighDpiImagePattern,{ { "paper", color{} },{ "ink", ink } }, 2.0 });
                }
                rect rect = client_rect(false);
                aGc.draw_texture(
                    point{ rect.right() - dpi_scale(iGap) + std::floor((dpi_scale(iGap) - iSubMenuArrow->second.extents().cx) / 2.0), std::floor((rect.height() - iSubMenuArrow->second.extents().cy) / 2.0) },
                    iSubMenuArrow->second);
            }
        }
        else
        {
            scoped_units su{ *this, units::Pixels };
            rect line = client_rect(false);
            line.y += 1.0_dip;
            line.cy = 1.0_dip;
            line.x += dpi_scale(iGap);
            line.cx -= dpi_scale(iGap * 2.0);
            color ink = color::Black;
            if (ink.similar_intensity(background_color(), 0.05))
                ink = ink.shaded(0x20);
            ink.set_alpha(0x40);
            aGc.fill_rect(line, ink);
        }
    }

    color menu_item_widget::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return widget::palette_color(aColorRole);
        if (aColorRole == color_role::Background && parent().has_palette_color(aColorRole))
            return parent().palette_color(aColorRole);
        return widget::palette_color(aColorRole);
    }

    bool menu_item_widget::can_capture() const
    {
        return false;
    }

    void menu_item_widget::mouse_entered(const point& aPosition)
    {
        widget::mouse_entered(aPosition);
        update();
        if (menu_item().available())
            menu().select_item_at(menu().find(menu_item()), menu_item().type() == menu_item_type::SubMenu);
    }

    void menu_item_widget::mouse_left()
    {
        widget::mouse_left();
        update();
        if (menu().has_selected_item() && menu().selected_item() == (menu().find(menu_item())) &&
            (menu_item().type() == menu_item_type::Action || (!menu_item().sub_menu().is_open() && !iSubMenuOpener)))
            menu().clear_selection();
    }

    void menu_item_widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left && menu_item().type() == menu_item_type::SubMenu)
            select_item();
    }

    void menu_item_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        widget::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left && menu_item().type() == menu_item_type::Action)
            select_item();
    }

    bool menu_item_widget::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
    {
        if (aScanCode == ScanCode_RETURN)
        {
            select_item(true);
            return true;
        }
        return false;
    }

    std::string menu_item_widget::mnemonic() const
    {
        return mnemonic_from_text(iText.text());
    }

    void menu_item_widget::mnemonic_execute()
    {
        select_item(true);
    }

    i_widget& menu_item_widget::mnemonic_widget()
    {
        return iText;
    }

    bool menu_item_widget::help_active() const
    {
        return menu_item().type() == menu_item_type::Action &&
            menu().has_selected_item() && menu().find(menu_item()) == menu().selected_item();
    }

    help_type menu_item_widget::help_type() const
    {
        return neogfx::help_type::Action;
    }

    i_string const& menu_item_widget::help_text() const
    {
        if (help_active())
            return menu_item().action().help_text();
        else
            return empty_string;
    }

    point menu_item_widget::sub_menu_position() const
    {
        if (menu().type() == menu_type::MenuBar)
            return non_client_rect().bottom_left() + root().window_position();
        else
            return non_client_rect().top_right() + root().window_position();
    }

    void menu_item_widget::init()
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(dpi_scale(neogfx::padding{ iGap, dpi_scale(2.0), iGap * (menu().type() == menu_type::Popup ? 2.0 : 1.0), dpi_scale(2.0) }));
        iLayout.set_spacing(dpi_scale(size{ iGap, 0.0 }));
        if (menu().type() == menu_type::Popup)
            iIcon.set_fixed_size(dpi_scale(iIconSize));
        else
            iIcon.set_fixed_size(size{});
        iSpacer.set_minimum_size(size{ 0.0, 0.0 });
        iText.set_font_role(font_role::Menu);
        auto text_updated = [this]()
        {
            auto m = mnemonic_from_text(iText.text());
            if (!m.empty())
                service<i_app>().add_mnemonic(*this);
            else
                service<i_app>().remove_mnemonic(*this);
        };
        iSink += iText.TextChanged(text_updated);
        text_updated();
        if (menu_item().type() == menu_item_type::Action)
        {
            auto action_changed = [this]()
            {
                iIcon.set_image(menu_item().action().is_unchecked() ? menu_item().action().image() : menu_item().action().checked_image());
                if (iIcon.image().is_empty() && menu_item().action().is_checked())
                {
                    const char* sTickPattern
                    {
                        "[16,8]"
                        "{0,paper}"
                        "{1,ink}"

                        "0000000000100000"
                        "0000000001100000"
                        "0000000001000000"
                        "0000000011000000"
                        "0000010010000000"
                        "0000011110000000"
                        "0000001100000000"
                        "0000001100000000"
                    };
                    const char* sTickHighDpiPattern
                    {
                        "[32,16]"
                        "{0,paper}"
                        "{1,ink}"

                        "00000000000000000000110000000000"
                        "00000000000000000000110000000000"
                        "00000000000000000001110000000000"
                        "00000000000000000011100000000000"
                        "00000000000000000011000000000000"
                        "00000000000000000011000000000000"
                        "00000000000000000111000000000000"
                        "00000000000000001110000000000000"
                        "00000000001100001100000000000000"
                        "00000000001100001100000000000000"
                        "00000000001110011100000000000000"
                        "00000000000111111000000000000000"
                        "00000000000011110000000000000000"
                        "00000000000011110000000000000000"
                        "00000000000001100000000000000000"
                        "00000000000001100000000000000000"
                    };
                    color ink = service<i_app>().current_style().palette().color(color_role::Text);
                    iIcon.set_image(!high_dpi() ?
                        image{ "neogfx::menu_item_widget::sTickPattern::" + ink.to_string(), sTickPattern,{ { "paper", color{} },{ "ink", ink } } } :
                        image{ "neogfx::menu_item_widget::sTickHighDpiPattern::" + ink.to_string(), sTickHighDpiPattern,{ { "paper", color{} },{ "ink", ink } }, 2.0 });
                }
                if (!iIcon.image().is_empty())
                    iIcon.set_fixed_size(dpi_scale(iIconSize));
                else if (menu().type() == menu_type::MenuBar)
                    iIcon.set_fixed_size(size{});
                iText.set_text(menu_item().action().menu_text());
                if (menu().type() != menu_type::MenuBar)
                    iShortcutText.set_text(menu_item().action().shortcut() != std::nullopt ? menu_item().action().shortcut()->as_text() : std::string());
                iSpacer.set_minimum_size(dpi_scale(size{ menu_item().action().shortcut() != std::nullopt && menu().type() != menu_type::MenuBar ? iGap * 2.0 : 0.0, 0.0 }));
                enable(menu_item().action().is_enabled());
            };
            iSink += menu_item().action().changed(action_changed);
            iSink += menu_item().action().checked(action_changed);
            iSink += menu_item().action().unchecked(action_changed);
            iSink += menu_item().action().enabled(action_changed);
            iSink += menu_item().action().disabled(action_changed);
            action_changed();
        }
        else
        {
            iSink += menu_item().sub_menu().opened([this]() {update(); });
            iSink += menu_item().sub_menu().closed([this]() {update(); });
            auto menu_changed = [this]() 
            { 
                iIcon.set_image(menu_item().sub_menu().image());
                iText.set_text(menu_item().sub_menu().title());
            };
            iSink += menu_item().sub_menu().menu_changed(menu_changed);
            menu_changed();
        }
        iSink += menu_item().selected([this]()
        {
            if (menu_item().type() == menu_item_type::Action)
                service<i_app>().help().activate(*this);
            else if (menu_item().type() == menu_item_type::SubMenu && menu_item().open_any_sub_menu() && menu().type() == menu_type::Popup)
            {
                if (!iSubMenuOpener)
                {
                    iSubMenuOpener = std::make_unique<neolib::callback_timer>(service<i_async_task>(), [this](neolib::callback_timer&)
                    {
                        destroyed_flag destroyed{ *this };
                        if (!menu_item().sub_menu().is_open())
                            menu().open_sub_menu().trigger(menu_item().sub_menu());
                        if (!destroyed)
                            update();
                        iSubMenuOpener.reset();
                    }, 250);
                }
            }
        });
        iSink += menu_item().deselected([this]()
        {
            if (menu_item().type() == menu_item_type::Action)
                service<i_app>().help().deactivate(*this);
            iSubMenuOpener.reset();
        });
    }

    void menu_item_widget::select_item(bool aOpenAnySubMenu)
    {
        destroyed_flag destroyed{ *this };
        if (!menu_item().available())
            return;
        if (menu_item().type() == menu_item_type::Action)
        {
            menu().clear_selection();
            if (destroyed)
                return;
            menu_item().action().triggered().trigger();
            if (destroyed)
                return;
            if (menu_item().action().is_checkable())
                menu_item().action().toggle();
            if (destroyed)
                return;
            i_menu* menuToClose = &menu();
            while (menuToClose->has_parent())
                menuToClose = &menuToClose->parent();
            if (menuToClose->type() == menu_type::MenuBar)
                menuToClose->clear_selection();
            else if (menuToClose->type() == menu_type::Popup && menuToClose->is_open())
                menuToClose->close();
            if (destroyed)
                return;
        }
        else
        {
            if (!menu_item().sub_menu().is_open())
            {
                menu().select_item_at(menu().find(menu_item()), aOpenAnySubMenu);
                if (destroyed)
                    return;
                menu().open_sub_menu().trigger(menu_item().sub_menu());
                if (destroyed)
                    return;
                update();
            }
            else if (menu().type() == menu_type::MenuBar)
            {
                menu_item().sub_menu().close();
                if (destroyed)
                    return;
                update();
            }
            else
            {
                auto& subMenu = menu_item().sub_menu();
                if (!subMenu.has_selected_item() && subMenu.has_available_items())
                    subMenu.select_item_at(subMenu.first_available_item(), false);
            }
        }
    }
}