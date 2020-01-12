// sub_texture.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <optional>
#include <neogfx/gfx/i_image.hpp>
#include <neogfx/gfx/i_sub_texture.hpp>

namespace neogfx
{
    class sub_texture : public i_sub_texture
    {
        // construction
    public:
        sub_texture(texture_id aAtlasId, i_texture& aAtlasTexture, const rect& aAtlasLocation, const size& aExtents);
        sub_texture(const i_sub_texture& aSubTexture);
        sub_texture(const i_sub_texture& aSubTexture, const rect& aAtlasLocation);
        ~sub_texture();
        // operations
    public:
        texture_id id() const override;
        texture_type type() const override;
        bool is_render_target() const override;
        const i_sub_texture& as_sub_texture() const override;
        dimension dpi_scale_factor() const override;
        texture_sampling sampling() const override;
        uint32_t samples() const override;
        texture_data_format data_format() const override;
        texture_data_type data_type() const override;
        bool is_empty() const override;
        size extents() const override;
        size storage_extents() const override;
        void set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment = 4u) override;
        void set_pixels(const i_image& aImage) override;
        void set_pixel(const point& aPosition, const colour& aColour) override;
        colour get_pixel(const point& aPosition) const override;
    public:
        int32_t bind(const std::optional<uint32_t>& aTextureUnit = std::optional<uint32_t>{}) const override;
    public:
        intptr_t native_handle() const override;
        std::shared_ptr<i_native_texture> native_texture() const override;
    public:
        texture_id atlas_id() const override;
        i_texture& atlas_texture() const override;
        const rect& atlas_location() const override;
        // attributes
    private:
        texture_id iAtlasId;
        i_texture* iAtlasTexture;
        rect iAtlasLocation;
        size iExtents;
    };

    typedef std::optional<sub_texture> optional_sub_texture;
}