// resource_manager.hpp
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
#include <neolib/core/variant.hpp>
#include <neolib/core/map.hpp>
#include "i_resource_manager.hpp"

namespace neogfx
{
    class resource_manager : public i_resource_manager
    {
    public:
        resource_manager();
        static resource_manager& instance();
    public:
        void merge(i_resource_manager& aResourceManager) override;
    public:
        using i_resource_manager::add_resource;
        using i_resource_manager::add_module_resource;
        using i_resource_manager::load_resource;
        void add_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize) override;
        void add_module_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize) override;
        void load_resource(i_string const& aUri, i_ref_ptr<i_resource>& aResult) override;
    public:
        void cleanup() override;
        void clean() override;
    public:
        neolib::i_map<i_string, neolib::i_variant<i_ref_ptr<i_resource>, i_weak_ref_ptr<i_resource>>> const& resources() override;
        neolib::i_map<i_string, neolib::i_variant<i_ref_ptr<i_resource>, i_weak_ref_ptr<i_resource>>> const& resource_archives() override;
    private:
        neolib::map<string, neolib::variant<ref_ptr<i_resource>, weak_ref_ptr<i_resource>>> iResources;
        neolib::map<string, neolib::variant<ref_ptr<i_resource>, weak_ref_ptr<i_resource>>> iResourceArchives;
    };
}