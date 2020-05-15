// i_skin.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>
#include <neogfx/gui/widget/i_scrollbar.hpp>
#include <neogfx/gui/widget/i_button.hpp>

namespace neogfx
{
    class i_skin
    {
    public:
        struct no_fallback : std::logic_error { no_fallback() : std::logic_error{ "neogfx::i_skin::no_fallback" } {} };
    public:
        virtual ~i_skin() = default;
    public:
        virtual const std::string& name() const = 0;
        virtual bool has_fallback() const = 0;
        virtual i_skin& fallback() = 0;
    public:
        virtual void activate() = 0;
        virtual void deactivate() = 0;
    public:
        virtual size preferred_size(skin_element aElement, const optional_size& aDesiredSize = {}) const = 0;
    public:
        virtual void draw_scrollbar(i_graphics_context& aGc, const i_skinnable_item& aItem, const i_scrollbar& aScrollbar) const = 0;
        virtual void draw_check_box(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const = 0;
        virtual void draw_radio_button(i_graphics_context& aGc, const i_skinnable_item& aItem, const button_checked_state& aCheckedState) const = 0;
        virtual void draw_tree_expander(i_graphics_context& aGc, const i_skinnable_item& aItem, bool aExpandedState) const = 0;
    };
}