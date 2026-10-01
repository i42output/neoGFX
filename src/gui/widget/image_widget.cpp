// image_widget.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/image_widget.hpp>


namespace neogfx
{
    image_widget::image_widget(const i_texture& aTexture, neogfx::aspect_ratio aAspectRatio, cardinal aPlacement) :
        iTexture{ aTexture }
    {
        AspectRatio.assign(aAspectRatio, false);
        Placement.assign(aPlacement, false);
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(const i_image& aImage, neogfx::aspect_ratio aAspectRatio, cardinal aPlacement) :
        iTexture{ aImage }
    {
        AspectRatio.assign(aAspectRatio, false);
        Placement.assign(aPlacement, false);
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_widget& aParent, const i_texture& aTexture, neogfx::aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aParent }, iTexture{ aTexture }
    {
        AspectRatio.assign(aAspectRatio, false);
        Placement.assign(aPlacement, false);
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_widget& aParent, const i_image& aImage, neogfx::aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aParent }, iTexture{ aImage }
    {
        AspectRatio.assign(aAspectRatio, false);
        Placement.assign(aPlacement, false);
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_layout& aLayout, const i_texture& aTexture, neogfx::aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aLayout }, iTexture{ aTexture }
    {
        AspectRatio.assign(aAspectRatio, false);
        Placement.assign(aPlacement, false);
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_layout& aLayout, const i_image& aImage, neogfx::aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aLayout }, iTexture{ aImage }
    {
        AspectRatio.assign(aAspectRatio, false);
        Placement.assign(aPlacement, false);
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::~image_widget()
    {
    }

    neogfx::size_policy image_widget::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    size image_widget::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size() || iTexture.is_empty() || size_policy() == size_constraint::DefaultMinimumExpanding)
            return widget::minimum_size(aAvailableSpace);
        size imageExtents = units_converter{ *this }.from_device_units(image_size() ? image_size().value() : iTexture.extents());
        if (Rotation.value() != 0.0)
            imageExtents = scoped_transform::rotated_extents(imageExtents, Rotation.value());
        size result = imageExtents + internal_spacing().size();
        if (DpiAutoScale.value())
            result *= (dpi_scale_factor() / iTexture.dpi_scale_factor());
        return to_units(*this, scoped_units::current_units(), result);
    }

    void image_widget::paint(i_graphics_context& aGc) const
    {
        if (iTexture.is_empty())
            return;
        if (service<i_debug>().layout_item() == this)
            aGc.flush();
        std::optional<scoped_transform> rotate;
        if (Rotation.value() != 0.0)
            rotate.emplace(aGc, Rotation.value(), client_rect().center());
        aGc.draw_texture(placement_rect(), iTexture, effectively_disabled() ? color(0xFF, 0xFF, 0xFF, 0x80) : ImageColor.value(), 
            effectively_disabled() ? shader_effect::Monochrome : ImageColor.value() != none ? shader_effect::Colorize : shader_effect::None);
        if (service<i_debug>().layout_item() == this)
            aGc.flush();
    }

    void image_widget::property_changed(i_property& aProperty)
    {
        if (&aProperty == &Rotation)
            update();
        widget::property_changed(aProperty);
    }

    const texture& image_widget::image() const
    {
        return iTexture;
    }

    const optional_size& image_widget::image_size() const
    {
        return ImageSize;
    }

    const color_or_gradient& image_widget::image_color() const
    {
        return ImageColor;
    }

    void image_widget::set_image(i_string const& aImageUri, dimension aDpiScaleFactor, texture_sampling aSampling)
    {
        set_image(neogfx::image{ aImageUri, aDpiScaleFactor, aSampling });
    }

    void image_widget::set_image(const i_image& aImage)
    {
        set_image(texture{ aImage });
    }

    void image_widget::set_image(const i_texture& aTexture)
    {
        size const oldSize = minimum_size();
        size const oldTextureSize = image().extents();
        iTexture = aTexture;
        ImageChanged();
        if (oldSize != minimum_size() || oldTextureSize != image().extents())
        {
            ImageGeometryChanged();
            if (visible() || effective_size_policy().ignore_visibility())
                update_layout();
        }
        update();
    }

    void image_widget::set_image_size(const i_optional<size>& aImageSize)
    {
        if (ImageSize.value() != aImageSize)
        {
            size const oldSize = minimum_size();
            ImageSize = optional_size{ aImageSize };
            if (oldSize != minimum_size())
            {
                ImageGeometryChanged();
                if (visible() || effective_size_policy().ignore_visibility())
                    update_layout();
            }
            update();
        }
    }

    void image_widget::set_image_color(const color_or_gradient& aImageColor)
    {
        ImageColor = aImageColor;
    }

    neogfx::aspect_ratio image_widget::aspect_ratio() const
    {
        return AspectRatio;
    }

    void image_widget::set_aspect_ratio(neogfx::aspect_ratio aAspectRatio)
    {
        AspectRatio = aAspectRatio;
    }

