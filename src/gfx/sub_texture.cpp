// sub_texture.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020, 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/texture_manager.hpp>
#include <neogfx/gfx/sub_texture.hpp>
#include "native/i_native_texture.hpp"

namespace neogfx
{
    sub_texture::sub_texture(texture_id aAtlasId, i_texture& aAtlasTexture, rect const& aAtlasLocation, size const& aExtents) :
        iAtlasId{ aAtlasId }, iAtlasTexture{ &aAtlasTexture }, iAtlasLocation{ aAtlasLocation }, iStorageExtents{ aAtlasTexture.storage_extents() }, iExtents{ aExtents }
    {
        service<i_texture_manager>().add_sub_texture(*this);
    }

    sub_texture::sub_texture(sub_texture const& aSubTexture) :
        iChild{ true }, iAtlasId{ aSubTexture.atlas_id() }, iAtlasTexture{ &aSubTexture.atlas_texture() }, iAtlasLocation{ aSubTexture.atlas_location() }, iStorageExtents{ aSubTexture.storage_extents() }, iExtents{ aSubTexture.extents() }
    {
        add_ref();
    }

    sub_texture::sub_texture(i_sub_texture const& aSubTexture) :
        iChild{ true }, iAtlasId{ aSubTexture.atlas_id() }, iAtlasTexture{ &aSubTexture.atlas_texture() }, iAtlasLocation{ aSubTexture.atlas_location() }, iStorageExtents{ aSubTexture.storage_extents() }, iExtents{ aSubTexture.extents() }
    {
        add_ref();
    }

    sub_texture::sub_texture(i_sub_texture const& aSubTexture, rect const& aAtlasLocation) :
        iChild{ true }, iAtlasId{ aSubTexture.atlas_id() }, iAtlasTexture{ &aSubTexture.atlas_texture() }, iAtlasLocation{ aAtlasLocation }, iStorageExtents{ aSubTexture.storage_extents() }, iExtents{ aAtlasLocation.extents() }
    {
        add_ref();
    }

    sub_texture::~sub_texture()
    {
        if (iChild)
            release();
    }

    void sub_texture::add_ref(long aCount) const noexcept
    {
        if (iChild)
        {
            service<i_texture_manager>().add_ref(atlas_texture().id(), aCount);
            service<i_texture_manager>().add_ref(id(), aCount);
        }
        else
            base_type::add_ref(aCount);
    }

    void sub_texture::release(long aCount) const
    {
        if (iChild)
        {
            service<i_texture_manager>().release(id(), aCount);
            service<i_texture_manager>().release(atlas_texture().id(), aCount);
        }
        else
            base_type::release(aCount);
    }

    long sub_texture::use_count() const noexcept
    {
        if (iChild)
            return service<i_texture_manager>().use_count(id());
        else
            return base_type::use_count();
    }

    texture_id sub_texture::id() const
    {
        return atlas_id();
    }

    i_string const& sub_texture::uri() const
    {
        return native_texture().uri();
    }

    rect const& sub_texture::part() const
    {
        return native_texture().part();
    }

    texture_type sub_texture::type() const
    {
        return texture_type::SubTexture;
    }

    bool sub_texture::is_render_target() const
    {
        return native_texture().is_render_target();
    }

    const i_render_target& sub_texture::as_render_target() const
    {
        return static_cast<i_native_texture const&>(native_texture());
    }

    i_render_target& sub_texture::as_render_target()
    {
        return static_cast<i_native_texture&>(native_texture());
    }

    const i_sub_texture& sub_texture::as_sub_texture() const
    {
        return *this;
    }

    dimension sub_texture::dpi_scale_factor() const
    {
        return native_texture().dpi_scale_factor();
    }

    neogfx::color_space sub_texture::color_space() const
    {
        return native_texture().color_space();
    }

    texture_sampling sub_texture::sampling() const
    {
        return native_texture().sampling();
    }

    uint32_t sub_texture::samples() const
    {
        return native_texture().samples();
    }

    texture_data_format sub_texture::data_format() const
    {
        return native_texture().data_format();
    }

    texture_data_type sub_texture::data_type() const
    {
        return native_texture().data_type();
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
        return iStorageExtents;
    }

    void sub_texture::set_pixels(rect const& aRect, void const* aPixelData, uint32_t aPackAlignment)
    {
        rect r = aRect;
        r.position() += atlas_location().position();
        r = r.intersection(atlas_location());
        if (r.cx != aRect.cx || r.cy != aRect.cy)
            throw bad_rectangle();
        native_texture().set_pixels(r, aPixelData, aPackAlignment);
    }

    void sub_texture::set_pixels(i_image const& aImage)
    {
        set_pixels(aImage, rect{ point{ 0.0, 0.0 }, aImage.extents() });
    }

    void sub_texture::set_pixels(i_image const& aImage, rect const& aImagePart)
    {
        size_u32 const imageExtents = aImage.extents();
        point_u32 const imagePartOrigin = aImagePart.position();
        size_u32 const imagePartExtents = aImagePart.extents();
        switch (aImage.color_format())
        {
        case color_format::RGBA8:
            {
                const uint8_t* imageData = static_cast<uint8_t const*>(aImage.cpixels());
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

    void sub_texture::set_pixel(point const& aPosition, color const& aColor)
    {
        native_texture().set_pixel(aPosition + atlas_location().position(), aColor);
    }

    color sub_texture::get_pixel(point const& aPosition) const
    {
        if (is_empty())
            throw texture_empty();
        if (aPosition.x < 0.0 || aPosition.y < 0.0 || aPosition.x >= extents().cx || aPosition.y >= extents().cy)
            return color{};
        return native_texture().get_pixel(aPosition + atlas_location().position());
    }

    i_vector<texture_line_segment> const& sub_texture::intersection(texture_line_segment const& aLine, rect const& aBoundingBox, vec2 const& aSampleSize, scalar aTolerance) const
    {
        auto existingResult = iIntersectionResultCache.find(std::make_tuple(aLine, aBoundingBox, aSampleSize, aTolerance));
        if (existingResult != iIntersectionResultCache.end())
            return existingResult->second;

        auto& result = iIntersectionResultCache[std::make_tuple(aLine, aBoundingBox, aSampleSize, aTolerance)];
        result = native_texture().intersection(texture_line_segment{ aLine.v1 + atlas_location().position().to_vec2(), aLine.v2 + atlas_location().position().to_vec2() }, 
            aBoundingBox + atlas_location().position(), aSampleSize, aTolerance);
        for (auto& segment : result)
        {
            segment.v1 -= atlas_location().position().to_vec2();
            segment.v2 -= atlas_location().position().to_vec2();
        }
        return result;
    }

    void sub_texture::bind(std::uint32_t aTextureUnit) const
    {
        if (is_empty())
            throw texture_empty();
        return native_texture().bind(aTextureUnit);
    }

    void sub_texture::unbind() const
    {
        if (is_empty())
            throw texture_empty();
        return native_texture().unbind();
    }

    intptr_t sub_texture::native_handle() const
    {
        return native_texture().native_handle();
    }

    i_texture& sub_texture::native_texture() const
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