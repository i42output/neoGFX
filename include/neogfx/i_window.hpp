// i_window.hpp
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
#include "i_surface.hpp"
#include "i_native_window.hpp"

namespace neogfx
{
	class i_window : public i_surface
	{
	public:
		virtual const i_native_window& native_surface() const = 0;
		virtual i_native_window& native_surface() = 0;
	public:
		virtual void activate() = 0;
		virtual void counted_enable(bool aEnable) = 0;
	};
}