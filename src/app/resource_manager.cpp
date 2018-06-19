// resource_manager.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/app/resource_manager.hpp>
#include <neogfx/app/module_resource.hpp>
#include <neogfx/app/resource.hpp>

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

	void resource_manager::add_resource(const std::string& aUri, const void* aResourceData, std::size_t aResourceSize)
	{
		iResources[aUri] = i_resource::pointer(std::make_shared<resource>(*this, aUri, aResourceData, aResourceSize));
	}

	void resource_manager::add_module_resource(const std::string& aUri, const void* aResourceData, std::size_t aResourceSize)
	{
		iResources[aUri] = i_resource::pointer(std::make_shared<module_resource>(aUri, aResourceData, aResourceSize));
	}

	i_resource::pointer resource_manager::load_resource(const std::string& aUri)
	{
		auto existing = iResources.find(aUri);
		if (existing != iResources.end())
		{
			if (existing->second.is<i_resource::pointer>())
				return static_variant_cast<i_resource::pointer>(existing->second);
			i_resource::weak_pointer ptr = static_variant_cast<i_resource::weak_pointer>(existing->second);
			if (!ptr.expired())
				return ptr.lock();
		}
		i_resource::pointer newResource = std::make_shared<resource>(*this, aUri);
		iResources[aUri] = i_resource::weak_pointer(newResource);
		return newResource;
	}

	void resource_manager::cleanup()
	{
		for (auto i = iResources.begin(); i != iResources.end();)
		{
			if (i->second.is<i_resource::weak_pointer>() && static_variant_cast<i_resource::weak_pointer&>(i->second).expired())
				i = iResources.erase(i);
			else
				++i;
		}
	}

	void resource_manager::clean()
	{
		decltype(iResources) resources;
		resources.swap(iResources);
		decltype(iResourceArchives) resourceArchives;
		resourceArchives.swap(iResourceArchives);
	}
}