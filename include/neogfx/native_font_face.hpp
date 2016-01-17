// native_font_face.hpp
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
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef u8
#undef u8
#include <hb.h>
#include <hb-ft.h>
#include <hb-ucdn\ucdn.h>
#define u8
#else
#include <hb.h>
#include <hb-ft.h>
#include <hb-unicode.h>
#endif
#include "geometry.hpp"
#include "i_surface.hpp"
#include "i_native_font.hpp"
#include "i_native_font_face.hpp"
#include "font.hpp"
#include "font_texture.hpp"

namespace neogfx
{
	class i_rendering_engine;

	class native_font_face : public i_native_font_face
	{
	private:
		typedef std::map<uint32_t, neogfx::glyph_texture> glyph_map;
	public:
		struct hb_handle
		{
			hb_font_t* font;
			hb_buffer_t* buf;
			hb_unicode_funcs_t* unicodeFuncs;
			hb_handle(FT_Face aHandle) :
				font(hb_ft_font_create(static_cast<FT_Face>(aHandle), NULL)),
				buf(hb_buffer_create()),
				unicodeFuncs(hb_buffer_get_unicode_funcs(buf))
			{
			}
			~hb_handle()
			{
				hb_font_destroy(font);
				hb_buffer_destroy(buf);
			}
		};
	public:
		native_font_face(i_rendering_engine& aRenderingEngine, i_native_font& aFont, font::style_e aStyle, font::point_size aSize, neogfx::size aDpiResolution, FT_Face aHandle);
		~native_font_face();
	public:
		virtual i_native_font& native_font();
		virtual const std::string& family_name() const;
		virtual font::style_e style() const;
		virtual font::point_size size() const;
		virtual const std::string& style_name() const;
		virtual dimension horizontal_dpi() const;
		virtual dimension vertical_dpi() const;
		virtual dimension height() const;
		virtual dimension line_spacing() const;
		virtual dimension kerning(uint32_t aFirstCodePoint, uint32_t aSecondCodePoint) const;
		virtual i_native_font_face& fallback() const;
		virtual void* handle() const;
		virtual void* aux_handle() const;
		virtual i_glyph_texture& glyph_texture(const glyph& aGlyph) const;
	private:
		i_rendering_engine& iRenderingEngine;
		i_native_font& iFont;
		font::style_e iStyle;
		std::string iStyleName;
		font::point_size iSize;
		neogfx::size iPixelDensityDpi;
		FT_Face iHandle;
		mutable hb_handle* iAuxHandle;
		mutable std::unique_ptr<i_native_font_face> iFallbackFont;
		mutable glyph_map iGlyphs;
		mutable std::vector<GLubyte> iGlyphTextureData;
		mutable std::vector<std::array<GLubyte, 3>> iSubpixelGlyphTextureData;
	};
}