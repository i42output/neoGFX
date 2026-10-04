// vulkan_texture.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023, 2026 Leigh Johnston.  All Rights Reserved.

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

#include <cmath>
#include <cstring>

#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include "../opengl/opengl_rendering_context.hpp"
#include "vulkan_error.hpp"
#include "vulkan_texture.hpp"

namespace neogfx
{
    namespace
    {
        struct vk_texture_format
        {
            VkFormat format;
            std::uint32_t texelSize;
        };

        // n.b. as opengl_texture (to_gl_enums): the internal format (BGRA and subpixel data are stored as RGBA)
        inline vk_texture_format to_vk_format(texture_data_format aDataFormat, texture_data_type aDataType)
        {
            switch (aDataFormat)
            {
            case texture_data_format::RGBA:
            case texture_data_format::BGRA:
            case texture_data_format::SubPixel:
                switch (aDataType)
                {
                case texture_data_type::UnsignedByte:
                    return { VK_FORMAT_R8G8B8A8_UNORM, 4u };
                case texture_data_type::Float:
                    return { VK_FORMAT_R32G32B32A32_SFLOAT, 16u };
                default:
                    throw std::logic_error("neogfx::to_vk_format: bad data type");
                }
            case texture_data_format::Red:
                switch (aDataType)
                {
                case texture_data_type::UnsignedByte:
                    return { VK_FORMAT_R8_UNORM, 1u };
                case texture_data_type::Float:
                    return { VK_FORMAT_R32_SFLOAT, 4u };
                default:
                    throw std::logic_error("neogfx::to_vk_format: bad data type");
                }
            default:
                throw std::logic_error("neogfx::to_vk_format: bad data format");
            }
        }

        inline std::uint32_t component_count(texture_data_format aDataFormat)
        {
            return aDataFormat == texture_data_format::Red ? 1u : 4u;
        }

        inline vulkan_sampler_state sampler_state(texture_sampling aSampling)
        {
            // n.b. as opengl_texture: clamped to a transparent border
            switch (aSampling)
            {
            case texture_sampling::Normal:
            default:
                return { true, true, false, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER };
            case texture_sampling::NormalMipmap:
                return { true, true, true, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER };
            case texture_sampling::Nearest:
            case texture_sampling::Scaled:
            case texture_sampling::Data:
                return { false, false, false, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER };
            }
        }

        constexpr std::int32_t MaxBleedGuardWidth_i32 = static_cast<std::int32_t>(MaxBleedGuardWidth);

        inline std::int32_t align_up(std::int32_t aValue, std::int32_t aAlignment)
        {
            return (aValue + aAlignment - 1) / aAlignment * aAlignment;
        }

        inline std::int32_t next_power_of_two(std::int32_t aValue, std::int32_t aMinimum)
        {
            auto result = aMinimum;
            while (result < aValue)
                result *= 2;
            return result;
        }

        inline std::uint32_t mip_levels(texture_sampling aSampling, size_u32 const& aExtents)
        {
            if (aSampling != texture_sampling::NormalMipmap)
                return 1u;
            return static_cast<std::uint32_t>(std::floor(std::log2(std::max(aExtents.cx, aExtents.cy)))) + 1u;
        }
    }

