// image.cpp
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
#include <libpng/png.h>
#include <openssl/sha.h>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/string_utils.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/app/resource_manager.hpp>

namespace neogfx
{
    image::image(dimension aDpiScaleFactor, texture_sampling aSampling) :
        iDpiScaleFactor{ aDpiScaleFactor }, 
        iColorFormat{ neogfx::color_format::RGBA8 },
        iSampling{ aSampling }
    {
    }

    image::image(const neogfx::size& aSize, const color& aColor, dimension aDpiScaleFactor, texture_sampling aSampling) :
        iDpiScaleFactor{ aDpiScaleFactor }, 
        iColorFormat{ neogfx::color_format::RGBA8 }, 
        iSampling{ aSampling }
    {
        resize(aSize);
        for (coordinate y = 0.0; y < aSize.cx; ++y)
            for (coordinate x = 0.0; x < aSize.cx; ++x)
                set_pixel(point{ x, y }, aColor);
    }

    image::image(const std::string& aUri, dimension aDpiScaleFactor, texture_sampling aSampling) :
        iResource{ resource_manager::instance().load_resource(aUri) },
        iUri{ aUri },
        iDpiScaleFactor{ aDpiScaleFactor },
        iColorFormat{ neogfx::color_format::RGBA8 },
        iSampling{ aSampling }
    {
        if (available())
            load();
    }

    image::image(const std::string& aImagePattern, const std::unordered_map<std::string, color>& aColorMap, dimension aDpiScaleFactor, texture_sampling aSampling) :
        image{ std::string{}, aImagePattern, aColorMap, aDpiScaleFactor, aSampling }
    {
    }

    image::image(const std::string& aUri, const std::string& aImagePattern, const std::unordered_map<std::string, color>& aColorMap, dimension aDpiScaleFactor, texture_sampling aSampling) :
        iUri{ aUri },
        iDpiScaleFactor{ aDpiScaleFactor },
        iColorFormat{ neogfx::color_format::RGBA8 },
        iSampling{ aSampling }
    {
        try
        {
            neolib::vecarray<std::string, 2> bits1;
            neolib::tokens(aImagePattern, std::string{ "[]" }, bits1, 2);
            neolib::vecarray<std::string, 2> bits2;
            neolib::tokens(bits1.at(0), std::string{ "," }, bits2, 2);
            std::vector<std::string> bits3;
            neolib::tokens(bits1.at(1), std::string{ "{}" }, bits3);
            if (bits3.size() < 2)
                throw error_parsing_image_pattern();

            basic_size<std::size_t> imageSize{ boost::lexical_cast<std::size_t>(bits2.at(0)), boost::lexical_cast<std::size_t>(bits2.at(1)) };

            std::unordered_map<char, std::string> colorKeyMap;
            for (std::size_t i = 0; i < bits3.size() - 1; ++i)
            {
                neolib::vecarray<std::string, 2> colorKeyMapEntry;
                neolib::tokens(bits3[i], std::string{ "," }, colorKeyMapEntry);
                colorKeyMap.insert(std::make_pair(colorKeyMapEntry.at(0).at(0), colorKeyMapEntry.at(1)));
            }
            
            resize(imageSize);
            const char* nextPixel = bits3.back().c_str();
            for (std::size_t y = 0; y < imageSize.cy; ++y)
                for (std::size_t x = 0; x < imageSize.cx; ++x)
                {
                    char pixelKey = *nextPixel++;
                    if (pixelKey == '\0')
                        throw error_parsing_image_pattern();
                    auto colorKey = colorKeyMap.find(pixelKey);
                    if (colorKey == colorKeyMap.end())
                        throw error_parsing_image_pattern();
                    auto colorMapKey = aColorMap.find(colorKey->second);
                    if (colorMapKey == aColorMap.end())
                        throw error_parsing_image_pattern();
                    set_pixel(basic_point<std::size_t>{ x, y }, colorMapKey->second);
                }

        }
        catch (std::out_of_range)
        {
            throw error_parsing_image_pattern();
        }
    }

    image::image(image&& aOther) :
        iResource{ std::move(aOther.iResource) },
        iUri{ std::move(aOther.iUri) },
        iError{ std::move(aOther.iError) },
        iDpiScaleFactor{ std::move(aOther.iDpiScaleFactor) },
        iColorFormat{ std::move(aOther.iColorFormat) },
        iData{ std::move(aOther.iData) },
        iSampling{ std::move(aOther.iSampling) },
        iSize{ std::move(aOther.iSize) }
    {
    }

