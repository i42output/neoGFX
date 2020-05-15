// i_image.hpp
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
#include <neogfx/core/primitives.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/app/i_resource.hpp>

namespace neogfx
{
    enum class color_format
    {
        RGBA8
    };

    class i_image : public i_resource
    {
    public:
        struct unknown_image_format : std::runtime_error { unknown_image_format() : std::runtime_error("neogfx::i_image::unknown_image_format") {} };
    public:
        virtual ~i_image() = default;
    public:
        virtual dimension dpi_scale_factor() const = 0;
        virtual neogfx::color_format color_format() const = 0;
        virtual texture_sampling sampling() const = 0;
        virtual texture_data_format data_format() const = 0;
        virtual const neogfx::size& extents() const = 0;
        virtual void resize(const neogfx::size& aNewSize) = 0;
        virtual const void* cpixels() const = 0;
        virtual const void* pixels() const = 0;
        virtual void* pixels() = 0;
        virtual color get_pixel(const point& aPoint) const = 0;
        virtual void set_pixel(const point& aPoint, const color& aColor) = 0;
    };
}