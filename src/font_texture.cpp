// font_texture.cpp
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
#include "font_texture.hpp"

namespace neogfx
{
	font_texture::font_texture(const size& aExtents, bool aSubPixelRendering) :
		iExtents(aExtents), iSubPixelRendering(aSubPixelRendering), iBinPack(aExtents, false)
	{
		GLint previousTexture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
		glGenTextures(1, &iHandle);
		glBindTexture(GL_TEXTURE_2D, iHandle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		std::vector<std::array<uint8_t, 3>> data(static_cast<std::size_t>(iExtents.cx * iExtents.cy), std::array<uint8_t, 3>{{0xFF, 0xFF, 0xFF}});
		glTexImage2D(GL_TEXTURE_2D, 0, aSubPixelRendering ? GL_RGB : GL_ALPHA, static_cast<GLsizei>(iExtents.cx), static_cast<GLsizei>(iExtents.cy), 0, aSubPixelRendering ? GL_RGB : GL_ALPHA, GL_UNSIGNED_BYTE, &data[0]);
		glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
	}

	font_texture::~font_texture()
	{
		glDeleteTextures(1, &iHandle);
	}

	const size& font_texture::extents() const
	{
		return iExtents;													
	}

	bool font_texture::allocate_glyph_space(const size& aSize, rect& aResult)
	{
		bool ok = iBinPack.insert(size(std::max(std::pow(2.0, std::ceil(std::log2(aSize.cx + (iSubPixelRendering ? 6 : 2)))), 16.0), std::max(std::pow(2.0, std::ceil(std::log2(aSize.cy + 2))), 16.0)), aResult);
		return ok;
	}

	void* font_texture::handle() const
	{
		return reinterpret_cast<void*>(iHandle);
	}

	glyph_texture::glyph_texture(const i_font_texture& aFontTexture, const rect& aFontTextureLocation, const size& aExtents, const point& aPlacement) :
		iFontTexture(aFontTexture), iFontTextureLocation(aFontTextureLocation), iExtents(aExtents), iPlacement(aPlacement)
	{
	}

	glyph_texture::~glyph_texture()
	{
	}

	const i_font_texture& glyph_texture::font_texture() const
	{
		return iFontTexture;
	}

	const rect& glyph_texture::font_texture_location() const
	{
		return iFontTextureLocation;
	}

	const size& glyph_texture::extents() const
	{
		return iExtents;
	}

	const point& glyph_texture::placement() const
	{
		return iPlacement;
	}
}