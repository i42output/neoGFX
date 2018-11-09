// texture.hpp
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
#include <vector>
#include <optional>
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
		texture(const neogfx::size& aExtents, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, const optional_colour& aColour = optional_colour());
		texture(const i_texture& aTexture);
		texture(const i_image& aImage);
		texture(const i_sub_texture& aSubTexture);
		~texture();
		// operations
	public:
		texture_id id() const override;
		texture_type type() const override;
		const i_sub_texture& as_sub_texture() const override;
		dimension dpi_scale_factor() const override;
		texture_sampling sampling() const override;
		uint32_t samples() const override;
		bool is_empty() const override;
		size extents() const override;
		size storage_extents() const override;
		void set_pixels(const rect& aRect, const void* aPixelData) override;
		void set_pixels(const i_image& aImage) override;
	public:
		std::shared_ptr<i_native_texture> native_texture() const override;
		// attributes
	private:
		std::shared_ptr<i_texture> iNativeTexture;
		optional_sub_texture iSubTexture;
	};

	typedef std::optional<texture> optional_texture;
}