// pen.hpp
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
#include "primitives.hpp"

namespace neogfx
{
	class pen
	{
		// construction
	public:
		pen() : iWidth(0) {}
		pen(const neogfx::colour& aColour) : iColour(aColour), iWidth(1) {}
		pen(const neogfx::colour& aColour, dimension aWidth) : iColour(aColour), iWidth(aWidth) {}
		// operations
	public:
		neogfx::colour colour() const { return iColour; }
		dimension width() const { return iWidth; }
	private:
		neogfx::colour iColour;
		dimension iWidth;
	};
}