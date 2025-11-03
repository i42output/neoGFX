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
            return iContextMenuActive;
        }
        void activate_context_menu() noexcept final
        {
            iContextMenuActive = true;
        }
        void deactivate_context_menu() noexcept final
        {
            iContextMenuActive = false;
        }
    private:
        bool iContextMenuActive = false;
    };
}

template<> neogfx::i_context_menu& services::start_service<neogfx::i_context_menu>()
{
    static neogfx::context_menu_service sContextMenuService{};
    return sContextMenuService;
}

namespace neogfx
{
    std::unique_ptr<popup_menu> context_menu::sWidget;

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

    popup_menu& context_menu::root_widget()
    {
        if (sWidget == nullptr)
            throw widget_not_created_yet();
        return *sWidget;
    }

    void context_menu::exec()
    {
        bool finished = false;
        menu().set_modal(true);
        menu().closed([&finished]()
        {
            finished = true;
        });
        if (sWidget != nullptr)
            sWidget = nullptr;
        sWidget = (iParent != nullptr ?
            std::make_unique<popup_menu>(*iParent, iPosition, menu(), iStyle) :
            std::make_unique<popup_menu>(iPosition, menu(), iStyle));
        service<i_context_menu>().activate_context_menu();
        PopupCreated(*sWidget);
        event_processing_context epc{ service<i_async_task>(), "neogfx::context_menu" };
        while (!finished && service<i_context_menu>().context_menu_active())
        {
            service<i_app>().process_events(epc);
        }
        sWidget = nullptr;
        if (service<i_context_menu>().context_menu_active())
            service<i_context_menu>().deactivate_context_menu();
    }
}