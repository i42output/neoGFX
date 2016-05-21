// i_native_texture.hpp
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
	class i_native_texture
	{
	public:
		virtual ~i_native_texture() {}
	public:
		virtual size extents() const = 0;
		virtual size storage_extents() const = 0;
	public:
		virtual void* handle() const = 0;
		virtual bool is_resident() const = 0;
		virtual const std::string& uri() const = 0;
	};
}