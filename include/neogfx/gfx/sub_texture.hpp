// sub_texture.hpp
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
#include <optional>
#include <neogfx/gfx/i_image.hpp>
#include <neogfx/gfx/i_sub_texture.hpp>

namespace neogfx
{
    class sub_texture : public reference_counted<i_sub_texture>
    {
        // types
    private:
        using base_type = reference_counted<i_sub_texture>;
        // construction
    public:
        sub_texture(texture_id aAtlasId, i_texture& aAtlasTexture, rect const& aAtlasLocation, size const& aExtents);
        sub_texture(sub_texture const& aSubTexture);
        sub_texture(i_sub_texture const& aSubTexture);
        sub_texture(i_sub_texture const& aSubTexture, const rect& aAtlasLocation);
        ~sub_texture();
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
        void set_pixels(rect const& aRect, void const* aPixelData, uint32_t aPackAlignment = 4u) final;
        void set_pixels(i_image const& aImage) final;
        void set_pixels(i_image const& aImage, rect const& aImagePart) final;
        void set_pixel(point const& aPosition, color const& aColor) final;
        color get_pixel(point const& aPosition) const final;
        i_vector<texture_line_segment> const& intersection(texture_line_segment const& aLine, rect const& aBoundingBox, vec2 const& aSampleSize = { 1.0, 1.0 }, scalar aTolerance = 0.0) const final;
    public:
        void bind(std::uint32_t aTextureUnit) const final;
        void unbind() const final;
    public:
        intptr_t native_handle() const final;
        i_texture& native_texture() const final;
    public:
        texture_id atlas_id() const final;
        i_texture& atlas_texture() const final;
        const rect& atlas_location() const final;
    public:
        void add_ref(long aCount = 1) const noexcept final;
        void release(long aCount = 1) const final;
        long use_count() const noexcept final;
        // attributes
    private:
        bool iChild = false;
        texture_id iAtlasId;
        i_texture* iAtlasTexture;
        rect iAtlasLocation;
        size iStorageExtents;
        size iExtents;
        mutable std::map<std::tuple<texture_line_segment, rect, vec2, scalar>, vector<texture_line_segment>> iIntersectionResultCache;
    };

    typedef std::optional<sub_texture> optional_sub_texture;
}