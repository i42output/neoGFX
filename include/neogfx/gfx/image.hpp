// image.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <boost/optional.hpp>
#include <neogfx/gfx/i_image.hpp>

namespace neogfx
{
	class image : public i_image
	{
	public:
		enum image_type_e
		{
			UnknownImage,
			PngImage
		};
	private:
		struct no_resource : std::logic_error { no_resource() : std::logic_error("neogfx::image::no_resource") {} };
	public:
		image(texture_sampling aSampling = texture_sampling::NormalMipmap);
		image(const neogfx::size& aSize, const colour& aColour = colour::Black, texture_sampling aSampling = texture_sampling::NormalMipmap);
		image(const std::string& aUri, texture_sampling aSampling = texture_sampling::NormalMipmap);
		template <typename T, std::size_t Width, std::size_t Height>
		image(const std::string& aUri, const T(&aImagePattern)[Height][Width], const std::unordered_map<T, colour>& aColourMap, texture_sampling aSampling = texture_sampling::NormalMipmap) : iUri(aUri), iColourFormat(neogfx::colour_format::RGBA8), iSampling(aSampling)
		{
			resize(neogfx::size{ Width, Height });
			for (std::size_t y = 0; y < Height; ++y)
				for (std::size_t x = 0; x < Width; ++x)
					set_pixel(point(x, y), aColourMap.find(aImagePattern[y][x])->second);
		}
		template <typename T, std::size_t Width, std::size_t Height>
		image(const T(&aImagePattern)[Height][Width], const std::unordered_map<T, colour>& aColourMap, texture_sampling aSampling = texture_sampling::NormalMipmap) : iColourFormat(neogfx::colour_format::RGBA8), iSampling(aSampling)
		{
			resize(neogfx::size{ Width, Height });
			for (std::size_t y = 0; y < Height; ++y)
				for (std::size_t x = 0; x < Width; ++x)
					set_pixel(point(x, y), aColourMap.find(aImagePattern[y][x])->second);
		}
		~image();
	public:
		virtual bool available() const;
		virtual std::pair<bool, double> downloading() const;
		virtual bool error() const;
		virtual const std::string& error_string() const;
	public:
		virtual const std::string& uri() const;
		virtual const void* cdata() const;
		virtual const void* data() const;
		virtual void* data();
		virtual std::size_t size() const;
		virtual hash_digest_type hash() const;
	public:
		virtual neogfx::colour_format colour_format() const;
		virtual texture_sampling sampling() const;
		virtual const neogfx::size& extents() const;
		virtual void resize(const neogfx::size& aNewSize);
		virtual colour get_pixel(const point& aPoint) const;
		virtual void set_pixel(const point& aPoint, const colour& aColour);
	private:
		bool has_resource() const;
		const i_resource& resource() const;
		image_type_e recognize() const;
		bool load();
		bool load_png();
	private:
		i_resource::pointer iResource;
		std::string iUri;
		boost::optional<std::string> iError;
		neogfx::colour_format iColourFormat;
		data_type iData;
		texture_sampling iSampling;
		neogfx::size iSize;
	};
}