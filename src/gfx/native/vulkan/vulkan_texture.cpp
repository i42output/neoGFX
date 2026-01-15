// vulkan_texture.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include "vulkan_error.hpp"
#include "vulkan_texture.hpp"

namespace neogfx
{
    namespace
    {
        inline std::tuple<vk::Format> to_vk_enums(texture_data_format aDataFormat, texture_data_type aDataType)
        {
            switch (aDataFormat)
            {
            case texture_data_format::RGBA:
            case texture_data_format::SubPixel:
                switch (aDataType)
                {
                case texture_data_type::UnsignedByte:
                    return std::make_tuple(vk::Format::eR8G8B8A8Uint);
                case texture_data_type::Float:
                    return std::make_tuple(vk::Format::eR32G32B32A32Sfloat);
                default:
                    throw std::logic_error("neogfx::to_vk_enums: bad data type");
                }
            case texture_data_format::Red:
                switch (aDataType)
                {
                case texture_data_type::UnsignedByte:
                    return std::make_tuple(vk::Format::eR8Uint);
                case texture_data_type::Float:
                    return std::make_tuple(vk::Format::eR32Sfloat);
                default:
                    throw std::logic_error("neogfx::to_vk_enums: bad data type");
                }
            default:
                throw std::logic_error("neogfx::to_vk_enums: bad data format");
            }
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
        iStorageSize{ [this, aSampling]() -> size_u32
            {
                switch (aSampling)
                {
                case texture_sampling::Data:
                    return decltype(iStorageSize){ iSize };
                case texture_sampling::NormalMipmap:
                    return decltype(iStorageSize){ size{ std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0) } };
                default:
                    return decltype(iStorageSize){ ((iSize.cx + 2 - 1) / 16 + 1) * 16, ((iSize.cy + 2 - 1) / 16 + 1) * 16 };
                }
            }() },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGame }
    {
        try
        {
            TODO;
            switch(sampling())
            {
            case texture_sampling::Normal:
                TODO;
                break;
            case texture_sampling::NormalMipmap:
                TODO;
                break;
            case texture_sampling::Nearest:
            case texture_sampling::Scaled:
                TODO;
                break;
            case texture_sampling::Data:
                TODO;
                break;
            }
            if (sampling() != texture_sampling::Multisample)
            {
                thread_local std::vector<value_type> data;
                data.clear();
                data.resize(iStorageSize.cx * 4 * iStorageSize.cy);
                if (aColor != std::nullopt)
                {
                    if constexpr (std::is_same_v<value_type, avec4u8>)
                        for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                            for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                                data[y * iStorageSize.cx + x + 0] = 
                                    value_type{
                                        aColor->red(),
                                        aColor->green(),
                                        aColor->blue(),
                                        aColor->alpha()
                                    };
                    else if constexpr (std::is_same_v<value_type, std::array<float, 4>>)
                        for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                            for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                                data[y * iStorageSize.cx + x + 0] = 
                                    value_type{
                                        aColor->red<float>(),
                                        aColor->green<float>(),
                                        aColor->blue<float>(),
                                        aColor->alpha<float>()
                                    };
                }
                TODO;
                if (sampling() == texture_sampling::NormalMipmap)
                {
                    TODO;
                }
            }
            else
            {
                if (aColor != std::nullopt)
                    throw multisample_texture_initialization_unsupported();
                TODO;
            }
            TODO;
        }
        catch (...)
        {
            TODO;
            throw;
        }
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
        iStorageSize{ [this, &aImage]() -> size_u32
            {
                switch (aImage.sampling())
                {
                case texture_sampling::Data:
                    return decltype(iStorageSize){ iSize };
                case texture_sampling::NormalMipmap:
                    return decltype(iStorageSize){ size{ std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0) } };
                default:
                    return decltype(iStorageSize){ ((iSize.cx + 2 - 1) / 16 + 1) * 16, ((iSize.cy + 2 - 1) / 16 + 1) * 16 };
                }
            }() },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGame }
    {
        try
        {
            TODO;
            switch(sampling())
            {
            case texture_sampling::Normal:
                TODO;
                break;
            case texture_sampling::NormalMipmap:
                TODO;
                break;
            case texture_sampling::Nearest:
                TODO;
                break;
            case texture_sampling::Data:
                TODO;
                break;
            case texture_sampling::Multisample:
                throw multisample_texture_initialization_unsupported();
                break;
            }
            switch (aImage.color_format())
            {
            case color_format::RGBA8:
                {
                    size_u32 const imageExtents = aImage.extents();
                    point_u32 const imagePartOrigin = aImagePart.position();
                    size_u32 const imagePartExtents = aImagePart.extents();
                    thread_local std::vector<value_type> data;
                    data.clear();
                    data.resize(iStorageSize.cx * 4 * iStorageSize.cy);
                    if constexpr (std::is_same_v<value_type, avec4u8>)
                    {
                        const std::uint8_t* imageData = static_cast<const std::uint8_t*>(aImage.cpixels());
                        for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                            for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                                for (std::size_t c = 0; c < 4; ++c)
                                    data[(iSize.cy + 1 - y) * iStorageSize.cx + x][c] = imageData[(y + imagePartOrigin.y - 1) * imageExtents.cx * 4 + (imagePartOrigin.x + x - 1) * 4 + c];
                    }
                    else if constexpr (std::is_same_v<value_type, std::array<float, 4>>)
                    {
                        const std::uint8_t* imageData = static_cast<const std::uint8_t*>(aImage.cpixels());
                        for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                            for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                                for (std::size_t c = 0; c < 4; ++c)
                                    data[(iSize.cy + 1 - y) * iStorageSize.cx + x][c] = imageData[(y + imagePartOrigin.y - 1) * imageExtents.cx * 4 + (imagePartOrigin.x + x - 1) * 4 + c] / 255.0f;
                    }
                    TODO;
                    if (sampling() == texture_sampling::NormalMipmap)
                    {
                        TODO;
                    }
                }
                break;
            default:
                throw unsupported_color_format();
                break;
            }
            TODO;
        }
        catch (...)
        {
            TODO;
            throw;
        }
    }

    template <typename T>
    vulkan_texture<T>::~vulkan_texture()
    {
        try
        {
            TODO;

            TargetDestroying.trigger();

            service<i_rendering_engine>().remove_target(*this);
        }
        catch (...)
        {
        }
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
        TODO;
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
    void vulkan_texture<T>::set_pixels(const rect& aRect, void const* aPixelData, std::uint32_t aStride, std::uint32_t aPackAlignment)
    {
        set_pixels(aRect, aPixelData, data_format(), aStride, aPackAlignment);
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const rect& aRect, void const* aPixelData, texture_data_format aDataFormat, std::uint32_t aStride, std::uint32_t aPackAlignment)
    {
        auto const adjustedRect = aRect + (sampling() != texture_sampling::Data ? point{ 1.0, 1.0 } : point{ 0.0, 0.0 });
        if (sampling() != texture_sampling::Multisample)
        {
            TODO;
        }
        else
            throw unsupported_sampling_type_for_function();
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const i_image& aImage)
    {
        set_pixels(rect{ point{}, aImage.extents() }, aImage.cpixels());
    }

    template <typename T>
    void vulkan_texture<T>::set_pixels(const i_image& aImage, const rect& aImagePart)
    {
        size_u32 const imageExtents = aImage.extents();
        point_u32 const imagePartOrigin = aImagePart.position();
        size_u32 const imagePartExtents = aImagePart.extents();
        switch (aImage.color_format())
        {
        case color_format::RGBA8:
            {
                const std::uint8_t* imageData = static_cast<const std::uint8_t*>(aImage.cpixels());
                thread_local std::vector<std::uint8_t> data;
                data.clear();
                data.resize(imagePartExtents.cx * 4 * imagePartExtents.cy);
                for (std::size_t y = 0; y < imagePartExtents.cy; ++y)
                    for (std::size_t x = 0; x < imagePartExtents.cx; ++x)
                        for (std::size_t c = 0; c < 4; ++c)
                            data[(imagePartExtents.cy - 1 - y) * imagePartExtents.cx * 4 + x * 4 + c] = imageData[(y + imagePartOrigin.y) * imageExtents.cx * 4 + (x + imagePartOrigin.x) * 4 + c];
                set_pixels(rect{ point{}, imagePartExtents }, &data[0]);
            }
            break;
        }
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
        return static_cast<VkImage>(iImage);
    }

    template <typename T>
    bool vulkan_texture<T>::is_resident() const
    {
        TODO;
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
        TODO;
    }

    template <typename T>
    graphics_operation::i_queue& vulkan_texture<T>::graphics_operation_queue() const
    {
        TODO;
    }

    template <typename T>
    void vulkan_texture<T>::bind(std::uint32_t aTextureUnit) const
    {
        TODO;
    }

    template <typename T>
    void vulkan_texture<T>::unbind() const
    {
        TODO;
    }

    template <typename T>
    intptr_t vulkan_texture<T>::native_handle() const
    {
        return reinterpret_cast<intptr_t>(handle());
    }

    template <typename T>
    i_texture& vulkan_texture<T>::native_texture() const
    {
        return const_cast<vulkan_texture<T>&>(*this); // todo: not happy with this cast
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
        return {};
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
    void vulkan_texture<T>::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    template <typename T>
    logical_coordinates vulkan_texture<T>::logical_coordinates() const
    {
        if (iLogicalCoordinates != std::nullopt)
            return *iLogicalCoordinates;
        neogfx::logical_coordinates result;
        switch (iLogicalCoordinateSystem)
        {
        case neogfx::logical_coordinate_system::Specified:
            throw logical_coordinates_not_specified();
            break;
        case neogfx::logical_coordinate_system::AutomaticGui:
            result.bottomLeft = vec2{ 0.0, extents().cy };
            result.topRight = vec2{ extents().cx, 0.0 };
            break;
        case neogfx::logical_coordinate_system::AutomaticGame:
            result.bottomLeft = vec2{ 0.0, 0.0 };
            result.topRight = vec2{ extents().cx, extents().cy };
            break;
        }
        return result;
    }

    template <typename T>
    void vulkan_texture<T>::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    template <typename T>
    rect_i32 vulkan_texture<T>::viewport() const
    {
        TODO;
    }

    template <typename T>
    rect_i32 vulkan_texture<T>::set_viewport(const rect_i32& aViewport) const
    {
        TODO;
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
        TODO;
        set_viewport(rect_i32{ point_i32{ 1, 1 }, extents().as<std::int32_t>() });
        TODO;
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
            service<i_rendering_engine>().deactivate_context();
            TargetDeactivated();
            return;
        }
        throw not_active();
    }

    template <typename T>
    color_space vulkan_texture<T>::color_space() const
    {
        return iColorSpace;
    }

    template <typename T>
    color vulkan_texture<T>::read_pixel(const point& aPosition) const
    {
        if (sampling() != neogfx::texture_sampling::Multisample)
        {
            scoped_render_target srt{ *this };
            avec4u8 pixel;
            TODO;
            return color{ pixel[0], pixel[1], pixel[2], pixel[3] };
        }
        else
            throw std::logic_error("neogfx::vulkan_texture::read_pixel: not yet implemented for multisample render targets");
    }

    template class vulkan_texture<std::uint8_t>;
    template class vulkan_texture<float>;
    template class vulkan_texture<avec4u8>;
    template class vulkan_texture<std::array<float, 4>>;
}