    template <typename T>
    vulkan_texture<T>::vulkan_texture(i_texture_manager& aManager, texture_id aId, const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat, neogfx::color_space aColorSpace, const optional_color& aColor) :
        iManager{ aManager },
        iId{ aId },
        iUri{ "neogfx::vulkan_texture::internal" },
        iPart{ aExtents },
        iDpiScaleFactor{ aDpiScaleFactor },
        iColorSpace{ aColorSpace },
        iSampling{ aSampling },
        iDataFormat{ aDataFormat },
        iSize{ aExtents },
        iStorageSize{ aSampling == texture_sampling::Data ?
            iSize :
            aSampling == texture_sampling::NormalMipmap ?
                size_i32{
                    next_power_of_two(static_cast<std::int32_t>(iSize.cx) + MaxBleedGuardWidth_i32 * 2, 16),
                    next_power_of_two(static_cast<std::int32_t>(iSize.cy) + MaxBleedGuardWidth_i32 * 2, 16) }.as<u32>() :
                size_i32{
                    align_up(static_cast<std::int32_t>(iSize.cx) + MaxBleedGuardWidth_i32 * 2, 16),
                    align_up(static_cast<std::int32_t>(iSize.cy) + MaxBleedGuardWidth_i32 * 2, 16) }.as<u32>() },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGame }
    {
        bool const multisample = (sampling() == texture_sampling::Multisample);
        if (aColor && multisample)
            throw std::logic_error("neogfx::vulkan_texture: clear colour unsupported for multisample sampling");
        auto const format = to_vk_format(iDataFormat, kDataType);
        iImage = backend().create_image(iStorageSize.cx, iStorageSize.cy, format.format, format.texelSize,
            multisample ? backend().sample_count(samples()) : VK_SAMPLE_COUNT_1_BIT, mip_levels(sampling(), iStorageSize));
        iImage->sampler = sampler_state(sampling());
        if (aColor)
            backend().clear_image(*iImage, { aColor->red<float>(), aColor->green<float>(), aColor->blue<float>(), aColor->alpha<float>() });
    }

    template <typename T>
    vulkan_texture<T>::vulkan_texture(i_texture_manager& aManager, texture_id aId, const i_image& aImage, const rect& aImagePart, texture_data_format aDataFormat) :
        iManager{ aManager },
        iId{ aId },
        iUri{ aImage.uri() },
        iPart{ aImagePart },
        iDpiScaleFactor{ aImage.dpi_scale_factor() },
        iColorSpace{ aImage.color_space() },
        iSampling{ aImage.sampling() },
        iDataFormat{ aDataFormat },
        iSize{ aImagePart.extents() },
        iStorageSize{ aImage.sampling() == texture_sampling::Data ?
            iSize :
            aImage.sampling() == texture_sampling::NormalMipmap ?
                size_i32{
                    next_power_of_two(static_cast<std::int32_t>(iSize.cx) + MaxBleedGuardWidth_i32 * 2, 16),
                    next_power_of_two(static_cast<std::int32_t>(iSize.cy) + MaxBleedGuardWidth_i32 * 2, 16) }.as<u32>() :
                size_i32{
                    align_up(static_cast<std::int32_t>(iSize.cx) + MaxBleedGuardWidth_i32 * 2, 16),
                    align_up(static_cast<std::int32_t>(iSize.cy) + MaxBleedGuardWidth_i32 * 2, 16) }.as<u32>() },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGame }
    {
        if constexpr (!std::is_same_v<value_type, avec4u8> && !std::is_same_v<value_type, std::array<float, 4>>)
            throw unsupported_color_format();
        else
        {
            constexpr bool normalize = std::is_same_v<value_type, std::array<float, 4>>;

            if (sampling() == texture_sampling::Multisample)
                throw multisample_texture_initialization_unsupported();
            if (aImage.color_format() != color_format::RGBA8)
                throw unsupported_color_format();

            size_u32 const imageExtents = aImage.extents();
            point_u32 const partOrigin = aImagePart.position();
            size_u32 const partExtents{ iSize };
            auto const bleedGuard = static_cast<std::uint32_t>(bleed_guard());

            if (partOrigin.x + partExtents.cx > imageExtents.cx || partOrigin.y + partExtents.cy > imageExtents.cy)
                throw std::logic_error("neogfx::vulkan_texture: image part out of range");
            if (bleedGuard * 2 + partExtents.cx > iStorageSize.cx || bleedGuard * 2 + partExtents.cy > iStorageSize.cy)
                throw std::logic_error("neogfx::vulkan_texture: bleed guard exceeds storage size");

            thread_local std::vector<value_type> data;
            // one element per texel; value_type already holds all four components
            data.assign(static_cast<std::size_t>(iStorageSize.cx) * iStorageSize.cy, value_type{});

            // n.b. BGRA data is stored as RGBA (cf. glTexImage2D with format GL_BGRA)
            bool const bgra = (iDataFormat == texture_data_format::BGRA);
            auto const* const imageData = static_cast<std::uint8_t const*>(aImage.cpixels());
            for (std::uint32_t y = 0; y < partExtents.cy; ++y)
            {
                auto const* const srcRow = imageData +
                    (static_cast<std::size_t>(partOrigin.y + y) * imageExtents.cx + partOrigin.x) * 4;
                // vertical flip, inset by the bleed guard on both axes
                auto* const dstRow = data.data() +
                    static_cast<std::size_t>(bleedGuard + partExtents.cy - 1 - y) * iStorageSize.cx + bleedGuard;
                bool const discardBackgroundMatte = aImage.discard_background_matte();
                for (std::uint32_t x = 0; x < partExtents.cx; ++x)
                {
                    auto const alpha = srcRow[x * 4 + 3];
                    for (std::size_t c = 0; c < 4; ++c)
                    {
                        auto const dstC = (bgra && c != 3u ? 2u - c : c);
                        if (alpha != 0u || !discardBackgroundMatte)
                        {
                            auto const component = srcRow[x * 4 + c];
                            if constexpr (normalize)
                                dstRow[x][dstC] = component / 255.0f;
                            else
                                dstRow[x][dstC] = component;
                        }
                        else
                            dstRow[x][dstC] = 0;
                    }
                }
            }

            auto const format = to_vk_format(iDataFormat, kDataType);
            iImage = backend().create_image(iStorageSize.cx, iStorageSize.cy, format.format, format.texelSize,
                VK_SAMPLE_COUNT_1_BIT, mip_levels(sampling(), iStorageSize));
            iImage->sampler = sampler_state(sampling());
            backend().upload_image(*iImage, 0u, 0u, iStorageSize.cx, iStorageSize.cy, data.data(), static_cast<std::size_t>(iStorageSize.cx) * sizeof(value_type));
        }
    }

    template <typename T>
    vulkan_texture<T>::~vulkan_texture()
    {
        unbind();

        if (vulkan_graphics_backend::instance() != nullptr)
        {
            backend().release_render_target(iImage.get());
            backend().destroy_image(iDepthStencil);
            backend().destroy_image(iImage);
        }

        TargetDestroying.trigger();

        service<i_rendering_engine>().remove_target(*this);
    }

    template <typename T>
    texture_id vulkan_texture<T>::id() const
    {
        return iId;
    }

    template <typename T>
    string const& vulkan_texture<T>::uri() const
    {
        return iUri;
    }

    template <typename T>
    rect const& vulkan_texture<T>::part() const
    {
        return iPart;
    }

    template <typename T>
    texture_type vulkan_texture<T>::type() const
    {
        return texture_type::Texture;
    }

    template <typename T>
    bool vulkan_texture<T>::is_render_target() const
    {
        return iDepthStencil != nullptr;
    }

    template <typename T>
    const i_render_target& vulkan_texture<T>::as_render_target() const
    {
        return *this;
    }

    template <typename T>
    i_render_target& vulkan_texture<T>::as_render_target()
    {
        return *this;
    }

    template <typename T>
    const i_sub_texture& vulkan_texture<T>::as_sub_texture() const
    {
        throw not_sub_texture();
    }

    template <typename T>
    dimension vulkan_texture<T>::dpi_scale_factor() const
    {
        return iDpiScaleFactor;
    }

    template <typename T>
    texture_sampling vulkan_texture<T>::sampling() const
    {
        switch (iSampling)
        {
        case texture_sampling::Multisample4x:
        case texture_sampling::Multisample8x:
        case texture_sampling::Multisample16x:
        case texture_sampling::Multisample32x:
            return texture_sampling::Multisample;
        default:
            return iSampling;
        }
    }

    template <typename T>
    std::uint32_t vulkan_texture<T>::samples() const
    {
        switch (iSampling)
        {
        case texture_sampling::Multisample:
        case texture_sampling::Multisample4x:
            return 4u;
        case texture_sampling::Multisample8x:
            return 8u;
        case texture_sampling::Multisample16x:
            return 16u;
        case texture_sampling::Multisample32x:
            return 32u;
        default:
            return 1u;
        }
    }

    template <typename T>
    texture_data_format vulkan_texture<T>::data_format() const
    {
        return iDataFormat;
    }

    template <typename T>
    texture_data_type vulkan_texture<T>::data_type() const
    {
        return kDataType;
    }

    template <typename T>
    bool vulkan_texture<T>::is_empty() const
    {
        return false;
    }

    template <typename T>
    size vulkan_texture<T>::extents() const
    {
        return iSize;
    }

    template <typename T>
    size vulkan_texture<T>::storage_extents() const
    {
        return iStorageSize;
    }

    template <typename T>
    dimension vulkan_texture<T>::bleed_guard() const
    {
        return iBleedGuard.value_or(0.0);
    }

    template <typename T>
    void vulkan_texture<T>::set_bleed_guard(i_optional<dimension> const& aWidth)
    {
        if (aWidth.has_value())
        {
            if (aWidth.value() <= MaxBleedGuardWidth)
                iBleedGuard = aWidth.value();
            else
                throw std::logic_error("neogfx::vulkan_texture::set_bleed_guard: Unsupported bleed guard width");
        }
        else
            iBleedGuard.reset();

        iUvCalculator.reset();
    }

    template <typename T>
    uv_calculator const& vulkan_texture<T>::uv_calculator(optional_aabb_2df const& aPart) const
    {
        if (iUvCalculator)
        {
            if (aPart)
                iUvCalculator->offsetOrPart = *aPart;
            else if (std::holds_alternative<aabb_2df>(iUvCalculator->offsetOrPart))
                iUvCalculator->offsetOrPart = to_game_rect(viewport(), extents().cy).bottom_left().to_vec2().as<float>();
            return *iUvCalculator;
        }

        auto const& logicalRect = to_game_rect(viewport(), extents().cy);

        std::variant<vec2f, aabb_2df> offsetOrPart;
        if (aPart)
            offsetOrPart = *aPart;
        else
            offsetOrPart = logicalRect.bottom_left().to_vec2().as<float>();

        std::optional<float> yFlip;

        iUvCalculator.emplace(
            logicalRect.extents().to_vec2().as<float>(),
            offsetOrPart,
            1.0f / storage_extents().to_vec2().as<float>(),
            yFlip);

        return *iUvCalculator;
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const rect& aRect, void const* aPixelData, std::uint32_t aStride, std::uint32_t aPackAlignment)
    {
        set_pixels(aRect, aPixelData, iDataFormat, aStride, aPackAlignment);
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const rect& aRect, void const* aPixelData, texture_data_format aDataFormat, std::uint32_t aStride, std::uint32_t aPackAlignment)
    {
        if (sampling() == texture_sampling::Multisample)
            throw unsupported_sampling_type_for_function();
        auto const adjustedRect = aRect + (sampling() != texture_sampling::Data ? point{ bleed_guard(), bleed_guard() } : point{ 0.0, 0.0 });
        auto const x = static_cast<std::uint32_t>(adjustedRect.x);
        auto const y = static_cast<std::uint32_t>(adjustedRect.y);
        auto const width = static_cast<std::uint32_t>(adjustedRect.cx);
        auto const height = static_cast<std::uint32_t>(adjustedRect.cy);
        // the data as glTextureSubImage2D reads it (GL_UNPACK_ROW_LENGTH and GL_UNPACK_ALIGNMENT): its format's components of
        // this texture's data type
        std::size_t const componentSize = (kDataType == texture_data_type::Float ? sizeof(float) : sizeof(std::uint8_t));
        std::size_t const sourceComponents = component_count(aDataFormat);
        std::size_t const sourceTexelSize = sourceComponents * componentSize;
        std::size_t const rowLength = (aStride != 0u ? aStride : width);
        std::size_t const alignment = std::max<std::size_t>(aPackAlignment, 1u);
        std::size_t const rowPitch = (rowLength * sourceTexelSize + alignment - 1u) / alignment * alignment;
        std::size_t const imageComponents = component_count(iDataFormat);
        if (sourceComponents == imageComponents && aDataFormat != texture_data_format::BGRA)
            backend().upload_image(*iImage, x, y, width, height, aPixelData, rowPitch);
        else
        {
            // converted to this texture's components (n.b. as OpenGL: BGRA swizzled; red expanded to (r, 0, 0, 1); red of RGBA)
            thread_local std::vector<std::uint8_t> converted;
            std::size_t const imageTexelSize = imageComponents * componentSize;
            converted.assign(static_cast<std::size_t>(width) * height * imageTexelSize, 0u);
            auto const* source = static_cast<std::uint8_t const*>(aPixelData);
            for (std::uint32_t row = 0u; row < height; ++row)
                for (std::uint32_t column = 0u; column < width; ++column)
                {
                    auto const* s = source + row * rowPitch + column * sourceTexelSize;
                    auto* d = converted.data() + (static_cast<std::size_t>(row) * width + column) * imageTexelSize;
                    for (std::size_t c = 0u; c < imageComponents; ++c)
                    {
                        std::size_t sourceComponent = c;
                        if (aDataFormat == texture_data_format::BGRA && c != 3u)
                            sourceComponent = 2u - c;
                        if (sourceComponent < sourceComponents)
                            std::memcpy(d + c * componentSize, s + sourceComponent * componentSize, componentSize);
                        else if (c == 3u)
                        {
                            if (componentSize == sizeof(float))
                            {
                                float const one = 1.0f;
                                std::memcpy(d + c * componentSize, &one, sizeof(one));
                            }
                            else
                                d[c] = 0xFFu;
                        }
                    }
                }
            backend().upload_image(*iImage, x, y, width, height, converted.data(), static_cast<std::size_t>(width) * imageTexelSize);
        }
        iPixelData.clear();
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const i_image& aImage)
    {
        set_pixels(aImage, rect{ point{}, aImage.extents() });
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const i_image& aImage, const rect& aImagePart)
    {
        size_u32 const imageExtents = aImage.extents();
        point_u32 const imagePartOrigin = aImagePart.position();
        size_u32 const imagePartExtents = aImagePart.extents();
        bool const discardBackgroundMatte = aImage.discard_background_matte();
        switch (aImage.color_format())
        {
        case color_format::RGBA8:
            {
                if (imagePartOrigin.x + imagePartExtents.cx > imageExtents.cx ||
                    imagePartOrigin.y + imagePartExtents.cy > imageExtents.cy)
                    throw std::logic_error("neogfx::vulkan_texture::set_pixels: image part out of range");

                auto const* const imageData = static_cast<std::uint8_t const*>(aImage.cpixels());
                thread_local std::vector<std::uint8_t> data;
                data.assign(static_cast<std::size_t>(imagePartExtents.cx) * imagePartExtents.cy * 4, 0u);

                for (std::size_t y = 0; y < imagePartExtents.cy; ++y)
                {
                    auto const* const srcRow = imageData +
                        ((y + imagePartOrigin.y) * static_cast<std::size_t>(imageExtents.cx) + imagePartOrigin.x) * 4;
                    auto* const dstRow = data.data() +
                        (imagePartExtents.cy - 1 - y) * static_cast<std::size_t>(imagePartExtents.cx) * 4;
                    for (std::size_t x = 0; x < imagePartExtents.cx; ++x)
                    {
                        if (srcRow[x * 4 + 3] != 0u || !discardBackgroundMatte)
                            std::copy_n(&srcRow[x * 4], 4, &dstRow[x * 4]);
                    }
                }
                set_pixels(rect{ point{}, imagePartExtents }, &data[0]);
            }
            break;
        default:
            throw std::logic_error("neogfx::vulkan_texture::set_pixels: unsupported color format");
        }
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const color& aColor)
    {
        backend().clear_image(*iImage, { aColor.red<float>(), aColor.green<float>(), aColor.blue<float>(), aColor.alpha<float>() });
        iPixelData.clear();
    }

    template <typename T>
    void vulkan_texture<T>::set_pixel(const point& aPosition, const color& aColor)
    {
        avec4u8 pixel{ aColor.red(), aColor.green(), aColor.blue(), aColor.alpha() };
        set_pixels(rect{ aPosition, size{1.0, 1.0} }, &pixel);
    }

    template <typename T>
    color vulkan_texture<T>::get_pixel(const point& aPosition) const
    {
        switch (sampling())
        {
        case texture_sampling::Normal:
        case texture_sampling::Nearest:
        case texture_sampling::Data:
            return read_pixel(aPosition);
        default:
            throw unsupported_sampling_type_for_function();
        }
    }

    template <typename T>
    void* vulkan_texture<T>::handle() const
    {
        return iImage.get();
    }

    template <typename T>
    bool vulkan_texture<T>::is_resident() const
    {
        return true;
    }

    template <typename T>
    dimension vulkan_texture<T>::horizontal_dpi() const
    {
        return dpi_scale_factor() * STANDARD_DPI_PPI;
    }

    template <typename T>
    dimension vulkan_texture<T>::vertical_dpi() const
    {
        return dpi_scale_factor() * STANDARD_DPI_PPI;
    }

    template <typename T>
    dimension vulkan_texture<T>::ppi() const
    {
        return size{ horizontal_dpi(), vertical_dpi() }.magnitude() / std::sqrt(2.0);
    }

    template <typename T>
    bool vulkan_texture<T>::metrics_available() const
    {
        return true;
    }

    template <typename T>
    dimension vulkan_texture<T>::em_size() const
    {
        return 0.0;
    }

    template <typename T>
    std::unique_ptr<i_rendering_context> vulkan_texture<T>::create_rendering_context(blending_mode aBlendingMode) const
    {
        // n.b. the rendering context shared by the native backends (see i_graphics_backend)
        return std::unique_ptr<i_rendering_context>(new opengl_rendering_context{ *this, aBlendingMode });
    }

    template <typename T>
    void vulkan_texture<T>::bind() const
    {
        if (iBoundTextureUnit.has_value())
        {
            bind(iBoundTextureUnit.value());
            return;
        }
        for (auto const& binding : vulkan_texture_bindings().unbound)
            if (static_cast<reserved_texture_unit>(binding.first()) > reserved_texture_unit::RESERVED_LAST)
            {
                bind(binding.first());
                return;
            }
        throw std::logic_error("neogfx::vulkan_texture::bind: texture bindings pool exhausted");
    }

    template <typename T>
    void vulkan_texture<T>::bind(std::uint32_t aTextureUnit) const
    {
        if (iBoundTextureUnit.has_value())
        {
            if (iBoundTextureUnit.value() == aTextureUnit)
            {
                auto existingPoolEntry = vulkan_texture_bindings().bound.find(iBoundTextureUnit.value());
                if (existingPoolEntry != vulkan_texture_bindings().bound.end())
                {
                    if (existingPoolEntry->second() == this)
                    {
                        do_bind(aTextureUnit);
                        return;
                    }
                    existingPoolEntry->second()->unbind();
                }
            }
            else
                unbind();
        }
        auto const previousTexture = do_bind(aTextureUnit);
        iBoundTextureUnit = aTextureUnit;
        iPreviouslyBoundTexture = previousTexture;
        auto existingPoolEntry = vulkan_texture_bindings().unbound.find(iBoundTextureUnit.value());
        if (existingPoolEntry != vulkan_texture_bindings().unbound.end())
            vulkan_texture_bindings().unbound.erase(existingPoolEntry);
        vulkan_texture_bindings().bound.emplace(iBoundTextureUnit.value(), this);
    }

    template <typename T>
    vulkan_image const* vulkan_texture<T>::do_bind(std::uint32_t aTextureUnit) const
    {
        auto const previousTexture = backend().bound_texture(aTextureUnit);
        backend().bind_texture(aTextureUnit, iImage.get());
        return previousTexture;
    }

    template <typename T>
    void vulkan_texture<T>::unbind() const
    {
        if (!iBoundTextureUnit.has_value())
            return;
        do_unbind();
    }

    template <typename T>
    void vulkan_texture<T>::do_unbind() const
    {
        if (vulkan_graphics_backend::instance() != nullptr)
            backend().bind_texture(iBoundTextureUnit.value(), iPreviouslyBoundTexture);
        auto existingPoolEntry = vulkan_texture_bindings().bound.find(iBoundTextureUnit.value());
        if (existingPoolEntry != vulkan_texture_bindings().bound.end() && existingPoolEntry->second() == this)
            vulkan_texture_bindings().bound.erase(existingPoolEntry);
        vulkan_texture_bindings().unbound.emplace(iBoundTextureUnit.value(), nullptr);
        iBoundTextureUnit = std::nullopt;
        iPreviouslyBoundTexture = nullptr;
    }

    template <typename T>
    intptr_t vulkan_texture<T>::native_handle() const
    {
        return reinterpret_cast<intptr_t>(handle());
    }

    template <typename T>
    i_texture& vulkan_texture<T>::native_texture() const
    {
        return const_cast<vulkan_texture<T>&>(*this);
    }

    template <typename T>
    render_target_type vulkan_texture<T>::target_type() const
    {
        return render_target_type::Texture;
    }

    template <typename T>
    void* vulkan_texture<T>::target_handle() const
    {
        return handle();
    }

    template <typename T>
    void* vulkan_texture<T>::target_device_handle() const
    {
        return nullptr;
    }

    template <typename T>
    pixel_format_t vulkan_texture<T>::pixel_format() const
    {
        return 0;
    }

    template <typename T>
    const i_texture& vulkan_texture<T>::target_texture() const
    {
        return *this;
    }

    template <typename T>
    point vulkan_texture<T>::target_origin() const
    {
        return point{ bleed_guard() };
    }

    template <typename T>
    size vulkan_texture<T>::target_extents() const
    {
        return extents();
    }

    template <typename T>
    neogfx::logical_coordinate_system vulkan_texture<T>::logical_coordinate_system() const
    {
        return iLogicalCoordinateSystem;
    }

    template <typename T>
    void vulkan_texture<T>::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const
    {
        iLogicalCoordinateSystem = aSystem;
        if (aSystem != neogfx::logical_coordinate_system::Specified)
            iLogicalCoordinates.reset();

        iUvCalculator.reset();
    }

    template <typename T>
    logical_coordinates vulkan_texture<T>::logical_coordinates() const
    {
        if (iLogicalCoordinates.has_value())
            return iLogicalCoordinates.value();

        switch (iLogicalCoordinateSystem)
        {
        case neogfx::logical_coordinate_system::AutomaticGui:
            return neogfx::logical_coordinates{
                viewport().bottom_left().as<scalar>().to_vec2(),
                viewport().top_right().as<scalar>().to_vec2() };
        case neogfx::logical_coordinate_system::AutomaticGame:
            return neogfx::logical_coordinates{
                to_game_rect(viewport(), target_extents().cy).bottom_left().as<scalar>().to_vec2(),
                to_game_rect(viewport(), target_extents().cy).top_right().as<scalar>().to_vec2() };
        }
        throw logical_coordinates_not_specified();
    }

    template <typename T>
    void vulkan_texture<T>::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) const
    {
        iLogicalCoordinates = aCoordinates;

        iUvCalculator.reset();
    }

    template <typename T>
    void vulkan_texture<T>::set_default_viewport() const
    {
        native_texture::set_default_viewport();

        iUvCalculator.reset();
    }

    template <typename T>
    void vulkan_texture<T>::set_viewport(const neogfx::viewport& aViewport) const
    {
        native_texture::set_viewport(aViewport);

        iUvCalculator.reset();
    }

    template <typename T>
    viewport vulkan_texture<T>::apply_viewport() const
    {
        auto const currentViewport = backend().viewport();
        auto previousViewport = to_gui_rect(game_rect{ currentViewport.position(), currentViewport.extents() }, extents().cy);

        auto const ourViewport = to_game_rect(viewport(), extents().cy).as<std::int32_t>();
        backend().set_viewport(ourViewport.x, ourViewport.y, ourViewport.cx, ourViewport.cy);

        return neogfx::viewport{ previousViewport };
    }

    template <typename T>
    void vulkan_texture<T>::activate_target() const
    {
        bool alreadyActive = target_active();
        if (!alreadyActive)
        {
            TargetActivating();
            service<i_rendering_engine>().activate_context(*this);
        }

        backend().texture_barrier();

        if (iDepthStencil == nullptr)
        {
            // n.b. as opengl_texture's frame buffer creation
            backend().enable_multisample(true);
            backend().enable_blending(true);
            backend().enable_depth_test(false);
            backend().set_depth_compare(VK_COMPARE_OP_LESS_OR_EQUAL);
            iDepthStencil = backend().create_depth_stencil_image(iImage->width, iImage->height, iImage->samples);
        }

        backend().set_render_target(iImage.get(), iDepthStencil.get());

        apply_viewport();

        iPixelData.clear();

        if (!alreadyActive)
            TargetActivated();
    }

    template <typename T>
    bool vulkan_texture<T>::target_active() const
    {
        return service<i_rendering_engine>().active_target() == this;
    }

    template <typename T>
    void vulkan_texture<T>::deactivate_target() const
    {
        if (target_active())
        {
            TargetDeactivating();

            backend().release_render_target(iImage.get());

            service<i_rendering_engine>().deactivate_context();

            if (!target_in_use())
                unbind();

            TargetDeactivated();

            return;
        }
        throw not_active();
    }

    template <typename T>
    bool vulkan_texture<T>::target_in_use() const
    {
        return iTargetUseCount != 0u;
    }

    template <typename T>
    void vulkan_texture<T>::target_add_ref() const
    {
        ++iTargetUseCount;
    }

    template <typename T>
    void vulkan_texture<T>::target_release() const
    {
        --iTargetUseCount;
    }

    template <typename T>
    color_space vulkan_texture<T>::color_space() const
    {
        return iColorSpace;
    }

    template <typename T>
    color vulkan_texture<T>::read_pixel(const point& aPosition, bool aCreateCache) const
    {
        if (sampling() != neogfx::texture_sampling::Multisample)
        {
            if (aPosition.x < 0.0 || aPosition.y < 0.0 || aPosition.x >= extents().cx || aPosition.y >= extents().cy)
                return color{};
            if (kDataType != texture_data_type::UnsignedByte)
                throw std::logic_error("neogfx::vulkan_texture::read_pixel: data type not yet implemented");
            value_type pixel;
            basic_point<std::int32_t> pos{ aPosition };
            pos += basic_point<std::int32_t>{ static_cast<std::int32_t>(bleed_guard()) };
            if (aCreateCache)
            {
                if (iPixelData.empty())
                {
                    iPixelData.resize(static_cast<std::size_t>(storage_extents().cx) * static_cast<std::size_t>(storage_extents().cy));
                    backend().read_image(*iImage, 0u, 0u, iImage->width, iImage->height, iPixelData.data());
                }
                pixel = iPixelData[static_cast<std::size_t>(pos.y * storage_extents().cx + pos.x)];
            }
            else
            {
                // n.b. as opengl_texture (glGetTextureSubImage at the position given)
                backend().read_image(*iImage, static_cast<std::uint32_t>(aPosition.x), static_cast<std::uint32_t>(aPosition.y), 1u, 1u, &pixel);
            }
            if constexpr (std::is_same_v<value_type, avec4u8> || std::is_same_v<value_type, std::array<float, 4>>)
            {
                // n.b. as opengl_texture: the texel read in the data format (BGRA data is stored as RGBA)
                switch (data_format())
                {
                case texture_data_format::RGBA:
                default:
                    return color{ pixel[2], pixel[1], pixel[0], pixel[3] };
                case texture_data_format::BGRA:
                    return color{ pixel[2], pixel[1], pixel[0], pixel[3] };
                }
            }
            else
                return color{ pixel, pixel, pixel, pixel };
        }
        else
            throw std::logic_error("neogfx::vulkan_texture::read_pixel: not yet implemented for multisample render targets");
    }

    template <typename T>
    vulkan_graphics_backend& vulkan_texture<T>::backend() const
    {
        return *vulkan_graphics_backend::instance();
    }

    template class vulkan_texture<std::uint8_t>;
    template class vulkan_texture<float>;
    template class vulkan_texture<avec4u8>;
    template class vulkan_texture<std::array<float, 4>>;
}
