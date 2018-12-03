// texture.cpp
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
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/texture_manager.hpp>
#include "native/i_native_texture.hpp"

namespace neogfx
{
	texture::texture()
	{
	}

	texture::texture(const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat, texture_data_type aDataType, const optional_colour& aColour) :
		iNativeTexture{ service<i_texture_manager>::instance().create_texture(aExtents, aDpiScaleFactor, aSampling, aDataFormat, aDataType, aColour) }
	{
	}

	texture::texture(const i_texture& aTexture) :
		iNativeTexture{ !aTexture.is_empty() ? aTexture.native_texture() : std::shared_ptr<i_native_texture>() }
	{
	}

	texture::texture(const i_image& aImage, texture_data_format aDataFormat, texture_data_type aDataType) :
		iNativeTexture{ service<i_texture_manager>::instance().create_texture(aImage, aDataFormat, aDataType) }
	{
	}

	texture::texture(const i_sub_texture& aSubTexture) :
		iNativeTexture{ !aSubTexture.atlas_texture().is_empty() ? aSubTexture.atlas_texture().native_texture() : std::shared_ptr<i_native_texture>() },
		iSubTexture{ aSubTexture }
	{
	}

	texture::~texture()
	{
	}

	texture_id texture::id() const
	{
		return native_texture()->id();
	}

	texture_type texture::type() const
	{
		return texture_type::Texture;
	}

	const i_sub_texture& texture::as_sub_texture() const
	{
		if (iSubTexture != std::nullopt)
			return *iSubTexture;
		throw not_sub_texture();
	}

	dimension texture::dpi_scale_factor() const
	{
		if (is_empty())
			return 1.0;
		return native_texture()->dpi_scale_factor();
	}

	texture_sampling texture::sampling() const
	{
		if (is_empty())
			return texture_sampling::NormalMipmap;
		return native_texture()->sampling();
	}

	uint32_t texture::samples() const
	{
		if (is_empty())
			return 1;
		return native_texture()->samples();
	}

	texture_data_format texture::data_format() const
	{
		if (is_empty())
			return texture_data_format::RGBA;
		return native_texture()->data_format();
	}

	texture_data_type texture::data_type() const
	{
		if (is_empty())
			return texture_data_type::UnsignedByte;
		return native_texture()->data_type();
	}

	bool texture::is_empty() const
	{
		return iNativeTexture == nullptr;
	}

	size texture::extents() const
	{
		if (is_empty())
			return size{};
		return native_texture()->extents();
	}

	size texture::storage_extents() const
	{
		if (is_empty())
			return size{};
		return native_texture()->storage_extents();
	}

	void texture::set_pixels(const rect& aRect, const void* aPixelData, uint32_t aPackAlignment)
	{
		native_texture()->set_pixels(aRect, aPixelData);
	}

	void texture::set_pixels(const i_image& aImage)
	{
		set_pixels(rect{ point{}, aImage.extents() }, aImage.cdata());
	}

	void texture::set_pixel(const point& aPosition, const colour& aColour)
	{
		if (is_empty())
			throw texture_empty();
		return iNativeTexture->set_pixel(aPosition, aColour);
	}

	colour texture::get_pixel(const point& aPosition) const
	{
		if (is_empty())
			throw texture_empty();
		return iNativeTexture->get_pixel(aPosition);
	}

	int32_t texture::bind(const std::optional<uint32_t>& aTextureUnit) const
	{
		if (is_empty())
			throw texture_empty();
		return iNativeTexture->bind(aTextureUnit);
	}

	std::shared_ptr<i_native_texture> texture::native_texture() const
	{
		if (is_empty())
			throw texture_empty();
		return iNativeTexture->native_texture();
	}
}