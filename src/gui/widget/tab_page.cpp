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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/tab_page.hpp>

namespace neogfx
{
    tab_page::tab_page(i_tab_page_container& aContainer, const std::string& aTabText) :
        tab_page{ aContainer, aContainer.add_tab(aTabText) }
    {
    }

    tab_page::tab_page(i_tab_page_container& aContainer, i_tab& aTab) :
        framed_scrollable_widget{ aContainer.page_layout(), frame_style::ContainerFrame }, iTab{ aTab }
    {
        aContainer.add_tab_page(aTab, *this);
    }

    tab_page::tab_page(i_widget& aParent, i_tab& aTab) :
        framed_scrollable_widget{ aParent, frame_style::ContainerFrame }, iTab{ aTab }
    {
    }

    tab_page::tab_page(i_layout& aLayout, i_tab& aTab) :
        framed_scrollable_widget{ aLayout, frame_style::ContainerFrame }, iTab{ aTab }
    {
    }

    neogfx::size_policy tab_page::size_policy() const
    {
        if (has_size_policy())
            return framed_scrollable_widget::size_policy();
        else
            return size_constraint::Expanding;
    }

    size tab_page::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (has_minimum_size() || (iTab.container().style() & tab_container_style::ResizeToTabs) == tab_container_style::ResizeToTabs)
            return framed_scrollable_widget::minimum_size(aAvailableSpace);
        else
            return size{};
    }

    size tab_page::maximum_size(const optional_size& aAvailableSpace) const
    {
        if (has_maximum_size() || size_policy() != size_constraint::Expanding)
            return framed_scrollable_widget::maximum_size(aAvailableSpace);
        else
            return size::max_size();
    }

    bool tab_page::transparent_background() const
    {
        return false;
    }

    void tab_page::paint_non_client_after(i_graphics_context& aGc) const
    {
        if (iTab.is_selected())
        {
            switch (iTab.container().style() & tab_container_style::TabAlignmentMask)
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

    color tab_page::background_color() const
    {
        if (has_background_color())
            return framed_scrollable_widget::background_color();
        else
            return container_background_color();
    }

    const i_tab& tab_page::tab() const
    {
        return iTab;
    }

    i_tab& tab_page::tab()
    {
        return iTab;
    }

    const widget& tab_page::as_widget() const
    {
        return *this;
    }

    widget& tab_page::as_widget()
    {
        return *this;
    }
}