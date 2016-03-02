// opengl_texture.cpp
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

#include "neogfx.hpp"
#include "opengl_error.hpp"
#include "opengl_texture.hpp"

namespace neogfx
{
	opengl_texture::opengl_texture(const i_image& aImage) :
		iSize(aImage.extents()), 
		iStorageSize{size{std::max(std::ceil(std::log2(iSize.cx + 2)), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0)}}, 
		iHandle(0), 
		iUri(aImage.uri())
	{
		GLint previousTexture;
		try
		{
			glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
			glCheck(glGenTextures(1, &iHandle));
			glCheck(glBindTexture(GL_TEXTURE_2D, iHandle));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			switch (aImage.colour_format())
			{
			case ColourFormatRGBA8:
				{
					const uint8_t* imageData = static_cast<const uint8_t*>(aImage.data());
					std::vector<uint8_t> data(iStorageSize.cx * 4 * iStorageSize.cy);
					for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
						for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
							for (std::size_t c = 0; c < 4; ++c)
								data[y * iStorageSize.cx * 4 + x * 4 + c] = imageData[(y - 1) * iSize.cx * 4 + (x - 1) * 4 + c];
					glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]));
				}
				break;
			default:
				throw unsupported_colour_format();
				break;
			}
			glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));
		}
		catch (...)
		{
			glCheck(glDeleteTextures(1, &iHandle));
			throw;
		}
	}

	opengl_texture::~opengl_texture()
	{
		glCheck(glDeleteTextures(1, &iHandle));
	}

	size opengl_texture::extents() const
	{
		return iSize;
	}

	size opengl_texture::storage_extents() const
	{
		return iStorageSize;
	}

	void* opengl_texture::handle() const
	{
		return reinterpret_cast<void*>(iHandle);
	}

	const std::string& opengl_texture::uri() const
	{
		return iUri;
	}
}