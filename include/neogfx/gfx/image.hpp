// image.hpp
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
#include <unordered_map>
#include <optional>
#include <neogfx/core/event.hpp>
#include <neogfx/gfx/i_image.hpp>

namespace neogfx
{
    class image : public reference_counted<i_image>
    {
    public:
        define_declared_event(Downloaded, downloaded)
        define_declared_event(FailedToDownload, failed_to_download)
    public:
        enum image_type_e
        {
            UnknownImage,
            PngImage
        };
    public:
        typedef neolib::vector<uint8_t> data_type;
        typedef data_type hash_digest_type;
    private:
        struct error_parsing_image_pattern : std::logic_error { error_parsing_image_pattern() : std::logic_error("neogfx::image::error_parsing_image_pattern") {} };
        struct no_resource : std::logic_error { no_resource() : std::logic_error("neogfx::image::no_resource") {} };
    public:
        image(dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, neogfx::color_space aColorSpace = neogfx::color_space::sRGB);
        image(const neogfx::size& aSize, const color& aColor = color::Black, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, neogfx::color_space aColorSpace = neogfx::color_space::sRGB);
        image(std::string const& aUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, neogfx::color_space aColorSpace = neogfx::color_space::sRGB);
        image(std::string const& aImagePattern, const std::unordered_map<std::string, color>& aColorMap, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, neogfx::color_space aColorSpace = neogfx::color_space::sRGB);
        image(std::string const& aUri, std::string const& aImagePattern, const std::unordered_map<std::string, color>& aColorMap, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, neogfx::color_space aColorSpace = neogfx::color_space::sRGB);
        image(image&& aOther);
        ~image();
    public:
        bool available() const override;
        bool downloading() const override;
        double downloading_progress() const override;
        bool error() const override;
        i_string const& error_string() const override;
    public:
        i_string const& uri() const override;
        const void* cdata() const override;
        const void* data() const override;
        void* data() override;
        std::size_t size() const override;
        hash_digest_type const& hash() const override;
    public:
        dimension dpi_scale_factor() const override;
        neogfx::color_space color_space() const override;
        neogfx::color_format color_format() const override;
        texture_sampling sampling() const override;
        texture_data_format data_format() const override;
        const neogfx::size& extents() const override;
        void resize(const neogfx::size& aNewSize) override;
        const void* cpixels() const override;
        const void* pixels() const override;
        void* pixels() override;
        color get_pixel(const point& aPoint) const override;
        void set_pixel(const point& aPoint, const color& aColor) override;
    private:
        bool has_resource() const;
        const i_resource& resource() const;
        image_type_e recognize() const;
        bool load();
        bool load_png();
    private:
        ref_ptr<i_resource> iResource;
        string iUri;
        std::optional<string> iError;
        dimension iDpiScaleFactor;
        neogfx::color_space iColorSpace;
        neogfx::color_format iColorFormat;
        data_type iData;
        mutable std::optional<data_type> iHash;
        texture_sampling iSampling;
        neogfx::size iSize;
    };
}