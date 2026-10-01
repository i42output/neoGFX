// menu_bar.cpp
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

#include <boost/locale.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/menu_item_widget.hpp>
#include <neogfx/gui/window/popup_menu.hpp>

namespace neogfx
{
    menu_bar::menu_bar() : 
        menu{ {}, menu_type::MenuBar },
        iLayout{ *this }, 
        iOpenSubMenu{ std::make_unique<popup_menu>(*this, point{}) }
    {
        init();
    }

    menu_bar::menu_bar(i_widget& aParent) : 
        widget{ aParent }, 
        menu{ {}, menu_type::MenuBar }, 
        iLayout{ *this }, 
        iOpenSubMenu{ std::make_unique<popup_menu>(*this, point{}) }
    {
        init();
    }

    menu_bar::menu_bar(i_layout& aLayout) : 
        widget{ aLayout }, 
        menu{ {}, menu_type::MenuBar }, 
        iLayout{ *this }, 
        iOpenSubMenu{ std::make_unique<popup_menu>(*this, point{}) }
    {
        init();
    }

    menu_bar::~menu_bar()
    {
        close_sub_menu();
        iOpenSubMenu.reset();
        remove_all();
    }

    const i_widget& menu_bar::as_widget() const
    {
        return *this;
    }

    i_widget& menu_bar::as_widget()
    {
        return *this;
    }

    size_policy menu_bar::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    bool menu_bar::visible() const
    {
        if (service<i_basic_services>().has_system_menu_bar())
            return false;
        return widget::visible();
    }

    double menu_bar::opacity() const
    {
        double result = widget::opacity();
        if (widget::parent().is_root() && !root().is_effectively_active())
            result *= 0.25;
        return result;
    }

    void menu_bar::show_mnemonics(bool aShow)
    {
        if (showing_mnemonics() == aShow)
            return;
        menu::show_mnemonics(aShow);
        update();
    }

    bool menu_bar::key_pressed(scan_code_e aScanCode, key_code_e, key_modifier)
    {
        iAltPressedAlone = (aScanCode == ScanCode_LALT);
        if (aScanCode != ScanCode_LALT)
            show_mnemonics(true);
        switch (aScanCode)
        {
        case ScanCode_LEFT:
            if (has_selected_item())
                select_item_at(previous_available_item(selected_item()));
            break;
        case ScanCode_RIGHT:
            if (has_selected_item())
                select_item_at(next_available_item(selected_item()));
            break;
        case ScanCode_DOWN:
            if (has_selected_item() && item_at(selected_item()).available())
            {
                auto& selectedItem = item_at(selected_item());
                if (selectedItem.type() == menu_item_type::SubMenu)
                {
                    if (!selectedItem.sub_menu().is_open())
                        OpenSubMenu(selectedItem.sub_menu());
                    if (selectedItem.sub_menu().has_available_items())
                        selectedItem.sub_menu().select_item_at(selectedItem.sub_menu().first_available_item(), false);
                }
            }
            break;
        case ScanCode_RETURN:
        case ScanCode_KEYPAD_ENTER:
            if (has_selected_item() && item_at(selected_item()).available())
            {
                auto& selectedItem = item_at(selected_item());
                if (selectedItem.type() == menu_item_type::Action)
                {
                    selectedItem.action().triggered()();
                    if (selectedItem.action().is_checkable())
                        selectedItem.action().toggle();
                    clear_selection();
                }
                else if (selectedItem.type() == menu_item_type::SubMenu && !selectedItem.sub_menu().is_open())
                    OpenSubMenu(selectedItem.sub_menu());
            }
            break;
        case ScanCode_ESCAPE:
            close_sub_menu(false);
            clear_selection();
            break;
        default:
            break;
        }
        return true;
    }

    bool menu_bar::key_released(scan_code_e aScanCode, key_code_e, key_modifier)
    {
        // as on Windows, pressing Alt on its own leaves the menu
        if (aScanCode == ScanCode_LALT && iAltPressedAlone)
        {
            iAltPressedAlone = false;
            close_sub_menu(false);
            clear_selection();
        }
        return true;
    }

