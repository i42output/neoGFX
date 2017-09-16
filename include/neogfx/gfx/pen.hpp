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

#include <neogfx/neogfx.hpp>

namespace neogfx
{
	class pen
	{
	public:
		// construction
	public:
		pen() : iWidth(0) {}
		pen(const effect_colour& aColour, bool aAntiAliased = true) : iColour(aColour), iWidth(1), iAntiAliased(aAntiAliased) {}
		pen(const effect_colour& aColour, dimension aWidth, bool aAntiAliased = true) : iColour(aColour), iWidth(aWidth), iAntiAliased(aAntiAliased) {}
		// operations
	public:
		neogfx::effect_colour colour() const { return iColour; }
		dimension width() const { return iWidth; }
		bool anti_aliased() const { return iAntiAliased; }
	private:
		effect_colour iColour;
		dimension iWidth;
		bool iAntiAliased;
	};

	typedef boost::optional<pen> optional_pen;
}