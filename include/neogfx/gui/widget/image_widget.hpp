// image_widget.hpp
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
#include <neolib/core/i_enum.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gfx/texture.hpp>

namespace neogfx
{
    class image_widget : public widget<>
    {
    public:
        define_event(ImageChanged, image_changed)
        define_event(ImageGeometryChanged, image_geometry_changed)
    public:
        image_widget(const i_texture& aTexture = texture{}, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_widget& aParent, const i_texture& aTexture = texture{}, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_widget& aParent, const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_layout& aLayout, const i_texture& aTexture = texture{}, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_layout& aLayout, const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
    public:
        virtual neogfx::size_policy size_policy() const;
        virtual size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const;
    public:
        virtual void paint(i_graphics_context& aGc) const;
    public:
        const texture& image() const;
        const color_or_gradient& image_color() const;
        void set_image(std::string const& aImageUri);
        void set_image(const i_image& aImage);
        void set_image(const i_texture& aImage);
        void set_image_color(const color_or_gradient& aImageColor);
        void set_aspect_ratio(neogfx::aspect_ratio aAspectRatio);
        void set_placement(cardinal aPlacement);
        void set_dpi_auto_scale(bool aDpiAutoScale);
    public:
        rect placement_rect() const;
    private:
        texture iTexture;
        color_or_gradient iColor;
        neogfx::aspect_ratio iAspectRatio;
        cardinal iPlacement;
        bool iDpiAutoScale;
    };
}