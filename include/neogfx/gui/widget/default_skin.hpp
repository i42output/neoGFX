// default_skin.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/i_skin.hpp>

namespace neogfx
{
    class default_skin : public i_skin
    {
    public:
        std::string const& name() const override;
        bool has_fallback() const override;
        i_skin& fallback() override;
    public:
        void activate() override;
        void deactivate() override;
    public:
        size preferred_size(skin_element aElement, optional_size const& aDesiredSize = {}) const override;
    public:
        void draw_scrollbar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_scrollbar& aScrollbar) const override;
        void draw_check_box(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const override;
        void draw_radio_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const override;
        void draw_tree_expander(i_graphics_context& aGc, const i_skinnable_item& aItem, bool aExpandedState) const override;
        void draw_progress_bar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_progress_bar& aProgressBar) const override;
        void draw_separators(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_layout& aLayout) const override;
    };
}