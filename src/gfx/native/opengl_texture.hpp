// opengl_texture.hpp
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
#include "opengl.hpp"
#include <neogfx/core/geometrical.hpp>
#include "i_native_texture.hpp"
#include <neogfx/gfx/i_image.hpp>
#include <neogfx/gfx/shader_array.hpp>

namespace neogfx
{
    class i_texture_manager;

    template <typename T>
    class opengl_texture : public i_native_texture
    {
    public:
        define_declared_event(TargetActivating, target_activating)
        define_declared_event(TargetActivated, target_activated)
        define_declared_event(TargetDeactivating, target_deactivating)
        define_declared_event(TargetDeactivated, target_deactivated)
    public:
        struct unsupported_color_format : std::runtime_error { unsupported_color_format() : std::runtime_error("neogfx::opengl_texture::unsupported_color_format") {} };
        struct multisample_texture_initialization_unsupported : std::logic_error { multisample_texture_initialization_unsupported() : std::logic_error("neogfx::opengl_texture::multisample_texture_initialization_unsupported") {} };
        struct unsupported_sampling_type_for_function : std::logic_error { unsupported_sampling_type_for_function() : std::logic_error("neogfx::opengl_texture::unsupported_sampling_type_for_function") {} };
    public:
        typedef T value_type;
        static constexpr texture_data_type kDataType = crack_shader_array_data_type<value_type>::DATA_TYPE;
    public:
        opengl_texture(i_texture_manager& aManager, texture_id aId, const neogfx::size& aExtents, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, texture_data_format aDataFormat = texture_data_format::RGBA, const optional_color& aColor = optional_color());
        opengl_texture(i_texture_manager& aManager, texture_id aId, const i_image& aImage, texture_data_format aDataFormat = texture_data_format::RGBA);
        ~opengl_texture();
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
        void* handle() const override;
        bool is_resident() const override;
        const std::string& uri() const override;
    public:
        dimension horizontal_dpi() const override;
        dimension vertical_dpi() const override;
        dimension ppi() const override;
        bool metrics_available() const override;
        dimension em_size() const override;
    public:
        std::unique_ptr<i_rendering_context> create_graphics_context(blending_mode aBlendingMode = blending_mode::Default) const override;
    public:
        int32_t bind(const std::optional<uint32_t>& aTextureUnit = std::optional<uint32_t>{}) const override;
    public:
        intptr_t native_handle() const override;
        std::shared_ptr<i_native_texture> native_texture() const override;
    public:
        render_target_type target_type() const override;
        void* target_handle() const override;
        void* target_device_handle() const override;
        i_rendering_engine::pixel_format_t pixel_format() const override;
        const i_texture& target_texture() const override;
        size target_extents() const override;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const override;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) override;
        neogfx::logical_coordinates logical_coordinates() const override;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) override;
    public:
        bool target_active() const override;
        void activate_target() const override;
        void deactivate_target() const override;
    public:
        color read_pixel(const point& aPosition) const override;
    private:
        i_texture_manager& iManager;
        texture_id iId;
        dimension iDpiScaleFactor;
        texture_sampling iSampling;
        texture_data_format iDataFormat;
        size_u32 iSize;
        size_u32 iStorageSize;
        GLuint iHandle;
        std::string iUri;
        neogfx::logical_coordinate_system iLogicalCoordinateSystem;
        std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        mutable GLuint iFrameBuffer;
        mutable GLuint iDepthStencilBuffer;
    };
}