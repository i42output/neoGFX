// i_native_texture.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gfx/i_render_target.hpp>

namespace neogfx
{
	class i_native_texture : public i_texture, public i_render_target
	{
	public:
		virtual void* handle() const = 0;
		virtual bool is_resident() const = 0;
		virtual const std::string& uri() const = 0;
	public:
		virtual size extents() const = 0;
	};
}