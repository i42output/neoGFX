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
#include <neogfx/gui/widget/image_widget.hpp>


namespace neogfx
{
    image_widget::image_widget(const i_texture& aTexture, aspect_ratio aAspectRatio, cardinal aPlacement) :
        iTexture{ aTexture }, iAspectRatio{ aAspectRatio }, iPlacement{ aPlacement }, iDpiAutoScale{ false }
    {
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(const i_image& aImage, aspect_ratio aAspectRatio, cardinal aPlacement) :
        iTexture{ aImage }, iAspectRatio{ aAspectRatio }, iPlacement{ aPlacement }, iDpiAutoScale{ false }
    {
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_widget& aParent, const i_texture& aTexture, aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aParent }, iTexture{ aTexture }, iAspectRatio{ aAspectRatio }, iPlacement{ aPlacement }, iDpiAutoScale{ false }
    {
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_widget& aParent, const i_image& aImage, aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aParent }, iTexture{ aImage }, iAspectRatio{ aAspectRatio }, iPlacement{ aPlacement }, iDpiAutoScale{ false }
    {
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_layout& aLayout, const i_texture& aTexture, aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aLayout }, iTexture{ aTexture }, iAspectRatio{ aAspectRatio }, iPlacement{ aPlacement }, iDpiAutoScale{ false }
    {
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
    }

    image_widget::image_widget(i_layout& aLayout, const i_image& aImage, aspect_ratio aAspectRatio, cardinal aPlacement) :
        widget{ aLayout }, iTexture{ aImage }, iAspectRatio{ aAspectRatio }, iPlacement{ aPlacement }, iDpiAutoScale{ false }
    {
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
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
        size result = units_converter{ *this }.from_device_units(iTexture.extents()) + internal_spacing().size();
        if (iDpiAutoScale)
            result *= (dpi_scale_factor() / iTexture.dpi_scale_factor());
        return to_units(*this, scoped_units::current_units(), result);
    }

    void image_widget::paint(i_graphics_context& aGc) const
    {
        if (iTexture.is_empty())
            return;
        aGc.draw_texture(placement_rect(), iTexture, effectively_disabled() ? color(0xFF, 0xFF, 0xFF, 0x80) : iColor, effectively_disabled() ? shader_effect::Monochrome : iColor != none ? shader_effect::Colorize : shader_effect::None);
    }

    const texture& image_widget::image() const
    {
        return iTexture;
    }

    const color_or_gradient& image_widget::image_color() const
    {
        return iColor;
    }

    void image_widget::set_image(i_string const& aImageUri)
    {
        set_image(neogfx::image{ aImageUri });
    }

    void image_widget::set_image(const i_image& aImage)
    {
        set_image(texture{ aImage });
    }

    void image_widget::set_image(const i_texture& aTexture)
    {
        size oldSize = minimum_size();
        size oldTextureSize = image().extents();
        iTexture = aTexture;
        ImageChanged.trigger();
        if (oldSize != minimum_size() || oldTextureSize != image().extents())
        {
            ImageGeometryChanged.trigger();
            if (has_parent_layout() && (visible() || parent_layout().ignore_visibility()))
                update_layout(true, true);
        }
        update();
    }

    void image_widget::set_image_color(const color_or_gradient& aImageColor)
    {
        iColor = aImageColor;
    }

    void image_widget::set_aspect_ratio(aspect_ratio aAspectRatio)
    {
        if (iAspectRatio != aAspectRatio)
        {
            iAspectRatio = aAspectRatio;
            update();
        }
    }

    void image_widget::set_placement(cardinal aPlacement)
    {
        if (iPlacement != aPlacement)
        {
            iPlacement = aPlacement;
            update();
        }
    }

    void image_widget::set_dpi_auto_scale(bool aDpiAutoScale)
    {
        if (iDpiAutoScale != aDpiAutoScale)
        {
            iDpiAutoScale = aDpiAutoScale;
            update();
        }
    }

    rect image_widget::placement_rect() const
    {
        scoped_units su{ *this, units::Pixels };
        auto imageExtents = iTexture.extents();
        if (iDpiAutoScale)
            imageExtents *= (dpi_scale_factor() / iTexture.dpi_scale_factor());
        rect placementRect{ point{}, imageExtents };
        auto const clientRect = client_rect();
        if (iAspectRatio == aspect_ratio::Stretch)
        {
            placementRect.cx = clientRect.width();
            placementRect.cy = clientRect.height();
        }
        else if (placementRect.height() >= placementRect.width())
        {
            switch (iAspectRatio)
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
            switch (iAspectRatio)
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
        switch (iPlacement)
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
        return floor_rasterized(placementRect);
    }
}