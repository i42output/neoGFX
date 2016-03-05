// font_texture.hpp
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
#include "i_font_texture.hpp"
#include "skyline_bin_pack.hpp"

namespace neogfx
{
	class font_texture : public i_font_texture
	{
	public:
		font_texture(const size& aExtents, bool aSubPixelRendering);
		~font_texture();
	public:
		virtual const size& extents() const;
		virtual bool allocate_glyph_space(const size& aSize, rect& aResult);
		virtual void* handle() const;
	private:
		size iExtents;
		bool iSubPixelRendering;
		GLuint iHandle;
		skyline_bin_pack iBinPack;
	};

	class glyph_texture : public i_glyph_texture
	{
	public:
		glyph_texture(const i_font_texture& aFontTexture, const rect& aFontTextureLocation, const size& aExtents, const point& aPlacement);
		~glyph_texture();
	public:
		virtual const i_font_texture& font_texture() const;
		virtual const rect& font_texture_location() const;
		virtual const size& extents() const;
		virtual const point& placement() const;
	private:
		const i_font_texture& iFontTexture;
		const rect iFontTextureLocation;
		const size iExtents;
		const point iPlacement;
	};
}