// sub_texture.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/sub_texture.hpp>
#include "native/i_native_texture.hpp"

namespace neogfx
{
    sub_texture::sub_texture(texture_id aAtlasId, i_texture& aAtlasTexture, const rect& aAtlasLocation, const size& aExtents) :
        iAtlasId(aAtlasId), iAtlasTexture(&aAtlasTexture), iAtlasLocation(aAtlasLocation), iExtents(aExtents)
    {
    }

    sub_texture::sub_texture(const i_sub_texture& aSubTexture) :
        iAtlasId(aSubTexture.atlas_id()), iAtlasTexture(&aSubTexture.atlas_texture()), iAtlasLocation(aSubTexture.atlas_location()), iExtents(aSubTexture.extents())
    {
    }

    sub_texture::sub_texture(const i_sub_texture& aSubTexture, const rect& aAtlasLocation) :
        iAtlasId(aSubTexture.atlas_id()), iAtlasTexture(&aSubTexture.atlas_texture()), iAtlasLocation(aAtlasLocation), iExtents(aAtlasLocation.extents())
    {
    }

    sub_texture::~sub_texture()
    {
    }

    texture_id sub_texture::id() const
    {
        return atlas_id();
    }

    texture_type sub_texture::type() const
    {
        return texture_type::SubTexture;
    }

    bool sub_texture::is_render_target() const
    {
        return native_texture()->is_render_target();
    }

    const i_sub_texture& sub_texture::as_sub_texture() const
    {
        return *this;
    }

    dimension sub_texture::dpi_scale_factor() const
    {
        return native_texture()->dpi_scale_factor();
    }

    texture_sampling sub_texture::sampling() const
    {
        return native_texture()->sampling();
    }

    uint32_t sub_texture::samples() const
    {
        return native_texture()->samples();
    }

    texture_data_format sub_texture::data_format() const
    {
        return native_texture()->data_format();
    }

    texture_data_type sub_texture::data_type() const
    {
        return native_texture()->data_type();
    }

    bool sub_texture::is_empty() const
    {
        return false;
    }

    size sub_texture::extents() const
    {
        if (is_empty())
            return size{};
        return iExtents;
    }

    size sub_texture::storage_extents() const
    {
        if (is_empty())
            return size{};
        return native_texture()->storage_extents();
    }

    void sub_texture::set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment)
    {
        rect r = aRect;
        r.position() += atlas_location().position();
        r = r.intersection(atlas_location());
        if (r.cx != aRect.cx || r.cy != aRect.cy)
            throw bad_rectangle();
        native_texture()->set_pixels(r, aPixelData, aPackAlignment);
    }

    void sub_texture::set_pixels(const i_image& aImage)
    {
        size_u32 imageExtents = aImage.extents();
        switch (aImage.colour_format())
        {
        case colour_format::RGBA8:
            {
                const uint8_t* imageData = static_cast<const uint8_t*>(aImage.cpixels());
                std::vector<uint8_t> data(imageExtents.cx * 4 * imageExtents.cy);
                for (std::size_t y = 0; y < imageExtents.cy; ++y)
                    for (std::size_t x = 0; x < imageExtents.cx; ++x)
                        for (std::size_t c = 0; c < 4; ++c)
                            data[(imageExtents.cy - 1 - y) * imageExtents.cx * 4 + x * 4 + c] = imageData[y * imageExtents.cx * 4 + x * 4 + c];
                set_pixels(rect{ point{}, aImage.extents() }, &data[0]);
            }
        }
    }

    void sub_texture::set_pixel(const point& aPosition, const colour& aColour)
    {
        native_texture()->set_pixel(aPosition + atlas_location().position(), aColour);
    }

    colour sub_texture::get_pixel(const point& aPosition) const
    {
        return native_texture()->get_pixel(aPosition + atlas_location().position());
    }

    int32_t sub_texture::bind(const std::optional<uint32_t>& aTextureUnit) const
    {
        return native_texture()->bind(aTextureUnit);
    }

    std::shared_ptr<i_native_texture> sub_texture::native_texture() const
    {
        return atlas_texture().native_texture();
    }

    texture_id sub_texture::atlas_id() const
    {
        return iAtlasId;
    }

    i_texture& sub_texture::atlas_texture() const
    {
        return *iAtlasTexture;
    }

    const rect& sub_texture::atlas_location() const
    {
        return iAtlasLocation;
    }
}