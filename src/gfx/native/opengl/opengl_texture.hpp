// opengl_texture.hpp
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

#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/i_image.hpp>
#include <neogfx/gfx/shader_array.hpp>
#include "opengl.hpp"
#include "../native_texture.hpp"

namespace neogfx
{
    class i_texture_manager;

    template <typename T>
    class opengl_texture : public native_texture
    {
        typedef opengl_texture<T> self_type;
    public:
        define_declared_event(TargetActivating, target_activating)
        define_declared_event(TargetActivated, target_activated)
        define_declared_event(TargetDeactivating, target_deactivating)
        define_declared_event(TargetDeactivated, target_deactivated)
    public:
        struct unsupported_color_format : std::runtime_error { unsupported_color_format() : std::runtime_error("neogfx::opengl_texture::unsupported_color_format") {} };
        struct multisample_texture_initialization_unsupported : std::logic_error { multisample_texture_initialization_unsupported() : std::logic_error("neogfx::opengl_texture::multisample_texture_initialization_unsupported") {} };
    public:
        typedef T value_type;
        static constexpr texture_data_type kDataType = crack_shader_array_data_type<value_type>::DATA_TYPE;
    public:
        opengl_texture(i_texture_manager& aManager, texture_id aId, const neogfx::size& aExtents, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, texture_data_format aDataFormat = texture_data_format::RGBA, neogfx::color_space aColorSpace = neogfx::color_space::sRGB, const optional_color& aColor = optional_color());
        opengl_texture(i_texture_manager& aManager, texture_id aId, const i_image& aImage, const rect& aImagePart, texture_data_format aDataFormat = texture_data_format::RGBA);
        ~opengl_texture();
    public:
        texture_id id() const final;
        string const& uri() const final;
        rect const& part() const final;
        texture_type type() const final;
        bool is_render_target() const final;
        const i_render_target& as_render_target() const final;
        i_render_target& as_render_target() final;
        const i_sub_texture& as_sub_texture() const final;
        dimension dpi_scale_factor() const final;
        texture_sampling sampling() const final;
        std::uint32_t samples() const final;
        texture_data_format data_format() const final;
        texture_data_type data_type() const final;
        bool is_empty() const final;
        size extents() const final;
        size storage_extents() const final;
        void set_pixels(const rect& aRect, void const* aPixelData, std::uint32_t aStride = 0u, std::uint32_t aPackAlignment = 4u) final;
        void set_pixels(const rect& aRect, void const* aPixelData, texture_data_format aDataFormat, std::uint32_t aStride = 0u, std::uint32_t aPackAlignment = 4u) final;
        void set_pixels(const i_image& aImage) final;
        void set_pixels(const i_image& aImage, const rect& aImagePart) final;
        void set_pixel(const point& aPosition, const color& aColor) final;
        color get_pixel(const point& aPosition) const final;
    public:
        void* handle() const final;
        bool is_resident() const final;
    public:
        dimension horizontal_dpi() const final;
        dimension vertical_dpi() const final;
        dimension ppi() const final;
        bool metrics_available() const final;
        dimension em_size() const final;
    public:
        std::unique_ptr<i_rendering_context> create_graphics_context(blending_mode aBlendingMode = blending_mode::Default) const final;
    public:
        void bind(std::uint32_t aTextureUnit) const final;
        void unbind() const final;
    public:
        intptr_t native_handle() const final;
        i_texture& native_texture() const final;
    public:
        render_target_type target_type() const final;
        void* target_handle() const final;
        void* target_device_handle() const final;
        pixel_format_t pixel_format() const final;
        const i_texture& target_texture() const final;
        point target_origin() const final;
        size target_extents() const final;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const final;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) final;
        neogfx::logical_coordinates logical_coordinates() const final;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) final;
    public:
        rect_i32 viewport() const final;
        rect_i32 set_viewport(const rect_i32& aViewport) const final;
    public:
        bool target_active() const final;
        void activate_target() const final;
        void deactivate_target() const final;
    public:
        neogfx::color_space color_space() const final;
        color read_pixel(const point& aPosition) const final;
    private:
        i_texture_manager& iManager;
        texture_id iId;
        string iUri;
        rect iPart;
        dimension iDpiScaleFactor;
        neogfx::color_space iColorSpace;
        texture_sampling iSampling;
        texture_data_format iDataFormat;
        size_u32 iSize;
        size_u32 iStorageSize;
        GLuint iHandle;
        neogfx::logical_coordinate_system iLogicalCoordinateSystem;
        std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        mutable std::optional<std::uint32_t> iBoundTextureUnit;
        mutable std::optional<std::int32_t> iPreviouslyBoundTexture;
        mutable GLuint iFrameBuffer;
        mutable GLuint iDepthStencilBuffer;
        mutable std::vector<T> iPixelData;
    };
}