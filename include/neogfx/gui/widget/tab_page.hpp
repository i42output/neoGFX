// tab_page.hpp
/*
neogfx C++ App/Game Engine
Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.

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
    class tab_page : public framed_scrollable_widget, public reference_counted<i_tab_page>
    {
        meta_object(framed_scrollable_widget)
    public:
        define_declared_event(Selected, selected)
        define_declared_event(Deselected, deselected)
    public:
        tab_page(i_tab_page_container& aContainer, std::string const& aTabText = {}, void* aData = nullptr);
        tab_page(i_tab_page_container& aContainer, i_tab& aTab, void* aData = nullptr);
        tab_page(i_widget& aParent, i_tab& aTab, void* aData = nullptr);
        tab_page(i_layout& aLayout, i_tab& aTab, void* aData = nullptr);
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        void paint_non_client_after(i_graphics_context& aGc) const override;
    public:
        color palette_color(color_role aColorRole) const override;
        color frame_color() const override;
        color inner_frame_color() const override;
    public:
        const i_tab& tab() const override;
        i_tab& tab() override;
        const widget<>& as_widget() const override;
        widget<>& as_widget() override;
        void* data() const override;
        void set_data(void* aData) override;
    private:
        void init();
    private:
        i_tab& iTab;
        void* iData = nullptr;
        sink iSink;
    };
}