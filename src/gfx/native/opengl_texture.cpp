// opengl_texture.cpp
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
#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include "opengl_error.hpp"
#include "opengl_helpers.hpp"
#include "opengl_rendering_context.hpp"
#include "opengl_texture.hpp"

namespace neogfx
{
    namespace
    {
        inline std::tuple<GLenum, GLenum, GLenum> to_gl_enum(texture_data_format aDataFormat, texture_data_type aDataType)
        {
            switch (aDataFormat)
            {
            case texture_data_format::RGBA:
            case texture_data_format::SubPixel:
                switch (aDataType)
                {
                case texture_data_type::UnsignedByte:
                    return std::make_tuple(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
                case texture_data_type::Float:
                    return std::make_tuple(GL_RGBA32F, GL_RGBA, GL_FLOAT);
                default:
                    throw std::logic_error("neogfx::to_gl_enum: bad data type");
                }
            case texture_data_format::Red:
                switch (aDataType)
                {
                case texture_data_type::UnsignedByte:
                    return std::make_tuple(GL_R8, GL_RED, GL_UNSIGNED_BYTE);
                case texture_data_type::Float:
                    return std::make_tuple(GL_R32F, GL_RED, GL_FLOAT);
                default:
                    throw std::logic_error("neogfx::to_gl_enum: bad data type");
                }
            default:
                throw std::logic_error("neogfx::to_gl_enum: bad data format");
            }
        }

        inline GLenum to_gl_enum(texture_sampling aSampling)
        {
            switch (aSampling)
            {
            case texture_sampling::Multisample:
                return GL_TEXTURE_2D_MULTISAMPLE;
            case texture_sampling::Data:
                return GL_TEXTURE_RECTANGLE;
            default:
                return GL_TEXTURE_2D;
            }
        }

        inline GLenum to_gl_binding_enum(texture_sampling aSampling)
        {
            switch (aSampling)
            {
            case texture_sampling::Multisample:
                return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
            case texture_sampling::Data:
                return GL_TEXTURE_BINDING_RECTANGLE;
            default:
                return GL_TEXTURE_BINDING_2D;
            }
        }
    }

