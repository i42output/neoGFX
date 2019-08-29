// i_skin.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/i_skinnable_item.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>

namespace neogfx
{
    class i_skin
    {
    public:
        virtual const std::string& name() const = 0;
        virtual bool has_fallback() const = 0;
        virtual i_skin& fallback() = 0;
    public:
        virtual void activate() = 0;
        virtual void deactivate() = 0;
    public:
        virtual void paint_non_client(i_skinnable_item& aItem, i_graphics_context& aGraphicsContext) const = 0;
        virtual void paint_non_client_after(i_skinnable_item& aItem, i_graphics_context& aGraphicsContext) const = 0;
        virtual void paint(i_skinnable_item& aItem, i_graphics_context& aGraphicsContext) const = 0;
    };
}