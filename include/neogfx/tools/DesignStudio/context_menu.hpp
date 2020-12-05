// context_menu.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    inline void display_element_context_menu(i_widget& aParent, i_element& aElement)
    {
        context_menu menu{ aParent, aParent.root().mouse_position() + aParent.root().window_position() };
        action actionSendToBack{ "Send To Back"_t };
        action actionBringToFont{ "Bring To Front"_t };
        auto& actionCut = service<i_app>().action_cut();
        auto& actionCopy = service<i_app>().action_copy();
        auto& actionPaste = service<i_app>().action_paste();
        auto& actionDelete = service<i_app>().action_delete();
        auto& actionSelectAll = service<i_app>().action_select_all();
        if (aElement.has_layout_item() && aElement.layout_item()->is_widget())
        {
            auto& caddyWidget = aElement.layout_item()->as_widget().parent();
            if (&*caddyWidget.parent().children().back() == &caddyWidget)
                actionSendToBack.disable();
            if (&*caddyWidget.parent().children().front() == &caddyWidget)
                actionBringToFont.disable();
            actionSendToBack.triggered([&]()
            {
                // todo: change order in element model
                aElement.root().visit([&](i_element& aElement)
                {
                    if (aElement.is_selected())
                    {
                        if (aElement.has_caddy())
                            aElement.caddy().send_to_back();
                    }
                });
                if (aElement.has_caddy())
                    aElement.caddy().children().front()->set_focus();
            });
            actionBringToFont.triggered([&]()
            {
                // todo: change order in element model
                aElement.root().reverse_visit([&](i_element& aElement)
                {
                    if (aElement.is_selected())
                    {
                        if (aElement.has_caddy())
                            aElement.caddy().bring_to_front();
                    }
                });
            });
            menu.menu().add_action(actionSendToBack);
            menu.menu().add_action(actionBringToFont);
            menu.menu().add_separator();
        }
        menu.menu().add_action(actionCut);
        menu.menu().add_action(actionCopy);
        menu.menu().add_action(actionPaste);
        menu.menu().add_action(actionDelete);
        menu.menu().add_separator();
        menu.menu().add_action(actionSelectAll);
        menu.exec();
    }

}