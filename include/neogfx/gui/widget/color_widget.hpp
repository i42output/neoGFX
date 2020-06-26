// color_widget.hpp
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
#include <neolib/core/variant.hpp>
#include <neogfx/app/i_resource.hpp>
#include <neogfx/core/color.hpp>
#include <neogfx/gui/widget/framed_widget.hpp>
#include <neogfx/gui/widget/button.hpp>

namespace neogfx
{
    class color_widget : public framed_widget<button>
    {
        typedef framed_widget<button> base_type;
    public:
        define_event(ColorChanged, color_changed)
    public:
        color_widget(const neogfx::color& aColor = neogfx::color{});
        color_widget(i_widget& aParent, const neogfx::color& aColor = neogfx::color{});
        color_widget(i_layout& aLayout, const neogfx::color& aColor = neogfx::color{});
    public:
        neogfx::color const& color() const;
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(const optional_size& aAvailableSpace = optional_size{}) const override;
    public:
        void paint(i_graphics_context& aGc) const override;
    private:
        neogfx::color iColor;
   };
}