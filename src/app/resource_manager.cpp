// resource_manager.cpp
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

#include <neogfx/neogfx.hpp>
#include <neolib/io/uri.hpp>
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

    void resource_manager::add_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize)
    {
        iResources[aUri] = ref_ptr<i_resource>{ make_ref<resource>(*this, aUri, aResourceData, aResourceSize) };
    }

    void resource_manager::add_module_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize)
    {
        iResources[aUri] = ref_ptr<i_resource>{ make_ref<module_resource>(aUri, aResourceData, aResourceSize) };
    }

    void resource_manager::load_resource(i_string const& aUri, i_ref_ptr<i_resource>& aResult)
    {
        auto existing = iResources.find(aUri);
        if (existing != iResources.end())
        {
            if (std::holds_alternative<ref_ptr<i_resource>>(existing->second))
            {
                aResult = std::get<ref_ptr<i_resource>>(existing->second);
                return;
            }
            weak_ref_ptr<i_resource> ptr = std::get<weak_ref_ptr<i_resource>>(existing->second);
            if (!ptr.expired())
            {
                aResult = ptr;
                return;
            }
        }
        if (neolib::uri{ aUri }.scheme().empty() && iResources.find(aUri.to_std_string_view().substr(0, aUri.to_std_string_view().rfind('#'))) == iResources.end())
            throw embedded_resource_not_found(aUri);
        auto newResource = make_ref<resource>(*this, aUri);
        iResources[aUri] = weak_ref_ptr<i_resource>{ newResource };
        aResult = newResource;
    }

    void resource_manager::cleanup()
    {
        for (auto i = iResources.begin(); i != iResources.end();)
        {
            if (std::holds_alternative<weak_ref_ptr<i_resource>>(i->second) && std::get<weak_ref_ptr<i_resource>>(i->second).expired())
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