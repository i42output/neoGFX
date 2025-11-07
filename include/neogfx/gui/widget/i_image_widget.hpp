// i_image_widget.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_image.hpp>
#include <neogfx/gfx/i_texture.hpp>

namespace neogfx
{
    class i_image_widget : public i_widget
    {
    public:
        virtual const i_texture& image() const = 0;
        virtual const i_optional<size>& image_size() const = 0;
        virtual const color_or_gradient& image_color() const = 0;
        virtual void set_image(i_string const& aImageUri) = 0;
        virtual void set_image(const i_image& aImage) = 0;
        virtual void set_image(const i_texture& aImage) = 0;
        virtual void set_image_size(const i_optional<size>& aImageSize) = 0;
        virtual void set_image_color(const color_or_gradient& aImageColor) = 0;
        virtual void set_aspect_ratio(neogfx::aspect_ratio aAspectRatio) = 0;
        virtual void set_placement(cardinal aPlacement) = 0;
        virtual void set_dpi_auto_scale(bool aDpiAutoScale) = 0;
    };
}