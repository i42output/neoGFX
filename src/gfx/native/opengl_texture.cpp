// opengl_texture.cpp
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
    }

    opengl_texture::opengl_texture(i_texture_manager& aManager, texture_id aId, const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat, texture_data_type aDataType, const optional_colour& aColour) :
        iManager{ aManager },
        iId{ aId },
        iDpiScaleFactor{ aDpiScaleFactor },
        iSampling{ aSampling },
        iDataFormat{ aDataFormat },
        iDataType{ aDataType },
        iSize{ aExtents },
        iStorageSize{ aSampling != texture_sampling::NormalMipmap ?
            (aSampling != texture_sampling::Data ? decltype(iStorageSize){((iSize.cx + 2 - 1) / 16 + 1) * 16, ((iSize.cy + 2 - 1) / 16 + 1) * 16} : decltype(iStorageSize){iSize}) :
            decltype(iStorageSize){size{std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0)}} },
        iHandle{ 0 },
        iUri{ "neogfx::opengl_texture::internal" },
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
                std::vector<uint8_t> data(iStorageSize.cx * 4 * iStorageSize.cy);
                if (aColour != std::nullopt)
                {
                    for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                        for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                        {
                            data[y * iStorageSize.cx * 4 + x * 4 + 0] = aColour->red();
                            data[y * iStorageSize.cx * 4 + x * 4 + 1] = aColour->green();
                            data[y * iStorageSize.cx * 4 + x * 4 + 2] = aColour->blue();
                            data[y * iStorageSize.cx * 4 + x * 4 + 3] = aColour->alpha();
                        }
                }
                glCheck(glTexImage2D(GL_TEXTURE_2D, 0, std::get<0>(to_gl_enum(iDataFormat, iDataType)), static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, std::get<1>(to_gl_enum(iDataFormat, iDataType)), std::get<2>(to_gl_enum(iDataFormat, iDataType)), data.empty() ? nullptr : &data[0]));
                if (sampling() == texture_sampling::NormalMipmap)
                {
                    glCheck(glGenerateMipmap(GL_TEXTURE_2D));
                }
            }
            else
            {
                if (aColour != std::nullopt)
                    throw multisample_texture_initialization_unsupported();
                glCheck(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples(), std::get<0>(to_gl_enum(iDataFormat, iDataType)), static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), true));
            }
            glCheck(glBindTexture(to_gl_enum(sampling()), static_cast<GLuint>(previousTexture)));
        }
        catch (...)
        {
            glCheck(glDeleteTextures(1, &iHandle));
            throw;
        }
    }

    opengl_texture::opengl_texture(i_texture_manager& aManager, texture_id aId, const i_image& aImage, texture_data_format aDataFormat, texture_data_type aDataType) :
        iManager{ aManager },
        iId{ aId },
        iDpiScaleFactor{ aImage.dpi_scale_factor() },
        iSampling{ aImage.sampling() },
        iDataFormat{ aDataFormat },
        iDataType{ aDataType },
        iSize{ aImage.extents() },
        iStorageSize{ aImage.sampling() != texture_sampling::NormalMipmap ? 
            (aImage.sampling() != texture_sampling::Data ? decltype(iStorageSize){((iSize.cx + 2 - 1) / 16 + 1) * 16, ((iSize.cy + 2 - 1) / 16 + 1) * 16} : decltype(iStorageSize){iSize}) :
            decltype(iStorageSize){size{std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0)}} },
        iHandle{ 0 },
        iUri{ aImage.uri() },
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
            switch (aImage.colour_format())
            {
            case colour_format::RGBA8:
                {
                    const uint8_t* imageData = static_cast<const uint8_t*>(aImage.cpixels());
                    std::vector<uint8_t> data(iStorageSize.cx * 4 * iStorageSize.cy);
                    for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
                        for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
                            for (std::size_t c = 0; c < 4; ++c)
                                data[(iSize.cy + 1 - y) * iStorageSize.cx * 4 + x * 4 + c] = imageData[(y - 1) * iSize.cx * 4 + (x - 1) * 4 + c];
                    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, std::get<0>(to_gl_enum(iDataFormat, iDataType)), static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, std::get<1>(to_gl_enum(iDataFormat, iDataType)), std::get<2>(to_gl_enum(iDataFormat, iDataType)), &data[0]));
                    if (sampling() == texture_sampling::NormalMipmap)
                    {
                        glCheck(glGenerateMipmap(GL_TEXTURE_2D));
                    }
                }
                break;
            default:
                throw unsupported_colour_format();
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

    opengl_texture::~opengl_texture()
    {
        if (iFrameBuffer != 0)
        {
            glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
            glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
        }
        glCheck(glDeleteTextures(1, &iHandle));
    }

    texture_id opengl_texture::id() const
    {
        return iId;
    }

    texture_type opengl_texture::type() const
    {
        return texture_type::Texture;
    }

    bool opengl_texture::is_render_target() const
    {
        return iFrameBuffer != 0;
    }

    const i_sub_texture& opengl_texture::as_sub_texture() const
    {
        throw not_sub_texture();
    }

    dimension opengl_texture::dpi_scale_factor() const
    {
        return iDpiScaleFactor;
    }

    texture_sampling opengl_texture::sampling() const
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

    uint32_t opengl_texture::samples() const
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

    texture_data_format opengl_texture::data_format() const
    {
        return iDataFormat;
    }

    texture_data_type opengl_texture::data_type() const
    {
        return iDataType;
    }

    bool opengl_texture::is_empty() const
    {
        return false;
    }

    size opengl_texture::extents() const
    {
        return iSize;
    }

    size opengl_texture::storage_extents() const
    {
        return iStorageSize;
    }

    void opengl_texture::set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment)
    {
        if (sampling() != texture_sampling::Multisample)
        {
            GLint previousTexture = bind(1);
            GLint previousPackAlignment;
            glCheck(glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousPackAlignment))
            glCheck(glPixelStorei(GL_UNPACK_ALIGNMENT, aPackAlignment));
            if (sampling() != texture_sampling::Data)
            {
                glCheck(glTexSubImage2D(to_gl_enum(sampling()), 0,
                    static_cast<GLint>(aRect.x), static_cast<GLint>(aRect.y), static_cast<GLsizei>(aRect.cx), static_cast<GLsizei>(aRect.cy),
                    std::get<1>(to_gl_enum(iDataFormat, iDataType)), std::get<2>(to_gl_enum(iDataFormat, iDataType)), aPixelData));
            }
            else
            {
                glCheck(glTexImage2D(to_gl_enum(sampling()), 0,
                    std::get<0>(to_gl_enum(iDataFormat, iDataType)),
                    static_cast<GLsizei>(aRect.cx), static_cast<GLsizei>(aRect.cy), 0,
                    std::get<1>(to_gl_enum(iDataFormat, iDataType)), std::get<2>(to_gl_enum(iDataFormat, iDataType)), aPixelData));
            }
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

    void opengl_texture::set_pixels(const i_image& aImage)
    {
        set_pixels(rect{ point{ 1.0, 1.0 }, aImage.extents() }, aImage.cpixels());
    }

    void opengl_texture::set_pixel(const point& aPosition, const colour& aColour)
    {
        std::array<uint8_t, 4> pixel{ aColour.red(), aColour.green(), aColour.blue(), aColour.alpha() };
        set_pixels(rect{ aPosition, size{1.0, 1.0} }, &pixel);
    }

    colour opengl_texture::get_pixel(const point& aPosition) const
    {
        switch (sampling())
        {
        case texture_sampling::Normal:
        case texture_sampling::Nearest:
        case texture_sampling::Data:
            throw std::logic_error("opengl_texture::get_pixel: function not yet implemented");
            break;
        default:
            throw unsupported_sampling_type_for_function();
        }
    }

    void* opengl_texture::handle() const
    {
        return reinterpret_cast<void*>(static_cast<intptr_t>(iHandle));
    }

    bool opengl_texture::is_resident() const
    {
        GLboolean resident;
        glCheck(glAreTexturesResident(1, &iHandle, &resident));
        return resident == GL_TRUE;
    }

    const std::string& opengl_texture::uri() const
    {
        return iUri;
    }

    dimension opengl_texture::horizontal_dpi() const
    {
        return dpi_scale_factor() * 96.0;
    }

    dimension opengl_texture::vertical_dpi() const
    {
        return dpi_scale_factor() * 96.0;
    }

    dimension opengl_texture::ppi() const
    {
        return size{ horizontal_dpi(), vertical_dpi() }.magnitude() / std::sqrt(2.0);
    }

    bool opengl_texture::metrics_available() const
    {
        return true;
    }

    dimension opengl_texture::em_size() const
    {
        return 0.0;
    }

    std::unique_ptr<i_rendering_context> opengl_texture::create_graphics_context(blending_mode aBlendingMode) const
    {
        return std::unique_ptr<i_rendering_context>(new opengl_rendering_context{ *this, aBlendingMode });
    }

    int32_t opengl_texture::bind(const std::optional<uint32_t>& aTextureUnit) const
    {
        if (aTextureUnit != std::nullopt)
        {
            glCheck(glActiveTexture(GL_TEXTURE0 + *aTextureUnit));
        }
        GLint previousTexture = 0;
        glCheck(glGetIntegerv(sampling() != texture_sampling::Multisample ? sampling() != texture_sampling::Data ? GL_TEXTURE_BINDING_2D : GL_TEXTURE_BINDING_RECTANGLE : GL_TEXTURE_BINDING_2D_MULTISAMPLE, &previousTexture));
        glCheck(glBindTexture(to_gl_enum(sampling()), static_cast<GLuint>(reinterpret_cast<std::intptr_t>(handle()))));
        return previousTexture;
    }

    std::shared_ptr<i_native_texture> opengl_texture::native_texture() const
    {
        return std::dynamic_pointer_cast<i_native_texture>(iManager.find_texture(id()));
    }

    render_target_type opengl_texture::target_type() const
    {
        return render_target_type::Texture;
    }

    void* opengl_texture::target_handle() const
    {
        return native_texture()->handle();
    }

    const i_texture& opengl_texture::target_texture() const
    {
        return *this;
    }

    size opengl_texture::target_extents() const
    {
        return extents();
    }

    neogfx::logical_coordinate_system opengl_texture::logical_coordinate_system() const
    {
        return iLogicalCoordinateSystem;
    }

    void opengl_texture::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    logical_coordinates opengl_texture::logical_coordinates() const
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

    void opengl_texture::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    void opengl_texture::activate_target() const
    {
        bool alreadyActive = target_active();
        if (!alreadyActive)
        {
            evTargetActivating.trigger();
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
            glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, to_gl_enum(sampling()), static_cast<GLuint>(reinterpret_cast<std::intptr_t>(native_texture()->handle())), 0));
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
            if (queryResult != static_cast<GLint>(reinterpret_cast<std::intptr_t>(native_texture()->handle())))
            {
                glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, to_gl_enum(sampling()), static_cast<GLuint>(reinterpret_cast<std::intptr_t>(native_texture()->handle())), 0));
            }
            glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
        }
        GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
            throw failed_to_create_framebuffer(glErrorString(status));
        glCheck(glViewport(1, 1, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT1 };
        glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
        if (!alreadyActive)
            evTargetActivated.trigger();
    }

    bool opengl_texture::target_active() const
    {
        return service<i_rendering_engine>().active_target() == this;
    }

    void opengl_texture::deactivate_target() const
    {
        if (target_active())
        {
            evTargetDeactivating.trigger();
            service<i_rendering_engine>().deactivate_context();
            evTargetDeactivated.trigger();
            return;
        }
        throw not_active();
    }

    colour opengl_texture::read_pixel(const point& aPosition) const
    {
        if (sampling() != neogfx::texture_sampling::Multisample)
        {
            scoped_render_target srt{ *this };
            std::array<uint8_t, 4> pixel;
            basic_point<GLint> pos{ aPosition };
            glCheck(glReadPixels(pos.x + 1, pos.y + 1, 1, 1, std::get<1>(to_gl_enum(iDataFormat, iDataType)), std::get<2>(to_gl_enum(iDataFormat, iDataType)), &pixel));
            return colour{ pixel[0], pixel[1], pixel[2], pixel[3] };
        }
        else
            throw std::logic_error("opengl_texture::read_pixel: not yet implemented for multisample render targets");
    }
}