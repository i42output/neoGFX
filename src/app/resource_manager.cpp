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

template<> neogfx::i_resource_manager& services::start_service<neogfx::i_resource_manager>()
{
    return neogfx::resource_manager::instance();
}

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

    void resource_manager::merge(i_resource_manager& aResourceManager)
    {
        for (auto const& r : aResourceManager.resources())
            iResources.insert(r.first(), r.second());
        for (auto const& ra : aResourceManager.resource_archives())
            iResourceArchives.insert(ra.first(), ra.second());
    }

    void resource_manager::add_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize)
    {
        iResources.insert(aUri, decltype(iResources)::mapped_type{ ref_ptr<i_resource>{ make_ref<resource>(*this, aUri, aResourceData, aResourceSize) } });
    }

    void resource_manager::add_module_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize)
    {
        iResources.insert(aUri, decltype(iResources)::mapped_type{ ref_ptr<i_resource>{ make_ref<resource>(*this, aUri, aResourceData, aResourceSize) } });
    }

    void resource_manager::load_resource(i_string const& aUri, i_ref_ptr<i_resource>& aResult)
    {
        auto existing = iResources.to_std_map().find(aUri);
        if (existing != iResources.to_std_map().end())
        {
            if (std::holds_alternative<ref_ptr<i_resource>>(existing->second.second()))
            {
                aResult = std::get<ref_ptr<i_resource>>(existing->second.second());
                return;
            }
            weak_ref_ptr<i_resource> ptr = std::get<weak_ref_ptr<i_resource>>(existing->second.second());
            if (!ptr.expired())
            {
                aResult = ptr;
                return;
            }
        }
        if (neolib::uri{ aUri }.scheme().empty() && iResources.to_std_map().find(aUri.to_std_string_view().substr(0, aUri.to_std_string_view().rfind('#'))) == iResources.to_std_map().end())
            throw embedded_resource_not_found(aUri);
        auto newResource = make_ref<resource>(*this, aUri);
        iResources[aUri] = decltype(iResources)::mapped_type{ weak_ref_ptr<i_resource>{ newResource } };
        aResult = newResource;
    }

    void resource_manager::cleanup()
    {
        for (auto i = iResources.to_std_map().begin(); i != iResources.to_std_map().end();)
        {
            if (std::holds_alternative<weak_ref_ptr<i_resource>>(i->second.second()) && std::get<weak_ref_ptr<i_resource>>(i->second.second()).expired())
                i = iResources.to_std_map().erase(i);
            else
                ++i;
        }
    }

    void resource_manager::clean()
    {
        decltype(iResources) resources;
        resources.to_std_map().swap(iResources.to_std_map());
        decltype(iResourceArchives) resourceArchives;
        resourceArchives.to_std_map().swap(iResourceArchives.to_std_map());
    }

    neolib::i_map<i_string, neolib::i_variant<i_ref_ptr<i_resource>, i_weak_ref_ptr<i_resource>>> const& resource_manager::resources()
    {
        return iResources;
    }

    neolib::i_map<i_string, neolib::i_variant<i_ref_ptr<i_resource>, i_weak_ref_ptr<i_resource>>> const& resource_manager::resource_archives()
    {
        return iResourceArchives;
    }
}