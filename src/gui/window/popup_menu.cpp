// popup_menu.cpp
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

#include <neogfx/neogfx.hpp>
#include <neolib/raii.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/window/popup_menu.hpp>
#include <neogfx/gui/widget/menu_item_widget.hpp>

namespace neogfx
{
    popup_menu::popup_menu(const point& aPosition, i_menu& aMenu, window_style aStyle) :
        window{ aPosition, aStyle, scrollbar_style::Menu, frame_style::SolidFrame },
        iParentWidget{ 0 }, 
        iMenu{ nullptr }, 
        iOpeningSubMenu{ false }
    {
        init();
        set_menu(aMenu, aPosition);
    }

    popup_menu::popup_menu(i_widget& aParent, const point& aPosition, i_menu& aMenu, window_style aStyle) :
        window{ aParent, aPosition, aStyle, scrollbar_style::Menu, frame_style::SolidFrame },
        iParentWidget{ &aParent }, 
        iMenu{ nullptr }, 
        iOpeningSubMenu{ false }
    {
        init();
        set_menu(aMenu, aPosition);
    }

    popup_menu::popup_menu(const point& aPosition, window_style aStyle) :
        window{ aPosition, aStyle, scrollbar_style::Menu, frame_style::SolidFrame },
        iParentWidget{ 0 }, 
        iMenu{ nullptr }, 
        iOpeningSubMenu{ false }
    {
        init();
    }

    popup_menu::popup_menu(i_widget& aParent, const point& aPosition, window_style aStyle) :
        window{ aParent, aPosition, aStyle, scrollbar_style::Menu, frame_style::SolidFrame }, 
        iParentWidget{ &aParent }, 
        iMenu{ nullptr }, 
        iOpeningSubMenu{ false }
    {
        init();
    }

    popup_menu::~popup_menu()
    {
        close_sub_menu();
        iOpenSubMenu.reset();
        if (has_menu() && menu().is_open())
            menu().close();
    }

    bool popup_menu::can_dismiss(const i_widget* aClickedWidget) const
    {
        return aClickedWidget == nullptr || 
            iParentWidget == nullptr || 
            (iParentWidget == aClickedWidget && (style() & (window_style::DismissOnParentClick | window_style::HideOnParentClick)) != window_style::Invalid) ||
            (iParentWidget != aClickedWidget && !iParentWidget->is_ancestor_of(*aClickedWidget));
    }

    i_window::dismissal_type_e popup_menu::dismissal_type() const
    {
        if ((style() & window_style::DismissOnOwnerClick) == window_style::DismissOnOwnerClick)
            return CloseOnDismissal;
        else if ((style() & window_style::HideOnOwnerClick) == window_style::HideOnOwnerClick)
            return HideOnDismissal;
        else
            return CannotDismiss;
    }

    bool popup_menu::dismissed() const
    {
        switch (dismissal_type())
        {
        case CloseOnDismissal:
            return surface().is_closed();
        case HideOnDismissal:
            return hidden();
        default:
            return false;
        }
    }

    void popup_menu::dismiss()
    {
        if (service<i_keyboard>().is_keyboard_grabbed_by(*this))
            service<i_keyboard>().ungrab_keyboard(*this);
        if ((style() & window_style::DismissOnOwnerClick) == window_style::DismissOnOwnerClick)
            close();
        else if ((style() & window_style::HideOnOwnerClick) == window_style::HideOnOwnerClick)
        {
            hide();
            if (has_menu() && menu().is_open())
                menu().close();
        }
    }

    double popup_menu::rendering_priority() const
    {
        if (visible())
            return 1.0;
        return window::rendering_priority();
    }

    bool popup_menu::has_menu() const
    {
        return iMenu != nullptr;
    }

    i_menu& popup_menu::menu() const
    {
        if (has_menu())
            return *iMenu;
        throw no_menu();
    }

