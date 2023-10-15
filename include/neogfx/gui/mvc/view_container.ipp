// view_container.ipp
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
#include <neogfx/gui/mvc/view_container.hpp>
#include <neogfx/gui/mvc/i_controller.hpp>

namespace neogfx::mvc
{
    inline view_stack::view_stack(i_layout& aLayout, i_view_container& aParent) :
        drag_drop_target<framed_scrollable_widget>{ aLayout, frame_style::NoFrame }, iLayout{ *this }, iParent{ aParent }
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{});
    }

    inline focus_policy view_stack::focus_policy() const
    {
        if (base_type::has_focus_policy())
            return base_type::focus_policy();
        return neogfx::focus_policy::StrongFocus;
    }

    template <typename Base>
    inline view_container<Base>::view_container(i_widget& aParent, view_container_style aStyle) :
        base_type{ aParent }, iStyle(aStyle), iLayout0{ *this }, iLayout1{ iLayout0 }, iViewStack{ base_type::page_layout(), *this }
    {
        base_type::set_padding(neogfx::padding{});
        iLayout0.set_padding(neogfx::padding{});
    }

    template <typename Base>
    inline view_container<Base>::view_container(i_layout& aLayout, view_container_style aStyle) :
        base_type{ aLayout }, iStyle(aStyle), iLayout0{ *this }, iLayout1{ iLayout0 }, iViewStack{ base_type::page_layout(), *this }
    {
        base_type::set_padding(neogfx::padding{});
        iLayout0.set_padding(neogfx::padding{});
    }

    template <typename Base>
    bool view_container<Base>::is_widget() const
    {
        return true;
    }

    template <typename Base>
    inline const i_widget& view_container<Base>::as_widget() const
    {
        return *this;
    }

    template <typename Base>
    inline i_widget& view_container<Base>::as_widget()
    {
        return *this;
    }

    template <typename Base>
    inline const i_tab_container& view_container<Base>::tab_container() const
    {
        return *this;
    }

    template <typename Base>
    inline i_tab_container& view_container<Base>::tab_container()
    {
        return *this;
    }

    template <typename Base>
    inline const view_stack& view_container<Base>::view_stack() const
    {
        return iViewStack;
    }

    template <typename Base>
    inline view_stack& view_container<Base>::view_stack()
    {
        return iViewStack;
    }

    template <typename Base>
    inline i_layout& view_container<Base>::page_layout()
    {
        return view_stack().layout();
    }

    template <typename Base>
    inline view_container_style view_container<Base>::style() const
    {
        return iStyle;
    }

    template <typename Base>
    inline void view_container<Base>::change_style(view_container_style aNewStyle)
    {
        if (iStyle != aNewStyle)
        {
            iStyle = aNewStyle;
            /* todo: update everything */
        }
    }

    template <typename Base>
    inline void view_container<Base>::add_controller(i_controller& aController)
    {
        iControllers.push_back(std::shared_ptr<i_controller>(std::shared_ptr<i_controller>(), &aController));
        aController.view_added([this](i_view& aView) { ViewAdded.trigger(aView); });
        aController.view_removed([this](i_view& aView) { ViewRemoved.trigger(aView); });
    }

    template <typename Base>
    inline void view_container<Base>::add_controller(std::shared_ptr<i_controller> aController)
    {
        iControllers.push_back(aController);
        aController->view_added([this](i_view& aView) { ViewAdded.trigger(aView); });
        aController->view_removed([this](i_view& aView) { ViewRemoved.trigger(aView); });
    }

    template <typename Base>
    inline void view_container<Base>::remove_controller(i_controller& aController)
    {
        for (auto c = iControllers.begin(); c != iControllers.end(); ++c)
        {
            if (&**c == &aController)
            {
                iControllers.erase(c);
                return;
            }
        }
        throw controller_not_found();
    }

    template <typename Base>
    inline bool view_container<Base>::is_managing_layout() const
    {
        return true;
    }
}