// texture.hpp
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
#include <vector>
#include <optional>
#include <neogfx/core/color.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/sub_texture.hpp>

namespace neogfx
{
    class i_image;
    class i_native_texture;

    class texture : public i_texture
    {
        // construction
    public:
        texture();
        texture(const neogfx::size& aExtents, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, texture_data_format aDataFormat = texture_data_format::RGBA, texture_data_type aDataType = texture_data_type::UnsignedByte, const optional_color& aColor = optional_color());
        texture(const i_texture& aTexture);
        texture(const i_image& aImage, texture_data_format aDataFormat = texture_data_format::RGBA, texture_data_type aDataType = texture_data_type::UnsignedByte);
        texture(const i_sub_texture& aSubTexture);
        ~texture();
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
        void set_pixel(const point& aPosition, const color& aColor) override;
        color get_pixel(const point& aPosition) const override;
    public:
        int32_t bind(const std::optional<uint32_t>& aTextureUnit = std::optional<uint32_t>{}) const override;
    public:
        intptr_t native_handle() const override;
        std::shared_ptr<i_native_texture> native_texture() const override;
        // attributes
    private:
        std::shared_ptr<i_texture> iNativeTexture;
        optional_sub_texture iSubTexture;
    };

    typedef std::optional<texture> optional_texture;
}