    cardinal image_widget::placement() const
    {
        return Placement;
    }

    void image_widget::set_placement(cardinal aPlacement)
    {
        Placement = aPlacement;
    }

    angle image_widget::rotation() const
    {
        return Rotation;
    }

    void image_widget::set_rotation(angle aRotation)
    {
        Rotation = aRotation;
    }

    bool image_widget::dpi_auto_scale() const
    {
        return DpiAutoScale;
    }

    void image_widget::set_dpi_auto_scale(bool aDpiAutoScale)
    {
        DpiAutoScale = aDpiAutoScale;
    }

    rect image_widget::placement_rect() const
    {
        scoped_units su{ *this, units::Pixels };
        auto imageExtents = image_size() ? image_size().value() : iTexture.extents();
        if (DpiAutoScale.value())
            imageExtents *= (dpi_scale_factor() / iTexture.dpi_scale_factor());
        rect placementRect{ point{}, imageExtents };
        auto clientRect = client_rect();
        if (Rotation.value() != 0.0)
        {
            auto const unrotated = scoped_transform::rotated_extents(clientRect.extents(), -Rotation.value());
            clientRect = rect{ point{
                clientRect.center().x - unrotated.cx / 2.0,
                clientRect.center().y - unrotated.cy / 2.0 }, unrotated };
        }
        if (AspectRatio.value() == aspect_ratio::Stretch)
        {
            placementRect.cx = clientRect.width();
            placementRect.cy = clientRect.height();
        }
        else if (placementRect.height() >= placementRect.width())
        {
            switch (AspectRatio.value())
            {
            case aspect_ratio::Ignore:
                if (placementRect.width() > clientRect.width())
                    placementRect.cx = clientRect.width();
                if (placementRect.height() > clientRect.height())
                    placementRect.cy = clientRect.height();
                break;
            case aspect_ratio::Keep:
                if (placementRect.width() > clientRect.width())
                {
                    placementRect.cx = clientRect.width();
                    placementRect.cy = placementRect.cx * imageExtents.cy / imageExtents.cx;
                }
                if (placementRect.height() > clientRect.height())
                {
                    placementRect.cy = clientRect.height();
                    placementRect.cx = placementRect.cy * imageExtents.cx / imageExtents.cy;
                }
                break;
            case aspect_ratio::KeepExpanding:
                if (placementRect.height() != clientRect.height())
                {
                    placementRect.cy = clientRect.height();
                    placementRect.cx = placementRect.cy * imageExtents.cx / imageExtents.cy;
                }
                break;
            }
        }
        else
        {
            switch (AspectRatio.value())
            {
            case aspect_ratio::Ignore:
                if (placementRect.width() > clientRect.width())
                    placementRect.cx = clientRect.width();
                if (placementRect.height() > clientRect.height())
                    placementRect.cy = clientRect.height();
                break;
            case aspect_ratio::Keep:
                if (placementRect.height() > clientRect.height())
                {
                    placementRect.cy = clientRect.height();
                    placementRect.cx = placementRect.cy * imageExtents.cx / imageExtents.cy;
                }
                if (placementRect.width() > clientRect.width())
                {
                    placementRect.cx = clientRect.width();
                    placementRect.cy = placementRect.cx * imageExtents.cy / imageExtents.cx;
                }
                break;
            case aspect_ratio::KeepExpanding:
                if (placementRect.width() != clientRect.width())
                {
                    placementRect.cx = clientRect.width();
                    placementRect.cy = placementRect.cx * imageExtents.cy / imageExtents.cx;
                }
                break;
            }
        }
        switch (Placement.value())
        {
        case cardinal::NorthWest:
            placementRect.position() = point{};
            break;
        case cardinal::North:
            placementRect.position() = point{ (clientRect.width() - placementRect.cx) / 2.0, 0.0 };
            break;
        case cardinal::NorthEast:
            placementRect.position() = point{ clientRect.width() - placementRect.width(), 0.0 };
            break;
        case cardinal::West:
            placementRect.position() = point{ 0.0, (clientRect.height() - placementRect.cy) / 2.0 };
            break;
        case cardinal::Center:
            placementRect.position() = point{ (clientRect.width() - placementRect.cx) / 2.0, (clientRect.height() - placementRect.cy) / 2.0 };
            break;
        case cardinal::East:
            placementRect.position() = point{ clientRect.width() - placementRect.width(), (clientRect.height() - placementRect.cy) / 2.0 };
            break;
        case cardinal::SouthWest:
            placementRect.position() = point{ 0.0, clientRect.height() - placementRect.height() };
            break;
        case cardinal::South:
            placementRect.position() = point{ (clientRect.width() - placementRect.cx) / 2.0, clientRect.height() - placementRect.height() };
            break;
        case cardinal::SouthEast:
            placementRect.position() = point{ clientRect.width() - placementRect.width(), clientRect.height() - placementRect.height() };
            break;
        }
        if (Rotation.value() != 0.0)
            placementRect.position() += clientRect.position();
        return floor_rasterized(placementRect);
    }
}