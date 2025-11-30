// context_menu.cpp
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
#include <neogfx/app/event_processing_context.hpp>
#include <neogfx/gui/widget/menu.hpp>
#include <neogfx/gui/window/context_menu.hpp>

namespace neogfx
{
    class context_menu_service : public i_context_menu
    {
    public:
        bool context_menu_active() const noexcept final
        {
            return iActiveContextMenu;
        }
        bool context_menu_cancelling() const noexcept final
        {
            return iCancelling;
        }
    public:
        void activate_context_menu(i_menu& aMenu) noexcept final
        {
            iActiveContextMenu = &aMenu;
        }
        void deactivate_context_menu() noexcept final
        {
            if (iActiveContextMenu)
            {
                close_context_menu();
                iActiveContextMenu = nullptr;
            }
        }
        void cancel_context_menu() noexcept final
        {
            if (iActiveContextMenu)
            {
                neolib::scoped_flag sf{ iCancelling };
                deactivate_context_menu();
            }
        }
        void close_context_menu() noexcept final
        {
            if (iActiveContextMenu && iActiveContextMenu->is_open())
            {
                iActiveContextMenu->clear_selection();
                iActiveContextMenu->close();
            }
        }
    private:
        i_menu* iActiveContextMenu = nullptr;
        bool iCancelling = false;
    };
}

template<> neogfx::i_context_menu& services::start_service<neogfx::i_context_menu>()
{
    static neogfx::context_menu_service sContextMenuService{};
    return sContextMenuService;
}

namespace neogfx
{
    context_menu::context_menu(const point& aPosition, window_style aStyle)
        : iMenu{ new neogfx::menu{} }, iParent{ nullptr }, iPosition{ aPosition }, iStyle{ aStyle }
    {
    }

    context_menu::context_menu(i_widget& aParent, const point& aPosition, window_style aStyle)
        : iMenu{ new neogfx::menu{} }, iParent{ &aParent }, iPosition{ aPosition }, iStyle{ aStyle }
    {
    }

    context_menu::~context_menu()
    {
    }

    i_menu& context_menu::menu()
    {
        return *iMenu;
    }

    popup_menu& context_menu::popup()
    {
        if (popup_ptr() == nullptr)
            throw popup_not_created_yet();

        return *popup_ptr();
    }

    context_menu::exit_reason context_menu::exec()
    {
        std::optional<exit_reason> exitReason;
        menu().set_modal(true);
        menu().closed([&]()
            {
                if (!service<i_context_menu>().context_menu_cancelling())
                    exitReason = exit_reason::Normal;
                else
                    exitReason = exit_reason::Cancelled;
            });
        if (popup_ptr() != nullptr)
            popup_ptr() = nullptr;
        popup_ptr() = (iParent != nullptr ?
            std::make_unique<popup_menu>(*iParent, iPosition, menu(), iStyle) :
            std::make_unique<popup_menu>(iPosition, menu(), iStyle));
        service<i_context_menu>().activate_context_menu(menu());
        PopupCreated(popup());
        event_processing_context epc{ service<i_async_task>(), "neogfx::context_menu" };
        while (!exitReason.has_value() && service<i_context_menu>().context_menu_active())
        {
            service<i_app>().process_events(epc);
        }
        popup_ptr() = nullptr;
        if (service<i_context_menu>().context_menu_active())
            service<i_context_menu>().deactivate_context_menu();
        return exitReason.value_or(exit_reason::Normal);
    }

    std::unique_ptr<popup_menu>& context_menu::popup_ptr()
    {
        static std::unique_ptr<popup_menu> sPopup;
        return sPopup;
    }
}