// glyph_texture.cpp
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
#include "glyph_texture.hpp"

namespace neogfx
{
	glyph_texture::glyph_texture(const i_sub_texture& aTexture, const point& aPlacement) :
		iTexture(aTexture), iPlacement(aPlacement)
	{
	}

	glyph_texture::~glyph_texture()
	{
	}

	const i_sub_texture& glyph_texture::texture() const
	{
		return iTexture;
	}

	const point& glyph_texture::placement() const
	{
		return iPlacement;
	}
}