// resource_manager.cpp
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

#include "neogfx.hpp"
#include "resource_manager.hpp"
#include "module_resource.hpp"

namespace neogfx
{	
	resource_manager::resource_manager()
	{
	}
	
	resource_manager& resource_manager::instance()
	{
		static resource_manager sInstance;
		return sInstance;
	}

	void resource_manager::add_resource(const std::string aResourcePath, const void* aResourceData, std::size_t aResourceSize)
	{
		iResources[aResourcePath] = std::make_unique<module_resource>(aResourcePath, aResourceData, aResourceSize);
	}
}