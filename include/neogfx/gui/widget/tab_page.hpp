// tab_page.hpp
/*
neogfx C++ GUI Library
Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/layout/border_layout.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/gui/widget/i_tab_page.hpp>
#include <neogfx/gui/widget/i_tab_page_container.hpp>

namespace neogfx
{
    class tab_page : public scrollable_widget, public i_tab_page
    {
    public:
        define_declared_event(Selected, selected)
        define_declared_event(Deselected, deselected)
    public:
        tab_page(i_tab_page_container& aContainer, const std::string& aTabText = {});
        tab_page(i_tab_page_container& aContainer, i_tab& aTab);
        tab_page(i_widget& aParent, i_tab& aTab);
        tab_page(i_layout& aLayout, i_tab& aTab);
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
        size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
    public:
        bool transparent_background() const override;
        void paint_non_client_after(i_graphics_context& aGraphicsContext) const override;
    public:
        colour background_colour() const override;
    public:
        const i_tab& tab() const override;
        i_tab& tab() override;
        const widget& as_widget() const override;
        widget& as_widget() override;
    private:
        i_tab& iTab;
        sink iSink;
    };
}