// image.hpp
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

#pragma once

#include "neogfx.hpp"
#include <boost/optional.hpp>
#include "i_image.hpp"

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
		image();
		image(const std::string& aPath);
		~image();
	public:
		virtual bool available() const;
		virtual std::pair<bool, double> downloading() const;
		virtual bool error() const;
		virtual const std::string& error_string() const;
	public:
		virtual const std::string& path() const;
		virtual const void* data() const;
		virtual void* data();
		virtual std::size_t size() const;
	public:
		virtual colour_format_e colour_format() const;
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
		std::string iPath;
		boost::optional<std::string> iError;
		colour_format_e iColourFormat;
		std::vector<uint8_t> iData;
		neogfx::size iSize;
	};
}