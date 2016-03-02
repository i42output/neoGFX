// opengl_texture.hpp
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
#include <GL/glew.h>
#include <GL/GL.h>
#include "geometry.hpp"
#include "i_native_texture.hpp"
#include "i_image.hpp"

namespace neogfx
{
	class opengl_texture : public i_native_texture
	{
	public:
		struct unsupported_colour_format : std::runtime_error { unsupported_colour_format() : std::runtime_error("neogfx::opengl_texture::unsupported_colour_format") {} };
	public:
		opengl_texture(const i_image& aImage);
		~opengl_texture();
	public:
		virtual size extents() const;
		virtual size storage_extents() const;
	public:
		virtual void* handle() const;
		virtual const std::string& uri() const;
	private:
		basic_size<uint32_t> iSize;
		basic_size<uint32_t> iStorageSize;
		GLuint iHandle;
		std::string iUri;
	};
}