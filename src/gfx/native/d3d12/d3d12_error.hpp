// d3d12_error.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <cstdint>
#include <stdexcept>
#include <string>

namespace neogfx
{
    // an HRESULT as text (e.g. "DXGI_ERROR_DEVICE_REMOVED (0x887A0005)")
    std::string dxErrorString(std::int32_t aResult);
    // throws d3d12_error (logged) if aResult is a failure
    void dxCheckError(std::int32_t aResult, char const* aFile, unsigned int aLine);

    #ifdef dxCheck
    #undef dxCheck
    #endif
    #define dxCheck(x) ::neogfx::dxCheckError(static_cast<std::int32_t>(x), __FILE__, __LINE__)

    struct d3d12_error : std::runtime_error
    {
        d3d12_error(std::string const& aMessage) : std::runtime_error("neogfx::d3d12_error: " + aMessage) {};
    };
}
