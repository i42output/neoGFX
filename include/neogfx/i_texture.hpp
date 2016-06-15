// i_texture.hpp
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
	class i_native_texture;

	class i_texture
	{
	public:
		struct texture_empty : std::logic_error { texture_empty() : std::logic_error("neogfx::i_texture::texture_empty") {} };
	public:
		virtual ~i_texture() {}
	public:
		virtual bool is_empty() const = 0;
		virtual size extents() const = 0;
		virtual size storage_extents() const = 0;
		virtual void set_pixels(const rect& aRect, void* aPixelData) = 0;
	public:
		virtual std::shared_ptr<i_native_texture> native_texture() const = 0;
	};
}