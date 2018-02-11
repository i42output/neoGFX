// opengl_texture.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include "opengl_error.hpp"
#include "opengl_texture.hpp"

namespace neogfx
{
	opengl_texture::opengl_texture(const neogfx::size& aExtents, texture_sampling aSampling, const optional_colour& aColour) :
		iSampling(aSampling),
		iSize(aExtents),
		iStorageSize{ size{ std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0) } },
		iHandle(0),
		iUri("neogfx::opengl_texture::internal")
	{
		GLint previousTexture;
		try
		{
			glCheck(glGetIntegerv(iSampling == texture_sampling::Normal || iSampling == texture_sampling::NormalMipmap ? GL_TEXTURE_BINDING_2D : GL_TEXTURE_BINDING_2D_MULTISAMPLE, &previousTexture));
			glCheck(glGenTextures(1, &iHandle));
			glCheck(glBindTexture(iSampling == texture_sampling::Normal || iSampling == texture_sampling::NormalMipmap ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, iHandle));
			if (iSampling == texture_sampling::Normal)
			{
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			}
			else if (iSampling == texture_sampling::NormalMipmap)
			{
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			}
			if (aColour != boost::none)
			{
				if (iSampling == texture_sampling::Multisample)
					throw multisample_texture_initialization_unsupported();
				std::vector<uint8_t> data(iStorageSize.cx * 4 * iStorageSize.cy);
				for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
					for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
					{
						data[y * iStorageSize.cx * 4 + x * 4 + 0] = aColour->red();
						data[y * iStorageSize.cx * 4 + x * 4 + 1] = aColour->green();
						data[y * iStorageSize.cx * 4 + x * 4 + 2] = aColour->blue();
						data[y * iStorageSize.cx * 4 + x * 4 + 3] = aColour->alpha();
					}
				glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]));
				if (iSampling == texture_sampling::NormalMipmap)
				{
					glCheck(glGenerateMipmap(GL_TEXTURE_2D));
				}
			}
			else
			{
				if (iSampling == texture_sampling::Normal || iSampling == texture_sampling::NormalMipmap)
				{
					glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
					if (iSampling == texture_sampling::NormalMipmap)
					{
						glCheck(glGenerateMipmap(GL_TEXTURE_2D));
					}
				}
				else
				{
					glCheck(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), true));
				}
			}
			glCheck(glBindTexture(iSampling == texture_sampling::Normal || iSampling == texture_sampling::NormalMipmap ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(previousTexture)));
		}
		catch (...)
		{
			glCheck(glDeleteTextures(1, &iHandle));
			throw;
		}
	}

	opengl_texture::opengl_texture(const i_image& aImage) :
		iSampling(aImage.sampling()),
		iSize(aImage.extents()), 
		iStorageSize{size{std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0)}},
		iHandle(0), 
		iUri(aImage.uri())
	{
		GLint previousTexture = 0;
		try
		{
			glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
			glCheck(glGenTextures(1, &iHandle));
			glCheck(glBindTexture(GL_TEXTURE_2D, iHandle));
			if (iSampling == texture_sampling::Normal)
			{
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			}
			else if (iSampling == texture_sampling::NormalMipmap)
			{
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
			}
			switch (aImage.colour_format())
			{
			case colour_format::RGBA8:
				{
					const uint8_t* imageData = static_cast<const uint8_t*>(aImage.data());
					std::vector<uint8_t> data(iStorageSize.cx * 4 * iStorageSize.cy);
					for (std::size_t y = 1; y < 1 + iSize.cy; ++y)
						for (std::size_t x = 1; x < 1 + iSize.cx; ++x)
							for (std::size_t c = 0; c < 4; ++c)
								data[y * iStorageSize.cx * 4 + x * 4 + c] = imageData[(y - 1) * iSize.cx * 4 + (x - 1) * 4 + c];
					glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(iStorageSize.cx), static_cast<GLsizei>(iStorageSize.cy), 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]));
					if (iSampling == texture_sampling::NormalMipmap)
					{
						glCheck(glGenerateMipmap(GL_TEXTURE_2D));
					}
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

	texture_sampling opengl_texture::sampling() const
	{
		return iSampling;
	}

	size opengl_texture::extents() const
	{
		return iSize;
	}

	size opengl_texture::storage_extents() const
	{
		return iStorageSize;
	}

	void opengl_texture::set_pixels(const rect& aRect, const void* aPixelData)
	{
		GLint previousTexture;
		if (iSampling == texture_sampling::Normal || iSampling == texture_sampling::NormalMipmap)
		{
			glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
			glCheck(glBindTexture(GL_TEXTURE_2D, iHandle));
			glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0,
				static_cast<GLint>(aRect.x + 1.0), static_cast<GLint>(aRect.y + 1.0), static_cast<GLsizei>(aRect.cx), static_cast<GLsizei>(aRect.cy),
				GL_RGBA, GL_UNSIGNED_BYTE, aPixelData));
			if (iSampling == texture_sampling::NormalMipmap)
			{
				glCheck(glGenerateMipmap(GL_TEXTURE_2D));
			}
			glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));
		}
		else
			throw multisample_texture_initialization_unsupported();
	}

	void* opengl_texture::handle() const
	{
		return reinterpret_cast<void*>(iHandle);
	}

	bool opengl_texture::is_resident() const
	{
		GLboolean resident;
		glCheck(glAreTexturesResident(1, &iHandle, &resident));
		return resident == GL_TRUE;
	}

	const std::string& opengl_texture::uri() const
	{
		return iUri;
	}
}