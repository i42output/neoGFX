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
        typedef image_widget property_context_type;
    public:
        define_event(ImageChanged, image_changed)
        define_event(ImageGeometryChanged, image_geometry_changed)
    public:
        image_widget(const i_texture& aTexture = texture{}, neogfx::aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(const i_image& aImage, neogfx::aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_widget& aParent, const i_texture& aTexture = texture{}, neogfx::aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_widget& aParent, const i_image& aImage, neogfx::aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_layout& aLayout, const i_texture& aTexture = texture{}, neogfx::aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        image_widget(i_layout& aLayout, const i_image& aImage, neogfx::aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal aPlacement = cardinal::Center);
        ~image_widget();
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        void paint(i_graphics_context& aGc) const override;
    public:
        void property_changed(i_property& aProperty) override;
    public:
        const texture& image() const override;
        const optional_size& image_size() const override;
        const color_or_gradient& image_color() const override;
        void set_image(i_string const& aImageUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap) override;
        void set_image(const i_image& aImage) override;
        void set_image(const i_texture& aImage) override;
        void set_image_size(const i_optional<size>& aImageSize) override;
        void set_image_color(const color_or_gradient& aImageColor) override;
        neogfx::aspect_ratio aspect_ratio() const;
        void set_aspect_ratio(neogfx::aspect_ratio aAspectRatio) override;
        cardinal placement() const;
        void set_placement(cardinal aPlacement) override;
        angle rotation() const override;
        void set_rotation(angle aRotation) override;
        bool dpi_auto_scale() const;
        void set_dpi_auto_scale(bool aDpiAutoScale) override;
    public:
        rect placement_rect() const;
    private:
        texture iTexture;
    public:
        define_property(property_category::other_appearance, optional_size, ImageSize, image_size)
        define_property(property_category::color, color_or_gradient, ImageColor, image_color)
        define_property(property_category::other_appearance, neogfx::aspect_ratio, AspectRatio, aspect_ratio, neogfx::aspect_ratio::Keep)
        define_property(property_category::other_appearance, cardinal, Placement, placement, cardinal::Center)
        define_property(property_category::hard_geometry, angle, Rotation, rotation, 0.0)
        define_property(property_category::other_appearance, bool, DpiAutoScale, dpi_auto_scale, false)
    };
}