    template <typename T>
    opengl_texture<T>::opengl_texture(i_texture_manager& aManager, texture_id aId, const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat, neogfx::color_space aColorSpace, const optional_color& aColor) :
        iManager{ aManager },
        iId{ aId },
        iUri{ "neogfx::opengl_texture::internal" },
        iPart{ aExtents },
        iDpiScaleFactor{ aDpiScaleFactor },
        iColorSpace{ aColorSpace },
        iSampling{ aSampling },
        iDataFormat{ aDataFormat },
        iSize{ aExtents },
        iStorageSize{ aSampling != texture_sampling::NormalMipmap ?
            (aSampling != texture_sampling::Data ? decltype(iStorageSize){((iSize.cx + 2 - 1) / 16 + 1) * 16, ((iSize.cy + 2 - 1) / 16 + 1) * 16} : decltype(iStorageSize){iSize}) :
            decltype(iStorageSize){size{std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0)}} },
        iHandle{ 0 },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGame },
        iFrameBuffer{ 0 },
        iDepthStencilBuffer{ 0 }
    {
        try
        {
            glCheck(glGenTextures(1, &iHandle));
            GLint previousTexture = bind(1);
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
            switch(sampling())
            {
            case texture_sampling::Normal:
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                break;
            case texture_sampling::NormalMipmap:
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
                break;
            case texture_sampling::Nearest:
            case texture_sampling::Scaled:
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                break;
            case texture_sampling::Data:
                glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                break;
            }
            if (sampling() != texture_sampling::Multisample)
            {
                std::vector<value_type> data(iStorageSize.cx * 4 * iStorageSize.cy);
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
                glCheck(glTexImage2D(to_gl_enum(sampling()), 0, std::get<0>(to_gl_enum(iDataFormat, kDataType)), static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, std::get<1>(to_gl_enum(iDataFormat, kDataType)), std::get<2>(to_gl_enum(iDataFormat, kDataType)), data.empty() ? nullptr : &data[0]));
                if (sampling() == texture_sampling::NormalMipmap)
                {
                    glCheck(glGenerateMipmap(GL_TEXTURE_2D));
                }
            }
            else
            {
                if (aColor != std::nullopt)
                    throw multisample_texture_initialization_unsupported();
                glCheck(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples(), std::get<0>(to_gl_enum(iDataFormat, kDataType)), static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), true));
            }
            glCheck(glBindTexture(to_gl_enum(sampling()), static_cast<GLuint>(previousTexture)));
        }
        catch (...)
        {
            glCheck(glDeleteTextures(1, &iHandle));
            throw;
        }
    }

    template <typename T>
    opengl_texture<T>::opengl_texture(i_texture_manager& aManager, texture_id aId, const i_image& aImage, const rect& aImagePart, texture_data_format aDataFormat) :
        iManager{ aManager },
        iId{ aId },
        iUri{ aImage.uri() },
        iPart{ aImagePart },
        iDpiScaleFactor{ aImage.dpi_scale_factor() },
        iColorSpace{ aImage.color_space() },
        iSampling{ aImage.sampling() },
        iDataFormat{ aDataFormat },
        iSize{ aImagePart.extents() },
        iStorageSize{ aImage.sampling() != texture_sampling::NormalMipmap ? 
            (aImage.sampling() != texture_sampling::Data ? decltype(iStorageSize){((iSize.cx + 2 - 1) / 16 + 1) * 16, ((iSize.cy + 2 - 1) / 16 + 1) * 16} : decltype(iStorageSize){iSize}) :
            decltype(iStorageSize){size{std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0)}} },
        iHandle{ 0 },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGame },
        iFrameBuffer{ 0 },
        iDepthStencilBuffer{ 0 }
    {
        try
        {
            
            glCheck(glGenTextures(1, &iHandle));
            GLint previousTexture = bind(1);
            switch(sampling())
            {
            case texture_sampling::Normal:
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                break;
            case texture_sampling::NormalMipmap:
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
                break;
            case texture_sampling::Nearest:
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                break;
            case texture_sampling::Data:
                glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
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
                    std::vector<value_type> data(iStorageSize.cx * 4 * iStorageSize.cy);
                    if constexpr (std::is_same_v<value_type, avec4u8>)
                    {
                        const uint8_t* imageData = static_cast<const uint8_t*>(aImage.cpixels());
                        for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                            for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                                for (std::size_t c = 0; c < 4; ++c)
                                    data[(iSize.cy + 1 - y) * iStorageSize.cx + x][c] = imageData[(y + imagePartOrigin.y - 1) * imageExtents.cx * 4 + (imagePartOrigin.x + x - 1) * 4 + c];
                    }
                    else if constexpr (std::is_same_v<value_type, std::array<float, 4>>)
                    {
                        const uint8_t* imageData = static_cast<const uint8_t*>(aImage.cpixels());
                        for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                            for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                                for (std::size_t c = 0; c < 4; ++c)
                                    data[(iSize.cy + 1 - y) * iStorageSize.cx + x][c] = imageData[(y + imagePartOrigin.y - 1) * imageExtents.cx * 4 + (imagePartOrigin.x + x - 1) * 4 + c] / 255.0f;
                    }
                    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, std::get<0>(to_gl_enum(iDataFormat, kDataType)), static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, std::get<1>(to_gl_enum(iDataFormat, kDataType)), std::get<2>(to_gl_enum(iDataFormat, kDataType)), &data[0]));
                    if (sampling() == texture_sampling::NormalMipmap)
                    {
                        glCheck(glGenerateMipmap(GL_TEXTURE_2D));
                    }
                }
                break;
            default:
                throw unsupported_color_format();
                break;
            }
            glCheck(glBindTexture(to_gl_enum(sampling()), static_cast<GLuint>(previousTexture)));
        }
        catch (...)
        {
            glCheck(glDeleteTextures(1, &iHandle));
            throw;
        }
    }

    template <typename T>
    opengl_texture<T>::~opengl_texture()
    {
        if (iFrameBuffer != 0)
        {
            glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
            glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
        }
        glCheck(glDeleteTextures(1, &iHandle));
    }

    template <typename T>
    texture_id opengl_texture<T>::id() const
    {
        return iId;
    }

    template <typename T>
    string const& opengl_texture<T>::uri() const
    {
        return iUri;
    }

    template <typename T>
    rect const& opengl_texture<T>::part() const
    {
        return iPart;
    }

    template <typename T>
    texture_type opengl_texture<T>::type() const
    {
        return texture_type::Texture;
    }

    template <typename T>
    bool opengl_texture<T>::is_render_target() const
    {
        return iFrameBuffer != 0;
    }

    template <typename T>
    const i_render_target& opengl_texture<T>::as_render_target() const
    {
        return *this;
    }

    template <typename T>
    i_render_target& opengl_texture<T>::as_render_target()
    {
        return *this;
    }

    template <typename T>
    const i_sub_texture& opengl_texture<T>::as_sub_texture() const
    {
        throw not_sub_texture();
    }

    template <typename T>
    dimension opengl_texture<T>::dpi_scale_factor() const
    {
        return iDpiScaleFactor;
    }

    template <typename T>
    texture_sampling opengl_texture<T>::sampling() const
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
    uint32_t opengl_texture<T>::samples() const
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
    texture_data_format opengl_texture<T>::data_format() const
    {
        return iDataFormat;
    }

    template <typename T>
    texture_data_type opengl_texture<T>::data_type() const
    {
        return kDataType;
    }

    template <typename T>
    bool opengl_texture<T>::is_empty() const
    {
        return false;
    }

    template <typename T>
    size opengl_texture<T>::extents() const
    {
        return iSize;
    }

    template <typename T>
    size opengl_texture<T>::storage_extents() const
    {
        return iStorageSize;
    }

    template <typename T>
    void opengl_texture<T>::set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment)
    {
        auto const adjustedRect = aRect + (sampling() != texture_sampling::Data ? point{ 1.0, 1.0 } : point{ 0.0, 0.0 });
        if (sampling() != texture_sampling::Multisample)
        {
            GLint previousTexture = bind(1);
            GLint previousPackAlignment;
            glCheck(glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousPackAlignment))
            glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, aPackAlignment));
            glCheck(glTexSubImage2D(to_gl_enum(sampling()), 0,
                static_cast<GLint>(adjustedRect.x), static_cast<GLint>(adjustedRect.y), 
                static_cast<GLsizei>(adjustedRect.cx), static_cast<GLsizei>(adjustedRect.cy),
                std::get<1>(to_gl_enum(iDataFormat, kDataType)), std::get<2>(to_gl_enum(iDataFormat, kDataType)), aPixelData));
            if (sampling() == texture_sampling::NormalMipmap)
            {
                glCheck(glGenerateMipmap(to_gl_enum(sampling())));
            }
            glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, previousPackAlignment));
            glCheck(glBindTexture(to_gl_enum(sampling()), static_cast<GLuint>(previousTexture)));
        }
        else
            throw unsupported_sampling_type_for_function();
    }

    template <typename T>
    void opengl_texture<T>::set_pixels(const i_image& aImage)
    {
        set_pixels(rect{ point{}, aImage.extents() }, aImage.cpixels());
    }

    template <typename T>
    void opengl_texture<T>::set_pixels(const i_image& aImage, const rect& aImagePart)
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

    template <typename T>
    void opengl_texture<T>::set_pixel(const point& aPosition, const color& aColor)
    {
        avec4u8 pixel{ aColor.red(), aColor.green(), aColor.blue(), aColor.alpha() };
        set_pixels(rect{ aPosition, size{1.0, 1.0} }, &pixel);
    }

    template <typename T>
    color opengl_texture<T>::get_pixel(const point& aPosition) const
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
    void* opengl_texture<T>::handle() const
    {
        return reinterpret_cast<void*>(static_cast<intptr_t>(iHandle));
    }

    template <typename T>
    bool opengl_texture<T>::is_resident() const
    {
        GLboolean resident;
        glCheck(glAreTexturesResident(1, &iHandle, &resident));
        return resident == GL_TRUE;
    }

    template <typename T>
    dimension opengl_texture<T>::horizontal_dpi() const
    {
        return dpi_scale_factor() * 96.0;
    }

    template <typename T>
    dimension opengl_texture<T>::vertical_dpi() const
    {
        return dpi_scale_factor() * 96.0;
    }

    template <typename T>
    dimension opengl_texture<T>::ppi() const
    {
        return size{ horizontal_dpi(), vertical_dpi() }.magnitude() / std::sqrt(2.0);
    }

    template <typename T>
    bool opengl_texture<T>::metrics_available() const
    {
        return true;
    }

    template <typename T>
    dimension opengl_texture<T>::em_size() const
    {
        return 0.0;
    }

    template <typename T>
    std::unique_ptr<i_rendering_context> opengl_texture<T>::create_graphics_context(blending_mode aBlendingMode) const
    {
        return std::unique_ptr<i_rendering_context>(new opengl_rendering_context{ *this, aBlendingMode });
    }

    template <typename T>
    int32_t opengl_texture<T>::bind(const std::optional<uint32_t>& aTextureUnit) const
    {
        if (aTextureUnit != std::nullopt)
            glCheck(glActiveTexture(GL_TEXTURE0 + *aTextureUnit));
        GLint previousTexture = 0;
        glCheck(glGetIntegerv(to_gl_binding_enum(sampling()), &previousTexture));
        glCheck(glBindTexture(to_gl_enum(sampling()), static_cast<GLuint>(reinterpret_cast<std::intptr_t>(handle()))));
        return previousTexture;
    }

    template <typename T>
    intptr_t opengl_texture<T>::native_handle() const
    {
        return reinterpret_cast<intptr_t>(handle());
    }

    template <typename T>
    i_texture& opengl_texture<T>::native_texture() const
    {
        return const_cast<opengl_texture<T>&>(*this); // todo: not happy with this cast
    }

    template <typename T>
    render_target_type opengl_texture<T>::target_type() const
    {
        return render_target_type::Texture;
    }

    template <typename T>
    void* opengl_texture<T>::target_handle() const
    {
        return handle();
    }

    template <typename T>
    void* opengl_texture<T>::target_device_handle() const
    {
        return nullptr;
    }

    template <typename T>
    pixel_format_t opengl_texture<T>::pixel_format() const
    {
        return 0;
    }

    template <typename T>
    const i_texture& opengl_texture<T>::target_texture() const
    {
        return *this;
    }

    template <typename T>
    point opengl_texture<T>::target_origin() const
    {
        return {};
    }

    template <typename T>
    size opengl_texture<T>::target_extents() const
    {
        return extents();
    }

    template <typename T>
    neogfx::logical_coordinate_system opengl_texture<T>::logical_coordinate_system() const
    {
        return iLogicalCoordinateSystem;
    }

    template <typename T>
    void opengl_texture<T>::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    template <typename T>
    logical_coordinates opengl_texture<T>::logical_coordinates() const
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
    void opengl_texture<T>::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    template <typename T>
    rect_i32 opengl_texture<T>::viewport() const
    {
        GLint currentViewport[4];
        glCheck(glGetIntegerv(GL_VIEWPORT, currentViewport));
        return rect_i32{ point_i32{ currentViewport[0], currentViewport[1] }, size_i32{ currentViewport[2], currentViewport[3] } };
    }

    template <typename T>
    rect_i32 opengl_texture<T>::set_viewport(const rect_i32& aViewport) const
    {
        auto const oldViewport = viewport();
        glCheck(glViewport(aViewport.x, aViewport.y, static_cast<GLsizei>(aViewport.cx), static_cast<GLsizei>(aViewport.cy)));
        return oldViewport;
    }

    template <typename T>
    void opengl_texture<T>::activate_target() const
    {
        bool alreadyActive = target_active();
        if (!alreadyActive)
        {
            TargetActivating.trigger();
            service<i_rendering_engine>().activate_context(*this);
        }
        bind(10);
        if (iFrameBuffer == 0)
        {
            glCheck(glEnable(GL_MULTISAMPLE));
            glCheck(glEnable(GL_BLEND));
            glCheck(glEnable(GL_DEPTH_TEST));
            glCheck(glDepthFunc(GL_LEQUAL));
            glCheck(glGenFramebuffers(1, &iFrameBuffer));
            glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, iFrameBuffer));
            glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, to_gl_enum(sampling()), static_cast<GLuint>(reinterpret_cast<std::intptr_t>(handle())), 0));
            glCheck(glGenRenderbuffers(1, &iDepthStencilBuffer));
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
            if (sampling() != texture_sampling::Multisample)
            {
                glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(storage_extents().cx), static_cast<GLsizei>(storage_extents().cy)));
            }
            else
            {
                glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples(), GL_DEPTH24_STENCIL8, static_cast<GLsizei>(storage_extents().cx), static_cast<GLsizei>(storage_extents().cy)));
            }
            glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
            glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
            glCheck(glClear(GL_DEPTH_BUFFER_BIT));
        }
        else
        {
            GLint currentFramebuffer;
            glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFramebuffer));
            if (static_cast<decltype(iFrameBuffer)>(currentFramebuffer) != iFrameBuffer)
            {
                glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, iFrameBuffer));
            }
            GLint queryResult = 0;
            glCheck(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &queryResult));
            if (queryResult == GL_TEXTURE)
            {
                glCheck(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &queryResult));
            }
            else
                queryResult = 0;
            if (queryResult != static_cast<GLint>(reinterpret_cast<std::intptr_t>(handle())))
            {
                glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, to_gl_enum(sampling()), static_cast<GLuint>(reinterpret_cast<std::intptr_t>(handle())), 0));
            }
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
        }
        GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
            throw failed_to_create_framebuffer(glErrorString(status));
        set_viewport(rect_i32{ point_i32{ 1, 1 }, extents().as<int32_t>() });
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT1 };
        glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
        if (!alreadyActive)
            TargetActivated.trigger();
    }

    template <typename T>
    bool opengl_texture<T>::target_active() const
    {
        return service<i_rendering_engine>().active_target() == this;
    }

    template <typename T>
    void opengl_texture<T>::deactivate_target() const
    {
        if (target_active())
        {
            TargetDeactivating.trigger();
            service<i_rendering_engine>().deactivate_context();
            TargetDeactivated.trigger();
            return;
        }
        throw not_active();
    }

    template <typename T>
    color_space opengl_texture<T>::color_space() const
    {
        return iColorSpace;
    }

    template <typename T>
    color opengl_texture<T>::read_pixel(const point& aPosition) const
    {
        if (sampling() != neogfx::texture_sampling::Multisample)
        {
            scoped_render_target srt{ *this };
            avec4u8 pixel;
            basic_point<GLint> pos{ aPosition };
            glCheck(glReadPixels(pos.x + 1, pos.y + 1, 1, 1, std::get<1>(to_gl_enum(iDataFormat, kDataType)), std::get<2>(to_gl_enum(iDataFormat, kDataType)), &pixel));
            return color{ pixel[0], pixel[1], pixel[2], pixel[3] };
        }
        else
            throw std::logic_error("neogfx::opengl_texture::read_pixel: not yet implemented for multisample render targets");
    }

    template class opengl_texture<uint8_t>;
    template class opengl_texture<float>;
    template class opengl_texture<avec4u8>;
    template class opengl_texture<std::array<float, 4>>;
}