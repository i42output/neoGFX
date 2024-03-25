// vulkan_error.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <vulkan/vk_enum_string_helper.h>

#include <neolib/core/string_utils.hpp>

#include "vulkan_error.hpp"

namespace neogfx
{
    vk::Result& vkGetError()
    {
        thread_local vk::Result result = vk::Result::eSuccess;
        return result;
    }

    std::string vkErrorString(vk::Result aErrorCode)
    {
        return to_string(aErrorCode);
    }

    vk::Result vkCheckError(const char* file, unsigned int line)
    {
        // Get the last error
        vk::Result errorCode = vkGetError();

        if (errorCode < vk::Result::eSuccess)
        {
            std::string fileString(file);
            std::string error = vkErrorString(errorCode);
            std::string errorMessage = "An internal Vulkan call failed in " +
                fileString.substr(fileString.find_last_of("\\/") + 1) + " (" + neolib::uint32_to_string<char>(line) + ") : " +
                error;
            service<neogfx::debug::logger>() << "neogfx (Vulkan): " << errorMessage << neogfx::endl;
            throw vk_error(errorMessage);
        }

        return errorCode;
    }
}
