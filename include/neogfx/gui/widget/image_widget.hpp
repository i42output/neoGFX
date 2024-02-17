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
#include <neogfx/gui/widget/i_image_widget.hpp>

namespace neogfx
{
    class image_widget : public widget<i_image_widget>
    {
        meta_object(widget<i_image_widget>)
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
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        void paint(i_graphics_context& aGc) const override;
    public:
        const texture& image() const override;
        const color_or_gradient& image_color() const override;
        void set_image(i_string const& aImageUri) override;
        void set_image(const i_image& aImage) override;
        void set_image(const i_texture& aImage) override;
        void set_image_color(const color_or_gradient& aImageColor) override;
        void set_aspect_ratio(neogfx::aspect_ratio aAspectRatio) override;
        void set_placement(cardinal aPlacement) override;
        void set_dpi_auto_scale(bool aDpiAutoScale) override;
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