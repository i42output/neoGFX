// texture.hpp
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

#include <neogfx/neogfx.hpp>
#include <vector>
#include <boost/optional.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/sub_texture.hpp>

namespace neogfx
{
	class i_image;
	class i_native_texture;

	class texture : public i_texture
	{
		// construction
	public:
		texture();
		texture(const neogfx::size& aExtents, texture_sampling aSampling = texture_sampling::NormalMipmap, const optional_colour& aColour = optional_colour());
		texture(const i_texture& aTexture);
		texture(const i_image& aImage);
		texture(const i_sub_texture& aSubTexture);
		~texture();
		// operations
	public:
		type_e type() const override;
		const i_sub_texture& as_sub_texture() const override;
		texture_sampling sampling() const override;
		bool is_empty() const override;
		size extents() const override;
		size storage_extents() const override;
		void set_pixels(const rect& aRect, const void* aPixelData) override;
		void set_pixels(const i_image& aImage) override;
	public:
		virtual std::shared_ptr<i_native_texture> native_texture() const override;
		// attributes
	private:
		std::shared_ptr<i_native_texture> iNativeTexture;
		optional_sub_texture iSubTexture;
	};

	typedef boost::optional<texture> optional_texture;
	typedef std::shared_ptr<const i_texture> texture_pointer;
	typedef std::pair<texture_pointer, optional_rect> texture_source;
	typedef std::vector<texture_source> texture_list;
	typedef boost::optional<texture_list> optional_texture_list;
	typedef texture_list::size_type texture_index;
	typedef std::shared_ptr<texture_list> texture_list_pointer;

	inline texture_pointer to_texture_pointer(const i_texture& aTexture)
	{
		return (aTexture.type() == i_texture::Texture ? static_cast<texture_pointer>(std::make_shared<texture>(aTexture)) : static_cast<texture_pointer>(std::make_shared<sub_texture>(aTexture.as_sub_texture())));
	}

	inline texture_list_pointer to_texture_list_pointer(const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect{})
	{
		return std::make_shared<texture_list>(1, texture_source{ to_texture_pointer(aTexture), aTextureRect });
	}

	inline texture_list_pointer to_texture_list_pointer(texture_pointer aTexture, const optional_rect& aTextureRect = optional_rect{})
	{
		return std::make_shared<texture_list>(1, texture_source{ aTexture, aTextureRect });
	}

	inline texture_list_pointer to_texture_list_pointer(texture_list& aTextureList, const i_texture& aTexture, const optional_rect& aTextureRect)
	{
		aTextureList.assign(1, texture_source{ to_texture_pointer(aTexture), aTextureRect });
		return texture_list_pointer{ texture_list_pointer{}, &aTextureList };
	}

	inline texture_list_pointer to_texture_list_pointer(texture_list& aTextureList, texture_pointer aTexture, const optional_rect& aTextureRect)
	{
		aTextureList.assign(1, texture_source{ aTexture, aTextureRect });
		return texture_list_pointer{ texture_list_pointer{}, &aTextureList };
	}
}