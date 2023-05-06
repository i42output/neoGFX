// vulkan_error.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <vulkan/vulkan.hpp>

namespace neogfx
{
    vk::Result& vkGetError();

    std::string vkErrorString(vk::Result aErrorCode);
    vk::Result vkCheckError(const char* file, unsigned int line);

    class scoped_vk_check
    {
    public:
        scoped_vk_check(const char* file, unsigned int line) : iFile{ file }, iLine{ line }
        {
        }
        ~scoped_vk_check()
        {
            vkCheckError(iFile, iLine);
        }
    private:
        const char* const iFile;
        unsigned int const iLine;
    };

    #ifdef vkCheck
    #undef vkCheck 
    #endif
    #define vkCheck(x) { scoped_vk_check svc{__FILE__, __LINE__}; vkGetError() = x; }

    struct vk_error : std::runtime_error
    {
        vk_error(std::string const& aMessage) : std::runtime_error("neogfx::vk_error: " + aMessage) {};
    };
}