    void popup_menu::set_menu(i_menu& aMenu, const point& aPosition)
    {
        if (has_menu())
            clear_menu();
        iMenu = &aMenu;
        if (iOpenSubMenu.get() == nullptr)
            iOpenSubMenu = std::make_unique<popup_menu>(*this, point{});
        if (!service<i_keyboard>().is_keyboard_grabbed_by(*this))
            service<i_keyboard>().grab_keyboard(*this);
        for (i_menu::item_index i = 0; i < menu().count(); ++i)
            menu_layout().add(std::make_shared<menu_item_widget>(*this, menu(), menu().item_at(i)));
        layout_items();
        menu().open();
        iSink += menu().item_added([this](i_menu::item_index aIndex)
        {
            menu_layout().add_at(aIndex, std::make_shared<menu_item_widget>(*this, menu(), menu().item_at(aIndex)));
            layout_items();
        });
        iSink += menu().item_removed([this](i_menu::item_index aIndex)
        {
            menu_layout().remove_at(aIndex);
            layout_items();
        });
        iSink += menu().item_changed([this](i_menu::item_index)
        {
            layout_items();
        });
        iSink += menu().item_selected([this](i_menu_item& aMenuItem)
        {
            if (!service<i_keyboard>().is_keyboard_grabbed_by(*this))
                service<i_keyboard>().grab_keyboard(*this);
            if (iOpenSubMenu->has_menu() && iOpenSubMenu->menu().is_open())
            {
                if (aMenuItem.type() == i_menu_item::Action ||
                    (aMenuItem.type() == i_menu_item::SubMenu && &iOpenSubMenu->menu() != &aMenuItem.sub_menu()))
                {
                    iOpenSubMenu->menu().close();
                }
            }
            scroll_to(menu_layout().get_widget_at<menu_item_widget>(menu().find(aMenuItem)));
            update();
        });
        iSink += menu().open_sub_menu([this](i_menu& aSubMenu)
        {
            if (!iOpeningSubMenu && aSubMenu.count() > 0)
            {
                neolib::scoped_flag sf{ iOpeningSubMenu };
                auto& itemWidget = menu_layout().get_widget_at<menu_item_widget>(menu().find(aSubMenu));
                close_sub_menu();
                iOpenSubMenu->set_menu(aSubMenu, itemWidget.sub_menu_position());
                iSink2 += iOpenSubMenu->menu().closed([this]()
                {
                    if (iOpenSubMenu->has_menu())
                        iOpenSubMenu->clear_menu();
                });
                iSink2 += iOpenSubMenu->Closed([this]()
                {
                    close_sub_menu();
                    iOpenSubMenu.reset();
                });
            }
        });
        service<i_window_manager>().move_window(*this, aPosition);
        resize(minimum_size());
        update_position();
        show();
    }

    void popup_menu::clear_menu()
    {
        if (iOpenSubMenu.get() != nullptr)
            iOpenSubMenu->clear_menu();
        if (has_menu() && menu().is_open())
        {
            menu().close();
        }
        iSink = sink{};
        iSink2 = sink{};
        iMenu = nullptr;
        hide();
        menu_layout().remove_all();
        if (service<i_keyboard>().is_keyboard_grabbed_by(*this))
            service<i_keyboard>().ungrab_keyboard(*this);
    }

    void popup_menu::resized()
    {
        window::resized();
        update_position();
    }

    size_policy popup_menu::size_policy() const
    {
        if (window::has_size_policy())
            return window::size_policy();
        return neogfx::size_policy::Minimum;
    }

    size popup_menu::minimum_size(const optional_size& aAvailableSpace) const
    {
        size result = window::minimum_size(aAvailableSpace);
        rect desktopRect = service<i_surface_manager>().desktop_rect(surface());
        result = result.min(desktopRect.extents());
        return result;
    }
    
    size popup_menu::maximum_size(const optional_size& aAvailableSpace) const
    {
        return minimum_size(aAvailableSpace);
    }

    colour popup_menu::background_colour() const
    {
        return window::has_background_colour() ?
            window::background_colour() :
            service<i_app>().current_style().palette().colour();
    }

    colour popup_menu::frame_colour() const
    {
        if (window::has_frame_colour())
            return window::frame_colour();
        colour result = background_colour().darker(0x30);
        if (result.similar_intensity(background_colour(), 0.05))
            result = result.dark() ? result.lighter(0x20) : result.darker(0x20);
        return result;
    }

