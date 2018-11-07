// opengl_texture.cpp
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
#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include "opengl_error.hpp"
#include "opengl_helpers.hpp"
#include "opengl_graphics_context.hpp"
#include "opengl_texture.hpp"

namespace neogfx
{
	opengl_texture::opengl_texture(i_texture_manager& aManager, texture_id aId, const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, const optional_colour& aColour) :
		iManager{ aManager },
		iId{ aId },
		iDpiScaleFactor{ aDpiScaleFactor },
		iSampling{ aSampling },
		iSize{ aExtents },
		iStorageSize{ size{ std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0) } },
		iHandle{ 0 },
		iUri{ "neogfx::opengl_texture::internal" },
		iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGame },
		iLogicalCoordinates{ vec2{ 0.0, 0.0}, aExtents.to_vec2() }
	{
		GLint previousTexture;
		try
		{
			glCheck(glGetIntegerv(iSampling != texture_sampling::Multisample ? GL_TEXTURE_BINDING_2D : GL_TEXTURE_BINDING_2D_MULTISAMPLE, &previousTexture));
			glCheck(glGenTextures(1, &iHandle));
			glCheck(glBindTexture(iSampling != texture_sampling::Multisample ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, iHandle));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
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
			else if (iSampling == texture_sampling::Nearest)
			{
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			}
			if (aColour != std::nullopt)
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
				if (iSampling != texture_sampling::Multisample)
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
			glCheck(glBindTexture(iSampling != texture_sampling::Multisample ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(previousTexture)));
		}
		catch (...)
		{
			glCheck(glDeleteTextures(1, &iHandle));
			throw;
		}
	}

	opengl_texture::opengl_texture(i_texture_manager& aManager, texture_id aId, const i_image& aImage) :
		iManager{ aManager },
		iId{ aId },
		iDpiScaleFactor{ aImage.dpi_scale_factor() },
		iSampling{ aImage.sampling() },
		iSize{ aImage.extents() },
		iStorageSize{size{std::max(std::pow(2.0, std::ceil(std::log2(iSize.cx + 2))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(iSize.cy + 2))), 16.0)}},
		iHandle{ 0 },
		iUri{ aImage.uri() }
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
			else if (iSampling == texture_sampling::Nearest)
			{
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
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

	texture_id opengl_texture::id() const
	{
		return iId;
	}

	texture_type opengl_texture::type() const
	{
		return texture_type::Texture;
	}

	const i_sub_texture& opengl_texture::as_sub_texture() const
	{
		throw not_sub_texture();
	}

	dimension opengl_texture::dpi_scale_factor() const
	{
		return iDpiScaleFactor;
	}

	texture_sampling opengl_texture::sampling() const
	{
		return iSampling;
	}

	bool opengl_texture::is_empty() const
	{
		return false;
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
		if (iSampling != texture_sampling::Multisample)
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

	void opengl_texture::set_pixels(const i_image& aImage)
	{
		set_pixels(rect{ point{}, aImage.extents() }, aImage.cdata());
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

	dimension opengl_texture::horizontal_dpi() const
	{
		return dpi_scale_factor() * 96.0;
	}

	dimension opengl_texture::vertical_dpi() const
	{
		return dpi_scale_factor() * 96.0;
	}

	dimension opengl_texture::ppi() const
	{
		return size{ horizontal_dpi(), vertical_dpi() }.magnitude() / std::sqrt(2.0);
	}

	bool opengl_texture::metrics_available() const
	{
		return true;
	}

	dimension opengl_texture::em_size() const
	{
		return 0.0;
	}

	std::unique_ptr<i_graphics_context> opengl_texture::create_graphics_context() const
	{
		return std::unique_ptr<i_graphics_context>(new opengl_graphics_context(*this));
	}

	std::shared_ptr<i_native_texture> opengl_texture::native_texture() const
	{
		return std::dynamic_pointer_cast<i_native_texture>(iManager.find_texture(id()));
	}

	render_target_type opengl_texture::target_type() const
	{
		return render_target_type::Texture;
	}

	void* opengl_texture::target_handle() const
	{
		return native_texture()->handle();
	}

	const i_texture& opengl_texture::target_texture() const
	{
		return *this;
	}

	size opengl_texture::target_extents() const
	{
		return extents();
	}

	neogfx::logical_coordinate_system opengl_texture::logical_coordinate_system() const
	{
		return iLogicalCoordinateSystem;
	}

	void opengl_texture::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
	{
		iLogicalCoordinateSystem = aSystem;
	}

	const neogfx::logical_coordinates& opengl_texture::logical_coordinates() const
	{
		return iLogicalCoordinates;
	}

	void opengl_texture::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
	{
		iLogicalCoordinates = aCoordinates;
	}

	bool opengl_texture::activate_target() const
	{
		bool alreadyActive = (service<i_rendering_engine>::instance().active_target() == this);
		service<i_rendering_engine>::instance().activate_context(*this);
		if (!alreadyActive)
		{
			glCheck(glEnable(GL_MULTISAMPLE));
			glCheck(glEnable(GL_BLEND));
			glCheck(glEnable(GL_DEPTH_TEST));
			glCheck(glDepthFunc(GL_LEQUAL));
			glCheck(glGenFramebuffers(1, &iFrameBuffer));
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
			glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sampling() != texture_sampling::Multisample ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, reinterpret_cast<GLuint>(native_texture()->handle()), 0));
			glCheck(glGenRenderbuffers(1, &iDepthStencilBuffer));
			glCheck(glBindRenderbuffer(GL_RENDERBUFFER, iDepthStencilBuffer));
			glCheck(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
			glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
			glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, iDepthStencilBuffer));
			glCheck(glClear(GL_DEPTH_BUFFER_BIT));
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_NO_ERROR && status != GL_FRAMEBUFFER_COMPLETE)
				throw failed_to_create_framebuffer(glErrorString(status));
			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, iFrameBuffer));
			glCheck(glViewport(0, 0, static_cast<GLsizei>(extents().cx), static_cast<GLsizei>(extents().cy)));
			GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
			glCheck(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
		}
		return true;
	}

	bool opengl_texture::deactivate_target() const
	{
		if (service<i_rendering_engine>::instance().active_target() == this)
		{
			glCheck(glDeleteRenderbuffers(1, &iDepthStencilBuffer));
			glCheck(glDeleteFramebuffers(1, &iFrameBuffer));
			service<i_rendering_engine>::instance().deactivate_context();
			return true;
		}
		return false;
	}
}