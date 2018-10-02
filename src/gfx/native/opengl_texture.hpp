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

namespace neogfx
{
	class opengl_texture : public i_native_texture
	{
	public:
		struct unsupported_colour_format : std::runtime_error { unsupported_colour_format() : std::runtime_error("neogfx::opengl_texture::unsupported_colour_format") {} };
		struct multisample_texture_initialization_unsupported : std::runtime_error{ multisample_texture_initialization_unsupported() : std::runtime_error("neogfx::opengl_texture::multisample_texture_initialization_unsupported") {} };
	public:
		opengl_texture(texture_id aId, const neogfx::size& aExtents, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, const optional_colour& aColour = optional_colour());
		opengl_texture(texture_id aId, const i_image& aImage);
		~opengl_texture();
	public:
		texture_id id() const override;
		dimension dpi_scale_factor() const override;
		texture_sampling sampling() const override;
		size extents() const override;
		size storage_extents() const override;
		void set_pixels(const rect& aRect, const void* aPixelData) override;
	public:
		void* handle() const override;
		bool is_resident() const override;
		const std::string& uri() const override;
	private:
		texture_id iId;
		dimension iDpiScaleFactor;
		texture_sampling iSampling;
		basic_size<uint32_t> iSize;
		basic_size<uint32_t> iStorageSize;
		GLuint iHandle;
		std::string iUri;
	};
}