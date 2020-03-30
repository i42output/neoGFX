// i_texture.hpp
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
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/color.hpp>
#include <neogfx/game/ecs_ids.hpp>

namespace neogfx
{
    typedef game::id_t texture_id;

    class i_native_texture;

    enum class texture_type : uint32_t
    {
        Texture,
        SubTexture
    };

    enum class texture_sampling : uint32_t
    {
        Normal,
        NormalMipmap,
        Nearest,
        Scaled,
        Data,
        Multisample,
        Multisample4x,
        Multisample8x,
        Multisample16x,
        Multisample32x
    };

    enum class texture_data_format : uint32_t
    {
        // todo: add remaining GL texture data formats
        RGBA        = 0x01,
        Red         = 0x02,
        SubPixel    = 0x03
    };

    enum class texture_data_type : uint32_t
    {
        // todo: add remaining GL texture data types
        UnsignedByte,
        Float
    };

    class i_sub_texture;
    class i_image;

    class i_texture
    {
    public:
        struct texture_empty : std::logic_error { texture_empty() : std::logic_error("neogfx::i_texture::texture_empty") {} };
        struct not_sub_texture : std::logic_error { not_sub_texture() : std::logic_error("neogfx::i_texture::not_sub_texture") {} };
    public:
        virtual ~i_texture() = default;
    public:
        virtual texture_id id() const = 0;
        virtual texture_type type() const = 0;
        virtual bool is_render_target() const = 0;
        virtual const i_sub_texture& as_sub_texture() const = 0;
        virtual dimension dpi_scale_factor() const = 0;
        virtual texture_sampling sampling() const = 0;
        virtual uint32_t samples() const = 0;
        virtual texture_data_format data_format() const = 0;
        virtual texture_data_type data_type() const = 0;
        virtual bool is_empty() const = 0;
        virtual size extents() const = 0;
        virtual size storage_extents() const = 0;
        virtual void set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment = 4u) = 0;
        virtual void set_pixels(const i_image& aImage) = 0;
        virtual void set_pixel(const point& aPosition, const color& aColor) = 0;
        virtual color get_pixel(const point& aPosition) const = 0;
    public:
        virtual int32_t bind(const std::optional<uint32_t>& aTextureUnit = std::optional<uint32_t>{}) const = 0;
    public:
        virtual intptr_t native_handle() const = 0;
        virtual std::shared_ptr<i_native_texture> native_texture() const = 0;
    };
}