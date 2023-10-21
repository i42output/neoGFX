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
#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/sub_texture.hpp>

namespace neogfx
{
    class i_image;
    class i_native_texture;

    class texture : public reference_counted<i_texture>
    {
        // construction
    public:
        texture();
        texture(const neogfx::size& aExtents, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, texture_data_format aDataFormat = texture_data_format::RGBA, texture_data_type aDataType = texture_data_type::UnsignedByte, neogfx::color_space aColorSpace = neogfx::color_space::sRGB, const optional_color& aColor = optional_color{});
        texture(const i_texture& aTexture);
        texture(const i_image& aImage, texture_data_format aDataFormat = texture_data_format::RGBA, texture_data_type aDataType = texture_data_type::UnsignedByte);
        texture(const i_image& aImage, const rect& aImagePart, texture_data_format aDataFormat = texture_data_format::RGBA, texture_data_type aDataType = texture_data_type::UnsignedByte);
        texture(const i_sub_texture& aSubTexture);
        ~texture();
        // operations
    public:
        texture_id id() const final;
        i_string const& uri() const final;
        rect const& part() const final;
        texture_type type() const final;
        bool is_render_target() const final;
        const i_render_target& as_render_target() const final;
        i_render_target& as_render_target() final;
        const i_sub_texture& as_sub_texture() const final;
        dimension dpi_scale_factor() const final;
        neogfx::color_space color_space() const final;
        texture_sampling sampling() const final;
        uint32_t samples() const final;
        texture_data_format data_format() const final;
        texture_data_type data_type() const final;
        bool is_empty() const final;
        size extents() const final;
        size storage_extents() const final;
        void set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment = 4u) final;
        void set_pixels(const i_image& aImage) final;
        void set_pixels(const i_image& aImage, const rect& aImagePart) final;
        void set_pixel(const point& aPosition, const color& aColor) final;
        color get_pixel(const point& aPosition) const final;
        i_vector<texture_line_segment> const& intersection(texture_line_segment const& aLine, rect const& aBoundingBox, vec2 const& aSampleSize = { 1.0, 1.0 }, scalar aTolerance = 0.0) const final;
    public:
        void bind(std::uint32_t aTextureUnit) const final;
        void unbind() const final;
    public:
        intptr_t native_handle() const final;
        i_texture& native_texture() const final;
        // attributes
    private:
        ref_ptr<i_texture> iNativeTexture;
        optional_sub_texture iSubTexture;
    };

    typedef std::optional<texture> optional_texture;
}