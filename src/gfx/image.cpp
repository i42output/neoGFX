// image.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <neogfx/gfx/image.hpp>
#include <neogfx/app/resource_manager.hpp>

namespace neogfx
{
	image::image(texture_sampling aSampling) : iColourFormat(neogfx::colour_format::RGBA8), iSampling(aSampling)
	{
	}

	image::image(const neogfx::size& aSize, const colour& aColour, texture_sampling aSampling) : iColourFormat(neogfx::colour_format::RGBA8), iSampling(aSampling)
	{
		resize(aSize);
		for (std::size_t y = 0; y < aSize.cx; ++y)
			for (std::size_t x = 0; x < aSize.cx; ++x)
				set_pixel(point(x, y), aColour);
	}

	image::image(const std::string& aUri, texture_sampling aSampling) : iResource(resource_manager::instance().load_resource(aUri)), iUri(aUri), iColourFormat(neogfx::colour_format::RGBA8), iSampling(aSampling)
	{
		if (available())
			load();
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
			return iError != boost::none;
	}

	const std::string& image::error_string() const
	{
		if (has_resource())
			return resource().error_string();
		else if (iError != boost::none)
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
		return const_cast<void*>(const_cast<const image*>(this)->data());
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

	colour_format image::colour_format() const
	{
		return iColourFormat;
	}

	texture_sampling image::sampling() const
	{
		return iSampling;
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

	colour image::get_pixel(const point& aPoint) const
	{
		switch (iColourFormat)
		{
		case neogfx::colour_format::RGBA8:
			{
				const uint8_t* pixel = &iData[static_cast<std::size_t>(aPoint.y * extents().cx * 4 + aPoint.x * 4)];
				return colour{pixel[0], pixel[1], pixel[2], pixel[3]};
			}
		default:
			return colour{};
		}
	}

	void image::set_pixel(const point& aPoint, const colour& aColour)
	{
		switch (iColourFormat)
		{
		case neogfx::colour_format::RGBA8:
			{
				uint8_t* pixel = &iData[static_cast<std::size_t>(aPoint.y * extents().cx * 4 + aPoint.x * 4)];
				pixel[0] = aColour.red();
				pixel[1] = aColour.green();
				pixel[2] = aColour.blue();
				pixel[3] = aColour.alpha();
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