    bool menu_bar::text_input(i_string const& aText)
    {
        // as on Windows, when the menu bar is being used with the keyboard an item's mnemonic selects it without Alt
        static boost::locale::generator gen;
        static std::locale loc = gen("en_US.UTF-8");
        auto const input = boost::locale::to_lower(aText.to_std_string(), loc);
        for (item_index i = 0; i < menu::count(); ++i)
        {
            auto& itemWidget = layout().get_widget_at<menu_item_widget>(i);
            auto const m = itemWidget.mnemonic();
            if (!m.empty() && item_at(i).available() && boost::locale::to_lower(m.as_std_string(), loc) == input)
            {
                itemWidget.mnemonic_execute();
                return true;
            }
        }
        service<i_basic_services>().system_beep();
        return true;
    }

    widget_part menu_bar::hit_test(const point&) const
    {
        return widget_part{ root().as_widget(), widget_part::Menu };
    }

    void menu_bar::init()
    {
        set_padding({});
        layout().set_padding(neogfx::padding{});

        iSink += root().dismissing_children([this](const i_widget* aClickedWidget)
        {
            if (aClickedWidget != this && (aClickedWidget == 0 || !is_ancestor_of(*aClickedWidget)))
                clear_selection();
        });
        iSink += ItemAdded([this](item_index aItemIndex)
        {
            layout().add_at(aItemIndex, make_ref<menu_item_widget>(*this, item_at(aItemIndex)));
        });
        iSink += ItemRemoved([this](item_index aItemIndex)
        {
            if (layout().is_widget_at(aItemIndex))
            {
                i_widget& w = layout().get_widget_at(aItemIndex);
                w.parent().remove(w);
            }
            layout().remove_at(aItemIndex);
        });
        iSink += ItemSelected([this](i_menu_item& aMenuItem)
        {
            if (iOpenSubMenu->has_menu())
            {
                if (aMenuItem.type() == menu_item_type::Action ||
                    (aMenuItem.type() == menu_item_type::SubMenu && &iOpenSubMenu->menu() != &aMenuItem.sub_menu() && aMenuItem.available()))
                {
                    close_sub_menu(false);
                    if (aMenuItem.type() == menu_item_type::SubMenu)
                        OpenSubMenu(aMenuItem.sub_menu());
                }
            }
            update();
        });
        iSink += SelectionCleared([this]()
        {
            if (service<i_keyboard>().is_keyboard_grabbed_by(*this))
                service<i_keyboard>().ungrab_keyboard(*this);
            close_sub_menu(false);
            show_mnemonics(false);
            update();
        });
        iSink += OpenSubMenu([this](i_menu& aSubMenu)
        {
            auto& itemWidget = layout().get_widget_at<menu_item_widget>(find(aSubMenu));
            close_sub_menu(false);
            if (!service<i_keyboard>().is_keyboard_grabbed_by(*this))
                service<i_keyboard>().grab_keyboard(*this);
            iOpenSubMenu->set_menu(aSubMenu, itemWidget.sub_menu_position());
            iSink2 = iOpenSubMenu->menu().closed([this]()
            {
                if (iOpenSubMenu->has_menu())
                    iOpenSubMenu->clear_menu();
            });
            iSink2 += iOpenSubMenu->Closed([this]()
            {
                close_sub_menu();
                iOpenSubMenu.reset();
            });
        });
        // as on Windows, pressing Alt on its own enters the menu, selecting its first item and showing mnemonics
        iSink += service<i_keyboard>().key_pressed([this](scan_code_e aScanCode, key_code_e, key_modifier)
        {
            iAltPressedAlone = (aScanCode == ScanCode_LALT);
        });
        iSink += service<i_keyboard>().key_released([this](scan_code_e aScanCode, key_code_e, key_modifier)
        {
            bool const altPressedAlone = iAltPressedAlone;
            iAltPressedAlone = false;
            if (aScanCode != ScanCode_LALT || !altPressedAlone || !visible() || !root().is_active() || 
                service<i_keyboard>().is_keyboard_grabbed_by(*this) || !has_available_items())
                return;
            service<i_keyboard>().grab_keyboard(*this);
            show_mnemonics(true);
            select_item_at(first_available_item(), false);
        });
        if (widget::parent().is_root())
        {
            iSink += root().window_event([this](neogfx::window_event& e)
            {
                switch (e.type())
                {
                case window_event_type::FocusGained:
                case window_event_type::FocusLost:
                    update();
                    break;
                default:
                    break;
                }
            });
        }
    }

    void menu_bar::close_sub_menu(bool aClearSelection)
    {
        iSink2 = sink{};
        if (iOpenSubMenu)
            iOpenSubMenu->clear_menu();
        if (aClearSelection)
            clear_selection();
    }

}