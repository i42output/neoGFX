// texture.cpp
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
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/texture_manager.hpp>
#include "native/i_native_texture.hpp"

namespace neogfx
{
    texture::texture()
    {
    }

    texture::texture(const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat, texture_data_type aDataType, neogfx::color_space aColorSpace, const optional_color& aColor) :
        iNativeTexture{ service<i_texture_manager>().create_texture(aExtents, aDpiScaleFactor, aSampling, aDataFormat, aDataType, aColorSpace, aColor) }
    {
    }

    texture::texture(const i_texture& aTexture) :
        iNativeTexture{ !aTexture.is_empty() ? aTexture.native_texture() : ref_ptr<i_texture>{} }
    {
    }

    texture::texture(const i_image& aImage, texture_data_format aDataFormat, texture_data_type aDataType) :
        iNativeTexture{ !aImage.is_empty() ? service<i_texture_manager>().create_texture(aImage, aDataFormat, aDataType) : ref_ptr<i_texture>{} }
    {
    }

    texture::texture(const i_image& aImage, const rect& aImagePart, texture_data_format aDataFormat, texture_data_type aDataType) :
        iNativeTexture{ !aImage.is_empty() ? service<i_texture_manager>().create_texture(aImage, aImagePart, aDataFormat, aDataType) : ref_ptr<i_texture>{} }
    {
    }

    texture::texture(const i_sub_texture& aSubTexture) :
        iNativeTexture{ !aSubTexture.atlas_texture().is_empty() ? aSubTexture.atlas_texture().native_texture() : ref_ptr<i_texture>{} },
        iSubTexture{ aSubTexture }
    {
    }

    texture::~texture()
    {
    }

    texture_id texture::id() const
    {
        return native_texture().id();
    }

    i_string const& texture::uri() const
    {
        return native_texture().uri();
    }

    rect const& texture::part() const
    {
        return native_texture().part();
    }

    texture_type texture::type() const
    {
        return texture_type::Texture;
    }

    bool texture::is_render_target() const
    {
        return native_texture().is_render_target();
    }

    const i_render_target& texture::as_render_target() const
    {
        return static_cast<i_native_texture&>(native_texture());
    }

    i_render_target& texture::as_render_target()
    {
        return static_cast<i_native_texture&>(native_texture());
    }

    const i_sub_texture& texture::as_sub_texture() const
    {
        if (iSubTexture != std::nullopt)
            return *iSubTexture;
        throw not_sub_texture();
    }

    dimension texture::dpi_scale_factor() const
    {
        if (is_empty())
            return 1.0;
        return native_texture().dpi_scale_factor();
    }

    color_space texture::color_space() const
    {
        if (is_empty())
            return neogfx::color_space::sRGB;
        return native_texture().color_space();
    }

    texture_sampling texture::sampling() const
    {
        if (is_empty())
            return texture_sampling::NormalMipmap;
        return native_texture().sampling();
    }

    uint32_t texture::samples() const
    {
        if (is_empty())
            return 1;
        return native_texture().samples();
    }

    texture_data_format texture::data_format() const
    {
        if (is_empty())
            return texture_data_format::RGBA;
        return native_texture().data_format();
    }

    texture_data_type texture::data_type() const
    {
        if (is_empty())
            return texture_data_type::UnsignedByte;
        return native_texture().data_type();
    }

    bool texture::is_empty() const
    {
        return iNativeTexture == nullptr;
    }

    size texture::extents() const
    {
        if (is_empty())
            return size{};
        return native_texture().extents();
    }

    size texture::storage_extents() const
    {
        if (is_empty())
            return size{};
        return native_texture().storage_extents();
    }

    void texture::set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment)
    {
        native_texture().set_pixels(aRect, aPixelData, aPackAlignment);
    }

    void texture::set_pixels(const i_image& aImage)
    {
        set_pixels(rect{ point{}, aImage.extents() }, aImage.cpixels());
    }

    void texture::set_pixels(const i_image& aImage, const rect& aImagePart)
    {
        size_u32 const imageExtents = aImage.extents();
        point_u32 const imagePartOrigin = aImagePart.position();
        size_u32 const imagePartExtents = aImagePart.extents();
        switch (aImage.color_format())
        {
        case color_format::RGBA8:
            {
                const uint8_t* imageData = static_cast<const uint8_t*>(aImage.cpixels());
                std::vector<uint8_t> data(imagePartExtents.cx * 4 * imagePartExtents.cy);
                for (std::size_t y = 0; y < imagePartExtents.cy; ++y)
                    for (std::size_t x = 0; x < imagePartExtents.cx; ++x)
                        for (std::size_t c = 0; c < 4; ++c)
                            data[(imagePartExtents.cy - 1 - y) * imagePartExtents.cx * 4 + x * 4 + c] = imageData[(y + imagePartOrigin.y) * imageExtents.cx * 4 + (x + imagePartOrigin.x) * 4 + c];
                set_pixels(rect{ point{}, imagePartExtents }, &data[0]);
            }
            break;
        }
    }

    void texture::set_pixel(const point& aPosition, const color& aColor)
    {
        if (is_empty())
            throw texture_empty();
        return native_texture().set_pixel(aPosition, aColor);
    }

    color texture::get_pixel(const point& aPosition) const
    {
        if (is_empty())
            throw texture_empty();
        return native_texture().get_pixel(aPosition);
    }

    int32_t texture::bind(const std::optional<uint32_t>& aTextureUnit) const
    {
        if (is_empty())
            throw texture_empty();
        return native_texture().bind(aTextureUnit);
    }

    intptr_t texture::native_handle() const
    {
        return native_texture().native_handle();
    }

    i_texture& texture::native_texture() const
    {
        if (is_empty())
            throw texture_empty();
        return *iNativeTexture;
    }
}