    bool popup_menu::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
    {
        switch (aScanCode)
        {
        case ScanCode_UP:
            if (menu().has_selected_item())
                menu().select_item_at(menu().previous_available_item(menu().selected_item()), false);
            else if (menu().has_available_items())
                menu().select_item_at(menu().first_available_item(), false);
            break;
        case ScanCode_DOWN:
            if (menu().has_selected_item())
                menu().select_item_at(menu().next_available_item(menu().selected_item()), false);
            else if (menu().has_available_items())
                menu().select_item_at(menu().first_available_item(), false);
            break;
        case ScanCode_LEFT:
            if (menu().has_parent())
            {
                if (menu().parent().type() == i_menu::Popup)
                    menu().close();
                else if (menu().parent().has_selected_item())
                    menu().parent().select_item_at(menu().parent().previous_available_item(menu().parent().selected_item()));
            }
            break;
        case ScanCode_RIGHT:
            if (menu().has_selected_item())
            {
                if (menu().item_at(menu().selected_item()).type() == i_menu_item::SubMenu)
                {
                    auto& subMenu = menu().item_at(menu().selected_item()).sub_menu();
                    if (!subMenu.is_open())
                        menu().open_sub_menu().trigger(subMenu);
                    if (subMenu.has_available_items())
                        subMenu.select_item_at(subMenu.first_available_item(), false);
                }
                else
                {
                    i_menu* m = &menu();
                    while (m->has_parent())
                        m = &m->parent();
                    if (m != &menu())
                    {
                        if (m->has_selected_item())
                            m->select_item_at(m->next_available_item(m->selected_item()), m->type() == i_menu::MenuBar);
                    }
                }
            }
            else if (menu().has_parent() && menu().parent().type() == i_menu::MenuBar)
            {
                if (menu().parent().has_selected_item())
                    menu().parent().select_item_at(menu().parent().next_available_item(menu().parent().selected_item()));
            }
            break;
        case ScanCode_RETURN:
            if (menu().has_selected_item() && menu().item_at(menu().selected_item()).available())
            {
                auto& selectedItem = menu().item_at(menu().selected_item());
                if (selectedItem.type() == i_menu_item::Action)
                {
                    selectedItem.action().triggered().async_trigger();
                    if (selectedItem.action().is_checkable())
                        selectedItem.action().toggle();
                    menu().clear_selection();
                    i_menu* menuToClose = &menu();
                    while (menuToClose->has_parent() && menuToClose->parent().type() == i_menu::Popup)
                        menuToClose = &menuToClose->parent();
                    if (menuToClose->type() == i_menu::Popup)
                        menuToClose->close();
                    if (menuToClose->has_parent() && menuToClose->parent().type() == i_menu::MenuBar)
                        menuToClose->parent().clear_selection();
                }
                else if (selectedItem.type() == i_menu_item::SubMenu && !selectedItem.sub_menu().is_open())
                    menu().open_sub_menu().trigger(selectedItem.sub_menu());
            }
            break;
        case ScanCode_ESCAPE:
            menu().clear_selection();
            menu().close();
            break;
        default:
            break;
        }
        return true;
    }

    bool popup_menu::key_released(scan_code_e, key_code_e, key_modifiers_e)
    {
        return true;
    }

    bool popup_menu::text_input(const std::string&)
    {
        service<i_basic_services>().system_beep();
        return true;
    }

    void popup_menu::init()
    {
        menu_layout().set_margins(neogfx::margins{});
        menu_layout().set_spacing(neogfx::size{});
        Closed([this]()
        {
            if (has_menu() && menu().is_open())
                menu().close();
        });
        if (has_menu())
            show();
    }

    void popup_menu::close_sub_menu()
    {
        if (iOpenSubMenu != nullptr)
        {
            iSink2 = sink{};
            iOpenSubMenu->clear_menu();
        }
    }

    void popup_menu::update_position()
    {
        auto& wm = service<i_window_manager>();
        rect desktopRect{ wm.desktop_rect(*this) };
        rect ourRect{ wm.window_rect(*this) };
        if (ourRect.bottom() > desktopRect.bottom())
            ourRect.position().y += (desktopRect.bottom() - ourRect.bottom());
        if (ourRect.right() > desktopRect.right())
            ourRect.position().x += (desktopRect.right() - ourRect.right());
        if (has_menu() && menu().has_parent() && has_parent_window(false))
        {
            if (menu().parent().type() == i_menu::MenuBar)
            {
                if (iParentWidget != nullptr)
                {
                    rect menuBarRect = iParentWidget->non_client_rect() + (!is_nested() ? wm.window_rect(parent_window()).position() : point{});
                    if (!ourRect.intersection(menuBarRect).empty())
                    {
                        if (desktopRect.bottom() - menuBarRect.bottom() > ourRect.height())
                            ourRect.position().y = menuBarRect.bottom();
                        else
                            ourRect.position().y = menuBarRect.top() - ourRect.height();
                    }
                }
            }
            else
            {
                rect parentRect{ wm.window_rect(parent_window()) };
                if (is_nested())
                    parentRect = point{};
                if (ourRect.intersection(parentRect).width() > 8.0)
                {
                    if (parentRect.right() + ourRect.width() < desktopRect.right())
                        ourRect.position().x = parentRect.right();
                    else
                        ourRect.position().x = parentRect.position().x - ourRect.width();
                }
            }
        }
        if (ourRect.position() != wm.window_rect(*this).position())
            wm.move_window(*this, ourRect.position());
    }
}