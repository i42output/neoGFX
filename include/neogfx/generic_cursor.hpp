// generic_cursor.hpp
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

namespace neogfx
{
	class generic_cursor
	{
		// types
	public:
		enum type_e
		{
			Custom,
			SystemArrow,
			SystemWait,
			SystemHand,
			SystemBusyTask,
			SystemIBeam,
			SystemNo,
			SystemSizeNorthSouth,
			SystemSizeEastWest,
			SystemSizeNorthEastSouthWest,
			SystemSizeNorthWestSouthEast
		};
		struct invalid_cursor_type : std::logic_error { invalid_cursor_type() : std::logic_error("neogfx::generic_cursor::invalid_cursor_type") {} };
		// construction
	public:
		generic_cursor(type_e aType) : iType(aType) {}
		virtual ~generic_cursor() {}
		// operations
	public:
		type_e type() const { return iType; }
	private:
		type_e iType;
	};
}