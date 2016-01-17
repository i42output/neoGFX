// i_font_texture.hpp
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
#include "geometry.hpp"

namespace neogfx
{
	class i_font_texture
	{
	public:
		virtual ~i_font_texture() {}
	public:
		virtual const size& extents() const = 0;
		virtual bool allocate_glyph_space(const size& aSize, rect& aResult) = 0;
		virtual void* handle() const = 0;
	};

	class i_glyph_texture
	{
	public:
		virtual ~i_glyph_texture() {}
	public:
		virtual const i_font_texture& font_texture() const = 0;
		virtual const rect& font_texture_location() const = 0;
		virtual const size& extents() const = 0;
		virtual const point& placement() const = 0;
	};
}