    image::~image()
    {
    }

    bool image::available() const
    {
        if (has_resource())
            return resource().available();
        else
            return true;
    }

    std::pair<bool, double> image::downloading() const
    {
        if (has_resource())
            return resource().downloading();
        else
            return std::make_pair(false, 100.0);
    }

    bool image::error() const
    {
        if (has_resource())
            return resource().error();
        else
            return iError != std::nullopt;
    }

    const std::string& image::error_string() const
    {
        if (has_resource())
            return resource().error_string();
        else if (iError != std::nullopt)
            return *iError;
        static const std::string sNoError;
        return sNoError;
    }

    const std::string& image::uri() const
    {
        return iUri;
    }

    const void* image::cdata() const
    {
        if (iData.empty())
            throw no_data();
        return &iData[0];
    }

    const void* image::data() const
    {
        return cdata();
    }

    void* image::data()
    {
        return const_cast<void*>(to_const(*this).data());
    }

    std::size_t image::size() const
    {
        return iData.size();
    }

    image::hash_digest_type image::hash() const
    {
        hash_digest_type result(SHA256_DIGEST_LENGTH);
        SHA256(static_cast<const uint8_t*>(cdata()), size(), &result[0]);
        return result;
    }

    dimension image::dpi_scale_factor() const
    {
        return iDpiScaleFactor;
    }

    color_format image::color_format() const
    {
        return iColorFormat;
    }

    texture_sampling image::sampling() const
    {
        return iSampling;
    }

    texture_data_format image::data_format() const
    {
        // todo: add support for other data formats
        return texture_data_format::RGBA;
    }

    const size& image::extents() const
    {
        return iSize;
    }

    void image::resize(const neogfx::size& aNewSize)
    {
        iSize = aNewSize;
        iData.resize(static_cast<std::size_t>(iSize.cx * iSize.cy * 4));
    }

    const void* image::cpixels() const
    {
        return cdata();
    }

    const void* image::pixels() const
    {
        return data();
    }

    void* image::pixels()
    {
        return data();
    }

    color image::get_pixel(const point& aPoint) const
    {
        switch (iColorFormat)
        {
        case neogfx::color_format::RGBA8:
            {
                const uint8_t* pixel = &iData[static_cast<std::size_t>(aPoint.y * extents().cx * 4 + aPoint.x * 4)];
                return color{pixel[0], pixel[1], pixel[2], pixel[3]};
            }
        default:
            return color{};
        }
    }

    void image::set_pixel(const point& aPoint, const color& aColor)
    {
        switch (iColorFormat)
        {
        case neogfx::color_format::RGBA8:
            {
                uint8_t* pixel = &iData[static_cast<std::size_t>(aPoint.y * extents().cx * 4 + aPoint.x * 4)];
                pixel[0] = aColor.red();
                pixel[1] = aColor.green();
                pixel[2] = aColor.blue();
                pixel[3] = aColor.alpha();
            }
        default:
            /* do nothing */
            break;
        }
    }

    bool image::has_resource() const
    {
        return iResource != nullptr;
    }

    const i_resource& image::resource() const
    {
        if (!has_resource())
            throw no_resource();
        return *iResource;
    }

    image::image_type_e image::recognize() const
    {
        if (has_resource())
        {
            if (resource().size() > 0)
            {
                if (resource().size() >= 4)
                {
                    const uint8_t* magic = static_cast<const uint8_t*>(resource().data());
                    if (magic[0] == 0x89 && magic[1] == 'P' && magic[2] == 'N' && magic[3] == 'G')
                        return PngImage;
                }
            }
        }
        return UnknownImage;
    }

    bool image::load()
    {
        if (!available())
            throw not_available();
        switch (recognize())
        {
        case PngImage:
            return load_png();
        default:
            throw unknown_image_format();
        }
    }

    bool image::load_png()
    {
        png_image image;
        std::memset(&image, 0, (sizeof image));
        image.version = PNG_IMAGE_VERSION;
        if (png_image_begin_read_from_memory(&image, resource().data(), resource().size()) != 0)
        {
            image.format = PNG_FORMAT_RGBA;
            iData.resize(PNG_IMAGE_SIZE(image));
            if (png_image_finish_read(&image, NULL, data(), 0, NULL) != 0)
            {
                iSize = neogfx::size(image.width, image.height);
                png_image_free(&image);
                return true;
            }
            else
            {
                png_image_free(&image);
                iError = image.message;
                return false;
            }
        }
        else
        {
            iError = image.message;
            return false;
        }
    }

}