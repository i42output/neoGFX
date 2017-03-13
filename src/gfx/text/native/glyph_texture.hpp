// glyph_texture.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/gfx/text/i_glyph_texture.hpp>

namespace neogfx
{
	class glyph_texture : public i_glyph_texture
	{
	public:
		glyph_texture(const i_sub_texture& aTexture, const point& aPlacement);
		~glyph_texture();
	public:
		virtual const i_sub_texture& texture() const;
		virtual const point& placement() const;
	private:
		const i_sub_texture& iTexture;
		const point iPlacement;
	};
}