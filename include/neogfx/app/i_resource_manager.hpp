// i_resource_manager.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include "i_resource.hpp"

namespace neogfx
{
    struct embedded_resource_not_found : std::runtime_error { embedded_resource_not_found(const std::string& aResource) : std::runtime_error{ "neogfx::embedded_resource_not_found: " + aResource } {} };

    class i_resource_manager
    {
    public:
        virtual void add_resource(const std::string& aUri, const void* aResourceData, std::size_t aResourceSize) = 0;
        virtual void add_module_resource(const std::string& aUri, const void* aResourceData, std::size_t aResourceSize) = 0;
        virtual i_resource::pointer load_resource(const std::string& aUri) = 0;
    public:
        virtual void cleanup() = 0;
        virtual void clean() = 0;
    };
}