// i_title_bar.hpp
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

#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/i_text_widget.hpp>
#include <neogfx/gui/widget/i_image_widget.hpp>

namespace neogfx
{
    class i_title_bar : public i_widget
    {
    public:
        typedef i_title_bar abstract_type;
    public:
        struct unsupported_operation : std::logic_error { unsupported_operation() : std::logic_error{ "neogfx::i_title_bar::unsupported_operation" } {} };
    public:
        virtual ~i_title_bar() = default;
    public:
        virtual void set_icon(i_texture const& aIcon) = 0;
        virtual i_image_widget const& icon_widget() const = 0;
        virtual i_image_widget& icon_widget() = 0;
        virtual i_string const& title() const = 0;
        virtual void set_title(i_string const& aTitle) = 0;
        virtual i_text_widget const& title_widget() const = 0;
        virtual i_text_widget& title_widget() = 0;
        // todo: get/set icon
    public:
    };
}