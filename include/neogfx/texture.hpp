// texture.hpp
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
#include "i_texture.hpp"

namespace neogfx
{
	class i_image;
	class i_native_texture;

	class texture : public i_texture
	{
		// construction
	public:
		texture();
		texture(const i_texture& aTexture);
		texture(const i_image& aImage);
		~texture();
		// operations
	public:
		virtual bool is_empty() const;
		virtual size extents() const;
		virtual size storage_extents() const;
	public:
		virtual std::shared_ptr<i_native_texture> native_texture() const;
		// attributes
	private:
		std::shared_ptr<i_native_texture> iNativeTexture;
	};
}