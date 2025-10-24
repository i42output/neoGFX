// tab_page.cpp
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

#include <neogfx/gui/widget/tab_page.hpp>

namespace neogfx
{
    tab_page::tab_page(i_tab_page_container& aContainer, std::string const& aTabText, void* aData) :
        tab_page{ aContainer, aContainer.add_tab(string{ aTabText }), aData }
    {
    }

    tab_page::tab_page(i_tab_page_container& aContainer, i_tab& aTab, void* aData) :
        framed_scrollable_widget{ aContainer.page_layout(), frame_style::ContainerFrame }, iTab{ aTab }, iData{ aData }
    {
        init();
        aContainer.add_tab_page(aTab, *this);
    }

    tab_page::tab_page(i_widget& aParent, i_tab& aTab, void* aData) :
        framed_scrollable_widget{ aParent, frame_style::ContainerFrame }, iTab{ aTab }, iData{ aData }
    {
        init();
    }

    tab_page::tab_page(i_layout& aLayout, i_tab& aTab, void* aData) :
        framed_scrollable_widget{ aLayout, frame_style::ContainerFrame }, iTab{ aTab }, iData{ aData }
    {
        init();
    }

    neogfx::size_policy tab_page::size_policy() const
    {
        if (has_size_policy())
            return framed_scrollable_widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Expanding;
    }

    size tab_page::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size() || (iTab.container().tab_container_style() & tab_container_style::ResizeToPages) == tab_container_style::ResizeToPages)
            return framed_scrollable_widget::minimum_size(aAvailableSpace);
        else
            return size{};
    }

    size tab_page::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (has_maximum_size() || size_policy() != size_constraint::Expanding)
            return framed_scrollable_widget::maximum_size(aAvailableSpace);
        else
            return size::max_size();
    }

    void tab_page::paint_non_client_after(i_graphics_context& aGc) const
    {
        if (iTab.is_selected() && iTab.container().as_widget().effectively_visible())
        {
            switch (iTab.container().tab_container_style() & tab_container_style::TabAlignmentMask)
            {
            case tab_container_style::TabAlignmentTop:
            case tab_container_style::TabAlignmentBottom:
                {
                    auto hole = to_client_coordinates(iTab.as_widget().non_client_rect().inflate(size{ -effective_frame_width() / 2.0, effective_frame_width() }).intersection(non_client_rect()));
                    hole = hole.intersection(to_client_coordinates(non_client_rect().deflate(size{ effective_frame_width() / 2.0, 0.0 })));
                    aGc.fill_rect(hole, inner_frame_color());
                    hole.deflate(size{ effective_frame_width() / 2.0, 0.0 });
                    aGc.fill_rect(hole, background_color());
                }
                break;
            case tab_container_style::TabAlignmentLeft:
            case tab_container_style::TabAlignmentRight:
                {
                    auto hole = to_client_coordinates(iTab.as_widget().non_client_rect().inflate(size{ effective_frame_width(), -effective_frame_width() / 2.0 }).intersection(non_client_rect()));
                    hole = hole.intersection(to_client_coordinates(non_client_rect().deflate(size{ 0.0, effective_frame_width() / 2.0 })));
                    aGc.fill_rect(hole, inner_frame_color());
                    hole.deflate(size{ 0.0, effective_frame_width() / 2.0 });
                    aGc.fill_rect(hole, background_color());
                }
                break;
            }
        }
        framed_scrollable_widget::paint_non_client_after(aGc);
    }

    color tab_page::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return framed_scrollable_widget::palette_color(aColorRole);
        else if (aColorRole == color_role::Background)
            return container_background_color();
        else
            return framed_scrollable_widget::palette_color(aColorRole);
    }

    color tab_page::frame_color() const
    {
        if (has_frame_color())
            return framed_scrollable_widget::frame_color();
        else
            return tab().as_widget().palette_color(color_role::Background).darker(0x10);
    }

    color tab_page::inner_frame_color() const
    {
        if (has_frame_color())
            return framed_scrollable_widget::inner_frame_color();
        else
            return tab().as_widget().palette_color(color_role::Base).shaded(0x20);
    }

    const i_tab& tab_page::tab() const
    {
        return iTab;
    }

    i_tab& tab_page::tab()
    {
        return iTab;
    }

    const widget<>& tab_page::as_widget() const
    {
        return *this;
    }

    widget<>& tab_page::as_widget()
    {
        return *this;
    }

    void* tab_page::data() const
    {
        return iData;
    }

    void tab_page::set_data(void* aData)
    {
        iData = aData;
        tab().set_data(aData);
    }

    void tab_page::init()
    {
        tab().set_data(data());
        set_background_opacity(1.0);
    }
}