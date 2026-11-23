// d3d12_error.cpp
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

#include <neogfx/neogfx.hpp>

#include <cstdio>

#include <neolib/core/string_utils.hpp>

#include "d3d12_error.hpp"

namespace neogfx
{
    std::string dxErrorString(std::int32_t aResult)
    {
        char code[16];
        std::snprintf(code, sizeof(code), "0x%08X", static_cast<std::uint32_t>(aResult));
        char const* name = nullptr;
        switch (static_cast<std::uint32_t>(aResult))
        {
        case 0x80004001u: name = "E_NOTIMPL"; break;
        case 0x80004002u: name = "E_NOINTERFACE"; break;
        case 0x80004003u: name = "E_POINTER"; break;
        case 0x80004005u: name = "E_FAIL"; break;
        case 0x8007000Eu: name = "E_OUTOFMEMORY"; break;
        case 0x80070057u: name = "E_INVALIDARG"; break;
        case 0x887A0001u: name = "DXGI_ERROR_INVALID_CALL"; break;
        case 0x887A0002u: name = "DXGI_ERROR_NOT_FOUND"; break;
        case 0x887A0004u: name = "DXGI_ERROR_UNSUPPORTED"; break;
        case 0x887A0005u: name = "DXGI_ERROR_DEVICE_REMOVED"; break;
        case 0x887A0006u: name = "DXGI_ERROR_DEVICE_HUNG"; break;
        case 0x887A0007u: name = "DXGI_ERROR_DEVICE_RESET"; break;
        case 0x887A000Au: name = "DXGI_ERROR_WAS_STILL_DRAWING"; break;
        case 0x887A0020u: name = "DXGI_ERROR_DRIVER_INTERNAL_ERROR"; break;
        case 0x887E0001u: name = "D3D12_ERROR_ADAPTER_NOT_FOUND"; break;
        case 0x887E0002u: name = "D3D12_ERROR_DRIVER_VERSION_MISMATCH"; break;
        default: break;
        }
        return name != nullptr ? std::string{ name } + " (" + code + ")" : std::string{ "HRESULT(" } + code + ")";
    }

    void dxCheckError(std::int32_t aResult, char const* aFile, unsigned int aLine)
    {
        if (aResult >= 0)
            return;
        std::string const file{ aFile };
        std::string const errorMessage = "An internal Direct3D 12 call failed in " +
            file.substr(file.find_last_of("\\/") + 1) + " (" + neolib::uint32_to_string<char>(aLine) + ") : " + dxErrorString(aResult);
        service<neogfx::debug::logger>() << "neogfx (Direct3D 12): " << errorMessage << std::endl;
        throw d3d12_error(errorMessage);
    }
}
