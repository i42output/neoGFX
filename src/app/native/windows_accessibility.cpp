// windows_accessibility.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>

#include <WinUser.h>

#include "windows_accessibility.hpp"

namespace neogfx
{
    namespace native::windows
    {
        std::uint32_t accessibility::text_cursor_width() const
        {
            DWORD caretWidth = 0;
            SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretWidth, 0);
            return static_cast<std::uint32_t>(caretWidth);
        }
    }
}